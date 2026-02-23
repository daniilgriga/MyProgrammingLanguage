SUBDIRS = tools frontend middle_end backend

.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) -s -C $@ all $(if $(DEBUG),DEBUG=$(DEBUG))

nasm:
	@$(MAKE) -s -C backend nasm

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -s -C $$dir clean; \
	done
