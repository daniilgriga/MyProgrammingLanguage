#pragma once

#include <stdint.h>
#include <stddef.h>

#include "x86_emitter.h"

// virtual memory addresses
#define TEXT_VADDR  0x400000   // .text section address
#define DATA_VADDR  0x600000   // .data section address

// elf file builder
struct ElfBuilder
{
    struct CodeBuffer* text_section;   // machine code (.text)
    struct CodeBuffer* data_section;   // data (.data)
    uint64_t entry_point;              // entry point (_start)
};

// ========== elf builder management ========== //

struct ElfBuilder* create_elf_builder();
void destroy_elf_builder (struct ElfBuilder* builder);
void set_entry_point (struct ElfBuilder* builder, uint64_t offset);

// ========== section operations ========== //

size_t get_text_offset (struct ElfBuilder* builder);
size_t get_data_offset (struct ElfBuilder* builder);
void add_data_bytes (struct ElfBuilder* builder, const uint8_t* bytes, size_t count);
void add_data_qword (struct ElfBuilder* builder, uint64_t value);
void align_data_section (struct ElfBuilder* builder, size_t alignment);

// ========== code generation ========== //

struct CodeBuffer* get_text_buffer (struct ElfBuilder* builder);
uint64_t get_data_absolute_addr (struct ElfBuilder* builder, size_t offset);

// ========== elf file generation ========== //

size_t get_header_size (void);   // sizeof(Elf64_Ehdr) + 2*sizeof(Elf64_Phdr)
int write_elf_executable (struct ElfBuilder* builder, const char* filename);
