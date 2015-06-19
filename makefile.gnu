#-zk0u = translate kanji to unicode... wwww
#-zk0 = kanji support~
#-zkl = current codepage

MFLAGS=-mh# -zm
CFLAGS=-zkl -wo -x #### -mc# -zdp# -zp16 -zq
OFLAGS=-ot -ox -ob -oh -or# -om -ol -ol+
FLAGS=-0 -d2 -l=dos -bt=dos $(OFLAGS) $(CFLAGS)
REMOVECOMMAND=rm
DIRSEP=/
SRC=src$(DIRSEP)
SRCLIB=$(SRC)lib$(DIRSEP)
JSMNLIB=$(SRCLIB)jsmn$(DIRSEP)
EXMMLIB=$(SRCLIB)exmm$(DIRSEP)

all: test.exe pcxtest.exe test2.exe scroll.exe maptest.exe maptest0.exe emsdump.exe emmtest.exe fmemtest.exe

#
#executables
#
scroll.exe: scroll.obj modex16.obj dos_kb.obj bitmap.obj mapread.obj jsmn.obj lib_head.obj
	wcl $(FLAGS) scroll.obj modex16.obj dos_kb.obj bitmap.obj mapread.obj jsmn.obj lib_head.obj# 16/lib/x/modex.lib
scroll.obj: $(SRC)scroll.c
	wcl $(FLAGS) -c $(SRC)scroll.c
test.exe: test.obj modex16.obj bitmap.obj lib_head.obj
	wcl $(FLAGS) test.obj modex16.obj bitmap.obj lib_head.obj

test2.exe: test2.obj modex16.obj bitmap.obj planar.obj lib_head.obj
	wcl $(FLAGS) test2.obj modex16.obj bitmap.obj planar.obj lib_head.obj

pcxtest.exe: pcxtest.obj modex16.obj bitmap.obj lib_head.obj
	wcl $(FLAGS) pcxtest.obj modex16.obj bitmap.obj lib_head.obj

maptest.exe: maptest.obj mapread.obj jsmn.obj modex16.obj bitmap.obj lib_head.obj
	wcl $(FLAGS) maptest.obj mapread.obj jsmn.obj modex16.obj bitmap.obj lib_head.obj

maptest0.exe: maptest0.obj fmapread.obj farjsmn.obj# modex16.obj bitmap.obj lib_head.obj
	wcl $(FLAGS) $(MFLAGS) maptest0.obj fmapread.obj farjsmn.obj# modex16.obj bitmap.obj lib_head.obj

emmtest.exe: emmtest.obj memory.obj
	wcl $(FLAGS) $(MFLAGS) emmtest.obj memory.obj

emsdump.exe: emsdump.obj memory.obj
	wcl $(FLAGS) $(MFLAGS) emsdump.obj memory.obj

fmemtest.exe: fmemtest.obj# memory.obj
	wcl $(FLAGS) $(MFLAGS) fmemtest.obj# memory.obj

#
#executable's objects
#
test.obj: $(SRC)test.c $(SRCLIB)modex16.h
	wcl $(FLAGS) -c $(SRC)test.c

test2.obj: $(SRC)test2.c $(SRCLIB)modex16.h
	wcl $(FLAGS) -c $(SRC)test2.c

pcxtest.obj: $(SRC)pcxtest.c $(SRCLIB)modex16.h
	wcl $(FLAGS) -c $(SRC)pcxtest.c

maptest.obj: $(SRC)maptest.c $(SRCLIB)modex16.h
	wcl $(FLAGS) -c $(SRC)maptest.c

maptest0.obj: $(SRC)maptest0.c# $(SRCLIB)modex16.h
	wcl $(FLAGS) $(MFLAGS) -c $(SRC)maptest0.c

emmtest.obj: $(SRC)emmtest.c
	wcl $(FLAGS) $(MFLAGS) -c $(SRC)emmtest.c

emsdump.obj: $(SRC)emsdump.c
	wcl $(FLAGS) $(MFLAGS) -c $(SRC)emsdump.c

fmemtest.obj: $(SRC)fmemtest.c
	wcl $(FLAGS) $(MFLAGS) -c $(SRC)fmemtest.c

#
#non executable objects libraries
#
modex16.obj: $(SRCLIB)modex16.h $(SRCLIB)modex16.c
	wcl $(FLAGS) -c $(SRCLIB)modex16.c

dos_kb.obj: $(SRCLIB)dos_kb.h $(SRCLIB)dos_kb.c
	wcl $(FLAGS) -c $(SRCLIB)dos_kb.c

bitmap.obj: $(SRCLIB)bitmap.h $(SRCLIB)bitmap.c
	wcl $(FLAGS) -c $(SRCLIB)bitmap.c

planar.obj: $(SRCLIB)planar.h $(SRCLIB)planar.c
	wcl $(FLAGS) -c $(SRCLIB)planar.c

mapread.obj: $(SRCLIB)mapread.h $(SRCLIB)mapread.c
	wcl $(FLAGS) -c $(SRCLIB)mapread.c

fmapread.obj: $(SRCLIB)fmapread.h $(SRCLIB)fmapread.c
	wcl $(FLAGS) $(MFLAGS) -c $(SRCLIB)fmapread.c

lib_head.obj: $(SRCLIB)lib_head.h $(SRCLIB)lib_head.c
	wcl $(FLAGS) -c $(SRCLIB)lib_head.c

jsmn.obj: $(JSMNLIB)jsmn.h $(JSMNLIB)jsmn.c
	wcl $(FLAGS) -c $(JSMNLIB)jsmn.c

farjsmn.obj: $(JSMNLIB)farjsmn.h $(JSMNLIB)farjsmn.c
	wcl $(FLAGS) $(MFLAGS) -c $(JSMNLIB)farjsmn.c

memory.obj: $(EXMMLIB)memory.h $(EXMMLIB)memory.c
	wcl $(FLAGS) $(MFLAGS) -c $(EXMMLIB)memory.c

#
#other~
#
clean: .symbolic
#	@$(REMOVECOMMAND) *.obj
	@$(REMOVECOMMAND) *.OBJ
#	@$(REMOVECOMMAND) *.out
#	@$(REMOVECOMMAND) *.OUT
	@$(REMOVECOMMAND) makefi~1
	@$(REMOVECOMMAND) __WCL__.LNK
#	@$(REMOVECOMMAND) *.smp
	@$(REMOVECOMMAND) *.SMP