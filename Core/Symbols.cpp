/*
   This file is part of Fennix Kernel.

   Fennix Kernel is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of
   the License, or (at your option) any later version.

   Fennix Kernel is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#include <symbols.hpp>
#include <memory.hpp>
#include <convert.h>
#include <debug.h>
#include <elf.h>

// #pragma GCC diagnostic ignored "-Wignored-qualifiers"

namespace SymbolResolver
{
	const NIF char *Symbols::GetSymbolFromAddress(uintptr_t Address)
	{
		SymbolTable Result{};
		foreach (auto tbl in this->SymTable)
		{
			if (tbl.Address <= Address &&
				tbl.Address > Result.Address)
				Result = tbl;
		}
		// debug("Address: %#lx, Function: %s",
		// 	  Address, Result.FunctionName);
		return Result.FunctionName;
	}

	void Symbols::AddSymbol(uintptr_t Address, const char *Name)
	{
		SymbolTable tbl{};
		tbl.Address = Address;
		tbl.FunctionName = (char *)Name;
		this->SymTable.push_back(tbl);
		this->SymbolTableExists = true;
	}

	__no_sanitize("alignment") void Symbols::AddSymbolInfoFromGRUB(uint64_t Num,
																   uint64_t EntSize,
																   __unused uint64_t Shndx,
																   uintptr_t Sections)
	{
		char *sections = reinterpret_cast<char *>(Sections);

		Elf_Sym *Symbols = nullptr;
		uint8_t *StringAddress = nullptr;

#if defined(a64) || defined(aa64)
		Elf64_Xword SymbolSize = 0;
		// Elf64_Xword StringSize = 0;
#elif defined(a32)
		Elf32_Word SymbolSize = 0;
		// Elf32_Word StringSize = 0;
#endif
		int64_t TotalEntries = 0;

		for (size_t i = 0; i < Num; ++i)
		{
			Elf_Shdr *sym = (Elf_Shdr *)&sections[EntSize * i];
			Elf_Shdr *str = (Elf_Shdr *)&sections[EntSize * sym->sh_link];

			if (sym->sh_type == SHT_SYMTAB &&
				str->sh_type == SHT_STRTAB)
			{
				Symbols = (Elf_Sym *)sym->sh_addr;
				StringAddress = (uint8_t *)str->sh_addr;
				SymbolSize = (int)sym->sh_size;
				// StringSize = (int)str->sh_size;
				// TotalEntries = Section.sh_size / sizeof(Elf64_Sym)
				TotalEntries = sym->sh_size / sym->sh_entsize;
				debug("Symbol table found, %d entries",
					  SymbolSize / sym->sh_entsize);
				UNUSED(SymbolSize);
				break;
			}
		}

		if (Symbols != nullptr && StringAddress != nullptr)
		{
			int64_t Index, MinimumIndex;
			for (int64_t i = 0; i < TotalEntries - 1; i++)
			{
				MinimumIndex = i;
				for (Index = i + 1; Index < TotalEntries; Index++)
					if (Symbols[Index].st_value < Symbols[MinimumIndex].st_value)
						MinimumIndex = Index;
				Elf_Sym tmp = Symbols[MinimumIndex];
				Symbols[MinimumIndex] = Symbols[i];
				Symbols[i] = tmp;
			}

			while (Symbols[0].st_value == 0)
			{
				if (TotalEntries <= 0)
					break;
				Symbols++;
				TotalEntries--;
			}

			if (TotalEntries <= 0)
			{
				error("Symbol table is empty");
				return;
			}

			debug("Symbol table loaded, %d entries (%ld KiB)",
				  TotalEntries, TO_KiB(TotalEntries * sizeof(SymbolTable)));
			Elf_Sym *sym = nullptr;
			const char *name = nullptr;
			for (int64_t i = 0, g = TotalEntries; i < g; i++)
			{
				sym = &Symbols[i];
				name = (const char *)&StringAddress[Symbols[i].st_name];
				if (strlen(name) == 0)
					continue;
				SymbolTable tbl{};
				tbl.Address = sym->st_value;
				tbl.FunctionName = (char *)name;
				this->SymTable.push_back(tbl);
				this->SymbolTableExists = true;

				// debug("Symbol %d: %#lx %s(%#lx)",
				// 	  i, tbl.Address,
				// 	  tbl.FunctionName,
				// 	  name);
			}
		}
	}

	void Symbols::AppendSymbols(uintptr_t ImageAddress, uintptr_t BaseAddress)
	{
		/* FIXME: Get only the required headers instead of the whole file */

		if (ImageAddress == 0 || Memory::Virtual().Check((void *)ImageAddress) == false)
		{
			error("Invalid image address %#lx", ImageAddress);
			return;
		}
		debug("Solving symbols for address: %#llx", ImageAddress);

#if defined(a64) || defined(aa64)
		Elf64_Ehdr *Header = (Elf64_Ehdr *)ImageAddress;
#elif defined(a32)
		Elf32_Ehdr *Header = (Elf32_Ehdr *)ImageAddress;
#endif
		if (Header->e_ident[0] != 0x7F &&
			Header->e_ident[1] != 'E' &&
			Header->e_ident[2] != 'L' &&
			Header->e_ident[3] != 'F')
		{
			error("Invalid ELF header");
			return;
		}

		Elf_Shdr *ElfSections = (Elf_Shdr *)(ImageAddress + Header->e_shoff);
		Elf_Sym *ElfSymbols = nullptr;
		char *strtab = nullptr;
		int64_t TotalEntries = 0;

		for (uint16_t i = 0; i < Header->e_shnum; i++)
		{
			switch (ElfSections[i].sh_type)
			{
			case SHT_SYMTAB:
				ElfSymbols = (Elf_Sym *)(ImageAddress + ElfSections[i].sh_offset);
				TotalEntries = ElfSections[i].sh_size / sizeof(Elf_Sym);
				debug("Symbol table found, %d entries", TotalEntries);
				break;
			case SHT_STRTAB:
				if (Header->e_shstrndx == i)
				{
					debug("String table found, %d entries", ElfSections[i].sh_size);
				}
				else
				{
					strtab = (char *)(ImageAddress + ElfSections[i].sh_offset);
					debug("String table found, %d entries", ElfSections[i].sh_size);
				}
				break;
			default:
				break;
			}
		}

		if (ElfSymbols != nullptr && strtab != nullptr)
		{
			int64_t Index, MinimumIndex;
			for (int64_t i = 0; i < TotalEntries - 1; i++)
			{
				MinimumIndex = i;
				for (Index = i + 1; Index < TotalEntries; Index++)
					if (ElfSymbols[Index].st_value < ElfSymbols[MinimumIndex].st_value)
						MinimumIndex = Index;
				Elf_Sym tmp = ElfSymbols[MinimumIndex];
				ElfSymbols[MinimumIndex] = ElfSymbols[i];
				ElfSymbols[i] = tmp;
			}

			while (ElfSymbols[0].st_value == 0)
			{
				ElfSymbols++;
				TotalEntries--;
			}

			trace("Symbol table loaded, %d entries (%ld KiB)",
				  TotalEntries, TO_KiB(TotalEntries * sizeof(SymbolTable)));

			/* TODO: maybe a checker for duplicated addresses? */
			Elf_Sym *sym = nullptr;
			const char *name = nullptr;
			for (int64_t i = 0, g = TotalEntries; i < g; i++)
			{
				sym = &ElfSymbols[i];
				name = &strtab[ElfSymbols[i].st_name];
				SymbolTable tbl{};
				tbl.Address = sym->st_value + BaseAddress;
				tbl.FunctionName = (char *)name;
				this->SymTable.push_back(tbl);
				this->SymbolTableExists = true;

				// debug("Symbol %d: %#llx %s", i,
				// 	  this->SymTable[i].Address,
				// 	  this->SymTable[i].FunctionName);
			}
		}
	}

	Symbols::Symbols(uintptr_t ImageAddress)
	{
		this->Image = (void *)ImageAddress;
		this->AppendSymbols(ImageAddress);
	}

	Symbols::~Symbols() {}
}
