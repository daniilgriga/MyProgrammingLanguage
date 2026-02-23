CC = gcc

FLAGS = -O3 -g -msse4.2 -mavx2 -DNDEBUG -ggdb3 -Wall -Wextra  -Waggressive-loop-optimizations         \
	   	-Wmissing-declarations -Wcast-align -Wchar-subscripts     			        				  \
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
		-fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,$\
		integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,$\
		returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

CFLAGS = -c $(FLAGS) -I./include -I../tools/include
LDFLAGS = $(FLAGS) -lm

BUILD_DIR = build

SOURCES_LIST = main.c tree.c simplification.c
SOURCES_TOOL_LIST = errors.c file.c keywords.c tree_io.c

SOURCES = $(SOURCES_LIST:%=src/%)

TOOL_OBJECTS = $(SOURCES_TOOL_LIST:%.c=../tools/build/%.o)
OBJECTS = $(SOURCES_LIST:%.c=$(BUILD_DIR)/%.o) $(TOOL_OBJECTS)

DEPS = $(SOURCES_LIST:%.c=$(BUILD_DIR)/%.d) $(SOURCES_TOOL_LIST:%.c=../tools/build/%.d)

EXECUTABLE = $(BUILD_DIR)/middle_end

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(OBJECTS) $(FLAGS) -lm -o $@

$(BUILD_DIR)/%.o: src/%.c makefile | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP $< -o $@

-include $(DEPS)

clean:
	rm -f build/*.o $(EXECUTABLE)
