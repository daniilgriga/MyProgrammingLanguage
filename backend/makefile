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

CFLAGS = -c $(FLAGS)
LDFLAGS = $(FLAGS) -lm

SOURCES_LIST = main.c backend_nasm.c tools.c file.c backend_bin.c

SOURCES = $(SOURCES_LIST:%=src/%)
OBJECTS = $(SOURCES_LIST:%.c=build/%.o)
DEPS = $(OBJECTS:%.o=%.d)
EXECUTABLE = build/backend

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@$(CC) $(LDFLAGS) $^ -o $@

-include $(DEPS)

build/%.o: src/%.c
	@$(CC) -I./include $(CFLAGS) -MMD -MP $< -o $@

nasm:
	nasm -g -f elf64 src/program.nasm -o build/output_nasm.o
	ld -m elf_x86_64 build/output_nasm.o -o build/nasm_test

clean:
	rm -f build/*.o $(EXECUTABLE)
