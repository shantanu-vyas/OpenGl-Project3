CC		:= ccache clang -g
SRCDIR		 := src
BUILDSRCDIR	 := build

TESTDIR		 := test
BUILDTESTDIR := build/test
BINTESTDIR	 := ./

INCLUDEDIR	 := include
HEADEREXT	 := h
SRCEXT		 := c
OBJEXT		 := o
EXEEXT		 :=

SOURCES	  := $(shell find $(SRCDIR)		 -type f -name *.$(SRCEXT))
HEADERS	  := $(shell find $(INCLUDEDIR)	 -type f -name *.$(HEADEREXT))
TESTS	  := $(shell find $(TESTDIR)	  -type f -name *.$(SRCEXT))
OBJECTS	  := $(addprefix $(BUILDSRCDIR)/, $(notdir $(SOURCES:.$(SRCEXT)=.$(OBJEXT))))
TOBJECTS  := $(addprefix $(BUILDTESTDIR)/, $(notdir $(TESTS:.$(SRCEXT)=.$(OBJEXT))))
TPROGRAMS := $(addprefix $(BINTESTDIR)/, $(notdir $(TESTS:.$(SRCEXT)=)))

LIB	   := -L/usr/lib64/ -Wall -lGL -lGLU -lGLEW -lglut -lm
#CFLAGS := -Wall -g -Wno-deprecated -march=native -std=c++11
#CFLAGS := -g 
INC	   := -I $(INCLUDEDIR) 
FLAGS  := $(INC)  

test: $(OBJECTS) $(TPROGRAMS) $(TOBJECTS)

$(BINTESTDIR)/% : $(BUILDTESTDIR)/%.$(OBJEXT) $(OBJECTS)
		$(CC) $(FLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BUILDTESTDIR)/%.o : $(TESTDIR)/%.$(SRCEXT) $(HEADERS)
		$(CC) $(FLAGS) $(CFLAGS) -c -o $@ $<

$(BUILDSRCDIR)/%.o : $(SRCDIR)/%.$(SRCEXT) $(HEADERS)
		$(CC) $(FLAGS) $(CFLAGS) -c -o $@ $<

clean:
		@echo "Cleaning build directory...";
		rm	-rf $(BUILDSRCDIR)/*.$(OBJEXT)
		rm	-rf $(BUILDTESTDIR)/*.$(OBJEXT)
