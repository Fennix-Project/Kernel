#ifndef __FENNIX_KERNEL_KERNEL_CONFIG_H__
#define __FENNIX_KERNEL_KERNEL_CONFIG_H__

#include <types.h>
#include <memory.hpp>

struct KernelConfig
{
    Memory::MemoryAllocatorType AllocatorType;
    bool SchedulerType;
    char DriverDirectory[256];
    char InitPath[256];
    bool InterruptsOnCrash;
    int Cores;
    bool UnlockDeadLock;
};

KernelConfig ParseConfig(char *Config);

#endif // !__FENNIX_KERNEL_KERNEL_CONFIG_H__
