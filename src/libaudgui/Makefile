LIB = ${LIB_PREFIX}audgui${LIB_SUFFIX}
LIB_MAJOR = 1
LIB_MINOR = 0

SRCS = confirm.c \
       effects-menu.c \
       equalizer.c \
       iface-menu.c \
       infopopup.c \
       infowin.c \
       init.c \
       jump-to-time.c \
       list.c \
       playlists.c \
       queue-manager.c \
       ui_gtk.c \
       ui_fileopener.c \
       ui_urlopener.c \
       ui_jumptotrack.c \
       ui_jumptotrack_cache.c \
       icons-stock.c \
       ui_about.c \
       ui_credits.c \
       ui_playlist_manager.c \
       urilist.c \
       util.c

INCLUDES = libaudgui.h \
           libaudgui-gtk.h \
           list.h

include ../../buildsys.mk
include ../../extra.mk

includesubdir = libaudgui

CPPFLAGS += -DHAVE_CONFIG_H -I.. -I../.. ${GLIB_CFLASG} ${GTK_CFLAGS} ${DBUS_CFLAGS} ${MOWGLI_CFLAGS} ${AUDACIOUS_DEFINES} ${REGEX_CFLAGS}
CFLAGS += -std=gnu99 ${LIB_CFLAGS}
LDFLAGS += $(AUDLDFLAGS)
LIBS += -lm ${GLIB_LIBS} ${MOWGLI_LIBS} ${GTK_LIBS} ${REGEX_LIBS} -L../libaudcore -laudcore
