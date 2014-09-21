CXX           = g++
CXXFLAGS      = -std=c++11 -O4 -Wall -I/usr/local/include
DEST          = /usr/local/bin
LDFLAGS       = -L/usr/local/lib
LIBS          = -lfftw3 -lboost_filesystem -lboost_system -lm
OBJS         := $(patsubst %.cpp,%.o,$(wildcard *.cpp))
OBJS         := $(OBJS) $(patsubst %.cpp,%.o,$(wildcard core/*.cpp))
OBJS         := $(OBJS) $(patsubst %.cpp,%.o,$(wildcard format/*.cpp))
OBJS         := $(OBJS) $(patsubst %.cpp,%.o,$(wildcard parser/*.cpp))
OBJS         := $(OBJS) $(patsubst %.cpp,%.o,$(wildcard score/*.cpp))
OBJS         := $(OBJS) $(patsubst %.cpp,%.o,$(wildcard vocal_lib/*.cpp))
PROGRAM       = Nakloid

all:            $(PROGRAM)

$(PROGRAM):     $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(LIBS) -o $(PROGRAM)

clean:;         rm -f *.o */*.o *~ $(PROGRAM)

install:        $(PROGRAM)
	install -s $(PROGRAM) $(DEST)
