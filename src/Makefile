SUBDIRS = libaudcore libaudgui libaudtag audacious

include ../extra.mk

ifeq ($(USE_EGGSM),yes)
SUBDIRS := libeggsmclient ${SUBDIRS}
endif

ifeq ($(USE_DBUS),yes)
SUBDIRS := libaudclient audtool ${SUBDIRS}
endif

include ../buildsys.mk
