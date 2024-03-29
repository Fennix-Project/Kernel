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

#ifndef __FENNIX_KERNEL_CPU_MEMBAR_H__
#define __FENNIX_KERNEL_CPU_MEMBAR_H__

#include <types.h>

namespace CPU
{
    namespace MemBar
    {
        nsa static inline void Barrier()
        {
#if defined(a86)
            asmv("" ::
                     : "memory");
#elif defined(aa64)
            asmv("dmb ish" ::
                     : "memory");
#endif
        }

        nsa static inline void Fence()
        {
#if defined(a86)
            asmv("mfence" ::
                     : "memory");
#elif defined(aa64)
            asmv("dmb ish" ::
                     : "memory");
#endif
        }

        nsa static inline void StoreFence()
        {
#if defined(a86)
            asmv("sfence" ::
                     : "memory");
#elif defined(aa64)
            asmv("dmb ishst" ::
                     : "memory");
#endif
        }

        nsa static inline void LoadFence()
        {
#if defined(a86)
            asmv("lfence" ::
                     : "memory");
#elif defined(aa64)
            asmv("dmb ishld" ::
                     : "memory");
#endif
        }
    }
}

#endif // !__FENNIX_KERNEL_CPU_MEMBAR_H__
