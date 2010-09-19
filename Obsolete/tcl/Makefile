CC    = gcc
OBJS  =	tkmain.o socket.o make.o
CFLAGS= -I/usr/local/include -I/usr/openwin/include
LIBS  =	-L/usr/local/lib -ltk4.0 -ltcl7.4 -L/usr/openwin/lib/X11 -lX \
	-lm -lsocket

Zblist:	$(OBJS)
	$(CC) $(CFLAGS) -o Zblist $(OBJS) $(LIBS)

testor:	testor.c
	$(CC) $(CFLAGS) -o testor testor.c -lsocket

install:
	cp Zblist zblist.tcl zfindfile.tcl zvarhelp.tcl $(ZPATH)
	cp zmake.tcl zfont.tcl $(ZPATH)
	[ -f $(ZPATH)/Zfindfile ] || ln -s $(ZPATH)/Zblist $(ZPATH)/Zfindfile
	[ -f $(ZPATH)/Zmake ] || ln -s $(ZPATH)/Zblist $(ZPATH)/Zmake
	[ -f $(ZPATH)/Zvarhelp ] || ln -s $(ZPATH)/Zblist $(ZPATH)/Zvarhelp
	[ -f $(ZPATH)/Zfont ] || ln -s $(ZPATH)/Zblist $(ZPATH)/Zfont

clean:
	rm -f *.o Zblist
