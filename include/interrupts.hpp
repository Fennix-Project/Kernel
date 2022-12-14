#ifndef __FENNIX_KERNEL_INTERRUPTS_H__
#define __FENNIX_KERNEL_INTERRUPTS_H__

#include <types.h>
#include <cpu.hpp>

namespace Interrupts
{
#ifdef DEBUG // For performance reasons
#define INT_FRAMES_MAX 512
#else
#define INT_FRAMES_MAX 8
#endif

#if defined(__amd64__)
    /* APIC::APIC */ extern void *apic[256];       // MAX_CPU
    /* APIC::Timer */ extern void *apicTimer[256]; // MAX_CPU
#elif defined(__i386__)
    /* APIC::APIC */ extern void *apic[256];       // MAX_CPU
    /* APIC::Timer */ extern void *apicTimer[256]; // MAX_CPU
#elif defined(__aarch64__)
#endif
    extern void *InterruptFrames[INT_FRAMES_MAX];

    void Initialize(int Core);
    void Enable(int Core);
    void InitializeTimer(int Core);
    void RemoveAll();

    class Handler
    {
    private:
        int InterruptNumber;

    protected:
        void SetInterruptNumber(int InterruptNumber) { this->InterruptNumber = InterruptNumber; }
        int GetInterruptNumber() { return InterruptNumber; }
        Handler(int InterruptNumber);
        ~Handler();

    public:
#if defined(__amd64__)
        virtual void OnInterruptReceived(CPU::x64::TrapFrame *Frame);
#elif defined(__i386__)
        virtual void OnInterruptReceived(void *Frame);
#elif defined(__aarch64__)
        virtual void OnInterruptReceived(void *Frame);
#endif
    };
}

#endif // !__FENNIX_KERNEL_INTERRUPTS_H__
