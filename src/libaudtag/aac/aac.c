
#include <libaudcore/tuple.h>
#include <libaudcore/vfs.h>
#include "../tag_module.h"
#include "aac.h"
#include "../util.h"

static const char* ID3v1GenreList[] = {
    "Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk",
    "Grunge", "Hip-Hop", "Jazz", "Metal", "New Age", "Oldies",
    "Other", "Pop", "R&B", "Rap", "Reggae", "Rock",
    "Techno", "Industrial", "Alternative", "Ska", "Death Metal", "Pranks",
    "Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk",
    "Fusion", "Trance", "Classical", "Instrumental", "Acid", "House",
    "Game", "Sound Clip", "Gospel", "Noise", "AlternRock", "Bass",
    "Soul", "Punk", "Space", "Meditative", "Instrumental Pop", "Instrumental Rock",
    "Ethnic", "Gothic", "Darkwave", "Techno-Industrial", "Electronic", "Pop-Folk",
    "Eurodance", "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta",
    "Top 40", "Christian Rap", "Pop/Funk", "Jungle", "Native American", "Cabaret",
    "New Wave", "Psychadelic", "Rave", "Showtunes", "Trailer", "Lo-Fi",
    "Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical",
    "Rock & Roll", "Hard Rock", "Folk", "Folk/Rock", "National Folk", "Swing",
    "Fast-Fusion", "Bebob", "Latin", "Revival", "Celtic", "Bluegrass", "Avantgarde",
    "Gothic Rock", "Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock", "Big Band",
    "Chorus", "Easy Listening", "Acoustic", "Humour", "Speech", "Chanson",
    "Opera", "Chamber Music", "Sonata", "Symphony", "Booty Bass", "Primus",
    "Porn Groove", "Satire", "Slow Jam", "Club", "Tango", "Samba",
    "Folklore", "Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle", "Duet",
    "Punk Rock", "Drum Solo", "A capella", "Euro-House", "Dance Hall",
    "Goa", "Drum & Bass", "Club House", "Hardcore", "Terror",
    "Indie", "BritPop", "NegerPunk", "Polsk Punk", "Beat",
    "Christian Gangsta", "Heavy Metal", "Black Metal", "Crossover", "Contemporary C",
    "Christian Rock", "Merengue", "Salsa", "Thrash Metal", "Anime", "JPop",
    "SynthPop",
};

gchar* atom_types[] = {"\251alb", "\251nam", "cprt", "\251art", "\251ART", "trkn", "\251day", "gnre", "desc"};

tag_module_t aac = {aac_can_handle_file, aac_populate_tuple_from_file, aac_write_tuple_to_file};



Atom *readAtom(VFSFile *fd,Atom *atom)
{    
    atom->size = read_BEint32(fd);
    atom->name = read_ASCII(fd,4);  
    atom->body = read_ASCII(fd,atom->size-8);
    atom->type = 0;
    return atom;
}

void writeAtom(VFSFile *fd, Atom *atom)
{     
    write_BEint32(fd,atom->size);
    vfs_fwrite(atom->name,4,1,fd);
    vfs_fwrite(atom->body,atom->size-8,1,fd);
}

void printAtom(Atom *atom)
{
     DEBUG("size = %x\n",atom->size); 
     DEBUG("name = %s\n",atom->name);
}

StrDataAtom *readStrDataAtom(VFSFile *fd,StrDataAtom *atom)
{
    atom->atomsize = read_BEint32(fd);
    atom->name = read_ASCII(fd,4);
    atom->datasize = read_BEint32(fd);
    atom->dataname = read_ASCII(fd,4);
    atom->vflag = read_BEint32(fd);
    atom->nullData = read_BEint32(fd);
    atom->data = read_ASCII(fd,atom->datasize-16);
    atom->type = 1;
    return atom;
}

void writeStrDataAtom(VFSFile *fd,StrDataAtom *atom)
{
    write_BEint32(fd,atom->atomsize);
    vfs_fwrite(atom->name,4,1,fd);
    write_BEint32(fd,atom->datasize);
    vfs_fwrite(atom->dataname,4,1,fd);
    write_BEint32(fd,atom->vflag);
    write_BEint32(fd,atom->nullData);
    vfs_fwrite(atom->data,atom->datasize-16,1,fd);
}

Atom *findAtom(VFSFile *fd,gchar* name)
{
    Atom *atom  = g_new0(Atom,1);
    atom = readAtom(fd,atom);
    while(g_strcmp0(atom->name,name) && !vfs_feof(fd))
    {
        g_free(atom);
        atom = g_new0(Atom,1);
        atom = readAtom(fd,atom);
    }
    if(vfs_feof(fd))
        return NULL;
    return atom;
}


Atom *getilstAtom(VFSFile *fd)
{
    Atom *moov = findAtom(fd,MOOV);
    //jump back the body of this atom and search his childs
    vfs_fseek(fd,-(moov->size-7), SEEK_CUR);
    //find the udta atom
    Atom *udta = findAtom(fd,UDTA);
    vfs_fseek(fd,-(udta->size-7), SEEK_CUR);
    Atom *meta = findAtom(fd,META);
    vfs_fseek(fd,-(meta->size-11), SEEK_CUR);
    Atom *ilst = findAtom(fd,ILST);
    ilstFileOffset = vfs_ftell(fd) -ilst->size;
    vfs_fseek(fd,-(ilst->size-7),SEEK_CUR);

    return ilst;

}

int getAtomID(gchar* name) {
    int i = 0;
    for (i = 0; i < MP4_ITEMS_NO; i++) {
        if (!g_strcmp0(name, atom_types[i]))
            return i;
    }
    return -1;
}

StrDataAtom *makeAtomWithData(const gchar* data,StrDataAtom *atom,int field)
{
    guint32 charsize = strlen(data);
    atom->atomsize = charsize + 24;
    atom->name = atom_types[field];
    atom->datasize = charsize+16;
    atom->dataname = "data";
    atom->vflag = 0x0;
    atom->nullData = 0x0;
    atom->data = (gchar*)data;
    atom->type = 1;
    return atom;

}

void writeAtomListToFile(VFSFile* fd, int offset, mowgli_list_t *list)
{
    //read free atom if we have any :D
    vfs_fseek(fd,ilstFileOffset+ilstAtom->size,SEEK_SET);
    Atom *free_atom = g_new0(Atom,1);
    free_atom = readAtom(fd,free_atom);
/*
    if(!g_strcom0(free_atom->name,"free"))
*/



    mowgli_node_t *n, *tn;
    MOWGLI_LIST_FOREACH_SAFE(n, tn, list->head)
    {

    }
}

gboolean aac_can_handle_file(VFSFile *f)
{
    Atom *first_atom  = g_new0(Atom,1);
    if(!g_strcmp0(first_atom->name,FTYP))
        return TRUE;
    return FALSE;
}


Tuple *aac_populate_tuple_from_file(Tuple *tuple,VFSFile *f)
{
    if(ilstAtom)
        g_free(ilstAtom);
    ilstAtom = getilstAtom(f);
    int size_read = 0;

    if(dataAtoms != NULL)
    {
        mowgli_node_t *n, *tn;
        MOWGLI_LIST_FOREACH_SAFE(n, tn, dataAtoms->head)
        {
            mowgli_node_delete(n,dataAtoms);
        }
    }
    dataAtoms = mowgli_list_create();

    while (size_read < ilstAtom->size)
    {
        StrDataAtom *a = g_new0(StrDataAtom,1);
        Atom *at = g_new0(Atom,1);
        at = readAtom(f,at);
        mowgli_node_add(at, mowgli_node_create(), dataAtoms);
        int atomtype = getAtomID(at->name);
        if(atomtype == -1)
        {
            size_read += at->size;           
            continue;
        }
        vfs_fseek(f,-(at->size),SEEK_CUR);
        a = readStrDataAtom(f,a);
        size_read += a->atomsize;

        switch (atomtype)
        {
            case MP4_ALBUM:
            {
                tuple_associate_string(tuple,FIELD_ALBUM,NULL,a->data);
            }break;
            case MP4_TITLE:
            {
                tuple_associate_string(tuple,FIELD_TITLE,NULL,a->data);
            }break;
            case MP4_COPYRIGHT:
            {
                tuple_associate_string(tuple,FIELD_COPYRIGHT,NULL,a->data);
            }break;
            case MP4_ARTIST:
            case MP4_ARTIST2:
            {
                tuple_associate_string(tuple,FIELD_ARTIST,NULL,a->data);
            }break;
            case MP4_TRACKNR:
            {
                //tuple_associate_string(tuple,FIELD_ALBUM,NULL,a->data);
            }break;
            case MP4_YEAR:
            {
                tuple_associate_int(tuple,FIELD_YEAR,NULL,atoi(a->data));
            }break;
            case MP4_GENRE:
            {
                guint8 *val = (guint8*)(a->data + (a->datasize-17));
                const gchar* genre = ID3v1GenreList[*val-1];
                tuple_associate_string(tuple,FIELD_GENRE,NULL,genre);
            }break;
            case MP4_COMMENT:
            {
                tuple_associate_string(tuple,FIELD_COMMENT,NULL,a->data);
            }break;
        }
    }
    return tuple;
}

gboolean aac_write_tuple_to_file(Tuple* tuple, VFSFile *f)
{
    guint32 newilstSize = 0;
    mowgli_node_t *n, *tn;
    mowgli_list_t *newdataAtoms;
    newdataAtoms = mowgli_list_create();
    
    MOWGLI_LIST_FOREACH_SAFE(n, tn, dataAtoms->head)
    {
        int atomtype = getAtomID(((StrDataAtom*)(n->data))->name);
        switch (atomtype)
        {
            case MP4_ALBUM:
            {
               const gchar* strVal = tuple_get_string(tuple,FIELD_ALBUM,NULL);
                if(strVal != NULL)
                {
                    StrDataAtom *atom = g_new0(StrDataAtom,1);
                    atom = makeAtomWithData(strVal,atom,MP4_ALBUM);
                    mowgli_node_add(atom, mowgli_node_create(), newdataAtoms);
                    newilstSize += atom->atomsize;
                }else
                {
                    mowgli_node_add(n->data, mowgli_node_create(), newdataAtoms);
                    newilstSize += ((Atom*)(n->data))->size;
                }
            }break;
            case MP4_TITLE:
            {
               const gchar* strVal = tuple_get_string(tuple,FIELD_TITLE,NULL);
                if(strVal != NULL)
                {
                    StrDataAtom *atom = g_new0(StrDataAtom,1);
                    atom = makeAtomWithData(strVal,atom,MP4_TITLE);
                    mowgli_node_add(atom, mowgli_node_create(), newdataAtoms);
                    newilstSize += atom->atomsize;
                }else
                {
                    mowgli_node_add(n->data, mowgli_node_create(), newdataAtoms);
                    newilstSize += ((Atom*)(n->data))->size;
                }
            }break;
            case MP4_COPYRIGHT:
            {
               const gchar* strVal = tuple_get_string(tuple,FIELD_COPYRIGHT,NULL);
                if(strVal != NULL)
                {
                    StrDataAtom *atom = g_new0(StrDataAtom,1);
                    atom = makeAtomWithData(strVal,atom,MP4_COPYRIGHT);
                    mowgli_node_add(atom, mowgli_node_create(), newdataAtoms);
                    newilstSize += atom->atomsize;
                }else
                {
                    mowgli_node_add(n->data, mowgli_node_create(), newdataAtoms);
                    newilstSize += ((Atom*)(n->data))->size;
                }
            }break;
            case MP4_ARTIST:
            case MP4_ARTIST2:
            {
               const gchar* strVal = tuple_get_string(tuple,FIELD_ARTIST,NULL);
                if(strVal != NULL)
                {
                    StrDataAtom *atom = g_new0(StrDataAtom,1);
                    atom = makeAtomWithData(strVal,atom,MP4_ARTIST2);
                    mowgli_node_add(atom, mowgli_node_create(), newdataAtoms);
                    newilstSize += atom->atomsize;
                }else
                {
                    mowgli_node_add(n->data, mowgli_node_create(), newdataAtoms);
                    newilstSize += ((Atom*)(n->data))->size;
                }
            }break;
            case MP4_TRACKNR:
            {
                //tuple_associate_string(tuple,FIELD_ALBUM,NULL,a->data);
            }break;
            case MP4_YEAR:
            {
                int iyear = tuple_get_int(tuple,FIELD_YEAR,NULL);
                gchar* strVal = g_strdup_printf("%d",iyear);
                if(strVal != NULL)
                {
                    StrDataAtom *atom = g_new0(StrDataAtom,1);
                    atom = makeAtomWithData(strVal,atom,MP4_YEAR);
                    mowgli_node_add(atom, mowgli_node_create(), newdataAtoms);
                    newilstSize += atom->atomsize;
                }else
                {
                    mowgli_node_add(n->data, mowgli_node_create(), newdataAtoms);
                    newilstSize += ((Atom*)(n->data))->size;
                }
            }break;
            case MP4_GENRE:
            {
/*
                guint8 *val = (guint8*)(a->data + (a->datasize-17));
                const gchar* genre = ID3v1GenreList[*val-1];
                tuple_associate_string(tuple,FIELD_GENRE,NULL,genre);
*/
            }break;
            case MP4_COMMENT:
            {
              const  gchar* strVal = tuple_get_string(tuple,FIELD_COMMENT,NULL);
                if(strVal != NULL)
                {
                    StrDataAtom *atom = g_new0(StrDataAtom,1);
                    atom = makeAtomWithData(strVal,atom,MP4_COMMENT);
                    mowgli_node_add(atom, mowgli_node_create(), newdataAtoms);
                    newilstSize += atom->atomsize;
                }else
                {
                    mowgli_node_add(n->data, mowgli_node_create(), newdataAtoms);
                    newilstSize += ((Atom*)(n->data))->size;
                }
            }break;
            default:
            {
                mowgli_node_add(n->data, mowgli_node_create(), newdataAtoms);
                newilstSize += ((Atom*)(n->data))->size;
            }break;
        }
    }

 //   writeAtomListToFile(f,offset,newdataAtoms);
    return FALSE;
}