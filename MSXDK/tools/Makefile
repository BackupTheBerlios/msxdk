#SUBDIRS = 3pp common appack arcreate
SUBDIRS = common arcreate dodsk

all clean install::
	@if [ ! -d include ]; then \
		mkdir include; \
	fi; \
	if [ ! -d lib ]; then \
		mkdir lib; \
	fi; \
	if [ ! -d ../bin ]; then \
		mkdir ../bin; \
	fi; \
	dirs="$(SUBDIRS)"; \
	for i in $$dirs; do \
		if (cd $$i && $(MAKE) $@); \
		then true; else exit 1; fi; \
	done
