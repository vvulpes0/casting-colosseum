.PATH : $(.CURDIR)/rbt

CFLAGS += -I${SDLDIR}/include
CFLAGS += -I${.CURDIR}/rbt -I${.CURDIR}/pocketmod -I${.CURDIR}/stb
CFLAGS += -DPOCKETMOD_MAX_CHANNELS=4
CFLAGS += -DPOCKETMOD_MAX_SAMPLES=31
CFLAGS += -DSTBI_ONLY_PNG
CFLAGS += -DSTBI_MAX_DIMENSIONS=2048
LDFLAGS += -L${.OBJDIR}

.if ${OS} == Darwin
CFLAGS += -F/Library/Frameworks
CFLAGS += -I/Library/Frameworks/SDL2.framework/Headers
CFLAGS += -arch arm64
CFLAGS += -arch x86_64
LDFLAGS += -framework SDL2
LDFLAGS += -framework CoreFoundation
AR = libtool
ARFLAGS = -static -o
RPATH = -Wl,-rpath,'@executable_path/../Frameworks/'
.endif

libobjs  = delete.o insert.o find.o rotate_right.o rotate_left.o

.MAIN : main

librbt.a : ${libobjs}
	${AR} ${ARFLAGS} ${.TARGET} ${.ALLSRC:M*.o}

${libobjs}     : rbt.h
delete.o       : delete.c
find.o         : find.c
insert.o       : insert.c
rotate_left.o  : rotate_left.c
rotate_right.o : rotate_right.c

rbt-test : rbt-test.o librbt.a
	${LINK.c} -lrbt -o ${.TARGET} ${.ALLSRC:M*.o}

main.o  : main.c
audio.o : audio.c

main : main.o audio.o entities.o queue.o librbt.a
	${LINK.c} -lrbt ${RPATH} -o ${.TARGET} ${.ALLSRC:M*.o}

queue-test : queue-test.o queue.o
	${LINK.c} -o ${.TARGET} ${.ALLSRC:M*.o}
