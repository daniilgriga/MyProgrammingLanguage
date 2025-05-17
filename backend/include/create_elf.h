#pragma once

#include "elf.h"

inline void filing_header (Elf64_Ehdr* header)
{
    // filling e_ident //
    header->e_ident[EI_MAG0] = ELFMAG0;          // 0x7F
    header->e_ident[EI_MAG1] = ELFMAG1;          // 'E'
    header->e_ident[EI_MAG2] = ELFMAG2;          // 'L'
    header->e_ident[EI_MAG3] = ELFMAG3;          // 'F'
    header->e_ident[EI_CLASS] = ELFCLASS64;      // 64-bit
    header->e_ident[EI_DATA] = ELFDATA2LSB;      // little-endian
    header->e_ident[EI_VERSION] = EV_CURRENT;    // current version
    header->e_ident[EI_OSABI] = ELFOSABI_SYSV;   // System V ABI
    header->e_ident[EI_ABIVERSION] = 0;          // ABI version
    // =============== //
    header->e_type = ET_EXEC;                    // executable file
    header->e_machine = EM_X86_64;               // architecture x86_64
    header->e_version = EV_CURRENT;              // version ELF
    header->e_entry = 0;                         // no entry point
    header->e_phoff = 0;                         // no program header table's
    header->e_shoff = 0;                         // no section header table's
    header->e_flags = 0;                         // no flags
    header->e_ehsize = sizeof(Elf64_Ehdr);       // ELF header's size in bytes
    header->e_phentsize = 0;                     // program record size (no table)
    header->e_phnum = 0;                         // number of programs (no)
    header->e_shentsize = 0;                     // section record size (no sections)
    header->e_shnum = 0;                         // number of sections (no)
    header->e_shstrndx = 0;                      // string section index (no)
}
