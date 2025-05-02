SUBDIRS = frontend backend

.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) -s -C $@ all

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -s -C $$dir clean; \
	done
