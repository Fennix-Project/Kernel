;   This file is part of Fennix Kernel.
;
;   Fennix Kernel is free software: you can redistribute it and/or
;   modify it under the terms of the GNU General Public License as
;   published by the Free Software Foundation, either version 3 of
;   the License, or (at your option) any later version.
;
;   Fennix Kernel is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;   GNU General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.

[bits 32]
KERNEL_STACK_SIZE equ 0x4000 ; 16KB

extern DetectCPUID
extern Detect64Bit
extern DetectPSE
extern DetectPAE
extern multiboot_main
extern LoadGDT32
extern BootPageTable
extern UpdatePageTable
extern GDT64.Ptr
extern GDT64.Code
extern GDT64.Data

section .bootstrap.data
MB_HeaderMagic:
	dq 0

MB_HeaderInfo:
	dq 0

section .bootstrap.text

global Multiboot2_start
Multiboot2_start:
	cli

	mov [MB_HeaderMagic], eax
	mov [MB_HeaderInfo], ebx

	call DetectCPUID
	cmp eax, 0
	je $

	call Detect64Bit
	cmp eax, 0
	je $

	call DetectPSE
	cmp eax, 0
	je $

	call DetectPAE
	cmp eax, 0
	je $

	mov ecx, cr4
	or ecx, 0x00000010 ; Set PSE in CR4
	or ecx, 0x00000020 ; Set PAE in CR4
	mov cr4, ecx

	call LoadGDT32
	call UpdatePageTable

	mov ecx, BootPageTable
	mov cr3, ecx

	mov ecx, 0xC0000080 ; EFER
	rdmsr
	or eax, 0x800 | 0x100 | 0x1 ; Set LME, LMA, SCE
	wrmsr

	mov ecx, cr0
	or ecx, 0x80000001 ; Set PG and PE in CR0
	mov cr0, ecx

	lgdt [GDT64.Ptr]
	jmp GDT64.Code:HigherHalfStart

extern UpdatePageTable64

[bits 64]
HigherHalfStart:
	mov ax, GDT64.Data
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	call UpdatePageTable64

	mov rsp, KernelStack + KERNEL_STACK_SIZE
	mov rdi, [MB_HeaderMagic]
	mov rsi, [MB_HeaderInfo]
	push rsi
	push rdi
	call multiboot_main
.Hang:
	hlt
	jmp .Hang

section .bootstrap.bss
align 16
KernelStack:
	resb KERNEL_STACK_SIZE
