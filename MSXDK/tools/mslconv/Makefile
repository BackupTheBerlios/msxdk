ISMINGW := $(shell uname | grep MINGW)
ifneq ($(ISMINGW),)
	EXTENSION = .exe
else
	EXTENSION = 
endif

TARGETS	 = mslconv$(EXTENSION)
OBJECTS  = main.o CMsl.o CMslkit.o CMwk.o CPatch.o CSample.o CTone.o CToneHeader.o CWave.o

INCDIR   = ../include
BINDIR   = ../../bin
LIBDIR   = ../lib
LIBS     = -lcommon

CXXFLAGS = -Wall -pedantic -O3 -I$(INCDIR)
ifneq ($(ISMINGW),)
	CXXFLAGS += -DMINGW
else
	CXXFLAGS += -DMAX_PATH=PATH_MAX
endif
LDFLAGS  = -L$(LIBDIR)

DEPDIR = .deps
DEPFILE = $(DEPDIR)/$(*F)

all: $(TARGETS)

mslconv$(EXTENSION): $(DEPDIR) $(OBJECTS)
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
