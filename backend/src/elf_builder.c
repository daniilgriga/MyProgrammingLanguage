#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <elf.h>

#include "elf_builder.h"
#include "x86_emitter.h"
#include "file.h"

// =========================== ELF64 FILE FORMAT =========================== //
//
// ELF file structure:
// +------------------+
// | ELF Header       |  <- describes file type, architecture, entry point
// +------------------+
// | Program Headers  |  <- describes segments for loading into memory
// +------------------+
// | .text section    |  <- executable code
// +------------------+
// | .data section    |  <- initialized data
// +------------------+
//
// use simplified ELF with only 2 loadable segments:
// - PT_LOAD #1: .text (executable)
// - PT_LOAD #2: .data (read/write)
//
// ========================================================================= //

// ========== elf builder management ========== //

struct ElfBuilder* create_elf_builder()
{
    struct ElfBuilder* builder = (struct ElfBuilder*) calloc (1, sizeof (struct ElfBuilder));
    if (!builder)
    {
        fprintf (stderr, "Error: failed to allocate ElfBuilder\n");
        exit(1);
    }

    builder->text_section = create_code_buffer (4096);
    builder->data_section = create_code_buffer (4096);
    builder->entry_point = 0;

    return builder;
}

void destroy_elf_builder (struct ElfBuilder* builder)
{
    if (builder)
    {
        destroy_code_buffer (builder->text_section);
        destroy_code_buffer (builder->data_section);
        free (builder);
    }
}

void set_entry_point (struct ElfBuilder* builder, uint64_t offset)
{
    assert (builder);
    builder->entry_point = TEXT_VADDR + offset;
}

// ========== section operations ========== //

size_t get_text_offset (struct ElfBuilder* builder)
{
    assert (builder);
    return builder->text_section->size;
}

size_t get_data_offset (struct ElfBuilder* builder)
{
    assert (builder);
    return builder->data_section->size;
}

void add_data_bytes (struct ElfBuilder* builder, const uint8_t* bytes, size_t count)
{
    assert (builder);
    assert (bytes);

    for (size_t i = 0; i < count; i++)
        emit_byte (builder->data_section, bytes[i]);
}

void add_data_qword (struct ElfBuilder* builder, uint64_t value)
{
    assert (builder);
    emit_qword (builder->data_section, value);
}

void align_data_section (struct ElfBuilder* builder, size_t alignment)
{
    assert (builder);

    size_t current = builder->data_section->size;
    size_t padding = (alignment - (current % alignment)) % alignment;

    for (size_t i = 0; i < padding; i++)
        emit_byte (builder->data_section, 0);
}

struct CodeBuffer* get_text_buffer (struct ElfBuilder* builder)
{
    assert (builder);
    return builder->text_section;
}

uint64_t get_data_absolute_addr (struct ElfBuilder* builder, size_t offset)
{
    assert (builder);
    (void) builder;  // unused but kept for API consistency
    return DATA_VADDR + offset;
}

size_t get_header_size (void)
{
    return sizeof (Elf64_Ehdr) + 2 * sizeof (Elf64_Phdr);
}

// ========== elf header creation ========== //

static void create_elf_header (Elf64_Ehdr* header, uint64_t entry, size_t phnum)
{
    assert (header);
    memset (header, 0, sizeof (Elf64_Ehdr));

    // magic number
    header->e_ident[EI_MAG0] = ELFMAG0;          // 0x7F
    header->e_ident[EI_MAG1] = ELFMAG1;          // 'E'
    header->e_ident[EI_MAG2] = ELFMAG2;          // 'L'
    header->e_ident[EI_MAG3] = ELFMAG3;          // 'F'
    header->e_ident[EI_CLASS] = ELFCLASS64;      // 64-bit
    header->e_ident[EI_DATA] = ELFDATA2LSB;      // little-endian
    header->e_ident[EI_VERSION] = EV_CURRENT;    // current version
    header->e_ident[EI_OSABI] = ELFOSABI_SYSV;   // System V ABI
    header->e_ident[EI_ABIVERSION] = 0;          // ABI version

    // elf header fields
    header->e_type = ET_EXEC;                    // executable file
    header->e_machine = EM_X86_64;               // x86-64 architecture
    header->e_version = EV_CURRENT;              // version
    header->e_entry = entry;                     // entry point
    header->e_phoff = sizeof (Elf64_Ehdr);       // program header table offset
    header->e_shoff = 0;                         // no section header table
    header->e_flags = 0;                         // no flags
    header->e_ehsize = sizeof (Elf64_Ehdr);      // ELF header size
    header->e_phentsize = sizeof (Elf64_Phdr);   // program header entry size
    header->e_phnum = (Elf64_Half)phnum;         // number of program headers
    header->e_shentsize = 0;                     // no section headers
    header->e_shnum = 0;                         // no sections
    header->e_shstrndx = 0;                      // no string table
}

// ========== program header creation ========== //

static void create_program_header (Elf64_Phdr* phdr, uint32_t type, uint32_t flags,
                                   uint64_t offset, uint64_t vaddr, uint64_t filesz)
{
    assert (phdr);
    memset (phdr, 0, sizeof (Elf64_Phdr));

    phdr->p_type = type;        // segment type
    phdr->p_flags = flags;      // segment flags (R/W/X)
    phdr->p_offset = offset;    // offset in file
    phdr->p_vaddr = vaddr;      // virtual address
    phdr->p_paddr = vaddr;      // physical address (same as virtual)
    phdr->p_filesz = filesz;    // size in file
    phdr->p_memsz = filesz;     // size in memory
    phdr->p_align = 0x1000;     // 4KB alignment
}

// ========== elf file writing ========== //

int write_elf_executable (struct ElfBuilder* builder, const char* filename)
{
    assert (builder);
    assert (filename);

    FILE* file = OpenFile (filename, "wb");
    if (file == NULL)
    {
        fprintf (stderr, "Error: cannot open file '%s' for writing\n", filename);
        return 1;
    }

    // calculate offsets
    size_t elf_header_size = sizeof (Elf64_Ehdr);
    size_t program_headers_size = 2 * sizeof (Elf64_Phdr);  // 2 PT_LOAD segments

    size_t text_offset = elf_header_size + program_headers_size;
    size_t text_size = builder->text_section->size;

    // align .data section to page boundary
    size_t data_offset = ((text_offset + text_size) + 0xFFFUL) & ~0xFFFUL;
    size_t data_size = builder->data_section->size;

    // create ELF header
    Elf64_Ehdr elf_header = {};
    create_elf_header (&elf_header, builder->entry_point, 2);

    // create program headers
    // map entire file from offset 0 to include headers
    Elf64_Phdr text_phdr = {};
    create_program_header (&text_phdr, PT_LOAD, PF_R | PF_X,        // read + execute
                          0, TEXT_VADDR, text_offset + text_size);  // map from file start

    Elf64_Phdr data_phdr = {};
    create_program_header (&data_phdr, PT_LOAD, PF_R | PF_W,  // read + write
                          data_offset, DATA_VADDR, data_size);

    // write ELF header
    if (fwrite (&elf_header, sizeof (Elf64_Ehdr), 1, file) != 1)
    {
        fprintf (stderr, "Error: failed to write ELF header\n");
        CloseFile (file);
        return 1;
    }

    // write program headers
    if (fwrite (&text_phdr, sizeof (Elf64_Phdr), 1, file) != 1 ||
        fwrite (&data_phdr, sizeof (Elf64_Phdr), 1, file) != 1)
    {
        fprintf (stderr, "Error: failed to write program headers\n");
        CloseFile (file);
        return 1;
    }

    // write .text section
    if (fwrite (builder->text_section->data, 1, text_size, file) != text_size)
    {
        fprintf (stderr, "Error: failed to write .text section\n");
        CloseFile (file);
        return 1;
    }

    // aligning for data_offset
    size_t current_pos = elf_header_size + program_headers_size + text_size;
    while (current_pos < data_offset)
    {
        fputc (0, file);
        current_pos++;
    }

    // write .data section
    if (data_size > 0)
    {
        if (fwrite (builder->data_section->data, 1, data_size, file) != data_size)
        {
            fprintf (stderr, "Error: failed to write .data section\n");
            CloseFile (file);
            return 1;
        }
    }

    CloseFile (file);

    fprintf (stderr, "Successfully created ELF executable: %s\n", filename);
    fprintf (stderr, "  .text: %zu bytes at 0x%x\n", text_size, TEXT_VADDR);
    fprintf (stderr, "  .data: %zu bytes at 0x%x\n", data_size, DATA_VADDR);
    fprintf (stderr, "  entry: 0x%lx\n", (unsigned long)builder->entry_point);

    return 0;
}
