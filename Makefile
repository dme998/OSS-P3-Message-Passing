
CC= gpp
RM= rm -vf
CFLAGS= -Wall -g 
CPPFLAGS= -I.
SRCFILES= oss.cpp user.cpp 
OBJFILES= $(patsubst %.cpp, %.o, $(SRCFILES))
PROGFILES= $(patsubst %.cpp, %, $(SRCFILES))

.PHONY: all clean

all: $(PROGFILES)

clean:
	$(RM) $(OBJFILES) $(PROGFILES)
