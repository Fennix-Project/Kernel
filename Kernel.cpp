#include "kernel.h"

#include <interrupts.hpp>
#include <display.hpp>
#include <symbols.hpp>
#include <memory.hpp>
#include <string.h>
#include <printf.h>
#include <time.hpp>
#include <debug.h>

BootInfo *bInfo = nullptr;
Video::Display *Display = nullptr;
SymbolResolver::Symbols *KernelSymbolTable = nullptr;

// For the Display class. Printing on first buffer.
extern "C" void putchar(char c) { Display->Print(c, 0); }

#ifdef __debug_vscode__
extern "C" int printf_(const char *format, ...);
extern "C" int vprintf_(const char *format, va_list arg);
#endif

void KPrint(const char *format, ...)
{
    Time tm = ReadClock();
    printf_("[%02ld:%02ld:%02ld] ", tm.Hour, tm.Minute, tm.Second);
    va_list args;
    va_start(args, format);
    vprintf_(format, args);
    va_end(args);
    putchar('\n');
    Display->SetBuffer(0);
}

EXTERNC void kernel_aarch64_entry(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
{
    trace("Hello, World!");
    while (1)
        CPU::Halt();
}

EXTERNC void kernel_entry(BootInfo *Info)
{
    trace("Hello, World!");
    InitializeMemoryManagement(Info);
    bInfo = (BootInfo *)KernelAllocator.RequestPages(TO_PAGES(sizeof(BootInfo)));
    memcpy(bInfo, Info, sizeof(BootInfo));
    debug("BootInfo structure is at %p", bInfo);
    Display = new Video::Display(bInfo->Framebuffer[0]);
    printf_("%s - %s(%s)\n", KERNEL_NAME, KERNEL_VERSION, GIT_COMMIT_SHORT);
    /**************************************************************************************/
    KPrint("Initializing GDT and IDT");
    Interrupts::Initialize();
    KPrint("Loading kernel symbols");
    KernelSymbolTable = new SymbolResolver::Symbols((uint64_t)Info->Kernel.FileBase);
    // printf_("%s\n", CPU::Vendor());
    // printf_("%s\n", CPU::Name());
    // printf_("%s\n", CPU::Hypervisor());
    Display->SetBuffer(0);
    for (size_t i = 0; i < 250; i++)
        KPrint("Hello, World! (%02ld)", i);
    asm("int $0x1");
    while (1)
        CPU::Halt();
}
