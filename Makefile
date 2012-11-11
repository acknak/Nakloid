CC            = gcc
CFLAGS        = -O4 -Wall -I/usr/local/include
DEST          = /usr/local/bin
LDFLAGS       = -L/usr/local/lib
LIBS          = -lstdc++ -lfftw3-3 -lm
OBJS          = $(patsubst %.c,%.o,$(wildcard *.cpp))
OBJS         := $(OBJS) $(patsubst %.c,%.o,$(wildcard PSOLA/*.cpp))
OBJS         := $(OBJS) $(patsubst %.c,%.o,$(wildcard parser/*.cpp))
OBJS         := $(OBJS) $(patsubst %.c,%.o,$(wildcard arranger/*.cpp))
PROGRAM       = Nakloid

all:            $(PROGRAM)

$(PROGRAM):     $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $(PROGRAM)

clean:;         rm -f *.o *~ $(PROGRAM)

install:        $(PROGRAM)
	install -s $(PROGRAM) $(DEST)
