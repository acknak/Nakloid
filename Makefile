CXX           = g++
CXXFLAGS      = -std=c++11 -O4 -Wall -I/usr/local/include
DEST          = /usr/local/bin
LDFLAGS       = -L/usr/local/lib
LIBS          = -lfftw3 -lboost_filesystem -lboost_system -lm
SRCS          = $(wildcard *.cpp)
SRCS         := $(SRCS) $(wildcard core/*.cpp)
SRCS         := $(SRCS) $(wildcard format/*.cpp)
SRCS         := $(SRCS) $(wildcard parser/*.cpp)
SRCS         := $(SRCS) $(wildcard score/*.cpp)
SRCS         := $(SRCS) $(wildcard vocal_lib/*.cpp)
OBJS         := $(SRCS:%.cpp=%.o)
DEPS         := $(SRCS:%.cpp=%.d)
PROGRAM       = Nakloid

all:            $(PROGRAM)

$(PROGRAM):     $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(LIBS) -o $(PROGRAM)

clean:;         rm -f *.o */*.o *~ $(PROGRAM)

install:        $(PROGRAM)
	install -s $(PROGRAM) $(DEST)

%.o: %.cpp
	$(CXX) -c -MMD -MP $(CXXFLAGS) -o $@ $<

-include $(DEPS)
