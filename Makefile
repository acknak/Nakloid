CC            = gcc
CFLAGS        = -O4 -Wall -I/usr/local/include
DEST          = /usr/local/bin
LDFLAGS       = -L/usr/local/lib
LIBS          = -lstdc++ -lfftw3-3 -lboost_filesystem -lm
OBJS          = $(patsubst %.c,%.o,$(wildcard *.cpp))
OBJS         := $(OBJS) $(patsubst %.c,%.o,$(wildcard core/*.cpp))
OBJS         := $(OBJS) $(patsubst %.c,%.o,$(wildcard format/*.cpp))
OBJS         := $(OBJS) $(patsubst %.c,%.o,$(wildcard parser/*.cpp))
OBJS         := $(OBJS) $(patsubst %.c,%.o,$(wildcard score/*.cpp))
OBJS         := $(OBJS) $(patsubst %.c,%.o,$(wildcard vocal_lib/*.cpp))
PROGRAM       = Nakloid

all:            $(PROGRAM)

$(PROGRAM):     $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) $(LIBS) -o $(PROGRAM)

clean:;         rm -f *.o *~ $(PROGRAM)

install:        $(PROGRAM)
	install -s $(PROGRAM) $(DEST)
