#/*****************************************************************************
#*
#*      ETSI STQ WI007 DSR Front-End Feature Extraction Algorithm
#*      C-language software implementation 
#*      Version 1.1.3   May, 2003
#*
#*****************************************************************************/
#
#	makefile
#

CFLAGS	      = -O -Aa

DEST	      =

EXTHDRS	      =

HDRS	      = FEfunc.h \
		fileio.h

INSTALL	      = /etc/install

LD	      = cc

LDFLAGS	      = -lm

LIBS	      =

MAKEFILE      = Makefile

OBJS	      = FEfunc.o \
		FrontEnd.o \
		fileio.o 

PRINT	      = a2ps

PROGRAM       = FrontEnd

SHELL	      = /bin/sh

SRCS	      = FEfunc.c \
		FrontEnd.c \
		fileio.c 

SYSHDRS	      = /usr/include/.unsupp/sys/_errno.h \
		/usr/include/errno.h \
		/usr/include/math.h \
		/usr/include/pwd.h \
		/usr/include/stdio.h \
		/usr/include/stdlib.h \
		/usr/include/string.h \
		/usr/include/sys/errno.h \
		/usr/include/sys/stdsyms.h

all:		$(PROGRAM)

$(OBJS):	$(SRCS) $(HDRS) $(EXTHDRS)
		@echo "Compiling ..."
		@$(LD) $(CFLAGS) -c $(SRCS)
		@echo "done"

$(PROGRAM):     $(OBJS) $(LIBS)
		@echo "Linking $(PROGRAM) ..."
		@$(LD) $(OBJS) $(LIBS) -o $(PROGRAM) $(LDFLAGS)
		@echo "done"

clean:;		@rm -f $(OBJS) core

clobber:;	@rm -f $(OBJS) $(PROGRAM) core tags

depend:;	@mkmf -f $(MAKEFILE) ROOT=$(ROOT)

echo:;		@echo $(HDRS) $(SRCS)

index:;		@ctags -wx $(HDRS) $(SRCS)

install:	$(PROGRAM)
		@echo Installing $(PROGRAM) in $(DEST)
		@-strip $(PROGRAM)
		@if [ $(DEST) != . ]; then \
		(rm -f $(DEST)/$(PROGRAM); $(INSTALL) -f $(DEST) $(PROGRAM); \
		 rm -f $(OBJS) $(PROGRAM)); fi

purify:;	@purify $(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM) -lm

print:;		@$(PRINT) $(HDRS) $(SRCS)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:		$(DEST)/$(PROGRAM)

$(DEST)/$(PROGRAM): $(SRCS) $(LIBS) $(HDRS) $(EXTHDRS)
		@$(MAKE) -f $(MAKEFILE) ROOT=$(ROOT) DEST=$(DEST) install
###
FEfunc.o: /usr/include/stdlib.h /usr/include/sys/stdsyms.h /usr/include/pwd.h \
	/usr/include/stdio.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/.unsupp/sys/_errno.h /usr/include/math.h
FrontEnd.o: /usr/include/stdio.h /usr/include/sys/stdsyms.h \
	/usr/include/stdlib.h /usr/include/pwd.h /usr/include/errno.h \
	/usr/include/sys/errno.h /usr/include/.unsupp/sys/_errno.h \
	/usr/include/string.h /usr/include/math.h fileio.h
fileio.o: /usr/include/stdio.h /usr/include/sys/stdsyms.h \
	/usr/include/stdlib.h /usr/include/pwd.h /usr/include/errno.h \
	/usr/include/sys/errno.h /usr/include/.unsupp/sys/_errno.h \
	/usr/include/string.h fileio.h
