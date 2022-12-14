#include <interrupts.hpp>

#include <syscalls.hpp>
#include <hashmap.hpp>
#include <smp.hpp>
#include <io.h>

#if defined(__amd64__)
#include "../Architecture/amd64/cpu/gdt.hpp"
#include "../Architecture/amd64/cpu/idt.hpp"
#include "../Architecture/amd64/acpi.hpp"
#include "../Architecture/amd64/cpu/apic.hpp"
#elif defined(__i386__)
#include "../Architecture/i686/cpu/gdt.hpp"
#include "../Architecture/i686/cpu/idt.hpp"
#elif defined(__aarch64__)
#endif

#include "../crashhandler.hpp"
#include "../kernel.h"

extern "C" SafeFunction void ExceptionHandler(void *Data) { CrashHandler::Handle(Data); }

namespace Interrupts
{
    HashMap<int, uint64_t> *RegisteredEvents;

#if defined(__amd64__)
    /* APIC::APIC */ void *apic[MAX_CPU];
    /* APIC::Timer */ void *apicTimer[MAX_CPU];
#elif defined(__i386__)
    /* APIC::APIC */ void *apic[MAX_CPU];
#elif defined(__aarch64__)
#endif
    void *InterruptFrames[INT_FRAMES_MAX];

    void Initialize(int Core)
    {
        static int once = 0;
        if (!once++)
            RegisteredEvents = new HashMap<int, uint64_t>;

#if defined(__amd64__)
        GlobalDescriptorTable::Init(Core);
        InterruptDescriptorTable::Init(Core);
        CPUData *CoreData = GetCPU(Core);
        CoreData->Checksum = CPU_DATA_CHECKSUM;
        CPU::x64::wrmsr(CPU::x64::MSR_GS_BASE, (uint64_t)CoreData);
        CPU::x64::wrmsr(CPU::x64::MSR_SHADOW_GS_BASE, (uint64_t)CoreData);
        CoreData->ID = Core;
        CoreData->IsActive = true;
        CoreData->SystemCallStack = (uint8_t *)((uintptr_t)KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE)) + STACK_SIZE);
        CoreData->Stack = (uintptr_t)KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE)) + STACK_SIZE;
        if (CoreData->Checksum != CPU_DATA_CHECKSUM)
        {
            KPrint("CPU %d checksum mismatch! %x != %x", Core, CoreData->Checksum, CPU_DATA_CHECKSUM);
            CPU::Stop();
        }
        debug("Stack for core %d is %#lx (Address: %#lx)", Core, CoreData->Stack, CoreData->Stack - STACK_SIZE);
        asmv("movq %0, %%rsp" ::"r"(CoreData->Stack));
        InitializeSystemCalls();
#elif defined(__i386__)
        warn("i386 is not supported yet");
#elif defined(__aarch64__)
        warn("aarch64 is not supported yet");
#endif
    }

    void Enable(int Core)
    {
#if defined(__amd64__)
        if (((ACPI::MADT *)PowerManager->GetMADT())->LAPICAddress != nullptr)
        {
            // TODO: This function is called by SMP too. Do not initialize timers that doesn't support multiple cores.
            apic[Core] = new APIC::APIC(Core);
            ((APIC::APIC *)apic[Core])->RedirectIRQs(Core);
        }
        else
        {
            error("LAPIC not found");
            // TODO: PIC
        }
#elif defined(__i386__)
        warn("i386 is not supported yet");
#elif defined(__aarch64__)
        warn("aarch64 is not supported yet");
#endif
    }

    void InitializeTimer(int Core)
    {
        // TODO: This function is called by SMP too. Do not initialize timers that doesn't support multiple cores.
#if defined(__amd64__)
        if (apic[Core] != nullptr)
            apicTimer[Core] = new APIC::Timer((APIC::APIC *)apic[Core]);
        else
        {
            fixme("apic not found");
        }
#elif defined(__i386__)
        warn("i386 is not supported yet");
#elif defined(__aarch64__)
        warn("aarch64 is not supported yet");
#endif
    }

    void RemoveAll()
    {
        for (int i = 0; i < CPU::x64::IRQ223; i++)
                RegisteredEvents->DeleteNode(i);
    }

    extern "C" SafeFunction void MainInterruptHandler(void *Data)
    {
#if defined(__amd64__)
        CPU::x64::TrapFrame *Frame = (CPU::x64::TrapFrame *)Data;

        memmove(InterruptFrames + 1, InterruptFrames, sizeof(InterruptFrames) - sizeof(InterruptFrames[0]));
        InterruptFrames[0] = (void *)Frame->rip;
        
        CPUData *CoreData = GetCurrentCPU();
        int Core = 0;
        if (likely(CoreData != nullptr))
            Core = CoreData->ID;

        // If this is false, we have a big problem.
        if (likely(Frame->InterruptNumber < CPU::x64::IRQ223 && Frame->InterruptNumber > CPU::x64::ISR0))
        {
            Handler *handler = (Handler *)RegisteredEvents->Get(Frame->InterruptNumber);
            if (likely(handler != (Handler *)0xdeadbeef))
                handler->OnInterruptReceived(Frame);
            else
                error("IRQ%ld is unhandled on CPU %d.", Frame->InterruptNumber - 32, Core);

            if (likely(apic[Core]))
            {
                ((APIC::APIC *)Interrupts::apic[Core])->EOI();
                // TODO: Handle PIC too
                return;
            }
            // TODO: PIC
        }
#elif defined(__i386__)
        void *Frame = Data;
#elif defined(__aarch64__)
        void *Frame = Data;
#endif
        error("HALT HALT HALT HALT HALT HALT HALT HALT HALT");
        CPU::Stop();
    }

    Handler::Handler(int InterruptNumber)
    {
        if (RegisteredEvents->Get(InterruptNumber) != (uint64_t)0xdeadbeef)
        {
            warn("IRQ%d is already registered.", InterruptNumber - 32);
            return;
        }

        debug("Registering interrupt handler for IRQ%d.", InterruptNumber - 32);
        this->InterruptNumber = InterruptNumber;
        RegisteredEvents->AddNode(InterruptNumber, (uint64_t)this);
    }

    Handler::~Handler()
    {
        debug("Unregistering interrupt handler for IRQ%d.", InterruptNumber - 32);
        if (RegisteredEvents->DeleteNode(InterruptNumber) == 0xdeadbeef)
            warn("Node %d not found.", InterruptNumber);
    }

#if defined(__amd64__)
    void Handler::OnInterruptReceived(CPU::x64::TrapFrame *Frame)
    {
        trace("Unhandled interrupt IRQ%d", Frame->InterruptNumber - 32);
#elif defined(__i386__)
    void Handler::OnInterruptReceived(void *Frame)
    {
        trace("Unhandled interrupt received");
#elif defined(__aarch64__)
    void Handler::OnInterruptReceived(void *Frame)
    {
        trace("Unhandled interrupt received");
#endif
    }
}
