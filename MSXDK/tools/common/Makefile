
TARGETS	 = libcommon.a
OBJECTS  = loadsave_file.o OptionParser.o
HEADERS	 = loadsave_file.h OptionParser.h

INCDIR   = ../include
LIBDIR   = ../lib
LIBS	 =

CXXFLAGS = -Wall -O2 -I$(INCDIR)


DEPDIR = .deps
DEPFILE = $(DEPDIR)/$(*F)

all: $(TARGETS)

libcommon.a: $(DEPDIR) $(OBJECTS)
	ar rvu $@ $(OBJECTS) $(LIBS)
	cp -f $(TARGETS) $(LIBDIR)
	cp -f $(HEADERS) $(INCDIR)

clean:
	$(RM) $(OBJECTS) $(TARGETS)
	$(RM) -r $(DEPDIR)

install: all
	
%.o : %.cpp
	$(CXX) -Wp,-MD,$(DEPFILE).d $(CXXFLAGS) -c -o $@ $<
	@cp $(DEPFILE).d $(DEPFILE).P; \
		sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $(DEPFILE).d >> $(DEPFILE).P; \
	rm -f $(DEPFILE).d

$(DEPDIR):
	@mkdir $(DEPDIR)

-include $(OBJECTS:%.o=$(DEPDIR)/%.P)

