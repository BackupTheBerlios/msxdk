ISMINGW := $(shell uname | grep MINGW)
ifneq ($(ISMINGW),)
	EXTENSION = .exe
else
	EXTENSION = 
endif

TARGETS	 = dodsk$(EXTENSION)
OBJECTS  = main.o FATDisk.o OptionParser.o

INCDIR   = ../include
BINDIR   = ../../bin
LIBDIR   = ../lib
LIBS     = 

CXXFLAGS = -Wall -pedantic -O2 -I$(INCDIR)
ifneq ($(ISMINGW),)
	CXXFLAGS += -DMINGW
else
	CXXFLAGS += -DMAX_PATH=PATH_MAX
endif
LDFLAGS  = -L$(LIBDIR)

DEPDIR = .deps
DEPFILE = $(DEPDIR)/$(*F)

all: $(TARGETS)

dodsk$(EXTENSION): $(DEPDIR) $(OBJECTS)
	g++ $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)
	strip $@

clean:
	$(RM) $(OBJECTS) $(TARGETS)
	$(RM) -r $(DEPDIR)	

install: all
	cp -f $(TARGETS) $(BINDIR)

%.o : %.cpp
	$(CXX) -Wp,-MD,$(DEPFILE).d $(CXXFLAGS) -c -o $@ $<
	@cp $(DEPFILE).d $(DEPFILE).P; \
		sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $(DEPFILE).d >> $(DEPFILE).P; \
	rm -f $(DEPFILE).d

$(DEPDIR):
	@mkdir $(DEPDIR)
	
-include $(OBJECTS:%.o=$(DEPDIR)/%.P)
