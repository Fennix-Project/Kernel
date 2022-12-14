#include <cpu.hpp>

#include <memory.hpp>
#include <convert.h>
#include <debug.h>
#include <smp.hpp>

#include "../kernel.h"

namespace CPU
{
	static bool SSEEnabled = false;

	char *Vendor()
	{
		static char Vendor[13];
#if defined(__amd64__)
		uint32_t eax, ebx, ecx, edx;
		x64::cpuid(0x0, &eax, &ebx, &ecx, &edx);
		memcpy_unsafe(Vendor + 0, &ebx, 4);
		memcpy_unsafe(Vendor + 4, &edx, 4);
		memcpy_unsafe(Vendor + 8, &ecx, 4);
#elif defined(__i386__)
		uint32_t eax, ebx, ecx, edx;
		x32::cpuid(0x0, &eax, &ebx, &ecx, &edx);
		memcpy_unsafe(Vendor + 0, &ebx, 4);
		memcpy_unsafe(Vendor + 4, &edx, 4);
		memcpy_unsafe(Vendor + 8, &ecx, 4);
#elif defined(__aarch64__)
		asmv("mrs %0, MIDR_EL1"
			 : "=r"(Vendor[0]));
#endif
		return Vendor;
	}

	char *Name()
	{
		static char Name[49];
#if defined(__amd64__)
		uint32_t eax, ebx, ecx, edx;
		x64::cpuid(0x80000002, &eax, &ebx, &ecx, &edx);
		memcpy_unsafe(Name + 0, &eax, 4);
		memcpy_unsafe(Name + 4, &ebx, 4);
		memcpy_unsafe(Name + 8, &ecx, 4);
		memcpy_unsafe(Name + 12, &edx, 4);
		x64::cpuid(0x80000003, &eax, &ebx, &ecx, &edx);
		memcpy_unsafe(Name + 16, &eax, 4);
		memcpy_unsafe(Name + 20, &ebx, 4);
		memcpy_unsafe(Name + 24, &ecx, 4);
		memcpy_unsafe(Name + 28, &edx, 4);
		x64::cpuid(0x80000004, &eax, &ebx, &ecx, &edx);
		memcpy_unsafe(Name + 32, &eax, 4);
		memcpy_unsafe(Name + 36, &ebx, 4);
		memcpy_unsafe(Name + 40, &ecx, 4);
		memcpy_unsafe(Name + 44, &edx, 4);
#elif defined(__i386__)
		uint32_t eax, ebx, ecx, edx;
		x32::cpuid(0x80000002, &eax, &ebx, &ecx, &edx);
		memcpy_unsafe(Name + 0, &eax, 4);
		memcpy_unsafe(Name + 4, &ebx, 4);
		memcpy_unsafe(Name + 8, &ecx, 4);
		memcpy_unsafe(Name + 12, &edx, 4);
		x32::cpuid(0x80000003, &eax, &ebx, &ecx, &edx);
		memcpy_unsafe(Name + 16, &eax, 4);
		memcpy_unsafe(Name + 20, &ebx, 4);
		memcpy_unsafe(Name + 24, &ecx, 4);
		memcpy_unsafe(Name + 28, &edx, 4);
		x32::cpuid(0x80000004, &eax, &ebx, &ecx, &edx);
		memcpy_unsafe(Name + 32, &eax, 4);
		memcpy_unsafe(Name + 36, &ebx, 4);
		memcpy_unsafe(Name + 40, &ecx, 4);
		memcpy_unsafe(Name + 44, &edx, 4);
#elif defined(__aarch64__)
		asmv("mrs %0, MIDR_EL1"
			 : "=r"(Name[0]));
#endif
		return Name;
	}

	char *Hypervisor()
	{
		static char Hypervisor[13];
#if defined(__amd64__)
		uint32_t eax, ebx, ecx, edx;
		x64::cpuid(0x40000000, &eax, &ebx, &ecx, &edx);
		memcpy_unsafe(Hypervisor + 0, &ebx, 4);
		memcpy_unsafe(Hypervisor + 4, &ecx, 4);
		memcpy_unsafe(Hypervisor + 8, &edx, 4);
#elif defined(__i386__)
		uint32_t eax, ebx, ecx, edx;
		x64::cpuid(0x40000000, &eax, &ebx, &ecx, &edx);
		memcpy_unsafe(Hypervisor + 0, &ebx, 4);
		memcpy_unsafe(Hypervisor + 4, &ecx, 4);
		memcpy_unsafe(Hypervisor + 8, &edx, 4);
#elif defined(__aarch64__)
		asmv("mrs %0, MIDR_EL1"
			 : "=r"(Hypervisor[0]));
#endif
		return Hypervisor;
	}

	bool Interrupts(InterruptsType Type)
	{
		switch (Type)
		{
		case Check:
		{
			uintptr_t Flags;
#if defined(__amd64__)
			asmv("pushfq");
			asmv("popq %0"
				 : "=r"(Flags));
			return Flags & (1 << 9);
#elif defined(__i386__)
			asmv("pushfl");
			asmv("popl %0"
				 : "=r"(Flags));
			return Flags & (1 << 9);
#elif defined(__aarch64__)
			asmv("mrs %0, daif"
				 : "=r"(Flags));
			return !(Flags & (1 << 2));
#endif
		}
		case Enable:
		{
#if defined(__amd64__) || defined(__i386__)
			asmv("sti");
#elif defined(__aarch64__)
			asmv("msr daifclr, #2");
#endif
			return true;
		}
		case Disable:
		{
#if defined(__amd64__) || defined(__i386__)
			asmv("cli");
#elif defined(__aarch64__)
			asmv("msr daifset, #2");
#endif
			return true;
		}
		}
		return false;
	}

	void *PageTable(void *PT)
	{
#if defined(__amd64__)
		if (PT)
			asmv("movq %0, %%cr3"
				 :
				 : "r"(PT));
		else
			asmv("movq %%cr3, %0"
				 : "=r"(PT));
#elif defined(__i386__)
		if (PT)
			asmv("movl %0, %%cr3"
				 :
				 : "r"(PT));
		else
			asmv("movl %%cr3, %0"
				 : "=r"(PT));
#elif defined(__aarch64__)
		if (PT)
			asmv("msr ttbr0_el1, %0"
				 :
				 : "r"(PT));
		else
			asmv("mrs %0, ttbr0_el1"
				 : "=r"(PT));
#endif
		return PT;
	}

	void InitializeFeatures(long Core)
	{
		bool PGESupport = false;
		bool SSESupport = false;
#if defined(__amd64__)
		static int BSP = 0;
		x64::CR0 cr0 = x64::readcr0();
		x64::CR4 cr4 = x64::readcr4();

		if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
		{
#if defined(__amd64__)
			CPU::x64::AMD::CPUID0x1 cpuid1amd;
#elif defined(__i386__)
			CPU::x32::AMD::CPUID0x1 cpuid1amd;
#endif
#if defined(__amd64__) || defined(__i386__)
			asmv("cpuid"
				 : "=a"(cpuid1amd.EAX.raw), "=b"(cpuid1amd.EBX.raw), "=c"(cpuid1amd.ECX.raw), "=d"(cpuid1amd.EDX.raw)
				 : "a"(0x1));
#endif
			if (cpuid1amd.EDX.PGE)
				PGESupport = true;
			if (cpuid1amd.EDX.SSE)
				SSESupport = true;
		}
		else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
		{
#if defined(__amd64__)
			CPU::x64::Intel::CPUID0x1 cpuid1intel;
#elif defined(__i386__)
			CPU::x32::Intel::CPUID0x1 cpuid1intel;
#endif
#if defined(__amd64__) || defined(__i386__)
			asmv("cpuid"
				 : "=a"(cpuid1intel.EAX.raw), "=b"(cpuid1intel.EBX.raw), "=c"(cpuid1intel.ECX.raw), "=d"(cpuid1intel.EDX.raw)
				 : "a"(0x1));
#endif
			if (cpuid1intel.EDX.PGE)
				PGESupport = true;
			if (cpuid1intel.EDX.SSE)
				SSESupport = true;
		}

		if (PGESupport)
		{
			debug("Enabling global pages support...");
			if (!BSP)
				KPrint("Global Pages is supported.");
			cr4.PGE = 1;
		}

		bool SSEEnableAfter = false;

		/* Not sure if my code is not working properly or something else is the issue. */
		if ((strcmp(Hypervisor(), x86_CPUID_VENDOR_TCG) != 0 &&
			 strcmp(Hypervisor(), x86_CPUID_VENDOR_VIRTUALBOX) != 0) &&
			SSESupport)
		{
			debug("Enabling SSE support...");
			if (!BSP)
				KPrint("SSE is supported.");
			cr0.EM = 0;
			cr0.MP = 1;
			cr4.OSFXSR = 1;
			cr4.OSXMMEXCPT = 1;

			CPUData *CoreData = GetCPU(Core);
			CoreData->Data.FPU = (CPU::x64::FXState *)KernelAllocator.RequestPages(TO_PAGES(sizeof(CPU::x64::FXState)));
			memset(CoreData->Data.FPU, 0, FROM_PAGES(TO_PAGES(sizeof(CPU::x64::FXState))));
			CoreData->Data.FPU->mxcsr = 0b0001111110000000;
			CoreData->Data.FPU->mxcsrmask = 0b1111111110111111;
			CoreData->Data.FPU->fcw = 0b0000001100111111;
			CPU::x64::fxrstor(CoreData->Data.FPU);

			SSEEnableAfter = true;
		}

		if (!BSP)
			KPrint("Enabling CPU cache.");

		cr0.NW = 0;
		cr0.CD = 0;
		cr0.WP = 1;

		x64::writecr0(cr0);

		// FIXME: I don't think this is reporting correctly. This has to be fixed asap.
		debug("Enabling UMIP, SMEP & SMAP support...");
		uint32_t eax, ebx, ecx, edx;
		x64::cpuid(0x1, &eax, &ebx, &ecx, &edx);
		if (edx & (1 << 2)) // https://en.wikipedia.org/wiki/Control_register
		{
			if (!BSP)
				KPrint("UMIP is supported.");
			debug("UMIP is supported.");
			// cr4.UMIP = 1;
		}
		if (edx & (1 << 7)) // https://en.wikipedia.org/wiki/Control_register#SMEP
							// https://web.archive.org/web/20160312223150/http://ncsi.com/nsatc11/presentations/wednesday/emerging_technologies/fischer.pdf
		{
			if (!BSP)
				KPrint("SMEP is supported.");
			debug("SMEP is supported.");
			// cr4.SMEP = 1;
		}
		if (edx & (1 << 20)) // https://en.wikipedia.org/wiki/Supervisor_Mode_Access_Prevention
		{
			if (!BSP)
				KPrint("SMAP is supported.");
			debug("SMAP is supported.");
			// cr4.SMAP = 1;
		}
		if (strcmp(Hypervisor(), x86_CPUID_VENDOR_VIRTUALBOX) != 0 &&
			strcmp(Hypervisor(), x86_CPUID_VENDOR_TCG) != 0)
		{
			debug("Writing CR4...");
			x64::writecr4(cr4);
			debug("Wrote CR4.");
		}
		else
		{
			if (!BSP)
			{
				if (strcmp(Hypervisor(), x86_CPUID_VENDOR_VIRTUALBOX) == 0)
					KPrint("VirtualBox detected. Not using UMIP, SMEP & SMAP");
				else if (strcmp(Hypervisor(), x86_CPUID_VENDOR_TCG) == 0)
					KPrint("QEMU (TCG) detected. Not using UMIP, SMEP & SMAP");
			}
		}
		debug("Enabling PAT support...");
		x64::wrmsr(x64::MSR_CR_PAT, 0x6 | (0x0 << 8) | (0x1 << 16));
		if (!BSP++)
			trace("Features for BSP initialized.");
		if (SSEEnableAfter)
			SSEEnabled = true;
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
	}

	uintptr_t Counter()
	{
		// TODO: Get the counter from the x2APIC or any other timer that is available. (TSC is not available on all CPUs)
		uintptr_t Counter;
#if defined(__amd64__)
		asmv("rdtsc"
			 : "=A"(Counter));
#elif defined(__i386__)
		asmv("rdtsc"
			 : "=A"(Counter));
#elif defined(__aarch64__)
		asmv("mrs %0, cntvct_el0"
			 : "=r"(Counter));
#endif
		return Counter;
	}

	x86SIMDType CheckSIMD()
	{
		if (unlikely(!SSEEnabled))
			return SIMD_NONE;

		static x86SIMDType SIMDType = SIMD_NONE;

		if (likely(SIMDType != SIMD_NONE))
			return SIMDType;

		if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
		{
#if defined(__amd64__)
			CPU::x64::AMD::CPUID0x1 cpuid1amd;
#elif defined(__i386__)
			CPU::x32::AMD::CPUID0x1 cpuid1amd;
#endif
#if defined(__amd64__) || defined(__i386__)
			asmv("cpuid"
				 : "=a"(cpuid1amd.EAX.raw), "=b"(cpuid1amd.EBX.raw), "=c"(cpuid1amd.ECX.raw), "=d"(cpuid1amd.EDX.raw)
				 : "a"(0x1));
#endif
			if (cpuid1amd.ECX.SSE4_2)
				SIMDType = SIMD_SSE42;
			else if (cpuid1amd.ECX.SSE4_1)
				SIMDType = SIMD_SSE41;
			else if (cpuid1amd.ECX.SSE3)
				SIMDType = SIMD_SSE3;
			else if (cpuid1amd.EDX.SSE2)
				SIMDType = SIMD_SSE2;
			else if (cpuid1amd.EDX.SSE)
				SIMDType = SIMD_SSE;

#ifdef DEBUG
			if (cpuid1amd.ECX.SSE4_2)
				debug("SSE4.2 is supported.");
			if (cpuid1amd.ECX.SSE4_1)
				debug("SSE4.1 is supported.");
			if (cpuid1amd.ECX.SSE3)
				debug("SSE3 is supported.");
			if (cpuid1amd.EDX.SSE2)
				debug("SSE2 is supported.");
			if (cpuid1amd.EDX.SSE)
				debug("SSE is supported.");
#endif

			return SIMDType;
		}
		else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
		{
#if defined(__amd64__)
			CPU::x64::Intel::CPUID0x1 cpuid1intel;
#elif defined(__i386__)
			CPU::x32::Intel::CPUID0x1 cpuid1intel;
#endif
#if defined(__amd64__) || defined(__i386__)
			asmv("cpuid"
				 : "=a"(cpuid1intel.EAX.raw), "=b"(cpuid1intel.EBX.raw), "=c"(cpuid1intel.ECX.raw), "=d"(cpuid1intel.EDX.raw)
				 : "a"(0x1));
#endif
			if (cpuid1intel.ECX.SSE4_2)
				SIMDType = SIMD_SSE42;
			else if (cpuid1intel.ECX.SSE4_1)
				SIMDType = SIMD_SSE41;
			else if (cpuid1intel.ECX.SSE3)
				SIMDType = SIMD_SSE3;
			else if (cpuid1intel.EDX.SSE2)
				SIMDType = SIMD_SSE2;
			else if (cpuid1intel.EDX.SSE)
				SIMDType = SIMD_SSE;

#ifdef DEBUG
			if (cpuid1intel.ECX.SSE4_2)
				debug("SSE4.2 is supported.");
			if (cpuid1intel.ECX.SSE4_1)
				debug("SSE4.1 is supported.");
			if (cpuid1intel.ECX.SSE3)
				debug("SSE3 is supported.");
			if (cpuid1intel.EDX.SSE2)
				debug("SSE2 is supported.");
			if (cpuid1intel.EDX.SSE)
				debug("SSE is supported.");
#endif

			return SIMDType;
		}

		return SIMD_NONE;
	}

	bool CheckSIMD(x86SIMDType Type)
	{
		if (unlikely(!SSEEnabled))
			return false;

		if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
		{
#if defined(__amd64__)
			CPU::x64::AMD::CPUID0x1 cpuid1amd;
#elif defined(__i386__)
			CPU::x32::AMD::CPUID0x1 cpuid1amd;
#endif
#if defined(__amd64__) || defined(__i386__)
			asmv("cpuid"
				 : "=a"(cpuid1amd.EAX.raw), "=b"(cpuid1amd.EBX.raw), "=c"(cpuid1amd.ECX.raw), "=d"(cpuid1amd.EDX.raw)
				 : "a"(0x1));
#endif
			if (Type == SIMD_SSE42)
				return cpuid1amd.ECX.SSE4_2;
			else if (Type == SIMD_SSE41)
				return cpuid1amd.ECX.SSE4_1;
			else if (Type == SIMD_SSE3)
				return cpuid1amd.ECX.SSE3;
			else if (Type == SIMD_SSE2)
				return cpuid1amd.EDX.SSE2;
			else if (Type == SIMD_SSE)
				return cpuid1amd.EDX.SSE;
		}
		else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
		{
#if defined(__amd64__)
			CPU::x64::Intel::CPUID0x1 cpuid1intel;
#elif defined(__i386__)
			CPU::x32::Intel::CPUID0x1 cpuid1intel;
#endif
#if defined(__amd64__) || defined(__i386__)
			asmv("cpuid"
				 : "=a"(cpuid1intel.EAX.raw), "=b"(cpuid1intel.EBX.raw), "=c"(cpuid1intel.ECX.raw), "=d"(cpuid1intel.EDX.raw)
				 : "a"(0x1));
#endif
			if (Type == SIMD_SSE42)
				return cpuid1intel.ECX.SSE4_2;
			else if (Type == SIMD_SSE41)
				return cpuid1intel.ECX.SSE4_1;
			else if (Type == SIMD_SSE3)
				return cpuid1intel.ECX.SSE3;
			else if (Type == SIMD_SSE2)
				return cpuid1intel.EDX.SSE2;
			else if (Type == SIMD_SSE)
				return cpuid1intel.EDX.SSE;
		}

		return false;
	}
}
