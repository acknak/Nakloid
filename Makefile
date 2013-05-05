CC            = gcc
CFLAGS        = -O4 -Wall -I/usr/local/include
DEST          = /usr/local/bin
LDFLAGS       = -L/usr/local/lib
LIBS          = -lstdc++ -lfftw3-3 -lboost_filesystem -lm
OBJS          = $(patsubst %.c,%.o,$(wildcard *.cpp))
OBJS         := $(OBJS) $(patsubst %.c,%.o,$(wildcard voiceDB/*.cpp))
OBJS         := $(OBJS) $(patsubst %.c,%.o,$(wildcard parser/*.cpp))
OBJS         := $(OBJS) $(patsubst %.c,%.o,$(wildcard PSOLA/*.cpp))
OBJS         := $(OBJS) $(patsubst %.c,%.o,$(wildcard score/*.cpp))
PROGRAM       = Nakloid

all:            $(PROGRAM)

$(PROGRAM):     $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) $(LIBS) -o $(PROGRAM)

clean:;         rm -f *.o *~ $(PROGRAM)

install:        $(PROGRAM)
	install -s $(PROGRAM) $(DEST)
