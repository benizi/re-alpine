# ========================================================================
# Copyright 1988-2007 University of Washington
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# 
# ========================================================================


# Program:	Portable C client makefile -- MS-DOS version
#
# Author:	Mark Crispin
#		Networks and Distributed Computing
#		Computing & Communications
#		University of Washington
#		Administration Building, AG-44
#		Seattle, WA  98195
#		Internet: MRC@CAC.Washington.EDU
#
# Date:		11 May 1989
# Last Edited:	23 May 2007


OS = wsk
EXTRAAUTHENTICATORS=
DEFAULTAUTHENTICATORS= ext md5 pla log
EXTRADRIVERS = 
DRIVERS = imap nntp pop3 mtx bezerk
DEFAULTDRIVER = mtx
CFLAGS= -AL /DCHUNKSIZE=4096 -nologo $(EXTRACFLAGS)
CC = cl

all:	mtest.exe

.c.obj:
	$(CC) -c $(CFLAGS) $*.c

osdep.h: os_$(OS).h
	copy os_$(OS).h osdep.h
	drivers $(EXTRADRIVERS) $(DRIVERS) dummy
	mkauths $(EXTRAAUTHENTICATORS) $(DEFAULTAUTHENTICATORS)
	ECHO #define DEFAULTPROTO $(DEFAULTDRIVER)proto >> LINKAGE.H
	ECHO   if (!mail_parameters (NIL,GET_GETS)) >> LINKAGE.C
	ECHO     mail_parameters (NIL,SET_GETS,(void *) dos_default_gets); >> LINKAGE.C
	ECHO mail_versioncheck (CCLIENTVERSION); >> LINKAGE.C

mtest.obj: mail.h smtp.h misc.h osdep.h mtest.c

mail.obj: mail.h misc.h osdep.h mail.c

misc.obj: mail.h misc.h misc.c

fdstring.obj: mail.h misc.h osdep.h fdstring.h fdstring.c

flstring.obj: mail.h misc.h osdep.h flstring.h flstring.c

netmsg.obj: mail.h misc.h netmsg.h osdep.h netmsg.c

newsrc.obj: mail.h misc.h newsrc.h osdep.h newsrc.c

rfc822.obj: mail.h rfc822.h misc.h rfc822.c

smanager.obj: mail.h misc.h smanager.c

utf8.obj: mail.h misc.h osdep.h utf8.h

utf8aux.obj: mail.h misc.h osdep.h utf8.h

imap4r1.obj: mail.h imap4r1.h misc.h osdep.h imap4r1.c

nntp.obj: mail.h nntp.h smtp.h rfc822.h misc.h osdep.h nntp.c

pop3.obj: mail.h rfc822.h misc.h osdep.h pop3.c

smtp.obj: mail.h smtp.h rfc822.h misc.h osdep.h smtp.c

os_$(OS).obj: mail.h osdep.h env_dos.h fs.h ftl.h nl.h tcp.h \
	os_$(OS).c fs_dos.c ftl_dos.c nl_dos.c env_dos.c pmatch.c write.c

mtxdos.obj: mail.h misc.h osdep.h mtxdos.c

bezrkdos.obj: mail.h misc.h osdep.h bezrkdos.c

dummydos.obj: mail.h dummy.h misc.h osdep.h dummydos.c

cclient.lib: mail.obj misc.obj fdstring.obj flstring.obj netmsg.obj \
	newsrc.obj rfc822.obj smanager.obj utf8.obj utf8aux.obj \
	imap4r1.obj nntp.obj pop3.obj smtp.obj os_$(OS).obj \
	mtxdos.obj bezrkdos.obj dummydos.obj
	erase cclient.lib
	lib cclient +mail+misc+fdstring+flstring+netmsg+newsrc+rfc822+smanager+utf8+utf8aux+imap4r1+nntp+pop3+smtp+os_$(OS)+mtxdos+bezrkdos+dummydos;

mtest.exe: cclient.lib mtest.obj
	mtest$(OS)
