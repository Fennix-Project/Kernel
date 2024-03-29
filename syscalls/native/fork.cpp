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

#include <syscalls.hpp>

#include <memory.hpp>
#include <lock.hpp>
#include <exec.hpp>
#include <errno.h>
#include <debug.h>

#include "../../syscalls.h"
#include "../../kernel.h"

using Tasking::PCB;
using Tasking::TCB;
using Tasking::TaskState::Ready;
using namespace Memory;

void sys_fork_return()
{
#if defined(a64)
	asmv("movq $0, %rax\n");  /* Return 0 */
	asmv("movq %r8, %rsp\n"); /* Restore stack pointer */
	asmv("movq %r8, %rbp\n"); /* Restore base pointer */
	asmv("swapgs\n");		  /* Swap GS back to the user GS */
	asmv("sti\n");			  /* Enable interrupts */
	asmv("sysretq\n");		  /* Return to rcx address in user mode */
#elif defined(a32)
#warning "sys_fork not implemented for i386"
#endif
	__builtin_unreachable();
}

/* https://pubs.opengroup.org/onlinepubs/009604499/functions/fork.html */
int sys_fork(SysFrm *Frame)
{
	assert(Frame != nullptr);

#ifdef a32
	return -ENOSYS;
#endif
	PCB *Parent = thisThread->Parent;
	TCB *Thread = thisThread;

	PCB *NewProcess =
		TaskManager->CreateProcess(Parent, Parent->Name,
								   Parent->Security.ExecutionMode,
								   true);

	if (!NewProcess)
	{
		error("Failed to create process for fork");
		return -EAGAIN;
	}

	NewProcess->PageTable = Parent->PageTable->Fork();

	TCB *NewThread =
		TaskManager->CreateThread(NewProcess,
								  0,
								  nullptr,
								  nullptr,
								  std::vector<AuxiliaryVector>(),
								  Thread->Info.Architecture,
								  Thread->Info.Compatibility,
								  true);

	NewThread->Rename(Thread->Name);

	if (!NewThread)
	{
		error("Failed to create thread for fork");
		return -EAGAIN;
	}

	TaskManager->UpdateFrame();

	NewThread->FPU = Thread->FPU;
	NewThread->Stack->Fork(Thread->Stack);
	NewThread->Info.Architecture = Thread->Info.Architecture;
	NewThread->Info.Compatibility = Thread->Info.Compatibility;
	NewThread->Security.IsCritical = Thread->Security.IsCritical;
	NewThread->Registers = Thread->Registers;
#if defined(a64)
	NewThread->Registers.rip = (uintptr_t)sys_fork_return;
	/* For sysretq */
	NewThread->Registers.rcx = Frame->ReturnAddress;
	NewThread->Registers.r8 = Frame->StackPointer;
#endif

#ifdef a86
	NewThread->GSBase = NewThread->ShadowGSBase;
	NewThread->ShadowGSBase = Thread->ShadowGSBase;
	NewThread->FSBase = Thread->FSBase;
#endif

	debug("ret addr: %#lx, stack: %#lx ip: %#lx", Frame->ReturnAddress,
		  Frame->StackPointer, (uintptr_t)sys_fork_return);
	debug("Forked thread \"%s\"(%d) to \"%s\"(%d)",
		  Thread->Name, Thread->ID,
		  NewThread->Name, NewThread->ID);
	NewThread->SetState(Ready);
	return (int)NewProcess->ID;
}
