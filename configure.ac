dnl ***
dnl *** Process this file with autoconf to produce a configure script.
dnl ***

dnl Require autoconf 2.59 or later
AC_PREREQ([2.59])

dnl Initialize
AC_INIT([audacious], [3.1-devel])
AC_COPYRIGHT([(C) 2005-2011 Audacious Team])
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AC_GNU_SOURCE

AC_CONFIG_HEADERS([config.h])
PKG_PROG_PKG_CONFIG

dnl buildsystem bootstrap
dnl =====================
dnl XXX kludge for FreeBSD
if test -d "/usr/X11R6/include"; then
    CPPFLAGS="$CPPFLAGS -I/usr/X11R6/include"
fi

AC_SUBST([PACKAGE], [AC_PACKAGE_TARNAME])
AC_SUBST([VERSION], [AC_PACKAGE_VERSION])
AC_DEFINE_UNQUOTED([PACKAGE], "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED([VERSION], "$VERSION", [Version number of package])

BUILDSYS_INIT
BUILDSYS_PROG_IMPLIB
BUILDSYS_SHARED_LIB
IMPLIB_LIBS=""
AC_SUBST([IMPLIB_LIBS])


dnl Build stamp
dnl ===========
AC_ARG_WITH([buildstamp],
 AS_HELP_STRING([--with-buildstamp=XXX], [Set build stamp to XXX]),
 AC_DEFINE_UNQUOTED([BUILDSTAMP], "$withval", [Build stamp]),
 AC_DEFINE_UNQUOTED([BUILDSTAMP], "unknown build", [Build stamp]))


dnl Large file support
dnl ==================
AC_SYS_LARGEFILE

dnl Activation of text console (MS-Windows ONLY)
dnl ============================================
AC_ARG_ENABLE([text-console],
[AS_HELP_STRING([--enable-text-console],[Enable text console for MS-Windows hosts])],
[use_text_console=$withval],
[use_text_console=no]
)

dnl Plugin stuff
dnl ========================
SHARED_SUFFIX="$PLUGIN_SUFFIX"
AC_SUBST([SHARED_SUFFIX])
AC_DEFINE_UNQUOTED([SHARED_SUFFIX], "${SHARED_SUFFIX}",
    [Define the shared module suffix extension on your platform.])

AC_MSG_CHECKING([for Windows or Macintosh])
case "$target" in
    *mingw*)
        AC_MSG_RESULT([Windows])
        if test "x$use_text_console" = "xno"; then
            LDFLAGS="$LDFLAGS -Wl,-subsystem,windows"
        fi
        ;;
    *apple*)
        AC_MSG_RESULT([Macintosh])
        LDFLAGS="$LDFLAGS -framework CoreFoundation -framework CoreServices -framework Carbon"
        ;;
    *)
        AC_MSG_RESULT([no])
        ;;
esac

dnl GNU gettext
dnl ===========
AM_GNU_GETTEXT([external])

if test "$MSGFMT" = ":" ; then
    AC_MSG_ERROR([msgfmt was not found!  You may need to install gettext.])
fi


dnl Checks common for core and plugins
dnl ==================================

AC_ARG_ENABLE([gtk3],
 [AS_HELP_STRING([--disable-gtk3], [Use GTK2 even if GTK3 is present])],
 [use_gtk3=$enableval], [use_gtk3=yes])

if test $use_gtk3 = yes ; then
    PKG_CHECK_MODULES([GTK], [gtk+-3.0 >= 3.0.0], [use_gtk3=yes], [use_gtk3=no])
fi

if test $use_gtk3 = no ; then
    PKG_CHECK_MODULES([GTK], [gtk+-2.0 >= 2.12.0], [use_gtk3=no],
     AC_MSG_ERROR([Cannot find GTK headers.  Aborting ...]))
fi

if test $use_gtk3 = yes ; then
    ADD_PC_REQUIRES([gtk+-3.0 >= 3.0.0])
else
    ADD_PC_REQUIRES([gtk+-2.0 >= 2.12.0])
fi

AC_SUBST([GTK_LIBS])
AC_SUBST([GTK_CFLAGS])


AUD_COMMON_PROGS


dnl Check for headers and functions
dnl ===============================
AC_CHECK_HEADERS([unistd.h fcntl.h fts.h execinfo.h sys/errno.h regex.h rxposix.h rx/rxposix.h])
AC_CHECK_FUNCS([sigwait backtrace])

### ---------------------------------------------------------------------------
dnl Path settings
dnl =============
AC_PATH_PROG([AUDACIOUS_PATH], [audacious], [no])

AUD_ARG_ENABLE([user-plugin-dir], [yes],
[per-user plugin dir])

if test "x$enable_user_plugin_dir" = "xno"; then
    AC_DEFINE([DISABLE_USER_PLUGIN_DIR], 1, [Define to disable per user plugin directory])
fi

AUD_ARG_ENABLE([rpath], [yes], [disable rpath])

if test "x$enable_rpath" = "xyes"; then
    AUDLDFLAGS="-Wl,-rpath,${libdir}"
fi

AC_SUBST([AUDLDFLAGS])

### ---------------------------------------------------------------------------
dnl Chardet support
dnl ===============
AUD_ARG_ENABLE([chardet], [yes],
[charset detection support],
[
    PKG_CHECK_MODULES([LIBGUESS], [libguess], [enable_chardet=yes], [enable_chardet=no])

    if test "x$enable_chardet" = "xyes"; then
        AC_DEFINE([USE_CHARDET], 1, [Define if character set detection enabled])
    fi
])
AUD_CONDITIONAL([USE_CHARDET], [enable_chardet])


### ---------------------------------------------------------------------------
dnl D-Bus support
dnl =============
AUD_ARG_ENABLE([dbus], [yes],
[D-Bus support],
[
    PKG_CHECK_MODULES([DBUS], [dbus-1 >= 0.60 dbus-glib-1 >= 0.60 gthread-2.0],
        [enable_dbus=yes],
        [enable_dbus=no])

    AC_PATH_PROG([DBUS_BINDING_TOOL], [dbus-binding-tool], [no])
    AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal], [no])
    if test "x$DBUS_BINDING_TOOL" = "xno" || test "x$GLIB_GENMARSHAL" = "xno" || test "x$enable_dbus" = "xno" ; then
        enable_dbus="no"
    else
        ADD_PC_REQUIRES([dbus-1 >= 0.60])
        ADD_PC_REQUIRES([dbus-glib-1 >= 0.60])
        AC_DEFINE([USE_DBUS], 1, [Define if D-Bus support enabled])
        AC_DEFINE([DBUS_SERVICES_DIR], "$datadir/dbus-1/services",
            [Location of D-Bus services directory])
        LIBAUDCLIENT_LDFLAGS="$AUDLDFLAGS -laudclient"
        AC_SUBST([DBUS_CFLAGS])
        AC_SUBST([DBUS_LIBS])
        AC_SUBST([DBUS_SERVICES_DIR])
        AC_SUBST([DBUS_BINDING_TOOL])
        AC_SUBST([LIBAUDCLIENT_LDFLAGS])
    fi
])
AUD_CONDITIONAL([USE_DBUS], [enable_dbus])


### ---------------------------------------------------------------------------
dnl regex support (gnu/oniguruma/pcre)
dnl ==================================
REGEX_LIBS=
REGEX_CFLAGS=

regex_selected=

AC_ARG_WITH([regexlib],
[AS_HELP_STRING([--with-regexlib[=gnu/oniguruma/pcre]],[Use the chosen regex library (def: gnu)])],
[regex_selected=$withval],
[regex_selected=gnu]
)

case $regex_selected in
  pcre)
    AUD_CHECK_MODULE([LIBPCRE], [libpcre], [>= 6.7], [PCRE])
    AC_DEFINE([USE_REGEX_PCRE], 1, [If this macro is defined, use PCRE regex library.])
    REGEX_LIBS="-lpcreposix $LIBPCRE_LIBS"
    REGEX_CFLAGS="$LIBPCRE_CFLAGS"
    ;;
  oniguruma)
    AC_CHECK_LIB([onig], [onig_new],
    [
        AC_DEFINE([USE_REGEX_ONIGURUMA], 1, [If this macro is defined, use Oniguruma regex library.])
        REGEX_LIBS="-lonig"
    ],[
        AC_MSG_ERROR([Cannot find Oniguruma regex library.])
    ])
    ;;
  *)
    AC_DEFINE([USE_REGEX_GNU], 1, [If this macro is defined, use GNU regex library.])

    AC_CHECK_LIB(c, regcomp, REGEX_LIBS="-lc",
      [AC_CHECK_LIB(regex, regcomp, REGEX_LIBS="-lregex",
        [AC_CHECK_LIB(rx, regcomp, REGEX_LIBS="-lrx",
          AC_MSG_ERROR([Cannot find GNU regex library.])
        )]
      )]
    )
    ;;
esac

AC_MSG_CHECKING([regex library type])
AC_MSG_RESULT($regex_selected)

AC_SUBST([REGEX_LIBS])
AC_SUBST([REGEX_CFLAGS])


### ---------------------------------------------------------------------------
dnl Pthread support
dnl ===============
PTHREAD_LIBS=
PTHREAD_CFLAGS=

AC_CHECK_LIB(pthread, pthread_create, PTHREAD_LIBS="-lpthread",
  [AC_CHECK_LIB(pthreads, pthread_create, PTHREAD_LIBS="-lpthreads",
    [AC_CHECK_LIB(pthreadGC2, pthread_create, PTHREAD_LIBS="-lpthreadGC2")]
  )]
)

AC_SUBST([PTHREAD_LIBS])
AC_SUBST([PTHREAD_CFLAGS])

### ---------------------------------------------------------------------------
dnl Session management support
dnl ==========================

AUD_ARG_ENABLE([sm], [yes],
[session management],
[
    PKG_CHECK_MODULES([SM], [sm >= 1.0], [
        enable_sm="yes"
    ],[
        enable_sm="no"
    ])

    AC_SUBST([SM_CFLAGS])
    AC_SUBST([SM_LIBS])

    PKG_CHECK_MODULES([ICE], [ice >= 1.0],
     [enable_sm="${enable_sm}"],
     [enable_sm="no"])

    AC_SUBST([ICE_CFLAGS])
    AC_SUBST([ICE_LIBS])

    PKG_CHECK_MODULES(GLIB214, glib-2.0 >= 2.14, enable_sm=$enable_sm,
     enable_sm=no)

    if test $enable_sm = yes ; then
        AC_DEFINE(USE_EGGSM, 1, Define if session management should be used)
    fi
])

AUD_CONDITIONAL([USE_EGGSM], [enable_sm])

### ---------------------------------------------------------------------------
dnl Ugly system checks
dnl ==================
AC_MSG_CHECKING([if __ELF__ is defined])
AC_EGREP_CPP([yes],
    [#ifdef __ELF__
        yes
     #endif
    ],
    [is_elf=yes],
    [is_elf=no]
)
AC_MSG_RESULT([$is_elf])

have_solaris=no
have_sun=no
have_mswindows=no
ARCH_DEFINES=""
case $host in
    *-*-openbsd* | *-*-netbsd* | *-*-mirbsd*)
        have_sun=yes
        if test "x${is_elf}" = "xno"; then
            AC_DEFINE([SYMBOL_PREFIX], "_", [Define to symbol prefix, if any])
        fi
    ;;
    *-*-darwin*)
        AC_DEFINE([SYMBOL_PREFIX], "_", [Define to symbol prefix, if any])
    ;;
    *-hpux-*)
        ARCH_DEFINES="-DHPUX"
    ;;
    *-*-solaris* | *-*-sunos*)
        have_solaris=yes
    ;;
    *-*-mingw*)
        have_mswindows=yes
    ;;
esac

arch_type=other
x86_asm=no
case $host in
    i386-*-* | i486-*-* | i586-*-* | i686-*-* | i86pc-*-*)
        arch_type=ix86
        case $host in
            *-*-netbsd* | *-*openbsd*)
            ;;
            *)
                ARCH_DEFINES="-DI386_ASSEM"
                x86_asm=yes
            ;;
        esac
    ;;
esac

AC_SUBST([ARCH_DEFINES])

dnl XXX delete ASAP -nenolod
AUD_CONDITIONAL([ARCH_X86], [arch_type], [ix86])
AUD_CONDITIONAL([USE_X86ASM], [x86_asm])
AUD_CONDITIONAL([HAVE_SOLARIS], [have_solaris])
AUD_CONDITIONAL([HAVE_SUN], [have_sun])
AUD_CONDITIONAL([HAVE_MSWINDOWS], [have_mswindows])

### ---------------------------------------------------------------------------
dnl Define plugin directories
dnl =========================
plugindir="$libdir/audacious"
AC_SUBST([plugindir])
AC_SUBST([pluginsubs])

AUD_DEFINE_PLUGIN_DIR([Transport])
AUD_DEFINE_PLUGIN_DIR([Input])
AUD_DEFINE_PLUGIN_DIR([Output])
AUD_DEFINE_PLUGIN_DIR([Effect])
AUD_DEFINE_PLUGIN_DIR([General])
AUD_DEFINE_PLUGIN_DIR([Visualization])
AUD_DEFINE_PLUGIN_DIR([Container])
AUD_DEFINE_PLUGIN_DIR([Discovery])


### ---------------------------------------------------------------------------
dnl XXX Work around some autoconf bugs.
if test "x$prefix" = "xNONE"; then
        prefix="${ac_default_prefix}"
fi

if test -z "$datadir"; then
    datadir="${prefix}/share"
fi

localedir="$datadir/locale"
AC_SUBST([localedir])

AUDACIOUS_DEFINES="-DPLUGINSUBS=$pluginsubs"
AUDACIOUS_DEFINES="$AUDACIOUS_DEFINES -DHARDCODE_BINDIR=\\\"$bindir\\\""
AUDACIOUS_DEFINES="$AUDACIOUS_DEFINES -DHARDCODE_DATADIR=\\\"$datadir/audacious\\\""
AUDACIOUS_DEFINES="$AUDACIOUS_DEFINES -DHARDCODE_PLUGINDIR=\\\"$plugindir\\\""
AUDACIOUS_DEFINES="$AUDACIOUS_DEFINES -DHARDCODE_LOCALEDIR=\\\"$localedir\\\""
AUDACIOUS_DEFINES="$AUDACIOUS_DEFINES -DHARDCODE_DESKTOPFILE=\\\"${datadir}/applications/audacious.desktop\\\""
AUDACIOUS_DEFINES="$AUDACIOUS_DEFINES -DHARDCODE_ICONFILE=\\\"${datadir}/pixmaps/audacious.png\\\""
AC_SUBST([AUDACIOUS_DEFINES])


### ---------------------------------------------------------------------------
dnl Output configuration files
dnl ==========================
AC_CONFIG_FILES([
    audacious.pc
    audclient.pc
    buildsys.mk
    extra.mk
    man/audtool.1
    man/audacious.1
])

BUILDSYS_TOUCH_DEPS
AC_OUTPUT

### ---------------------------------------------------------------------------
dnl Print out the results
dnl =====================
echo ""
echo "Configuration:"
echo ""
echo "  Install path:                           $prefix"

if test "x$AUDACIOUS_PATH" != "xno"; then
echo "  Current Audacious executable:           $AUDACIOUS_PATH"
fi

echo "  Allow user plugin dir:                  $enable_user_plugin_dir"
echo ""
echo "  Automatic character code detection:     $enable_chardet"
echo "  D-Bus support:                          $enable_dbus"
echo "  Session management (eggsm)              $enable_sm"
echo ""
echo "  SSE2:                                   $enable_sse2"
echo "  AltiVec:                                $enable_altivec"
echo ""

if test "x$AUDACIOUS_PATH" != "xno" && test "x$prefix/bin/audacious" != "x$AUDACIOUS_PATH"; then
    audlocation=`dirname $AUDACIOUS_PATH`
    echo "**"
    echo "** NOTICE! Configure has discovered that you already have Audacious installed"
    echo "** and it does not match with the given --prefix. You have Audacious installed"
    echo "** in ${audlocation} and you chose ${prefix}/bin."
    echo "**"
    echo "** If you don't want two copies of Audacious installed, rerun configure with"
    echo "** the --prefix option set to the location of the old Audacious, or uninstall"
    echo "** the old Audacious."
    echo "**"
fi

if test "x$enable_dbus" != "xyes"; then
    echo "**"
    echo "** WARNING! DBUS support is disabled. This means that various features"
    echo "** the user might expect (such as remotely adding files to session via"
    echo "** commandline) will not work!"
    echo "**"
fi
