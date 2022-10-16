#include "crashhandler.hpp"

#include <display.hpp>
#include <printf.h>
#include <debug.h>
#include <cpu.hpp>

#if defined(__amd64__)
#include "../Architecture/amd64/cpu/gdt.hpp"
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif

#include "../kernel.h"

#if defined(__amd64__) || defined(__i386__)
static const char *PagefaultDescriptions[] = {
    "Supervisory process tried to read a non-present page entry",
    "Supervisory process tried to read a page and caused a protection fault",
    "Supervisory process tried to write to a non-present page entry",
    "Supervisory process tried to write a page and caused a protection fault",
    "User process tried to read a non-present page entry",
    "User process tried to read a page and caused a protection fault",
    "User process tried to write to a non-present page entry",
    "User process tried to write a page and caused a protection fault"};
#endif

#if defined(__amd64__)
void DivideByZeroExceptionHandler(CPU::x64::TrapFrame *Frame);
void DebugExceptionHandler(CPU::x64::TrapFrame *Frame);
void NonMaskableInterruptExceptionHandler(CPU::x64::TrapFrame *Frame);
void BreakpointExceptionHandler(CPU::x64::TrapFrame *Frame);
void OverflowExceptionHandler(CPU::x64::TrapFrame *Frame);
void BoundRangeExceptionHandler(CPU::x64::TrapFrame *Frame);
void InvalidOpcodeExceptionHandler(CPU::x64::TrapFrame *Frame);
void DeviceNotAvailableExceptionHandler(CPU::x64::TrapFrame *Frame);
void DoubleFaultExceptionHandler(CPU::x64::TrapFrame *Frame);
void CoprocessorSegmentOverrunExceptionHandler(CPU::x64::TrapFrame *Frame);
void InvalidTSSExceptionHandler(CPU::x64::TrapFrame *Frame);
void SegmentNotPresentExceptionHandler(CPU::x64::TrapFrame *Frame);
void StackFaultExceptionHandler(CPU::x64::TrapFrame *Frame);
void GeneralProtectionExceptionHandler(CPU::x64::TrapFrame *Frame);
void PageFaultExceptionHandler(CPU::x64::TrapFrame *Frame);
void x87FloatingPointExceptionHandler(CPU::x64::TrapFrame *Frame);
void AlignmentCheckExceptionHandler(CPU::x64::TrapFrame *Frame);
void MachineCheckExceptionHandler(CPU::x64::TrapFrame *Frame);
void SIMDFloatingPointExceptionHandler(CPU::x64::TrapFrame *Frame);
void VirtualizationExceptionHandler(CPU::x64::TrapFrame *Frame);
void SecurityExceptionHandler(CPU::x64::TrapFrame *Frame);
void UnknownExceptionHandler(CPU::x64::TrapFrame *Frame);
void UserModeExceptionHandler(CPU::x64::TrapFrame *Frame);
#endif

namespace CrashHandler
{
    __attribute__((no_stack_protector)) void printfWrapper(char c, void *unused)
    {
        Display->Print(c, 255, true);
        UNUSED(unused);
    }

    __attribute__((no_stack_protector)) void EHPrint(const char *Format, ...)
    {
        va_list args;
        va_start(args, Format);
        vfctprintf(printfWrapper, NULL, Format, args);
        va_end(args);
    }

    __attribute__((no_stack_protector)) void Handle(void *Data)
    {
        CPU::Interrupts(CPU::Disable);

#if defined(__amd64__)
        CPU::x64::TrapFrame *Frame = (CPU::x64::TrapFrame *)Data;
        error("Exception: %#lx", Frame->int_num);

        if (Frame->cs != GDT_USER_CODE && Frame->cs != GDT_USER_DATA)
        {
            debug("Exception in kernel mode");
            Display->CreateBuffer(0, 0, 255);
        }
        else
        {
            debug("Exception in user mode");
            UserModeExceptionHandler(Frame);
            return;
        }

        debug("Reading control registers...");
        CPU::x64::CR0 cr0 = CPU::x64::readcr0();
        CPU::x64::CR2 cr2 = CPU::x64::readcr2();
        CPU::x64::CR3 cr3 = CPU::x64::readcr3();
        CPU::x64::CR4 cr4 = CPU::x64::readcr4();
        CPU::x64::CR8 cr8 = CPU::x64::readcr8();
        CPU::x64::EFER efer;
        efer.raw = CPU::x64::rdmsr(CPU::x64::MSR_EFER);

        uint64_t dr0, dr1, dr2, dr3, dr6;
        CPU::x64::DR7 dr7;

        // store debug registers
        debug("Reading debug registers...");
        asm volatile("movq %%dr0, %0"
                     : "=r"(dr0));
        asm volatile("movq %%dr1, %0"
                     : "=r"(dr1));
        asm volatile("movq %%dr2, %0"
                     : "=r"(dr2));
        asm volatile("movq %%dr3, %0"
                     : "=r"(dr3));
        asm volatile("movq %%dr6, %0"
                     : "=r"(dr6));
        asm volatile("movq %%dr7, %0"
                     : "=r"(dr7));

        switch (Frame->int_num)
        {
        case CPU::x64::DivideByZero:
        {
            DivideByZeroExceptionHandler(Frame);
            break;
        }
        case CPU::x64::Debug:
        {
            DebugExceptionHandler(Frame);
            break;
        }
        case CPU::x64::NonMaskableInterrupt:
        {
            NonMaskableInterruptExceptionHandler(Frame);
            break;
        }
        case CPU::x64::Breakpoint:
        {
            BreakpointExceptionHandler(Frame);
            break;
        }
        case CPU::x64::Overflow:
        {
            OverflowExceptionHandler(Frame);
            break;
        }
        case CPU::x64::BoundRange:
        {
            BoundRangeExceptionHandler(Frame);
            break;
        }
        case CPU::x64::InvalidOpcode:
        {
            InvalidOpcodeExceptionHandler(Frame);
            break;
        }
        case CPU::x64::DeviceNotAvailable:
        {
            DeviceNotAvailableExceptionHandler(Frame);
            break;
        }
        case CPU::x64::DoubleFault:
        {
            DoubleFaultExceptionHandler(Frame);
            break;
        }
        case CPU::x64::CoprocessorSegmentOverrun:
        {
            CoprocessorSegmentOverrunExceptionHandler(Frame);
            break;
        }
        case CPU::x64::InvalidTSS:
        {
            InvalidTSSExceptionHandler(Frame);
            break;
        }
        case CPU::x64::SegmentNotPresent:
        {
            SegmentNotPresentExceptionHandler(Frame);
            break;
        }
        case CPU::x64::StackSegmentFault:
        {
            StackFaultExceptionHandler(Frame);
            break;
        }
        case CPU::x64::GeneralProtectionFault:
        {
            GeneralProtectionExceptionHandler(Frame);
            break;
        }
        case CPU::x64::PageFault:
        {
            PageFaultExceptionHandler(Frame);
            break;
        }
        case CPU::x64::x87FloatingPoint:
        {
            x87FloatingPointExceptionHandler(Frame);
            break;
        }
        case CPU::x64::AlignmentCheck:
        {
            AlignmentCheckExceptionHandler(Frame);
            break;
        }
        case CPU::x64::MachineCheck:
        {
            MachineCheckExceptionHandler(Frame);
            break;
        }
        case CPU::x64::SIMDFloatingPoint:
        {
            SIMDFloatingPointExceptionHandler(Frame);
            break;
        }
        case CPU::x64::Virtualization:
        {
            VirtualizationExceptionHandler(Frame);
            break;
        }
        case CPU::x64::Security:
        {
            SecurityExceptionHandler(Frame);
            break;
        }
        default:
        {
            UnknownExceptionHandler(Frame);
            break;
        }
        }

        EHPrint("\e7981FCTechnical Informations on CPU %ld:\n",
                CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE));
        EHPrint("FS=%#lx  GS=%#lx  SS=%#lx  CS=%#lx  DS=%#lx\n",
                CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE), CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE),
                Frame->ss, Frame->cs, Frame->ds);
        EHPrint("R8=%#lx  R9=%#lx  R10=%#lx  R11=%#lx\n", Frame->r8, Frame->r9, Frame->r10, Frame->r11);
        EHPrint("R12=%#lx  R13=%#lx  R14=%#lx  R15=%#lx\n", Frame->r12, Frame->r13, Frame->r14, Frame->r15);
        EHPrint("RAX=%#lx  RBX=%#lx  RCX=%#lx  RDX=%#lx\n", Frame->rax, Frame->rbx, Frame->rcx, Frame->rdx);
        EHPrint("RSI=%#lx  RDI=%#lx  RBP=%#lx  RSP=%#lx\n", Frame->rsi, Frame->rdi, Frame->rbp, Frame->rsp);
        EHPrint("RIP=%#lx  RFL=%#lx  INT=%#lx  ERR=%#lx  EFER=%#lx\n", Frame->rip, Frame->rflags.raw, Frame->int_num, Frame->error_code, efer.raw);
        EHPrint("CR0=%#lx  CR2=%#lx  CR3=%#lx  CR4=%#lx  CR8=%#lx\n", cr0.raw, cr2.raw, cr3.raw, cr4.raw, cr8.raw);
        EHPrint("DR0=%#lx  DR1=%#lx  DR2=%#lx  DR3=%#lx  DR6=%#lx  DR7=%#lx\n", dr0, dr1, dr2, dr3, dr6, dr7.raw);

        EHPrint("\eFC797BCR0: PE:%s     MP:%s     EM:%s     TS:%s\n     ET:%s     NE:%s     WP:%s     AM:%s\n     NW:%s     CD:%s     PG:%s\n     R0:%#x R1:%#x R2:%#x\n",
                cr0.PE ? "True " : "False", cr0.MP ? "True " : "False", cr0.EM ? "True " : "False", cr0.TS ? "True " : "False",
                cr0.ET ? "True " : "False", cr0.NE ? "True " : "False", cr0.WP ? "True " : "False", cr0.AM ? "True " : "False",
                cr0.NW ? "True " : "False", cr0.CD ? "True " : "False", cr0.PG ? "True " : "False",
                cr0._reserved0, cr0._reserved1, cr0._reserved2);

        EHPrint("\eFCBD79CR2: PFLA: %#lx\n",
                cr2.PFLA);

        EHPrint("\e79FC84CR3: PWT:%s     PCD:%s    PDBR:%#lx\n",
                cr3.PWT ? "True " : "False", cr3.PCD ? "True " : "False", cr3.PDBR);

        EHPrint("\eBD79FCCR4: VME:%s     PVI:%s     TSD:%s      DE:%s\n     PSE:%s     PAE:%s     MCE:%s     PGE:%s\n     PCE:%s    UMIP:%s  OSFXSR:%s OSXMMEXCPT:%s\n    LA57:%s    VMXE:%s    SMXE:%s   PCIDE:%s\n OSXSAVE:%s    SMEP:%s    SMAP:%s     PKE:%s\n     R0:%#x R1:%#x R2:%#x\n",
                cr4.VME ? "True " : "False", cr4.PVI ? "True " : "False", cr4.TSD ? "True " : "False", cr4.DE ? "True " : "False",
                cr4.PSE ? "True " : "False", cr4.PAE ? "True " : "False", cr4.MCE ? "True " : "False", cr4.PGE ? "True " : "False",
                cr4.PCE ? "True " : "False", cr4.UMIP ? "True " : "False", cr4.OSFXSR ? "True " : "False", cr4.OSXMMEXCPT ? "True " : "False",
                cr4.LA57 ? "True " : "False", cr4.VMXE ? "True " : "False", cr4.SMXE ? "True " : "False", cr4.PCIDE ? "True " : "False",
                cr4.OSXSAVE ? "True " : "False", cr4.SMEP ? "True " : "False", cr4.SMAP ? "True " : "False", cr4.PKE ? "True " : "False",
                cr4._reserved0, cr4._reserved1, cr4._reserved2);

        EHPrint("\e79FCF5CR8: TPL:%d\n", cr8.TPL);

        EHPrint("\eFCFC02RFL: CF:%s     PF:%s     AF:%s     ZF:%s\n     SF:%s     TF:%s     IF:%s     DF:%s\n     OF:%s   IOPL:%s     NT:%s     RF:%s\n     VM:%s     AC:%s    VIF:%s    VIP:%s\n     ID:%s     AlwaysOne:%d\n     R0:%#x R1:%#x R2:%#x R3:%#x\n",
                Frame->rflags.CF ? "True " : "False", Frame->rflags.PF ? "True " : "False", Frame->rflags.AF ? "True " : "False", Frame->rflags.ZF ? "True " : "False",
                Frame->rflags.SF ? "True " : "False", Frame->rflags.TF ? "True " : "False", Frame->rflags.IF ? "True " : "False", Frame->rflags.DF ? "True " : "False",
                Frame->rflags.OF ? "True " : "False", Frame->rflags.IOPL ? "True " : "False", Frame->rflags.NT ? "True " : "False", Frame->rflags.RF ? "True " : "False",
                Frame->rflags.VM ? "True " : "False", Frame->rflags.AC ? "True " : "False", Frame->rflags.VIF ? "True " : "False", Frame->rflags.VIP ? "True " : "False",
                Frame->rflags.ID ? "True " : "False", Frame->rflags.always_one,
                Frame->rflags._reserved0, Frame->rflags._reserved1, Frame->rflags._reserved2, Frame->rflags._reserved3);

        EHPrint("\eA0F0F0DR7: LDR0:%s     GDR0:%s     LDR1:%s     GDR1:%s\n     LDR2:%s     GDR2:%s     LDR3:%s     GDR3:%s\n     CDR0:%s     SDR0:%s     CDR1:%s     SDR1:%s\n     CDR2:%s     SDR2:%s     CDR3:%s     SDR3:%s\n     R:%#x\n",
                dr7.LocalDR0 ? "True " : "False", dr7.GlobalDR0 ? "True " : "False", dr7.LocalDR1 ? "True " : "False", dr7.GlobalDR1 ? "True " : "False",
                dr7.LocalDR2 ? "True " : "False", dr7.GlobalDR2 ? "True " : "False", dr7.LocalDR3 ? "True " : "False", dr7.GlobalDR3 ? "True " : "False",
                dr7.ConditionsDR0 ? "True " : "False", dr7.SizeDR0 ? "True " : "False", dr7.ConditionsDR1 ? "True " : "False", dr7.SizeDR1 ? "True " : "False",
                dr7.ConditionsDR2 ? "True " : "False", dr7.SizeDR2 ? "True " : "False", dr7.ConditionsDR3 ? "True " : "False", dr7.SizeDR3 ? "True " : "False",
                dr7.Reserved);

        EHPrint("\e009FF0EFER: SCE:%s      LME:%s      LMA:%s      NXE:%s\n     SVME:%s    LMSLE:%s    FFXSR:%s      TCE:%s\n     R0:%#x R1:%#x R2:%#x\n",
                efer.SCE ? "True " : "False", efer.LME ? "True " : "False", efer.LMA ? "True " : "False", efer.NXE ? "True " : "False",
                efer.SVME ? "True " : "False", efer.LMSLE ? "True " : "False", efer.FFXSR ? "True " : "False", efer.TCE ? "True " : "False",
                efer.Reserved0, efer.Reserved1, efer.Reserved2);

        // restore debug registers
        debug("Restoring debug registers...");
        asm volatile("movq %0, %%dr0"
                     :
                     : "r"(dr0));
        asm volatile("movq %0, %%dr1"
                     :
                     : "r"(dr1));
        asm volatile("movq %0, %%dr2"
                     :
                     : "r"(dr2));
        asm volatile("movq %0, %%dr3"
                     :
                     : "r"(dr3));
        asm volatile("movq %0, %%dr6"
                     :
                     : "r"(dr6));
        asm volatile("movq %0, %%dr7"
                     :
                     : "r"(dr7));

        struct StackFrame
        {
            struct StackFrame *rbp;
            uint64_t rip;
        };

        struct StackFrame *frames = (struct StackFrame *)Frame->rbp; // (struct StackFrame *)__builtin_frame_address(0);

        debug("Stack tracing...");
        EHPrint("\e7981FC\nStack Trace:\n");

        if (!frames || !frames->rip || !frames->rbp)
        {
            EHPrint("\eFF0000\n< No stack trace available. >\n");
            Display->SetBuffer(255);
            while (1)
                CPU::Stop();
        }
        else
        {
            frames = (struct StackFrame *)Frame->rbp;
            if (Frame->rip >= 0xFFFFFFFF80000000 && Frame->rip <= (uint64_t)&_kernel_end)
                debug("%p-%s <- Exception", (void *)Frame->rip, KernelSymbolTable->GetSymbolFromAddress(Frame->rip));
            else
                debug("%p-OUTSIDE KERNEL <- Exception", (void *)Frame->rip);
            for (uint64_t frame = 0; frame < 100; ++frame)
            {
                if (!frames->rip)
                    break;
                if (frames->rip >= 0xFFFFFFFF80000000 && frames->rip <= (uint64_t)&_kernel_end)
                    debug("%p-%s", (void *)frames->rip, KernelSymbolTable->GetSymbolFromAddress(frames->rip));
                else
                    debug("%p-OUTSIDE KERNEL", (void *)frames->rip);
                frames = frames->rbp;
            }
        }

        if (!frames->rip || !frames->rbp)
        {
            EHPrint("\e2565CC%p", (void *)Frame->rip);
            EHPrint("\e7925CC-");
            EHPrint("\eAA25C%s", KernelSymbolTable->GetSymbolFromAddress(Frame->rip));
            EHPrint("\e7981FC <- Exception");
            EHPrint("\eFF0000\n< No stack trace available. >\n");
            Display->SetBuffer(255);
            while (1)
                CPU::Stop();
        }
        else
        {
            EHPrint("\e2565CC%p", (void *)Frame->rip);
            EHPrint("\e7925CC-");
            if (Frame->rip >= 0xFFFFFFFF80000000 && Frame->rip <= (uint64_t)&_kernel_end)
                EHPrint("\eAA25CC%s", KernelSymbolTable->GetSymbolFromAddress(Frame->rip));
            else
                EHPrint("Outside Kernel");
            EHPrint("\e7981FC <- Exception");
            for (uint64_t frame = 0; frame < 20; ++frame)
            {
                if (!frames->rip)
                    break;
                EHPrint("\n");
                EHPrint("\e2565CC%p", (void *)frames->rip);
                EHPrint("\e7925CC-");
                if (frames->rip >= 0xFFFFFFFF80000000 && frames->rip <= (uint64_t)&_kernel_end)
                    EHPrint("\e25CCC9%s", KernelSymbolTable->GetSymbolFromAddress(frames->rip));
                else
                    EHPrint("\eFF4CA9Outside Kernel");
                frames = frames->rbp;
            }
        }

#elif defined(__i386__)
        void *Frame = Data;
#elif defined(__aarch64__)
        void *Frame = Data;
#endif

        Display->SetBuffer(255);
        while (1)
            CPU::Stop();
    }
}

#if defined(__amd64__)
// #define staticbuffer(name) char name[] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"

void DivideByZeroExceptionHandler(CPU::x64::TrapFrame *Frame)
{
    fixme("Divide by zero exception");
}
void DebugExceptionHandler(CPU::x64::TrapFrame *Frame)
{
    CrashHandler::EHPrint("\eDD2920System crashed!\n");
    CrashHandler::EHPrint("Kernel triggered debug exception.");
}
void NonMaskableInterruptExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("NMI exception"); }
void BreakpointExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Breakpoint exception"); }
void OverflowExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Overflow exception"); }
void BoundRangeExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Bound range exception"); }
void InvalidOpcodeExceptionHandler(CPU::x64::TrapFrame *Frame)
{
    CrashHandler::EHPrint("\eDD2920System crashed!\n");
    CrashHandler::EHPrint("Kernel tried to execute an invalid opcode.");
}
void DeviceNotAvailableExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Device not available exception"); }
void DoubleFaultExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Double fault exception"); }
void CoprocessorSegmentOverrunExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Coprocessor segment overrun exception"); }
void InvalidTSSExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Invalid TSS exception"); }
void SegmentNotPresentExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Segment not present exception"); }
void StackFaultExceptionHandler(CPU::x64::TrapFrame *Frame)
{
    // staticbuffer(descbuf);
    // staticbuffer(desc_ext);
    // staticbuffer(desc_table);
    // staticbuffer(desc_idx);
    // staticbuffer(desc_tmp);
    // CPU::x64::SelectorErrorCode SelCode = {.raw = Frame->error_code};
    // switch (SelCode.Table)
    // {
    // case 0b00:
    //     memcpy(desc_tmp, "GDT", 3);
    //     break;
    // case 0b01:
    //     memcpy(desc_tmp, "IDT", 3);
    //     break;
    // case 0b10:
    //     memcpy(desc_tmp, "LDT", 3);
    //     break;
    // case 0b11:
    //     memcpy(desc_tmp, "IDT", 3);
    //     break;
    // default:
    //     memcpy(desc_tmp, "Unknown", 7);
    //     break;
    // }
    // debug("external:%d table:%d idx:%#x", SelCode.External, SelCode.Table, SelCode.Idx);
    // sprintf_(descbuf, "Stack segment fault at address %#lx", Frame->rip);
    // CrashHandler::EHPrint(descbuf);
    // sprintf_(desc_ext, "External: %d", SelCode.External);
    // CrashHandler::EHPrint(desc_ext);
    // sprintf_(desc_table, "Table: %d (%s)", SelCode.Table, desc_tmp);
    // CrashHandler::EHPrint(desc_table);
    // sprintf_(desc_idx, "%s Index: %#x", desc_tmp, SelCode.Idx);
    // CrashHandler::EHPrint(desc_idx);
    // CrashHandler::EHPrint("\eDD2920System crashed!\n");
    // CrashHandler::EHPrint("More info about the exception:\n");
}
void GeneralProtectionExceptionHandler(CPU::x64::TrapFrame *Frame)
{
    // staticbuffer(descbuf);
    // staticbuffer(desc_ext);
    // staticbuffer(desc_table);
    // staticbuffer(desc_idx);
    // staticbuffer(desc_tmp);
    // SelectorErrorCode SelCode = {.raw = ERROR_CODE};
    // switch (SelCode.Table)
    // {
    // case CPU::x64::0b00:
    //     memcpy(desc_tmp, "GDT", 3);
    //     break;
    // case CPU::x64::0b01:
    //     memcpy(desc_tmp, "IDT", 3);
    //     break;
    // case CPU::x64::0b10:
    //     memcpy(desc_tmp, "LDT", 3);
    //     break;
    // case CPU::x64::0b11:
    //     memcpy(desc_tmp, "IDT", 3);
    //     break;
    // default:
    //     memcpy(desc_tmp, "Unknown", 7);
    //     break;
    // }
    // debug("external:%d table:%d idx:%#x", SelCode.External, SelCode.Table, SelCode.Idx);
    // CurrentDisplay->SetPrintColor(0xDD2920);
    // SET_PRINT_MID((char *)"System crashed!", FHeight(6));
    // CurrentDisplay->ResetPrintColor();
    // SET_PRINT_MID((char *)"More info about the exception:", FHeight(4));
    // sprintf_(descbuf, "Kernel performed an illegal operation at address %#lx", RIP);
    // SET_PRINT_MID((char *)descbuf, FHeight(5));
    // sprintf_(desc_ext, "External: %d", SelCode.External);
    // SET_PRINT_MID((char *)desc_ext, FHeight(3));
    // sprintf_(desc_table, "Table: %d (%s)", SelCode.Table, desc_tmp);
    // SET_PRINT_MID((char *)desc_table, FHeight(2));
    // sprintf_(desc_idx, "%s Index: %#x", desc_tmp, SelCode.Idx);
    // SET_PRINT_MID((char *)desc_idx, FHeight(1));
}
void PageFaultExceptionHandler(CPU::x64::TrapFrame *Frame)
{
    // err("Kernel Exception");
    // PageFaultErrorCode params = {.raw = (uint32_t)ERROR_CODE};

    // // We can't use an allocator in exceptions (because that can cause another exception!) so, we'll just use a static buffer.
    // staticbuffer(ret_err);
    // staticbuffer(page_present);
    // staticbuffer(page_write);
    // staticbuffer(page_user);
    // staticbuffer(page_reserved);
    // staticbuffer(page_fetch);
    // staticbuffer(page_protection);
    // staticbuffer(page_shadow);
    // staticbuffer(page_sgx);

    // CurrentDisplay->SetPrintColor(0xDD2920);
    // SET_PRINT_MID((char *)"System crashed!", FHeight(12));
    // CurrentDisplay->ResetPrintColor();
    // sprintf_(ret_err, "An exception occurred at %#lx by %#lx", cr2.PFLA, RIP);
    // SET_PRINT_MID((char *)ret_err, FHeight(11));
    // sprintf_(page_present, "Page: %s", params.P ? "Present" : "Not Present");
    // SET_PRINT_MID((char *)page_present, FHeight(10));
    // sprintf_(page_write, "Write Operation: %s", params.W ? "Read-Only" : "Read-Write");
    // SET_PRINT_MID((char *)page_write, FHeight(9));
    // sprintf_(page_user, "Processor Mode: %s", params.U ? "User-Mode" : "Kernel-Mode");
    // SET_PRINT_MID((char *)page_user, FHeight(8));
    // sprintf_(page_reserved, "CPU Reserved Bits: %s", params.R ? "Reserved" : "Unreserved");
    // SET_PRINT_MID((char *)page_reserved, FHeight(7));
    // sprintf_(page_fetch, "Caused By An Instruction Fetch: %s", params.I ? "Yes" : "No");
    // SET_PRINT_MID((char *)page_fetch, FHeight(6));
    // sprintf_(page_protection, "Caused By A Protection-Key Violation: %s", params.PK ? "Yes" : "No");
    // SET_PRINT_MID((char *)page_protection, FHeight(5));
    // sprintf_(page_shadow, "Caused By A Shadow Stack Access: %s", params.SS ? "Yes" : "No");
    // SET_PRINT_MID((char *)page_shadow, FHeight(4));
    // sprintf_(page_sgx, "Caused By An SGX Violation: %s", params.SGX ? "Yes" : "No");
    // SET_PRINT_MID((char *)page_sgx, FHeight(3));
    // if (ERROR_CODE & 0x00000008)
    // {
    //     SET_PRINT_MID((char *)"One or more page directory entries contain reserved bits which are set to 1.", FHeight(2));
    // }
    // else
    // {
    //     SET_PRINT_MID((char *)pagefault_message[ERROR_CODE & 0b111], FHeight(2));
    // }
    // err("\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s", page_present, page_write, page_user, page_reserved, page_fetch, page_protection, page_shadow, page_sgx);
}
void x87FloatingPointExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("x87 floating point exception"); }
void AlignmentCheckExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Alignment check exception"); }
void MachineCheckExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Machine check exception"); }
void SIMDFloatingPointExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("SIMD floating point exception"); }
void VirtualizationExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Virtualization exception"); }
void SecurityExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Security exception"); }
void UnknownExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Unknown exception"); }

void UserModeExceptionHandler(CPU::x64::TrapFrame *Frame)
{
    switch (Frame->int_num)
    {
    case CPU::x64::DivideByZero:
    {
        break;
    }
    case CPU::x64::Debug:
    {
        break;
    }
    case CPU::x64::NonMaskableInterrupt:
    {
        break;
    }
    case CPU::x64::Breakpoint:
    {
        break;
    }
    case CPU::x64::Overflow:
    {
        break;
    }
    case CPU::x64::BoundRange:
    {
        break;
    }
    case CPU::x64::InvalidOpcode:
    {
        break;
    }
    case CPU::x64::DeviceNotAvailable:
    {
        break;
    }
    case CPU::x64::DoubleFault:
    {
        break;
    }
    case CPU::x64::CoprocessorSegmentOverrun:
    {
        break;
    }
    case CPU::x64::InvalidTSS:
    {
        break;
    }
    case CPU::x64::SegmentNotPresent:
    {
        break;
    }
    case CPU::x64::StackSegmentFault:
    {
        break;
    }
    case CPU::x64::GeneralProtectionFault:
    {
        break;
    }
    case CPU::x64::PageFault:
    {
        break;
    }
    case CPU::x64::x87FloatingPoint:
    {
        break;
    }
    case CPU::x64::AlignmentCheck:
    {
        break;
    }
    case CPU::x64::MachineCheck:
    {
        break;
    }
    case CPU::x64::SIMDFloatingPoint:
    {
        break;
    }
    case CPU::x64::Virtualization:
    {
        break;
    }
    case CPU::x64::Security:
    {
        break;
    }
    default:
    {
        break;
    }
    }
}
#endif