###########################################################################
#    Copyright (c) 1991 - 1993 Heinz W. Werntges.  All rights reserved.
#    Distributed by Free Software Foundation, Inc.
#
# This file is part of HP2xx.
#
# HP2xx is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
# to anyone for the consequences of using it or for whether it serves any
# particular purpose or works at all, unless he says so in writing.  Refer
# to the GNU General Public License, Version 2 or later, for full details
#
# Everyone is granted permission to copy, modify and redistribute
# HP2xx, but only under the conditions described in the GNU General Public
# License.  A copy of this license is supposed to have been
# given to you along with HP2xx so you can know your rights and
# responsibilities.  It should be in a file named COPYING.  Among other
# things, the copyright notice and this notice must be preserved on all
# copies.
###########################################################################
#
# Generic Makefile for misc. versions of hp2xx; strong emphasis on UNIXes
#

# Utilities etc.:
#
SHELL	=	/bin/sh

CHMOD	=	chmod
CP	=	cp
MKINFO	=	makeinfo
RM	=	rm
STRIP	=	strip

# Installation sites (GNU conventions):
#
prefix	=	/usr/local
bindir	=	$(prefix)/bin
includedir =	$(prefix)/include
mandir	=	$(prefix)/man
man1dir	=	$(mandir)/man1
infodir	=	$(prefix)/info

# CC and CFLAGS set for gcc, but any ANSI-C compiler should work.
# For non-gcc compilers, simply set CFLAGS = -O (to start with)
#
CC	= gcc
CFLAGS	= -O2 -Wall -ansi -fstrength-reduce -finline-functions

LFLAGS	=		# only sometimes needed; see below for examples
CPPFLAGS= -I.		# only sometimes needed; see below for examples
BINDCMD = 		# only needed for EMX and DJGPP DOS extenders



#############################################################################
# MAIN CONFIGURATION TASK:
# Define the target machine / OS by uncommenting the appropriate lines below.
# Remember to comment-out the defaults (a) if they do not apply!
#
# Generic UNIX + X11 previewer (default)
#
DEFINES   = -DUNIX -DHAS_UNIX_X11
PREVIEWER = to_x11
ALL_LIBS  = -lX11 -lm		Maybe -lX instead of -lX11 is needed?
#
# Generic UNIX, no previewer
#
# DEFINES   = -DUNIX
# PREVIEWER = no_prev
# ALL_LIBS  = -lm
#
# Convex-OS
#
# DEFINES   = -DUNIX -DHAS_UNIX_X11
# PREVIEWER = to_x11
# ALL_LIBS  = -lX11 -lm
# CC	  = cc
# CFLAGS	  = -O
#
# SunOS + X11 previewer
#
# LFLAGS    = -L$(OPENWINHOME)/lib
# CPPFLAGS  = -I$(OPENWINHOME)/include
# DEFINES   = -DUNIX -DHAS_UNIX_X11
# PREVIEWER = to_x11
# ALL_LIBS  = -lX -lm
#
# SunOS + Sunview previewer
#
# DEFINES   = -DUNIX -DHAS_UNIX_SUNVIEW
# PREVIEWER = to_sunvw
# ALL_LIBS  = -lsunwindow -lsuntool -lpixrect -lm
#
# OS/2 2.x + PM previewer, gcc/emx
#
# DEFINES   = -DOS2 -DHAS_OS2_PM
# PREVIEWER = to_pm
# ALL_LIBS  = -los2 -lm
# BINDCMD   = emxbind -p c:\emx\bin\emxl.exe $(PROGRAM)
#
# OS/2 2.x + EMX full-screen previewer		# Planned only !
#
# DEFINES   = -DOS2 -DHAS_OS2_EMX
# PREVIEWER = to_os2
# ALL_LIBS  = -los2 -l16bit -lm
# BINDCMD   = emxbind -f c:\emx\bin\emxl.exe $(PROGRAM)
#
#
#--------------------------------------------------------------------------
# Below find *fragments* of how to augment this makefile for other systems.
# Note that I simply collected all definitions I am currently aware of,
# but that this collection is still subject to change.
#
# You are welcome to improve it!
#
# Note also that most of the systems listed below are already supported
# by special makefiles, though I'd like to make these obsolete by a
# future version of this file. Currently, use the special makefile!
#
# I could not test the settings below, lacking a well-behaved DOS make.exe:
# DOS/DJGPP + DJ previewer		# Use dosdjgpp.mak !
#
# DEFINES   = -DDOS -DGNU -DHAS_DOS_DJGR
# PREVIEWER = to_dj_gr
# ALL_LIBS  = -lgr -lm
# BINDCMD   = copy /b c:\djgpp\bin\stub.exe+$(PROGRAM) $(PROGRAM).exe
#
# DOS / Borland bcc			# Use borland.mak !
#
# DEFINES   = -DDOS -DHAS_DOS_VGA
# PREVIEWER = to_vga
# ALL_LIBS  = -lm
#
# ATARI ST/TT
#
# DEFINES   = -DATARI			# Use atari.mak !
# PREVIEWER = to_atari
# ALL_LIBS  = -lm 	# Add whatever may be needed for Ataris
#
# AMIGA
#
# DEFINES   = -DAMIGA			# Use amiga.mak !
# PREVIEWER = to_amiga
# ALL_LIBS  = -lm 	# Add whatever may be needed for Amigas
#
# VAX/VMS	# I am still waiting for the .MMS file from the VAX people...
#
# DEFINES   = -DVAX
# PREVIEWER = to_uis
# ALL_LIBS  = -lm 	# Add whatever may be needed for VMS
#
#############################################################################
# Decide below if you want to include some extra modes:
#	Recommendation: Add PIC & PAC only for ATARI
#
# Standard setting: No extras:
EX_SRC	=
EX_OBJ	=
EX_DEFS	=
#
# Include extras:
# EX_SRC	= to_pic.c to_pac.c
# EX_OBJ	= to_pic.o to_pac.o
# EX_DEFS	= -DPIC_PAC
#
#############################################################################
# No user-serviceable parts below!
#############################################################################

COMMON_INCS	= hp2xx.h bresnham.h
ALL_INCS	= $(COMMON_INCS) chardraw.h charset0.h getopt.h x11.h ilbm.h iff.h
ALL_CFLAGS	= -c $(CFLAGS)


# $(SRCS) NOT USED YET:
SRCS	= hp2xx.c hpgl.c picbuf.c bresnham.c chardraw.c getopt.c getopt1.c \
	  $(ALL_INCS)\
	  to_mf.c to_pcx.c to_pcl.c to_eps.c to_img.c to_pbm.c $(PREVIEWER).c \
	  $(EX_SRC)

OBJS	= hp2xx.o hpgl.o picbuf.o bresnham.o chardraw.o getopt.o getopt1.o \
	  to_mf.o to_pcx.o to_pcl.o to_eps.o to_img.o to_pbm.o $(PREVIEWER).o \
	  $(EX_OBJ)

PROGRAM	= hp2xx

default:
	@echo "This makefile needs manual configuration! Edit it now!
	@echo "After configuration, you may use the following commands:
	@echo   ""
	@echo   "make all             Compile everything"
	@echo   "make check           Run a color application in preview mode"
	@echo   "make install-bin     Install the binary file"
	@echo   "make install-man     Install the manual page"
	@echo   "make install-info    Create and install the info file"
	@echo   "make install         Install everything"
	@echo   "make mostlyclean"
	@echo   "make clean"
	@echo   "make realclean"



#########################################################################
#									#
# 			Implicit Rules					#
#									#
#########################################################################

all: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(ALL_LIBS) -o $(PROGRAM)
	$(BINDCMD)


.c.o:	$< $(COMMON_INCS)
	-$(RM) -f $@
	$(CC) $(DEFINES) $(EX_DEFS) $(ALL_CFLAGS) $(CPPFLAGS) $<


#########################################################################
#									#
# 			Explicit Rules					#
#									#
#########################################################################

hp2xx.info:	../doc/hp2xxinf.tex
		$(CP) ../doc/hp2xxinf.tex hp2xx.texinfo
		$(MKINFO) hp2xx.texinfo

bresnham.o:	$< bresnham.h

chardraw.o:	$< $(COMMON_INCS) chardraw.h charset0.h

getopt.o:	$< getopt.h

getopt1.o:	$< getopt.h

hp2xx.o:	$< $(COMMON_INCS) getopt.h

hpgl.o:		$< $(COMMON_INCS) chardraw.h

ilbm.o:		$< $(COMMON_INCS) ilbm.h iff.h

to_x11.o:	$< $(COMMON_INCS) x11.h

#########################################################################

check:
	./$(PROGRAM) -m pre -c1234567 -h150 -w150 ../hp-tests/colors.hp

clean:
	-make mostlyclean
	-$(RM) -f core a.out ./hp2xx.texinfo ./hp2xx.info

distclean:
	@echo make distclean -- not supported yet.

dist:
	@echo make dist -- not supported yet.

mostlyclean:
	-$(RM) -f *.o

realclean:
	-make clean
	-$(RM) -f $(PROGRAM)

TAGS:
	@echo make TAGS -- not supported nor needed.

install:	install-exe install-info install-man

install-exe:	$(PROGRAM)
		-$(STRIP) $(PROGRAM)
		$(CP) $(PROGRAM) $(bindir)
		$(CHMOD) 755 $(bindir)/$(PROGRAM)

install-info:	hp2xx.info
		$(CP) hp2xx.info $(infodir)
		$(CHMOD) 644 $(infodir)/hp2xx.info

install-man:	../doc/hp2xx.1
		$(CP) ../doc/hp2xx.1 $(man1dir)
		$(CHMOD) 644 $(man1dir)/hp2xx.1

