CC = gcc

FLAGS = -O3 -g -msse4.2 -mavx2 -DNDEBUG -ggdb3 -Wall -Wextra  -Waggressive-loop-optimizations         \
	   	-Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts             				  \
		-Wconversion -Wempty-body -Wfloat-equal										  				  \
		-Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline 				  \
		-Wlogical-op -Wopenmp-simd -Wpacked                                           				  \
		-Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion      				  \
		-Wstrict-overflow=2 -Wsuggest-attribute=noreturn                              				  \
		-Wsuggest-final-methods -Wsuggest-final-types -Wswitch-default                				  \
		-Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused                 				  \
		-Wvariadic-macros -Wno-missing-field-initializers -Wno-narrowing              				  \
		-Wno-varargs -Wstack-protector                                                				  \
		-fstack-protector -fstrict-overflow -flto -fno-omit-frame-pointer             				  \
		-pie -fPIE -Werror=vla                                                        				  \

CFLAGS = -c $(FLAGS) -I./include
DPFLAGS = -MMD -MP

BUILD_DIR = build

SOURCES_LIST = errors.c file.c log.c keywords.c

SOURCES = $(SOURCES_LIST:%=src/%)
OBJECTS = $(SOURCES_LIST:%.c=$(BUILD_DIR)/%.o)

DEPS = $(OBJECTS:%.o=%.d)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: src/%.c makefile | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DPFLAGS) $< -o $@

all: $(OBJECTS)

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
