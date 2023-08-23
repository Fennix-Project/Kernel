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

#include <smp.hpp>

#include <memory.hpp>
#include <ints.hpp>
#include <assert.h>
#include <cpu.hpp>
#include <atomic>

#include <smp.hpp>

#include <memory.hpp>
#include <ints.hpp>
#include <assert.h>
#include <cpu.hpp>
#include <atomic>

#include "../../../kernel.h"
#include "../acpi.hpp"
#include "apic.hpp"

enum SMPTrampolineAddress
{
	PAGE_TABLE = 0x500,
	START_ADDR = 0x520,
	STACK = 0x570,
	GDT = 0x580,
	IDT = 0x590,
	CORE = 0x600,
	TRAMPOLINE_START = 0x2000
};

std::atomic_bool CPUEnabled = false;

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
static __aligned(0x1000) CPUData CPUs[MAX_CPU] = {0};

SafeFunction CPUData *GetCPU(long id) { return &CPUs[id]; }

SafeFunction CPUData *GetCurrentCPU()
{
	uint64_t ret = 0;
	if (!(&CPUs[ret])->IsActive)
	{
		// error("CPU %d is not active!", ret); FIXME
		return &CPUs[0];
	}
	assert((&CPUs[ret])->Checksum == CPU_DATA_CHECKSUM);
	return &CPUs[ret];
}

namespace SMP
{
	int CPUCores = 0;

	void Initialize(void *_madt)
	{
		ACPI::MADT *madt = (ACPI::MADT *)_madt;

		int Cores = madt->CPUCores + 1;

		if (Config.Cores > madt->CPUCores + 1)
			KPrint("More cores requested than available. Using %d cores", madt->CPUCores + 1);
		else if (Config.Cores != 0)
			Cores = Config.Cores;

		CPUCores = Cores;

		fixme("SMP::Initialize() is not implemented!");
	}
}
