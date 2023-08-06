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

#ifndef __FENNIX_KERNEL_INTERNAL_MEMORY_H__
#define __FENNIX_KERNEL_INTERNAL_MEMORY_H__

#ifdef __cplusplus
#include <filesystem.hpp>
#include <boot/binfo.h>
#include <bitmap.hpp>
#include <lock.hpp>
#include <std.hpp>
#include <atomic>
#include <cstddef>
#endif // __cplusplus
#include <types.h>

#ifdef __cplusplus

extern uintptr_t _bootstrap_start, _bootstrap_end;
extern uintptr_t _kernel_start, _kernel_end;
extern uintptr_t _kernel_text_start, _kernel_text_end;
extern uintptr_t _kernel_data_start, _kernel_data_end;
extern uintptr_t _kernel_rodata_start, _kernel_rodata_end;
extern uintptr_t _kernel_bss_start, _kernel_bss_end;

// kilobyte
#define TO_KiB(d) ((d) / 1024)
// megabyte
#define TO_MiB(d) ((d) / 1024 / 1024)
// gigabyte
#define TO_GiB(d) ((d) / 1024 / 1024 / 1024)
// terabyte
#define TO_TiB(d) ((d) / 1024 / 1024 / 1024 / 1024)
// petabyte
#define TO_PiB(d) ((d) / 1024 / 1024 / 1024 / 1024 / 1024)
// exobyte
#define TO_EiB(d) ((d) / 1024 / 1024 / 1024 / 1024 / 1024 / 1024)

#define PAGE_SIZE 0x1000		// 4KB
#define PAGE_SIZE_4K PAGE_SIZE	// 4KB
#define PAGE_SIZE_2M 0x200000	// 2MB
#define PAGE_SIZE_4M 0x400000	// 4MB
#define PAGE_SIZE_1G 0x40000000 // 1GB

#define STACK_SIZE 0x4000	   // 16kb
#define USER_STACK_SIZE 0x2000 // 8kb

// To pages
#define TO_PAGES(d) (((d) + PAGE_SIZE - 1) / PAGE_SIZE)
// From pages
#define FROM_PAGES(d) ((d)*PAGE_SIZE)

#if defined(a64) || defined(aa64)
#define NORMAL_VMA_OFFSET 0xFFFF800000000000
#define KERNEL_VMA_OFFSET 0xFFFFFFFF80000000
#define KERNEL_HEAP_BASE 0xFFFFA00000000000
#define USER_HEAP_BASE 0xFFFFB00000000000
#define USER_STACK_BASE 0xFFFFEFFFFFFF0000
#elif defined(a32)
#define NORMAL_VMA_OFFSET 0x80000000
#define KERNEL_VMA_OFFSET 0xC0000000
#define KERNEL_HEAP_BASE 0xA0000000
#define USER_HEAP_BASE 0xB0000000
#define USER_STACK_BASE 0xEFFFFFFF
#endif

namespace Memory
{
	enum MemoryAllocatorType
	{
		None,
		Pages,
		XallocV1,
		liballoc11
	};

	/**
	 * @brief https://wiki.osdev.org/images/4/41/64-bit_page_tables1.png
	 * @brief https://wiki.osdev.org/images/6/6b/64-bit_page_tables2.png
	 */
	enum PTFlag
	{
		/** @brief Present */
		P = 1 << 0,

		/** @brief Read/Write */
		RW = 1 << 1,

		/** @brief User/Supervisor */
		US = 1 << 2,

		/** @brief Write-Through */
		PWT = 1 << 3,

		/** @brief Cache Disable */
		PCD = 1 << 4,

		/** @brief Accessed */
		A = 1 << 5,

		/** @brief Dirty */
		D = 1 << 6,

		/** @brief Page Size */
		PS = 1 << 7,

		/** @brief Global */
		G = 1 << 8,

		/** @brief Available 0 */
		AVL0 = 1 << 9,

		/** @brief Available 1 */
		AVL1 = 1 << 10,

		/** @brief Available 2 */
		AVL2 = 1 << 11,

		/** @brief Page Attribute Table */
		PAT = 1 << 12,

		/** @brief Available 3 */
		AVL3 = (uint64_t)1 << 52,

		/** @brief Available 4 */
		AVL4 = (uint64_t)1 << 53,

		/** @brief Available 5 */
		AVL5 = (uint64_t)1 << 54,

		/** @brief Available 6 */
		AVL6 = (uint64_t)1 << 55,

		/** @brief Available 7 */
		AVL7 = (uint64_t)1 << 56,

		/** @brief Available 8 */
		AVL8 = (uint64_t)1 << 57,

		/** @brief Available 9 */
		AVL9 = (uint64_t)1 << 58,

		/** @brief Protection Key 0 */
		PK0 = (uint64_t)1 << 59,

		/** @brief Protection Key 1 */
		PK1 = (uint64_t)1 << 60,

		/** @brief Protection Key 2 */
		PK2 = (uint64_t)1 << 61,

		/** @brief Protection Key 3 */
		PK3 = (uint64_t)1 << 62,

		/** @brief Execute Disable */
		XD = (uint64_t)1 << 63
	};

	union __packed PageTableEntry
	{
		struct
		{
#if defined(a64)
			uintptr_t Present : 1;			  // 0
			uintptr_t ReadWrite : 1;		  // 1
			uintptr_t UserSupervisor : 1;	  // 2
			uintptr_t WriteThrough : 1;		  // 3
			uintptr_t CacheDisable : 1;		  // 4
			uintptr_t Accessed : 1;			  // 5
			uintptr_t Dirty : 1;			  // 6
			uintptr_t PageAttributeTable : 1; // 7
			uintptr_t Global : 1;			  // 8
			uintptr_t Available0 : 1;		  // 9
			uintptr_t Available1 : 1;		  // 10
			uintptr_t Available2 : 1;		  // 11
			uintptr_t Address : 40;			  // 12-51
			uintptr_t Available3 : 1;		  // 52
			uintptr_t Available4 : 1;		  // 53
			uintptr_t Available5 : 1;		  // 54
			uintptr_t Available6 : 1;		  // 55
			uintptr_t Available7 : 1;		  // 56
			uintptr_t Available8 : 1;		  // 57
			uintptr_t Available9 : 1;		  // 58
			uintptr_t ProtectionKey : 4;	  // 59-62
			uintptr_t ExecuteDisable : 1;	  // 63
#elif defined(a32)
			uintptr_t Present : 1;			  // 0
			uintptr_t ReadWrite : 1;		  // 1
			uintptr_t UserSupervisor : 1;	  // 2
			uintptr_t WriteThrough : 1;		  // 3
			uintptr_t CacheDisable : 1;		  // 4
			uintptr_t Accessed : 1;			  // 5
			uintptr_t Dirty : 1;			  // 6
			uintptr_t PageAttributeTable : 1; // 7
			uintptr_t Global : 1;			  // 8
			uintptr_t Available0 : 1;		  // 9
			uintptr_t Available1 : 1;		  // 10
			uintptr_t Available2 : 1;		  // 11
			uintptr_t Address : 20;			  // 12-31
#elif defined(aa64)
#endif
		};
		uintptr_t raw;

		/** @brief Set Address */
		void SetAddress(uintptr_t _Address)
		{
#if defined(a64)
			_Address &= 0x000000FFFFFFFFFF;
			this->raw &= 0xFFF0000000000FFF;
			this->raw |= (_Address << 12);
#elif defined(a32)
			_Address &= 0x000FFFFF;
			this->raw &= 0xFFC00003;
			this->raw |= (_Address << 12);
#elif defined(aa64)
			_Address &= 0x000000FFFFFFFFFF;
			this->raw &= 0xFFF0000000000FFF;
			this->raw |= (_Address << 12);
#endif
		}

		/** @brief Get Address */
		uintptr_t GetAddress()
		{
#if defined(a64)
			return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#elif defined(a32)
			return ((uintptr_t)(this->raw & 0x003FFFFF000) >> 12);
#elif defined(aa64)
			return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#endif
		}
	};

	struct __packed PageTableEntryPtr
	{
#if defined(a64)
		PageTableEntry Entries[512];
#elif defined(a32)
		PageTableEntry Entries[1024];
#elif defined(aa64)
#endif
	};

	union __packed PageDirectoryEntry
	{
#if defined(a64)
		struct
		{
			uintptr_t Present : 1;		  // 0
			uintptr_t ReadWrite : 1;	  // 1
			uintptr_t UserSupervisor : 1; // 2
			uintptr_t WriteThrough : 1;	  // 3
			uintptr_t CacheDisable : 1;	  // 4
			uintptr_t Accessed : 1;		  // 5
			uintptr_t Available0 : 1;	  // 6
			uintptr_t PageSize : 1;		  // 7
			uintptr_t Available1 : 4;	  // 8-11
			uintptr_t Address : 40;		  // 12-51
			uintptr_t Available2 : 11;	  // 52-62
			uintptr_t ExecuteDisable : 1; // 63
		};

		struct
		{
			uintptr_t Present : 1;			  // 0
			uintptr_t ReadWrite : 1;		  // 1
			uintptr_t UserSupervisor : 1;	  // 2
			uintptr_t WriteThrough : 1;		  // 3
			uintptr_t CacheDisable : 1;		  // 4
			uintptr_t Accessed : 1;			  // 5
			uintptr_t Dirty : 1;			  // 6
			uintptr_t PageSize : 1;			  // 7
			uintptr_t Global : 1;			  // 8
			uintptr_t Available0 : 1;		  // 9
			uintptr_t Available1 : 1;		  // 10
			uintptr_t Available2 : 1;		  // 11
			uintptr_t PageAttributeTable : 1; // 12
			uintptr_t Reserved0 : 8;		  // 13-20
			uintptr_t Address : 31;			  // 21-51
			uintptr_t Available3 : 1;		  // 52
			uintptr_t Available4 : 1;		  // 53
			uintptr_t Available5 : 1;		  // 54
			uintptr_t Available6 : 1;		  // 55
			uintptr_t Available7 : 1;		  // 56
			uintptr_t Available8 : 1;		  // 57
			uintptr_t Available9 : 1;		  // 58
			uintptr_t ProtectionKey : 4;	  // 59-62
			uintptr_t ExecuteDisable : 1;	  // 63
		} TwoMiB;
#elif defined(a32)
		struct
		{
			uintptr_t Present : 1;		  // 0
			uintptr_t ReadWrite : 1;	  // 1
			uintptr_t UserSupervisor : 1; // 2
			uintptr_t WriteThrough : 1;	  // 3
			uintptr_t CacheDisable : 1;	  // 4
			uintptr_t Accessed : 1;		  // 5
			uintptr_t Available0 : 1;	  // 6
			uintptr_t PageSize : 1;		  // 7
			uintptr_t Available1 : 4;	  // 8-11
			uintptr_t Address : 20;		  // 12-31
		};

		struct
		{
			uintptr_t Present : 1;			  // 0
			uintptr_t ReadWrite : 1;		  // 1
			uintptr_t UserSupervisor : 1;	  // 2
			uintptr_t WriteThrough : 1;		  // 3
			uintptr_t CacheDisable : 1;		  // 4
			uintptr_t Accessed : 1;			  // 5
			uintptr_t Dirty : 1;			  // 6
			uintptr_t PageSize : 1;			  // 7
			uintptr_t Global : 1;			  // 8
			uintptr_t Available0 : 1;		  // 9
			uintptr_t Available1 : 1;		  // 10
			uintptr_t Available2 : 1;		  // 11
			uintptr_t PageAttributeTable : 1; // 12
			uintptr_t Address0 : 8;			  // 13-20
			uintptr_t Reserved0 : 1;		  // 21
			uintptr_t Address1 : 10;		  // 22-31
		} FourMiB;
#elif defined(aa64)
#endif
		uintptr_t raw;

		/** @brief Set PageTableEntryPtr address */
		void SetAddress(uintptr_t _Address)
		{
#if defined(a64)
			_Address &= 0x000000FFFFFFFFFF;
			this->raw &= 0xFFF0000000000FFF;
			this->raw |= (_Address << 12);
#elif defined(a32)
			_Address &= 0x000FFFFF;
			this->raw &= 0xFFC00003;
			this->raw |= (_Address << 12);
#elif defined(aa64)
			_Address &= 0x000000FFFFFFFFFF;
			this->raw &= 0xFFF0000000000FFF;
			this->raw |= (_Address << 12);
#endif
		}

		/** @brief Get PageTableEntryPtr address */
		uintptr_t GetAddress()
		{
#if defined(a64)
			return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#elif defined(a32)
			return ((uintptr_t)(this->raw & 0x003FFFFF000) >> 12);
#elif defined(aa64)
			return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#endif
		}
	};

	struct __packed PageDirectoryEntryPtr
	{
		PageDirectoryEntry Entries[512];
	};

	union __packed PageDirectoryPointerTableEntry
	{
#if defined(a64)
		struct
		{
			uintptr_t Present : 1;		  // 0
			uintptr_t ReadWrite : 1;	  // 1
			uintptr_t UserSupervisor : 1; // 2
			uintptr_t WriteThrough : 1;	  // 3
			uintptr_t CacheDisable : 1;	  // 4
			uintptr_t Accessed : 1;		  // 5
			uintptr_t Available0 : 1;	  // 6
			uintptr_t PageSize : 1;		  // 7
			uintptr_t Available1 : 4;	  // 8-11
			uintptr_t Address : 40;		  // 12-51
			uintptr_t Available2 : 11;	  // 52-62
			uintptr_t ExecuteDisable : 1; // 63
		};

		struct
		{
			uintptr_t Present : 1;			  // 0
			uintptr_t ReadWrite : 1;		  // 1
			uintptr_t UserSupervisor : 1;	  // 2
			uintptr_t WriteThrough : 1;		  // 3
			uintptr_t CacheDisable : 1;		  // 4
			uintptr_t Accessed : 1;			  // 5
			uintptr_t Dirty : 1;			  // 6
			uintptr_t PageSize : 1;			  // 7
			uintptr_t Global : 1;			  // 8
			uintptr_t Available0 : 1;		  // 9
			uintptr_t Available1 : 1;		  // 10
			uintptr_t Available2 : 1;		  // 11
			uintptr_t PageAttributeTable : 1; // 12
			uintptr_t Reserved0 : 17;		  // 13-29
			uintptr_t Address : 22;			  // 30-51
			uintptr_t Available3 : 1;		  // 52
			uintptr_t Available4 : 1;		  // 53
			uintptr_t Available5 : 1;		  // 54
			uintptr_t Available6 : 1;		  // 55
			uintptr_t Available7 : 1;		  // 56
			uintptr_t Available8 : 1;		  // 57
			uintptr_t Available9 : 1;		  // 58
			uintptr_t ProtectionKey : 4;	  // 59-62
			uintptr_t ExecuteDisable : 1;	  // 63
		} OneGiB;
#elif defined(aa64)
#endif
		uintptr_t raw;

		/** @brief Set PageDirectoryEntryPtr address */
		void SetAddress(uintptr_t _Address)
		{
#if defined(a64)
			_Address &= 0x000000FFFFFFFFFF;
			this->raw &= 0xFFF0000000000FFF;
			this->raw |= (_Address << 12);
#elif defined(aa64)
			_Address &= 0x000000FFFFFFFFFF;
			this->raw &= 0xFFF0000000000FFF;
			this->raw |= (_Address << 12);
#endif
		}

		/** @brief Get PageDirectoryEntryPtr address */
		uintptr_t GetAddress()
		{
#if defined(a64)
			return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#elif defined(a32)
			return 0;
#elif defined(aa64)
			return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#endif
		}
	};

	struct __packed PageDirectoryPointerTableEntryPtr
	{
		PageDirectoryPointerTableEntry Entries[512];
	};

	union __packed PageMapLevel4
	{
#if defined(a64)
		struct
		{
			uintptr_t Present : 1;		  // 0
			uintptr_t ReadWrite : 1;	  // 1
			uintptr_t UserSupervisor : 1; // 2
			uintptr_t WriteThrough : 1;	  // 3
			uintptr_t CacheDisable : 1;	  // 4
			uintptr_t Accessed : 1;		  // 5
			uintptr_t Available0 : 1;	  // 6
			uintptr_t Reserved0 : 1;	  // 7
			uintptr_t Available1 : 4;	  // 8-11
			uintptr_t Address : 40;		  // 12-51
			uintptr_t Available2 : 11;	  // 52-62
			uintptr_t ExecuteDisable : 1; // 63
		};
#elif defined(aa64)
#endif
		uintptr_t raw;

		/** @brief Set PageDirectoryPointerTableEntryPtr address */
		void SetAddress(uintptr_t _Address)
		{
#if defined(a64)
			_Address &= 0x000000FFFFFFFFFF;
			this->raw &= 0xFFF0000000000FFF;
			this->raw |= (_Address << 12);
#elif defined(aa64)
			_Address &= 0x000000FFFFFFFFFF;
			this->raw &= 0xFFF0000000000FFF;
			this->raw |= (_Address << 12);
#endif
		}

		/** @brief Get PageDirectoryPointerTableEntryPtr address */
		uintptr_t GetAddress()
		{
#if defined(a64)
			return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#elif defined(a32)
			return 0;
#elif defined(aa64)
			return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#endif
		}
	};

	struct __packed PageMapLevel4Ptr
	{
		PageMapLevel4 Entries[512];
	};

	union __packed PageMapLevel5
	{
#if defined(a64)
		struct
		{
			uintptr_t Present : 1;		  // 0
			uintptr_t ReadWrite : 1;	  // 1
			uintptr_t UserSupervisor : 1; // 2
			uintptr_t WriteThrough : 1;	  // 3
			uintptr_t CacheDisable : 1;	  // 4
			uintptr_t Accessed : 1;		  // 5
			uintptr_t Available0 : 1;	  // 6
			uintptr_t Reserved0 : 1;	  // 7
			uintptr_t Available1 : 4;	  // 8-11
			uintptr_t Address : 40;		  // 12-51
			uintptr_t Available2 : 11;	  // 52-62
			uintptr_t ExecuteDisable : 1; // 63
		};
#elif defined(aa64)
#endif
		uintptr_t raw;

		/** @brief Set PageMapLevel4Ptr address */
		void SetAddress(uintptr_t _Address)
		{
#if defined(a64)
			_Address &= 0x000000FFFFFFFFFF;
			this->raw &= 0xFFF0000000000FFF;
			this->raw |= (_Address << 12);
#elif defined(aa64)
			_Address &= 0x000000FFFFFFFFFF;
			this->raw &= 0xFFF0000000000FFF;
			this->raw |= (_Address << 12);
#endif
		}

		/** @brief Get PageMapLevel4Ptr address */
		uintptr_t GetAddress()
		{
#if defined(a64)
			return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#elif defined(a32)
			return 0;
#elif defined(aa64)
			return (this->raw & 0x000FFFFFFFFFF000) >> 12;
#endif
		}
	};

	class PageTable
	{
	public:
#if defined(a64)
		PageMapLevel4 Entries[512];
#elif defined(a32)
		PageDirectoryEntry Entries[1024];
#elif defined(aa64)
#endif

		/**
		 * @brief Update CR3 with this PageTable
		 */
		void Update();

		/**
		 * @brief Fork this PageTable
		 *
		 * @return A new PageTable with the same content
		 */
		PageTable Fork();
	} __aligned(0x1000);

	class Physical
	{
	private:
		NewLock(MemoryLock);

		std::atomic_uint64_t TotalMemory = 0;
		std::atomic_uint64_t FreeMemory = 0;
		std::atomic_uint64_t ReservedMemory = 0;
		std::atomic_uint64_t UsedMemory = 0;
		uint64_t PageBitmapIndex = 0;
		Bitmap PageBitmap;

		void ReserveEssentials();

	public:
		Bitmap GetPageBitmap() { return PageBitmap; }

		/**
		 * @brief Get Total Memory
		 *
		 * @return uint64_t
		 */
		uint64_t GetTotalMemory();

		/**
		 * @brief Get Free Memory
		 *
		 * @return uint64_t
		 */
		uint64_t GetFreeMemory();

		/**
		 * @brief Get Reserved Memory
		 *
		 * @return uint64_t
		 */
		uint64_t GetReservedMemory();

		/**
		 * @brief Get Used Memory
		 *
		 * @return uint64_t
		 */
		uint64_t GetUsedMemory();

		/**
		 * @brief Swap page
		 *
		 * @param Address Address of the page
		 * @return true if swap was successful
		 * @return false if swap was unsuccessful
		 */
		bool SwapPage(void *Address);

		/**
		 * @brief Swap pages
		 *
		 * @param Address Address of the pages
		 * @param PageCount Number of pages
		 * @return true if swap was successful
		 * @return false if swap was unsuccessful
		 */
		bool SwapPages(void *Address, size_t PageCount);

		/**
		 * @brief Unswap page
		 *
		 * @param Address Address of the page
		 * @return true if unswap was successful
		 * @return false if unswap was unsuccessful
		 */
		bool UnswapPage(void *Address);

		/**
		 * @brief Unswap pages
		 *
		 * @param Address Address of the pages
		 * @param PageCount Number of pages
		 * @return true if unswap was successful
		 * @return false if unswap was unsuccessful
		 */
		bool UnswapPages(void *Address, size_t PageCount);

		/**
		 * @brief Lock page
		 *
		 * @param Address Address of the page
		 */
		void LockPage(void *Address);

		/**
		 * @brief Lock pages
		 *
		 * @param Address Address of the pages
		 * @param PageCount Number of pages
		 */
		void LockPages(void *Address, size_t PageCount);

		void ReservePage(void *Address);
		void ReservePages(void *Address, size_t PageCount);
		void UnreservePage(void *Address);
		void UnreservePages(void *Address, size_t PageCount);

		/**
		 * @brief Request page
		 *
		 * @return void* Allocated page address
		 */
		void *RequestPage();

		/**
		 * @brief Request pages
		 *
		 * @param PageCount Number of pages
		 * @return void* Allocated pages address
		 */
		void *RequestPages(std::size_t Count);

		/**
		 * @brief Free page
		 *
		 * @param Address Address of the page
		 */
		void FreePage(void *Address);

		/**
		 * @brief Free pages
		 *
		 * @param Address Address of the pages
		 * @param PageCount Number of pages
		 */
		void FreePages(void *Address, size_t Count);

		/** @brief Do not use. */
		void Init();

		/** @brief Do not use. */
		Physical();

		/** @brief Do not use. */
		~Physical();
	};

	class Virtual
	{
	private:
		NewLock(MemoryLock);
		PageTable *Table = nullptr;

	public:
		enum MapType
		{
			NoMapType,
			FourKiB,
			TwoMiB,
			FourMiB,
			OneGiB
		};

		class PageMapIndexer
		{
		public:
#if defined(a64)
			uintptr_t PMLIndex = 0;
			uintptr_t PDPTEIndex = 0;
#endif
			uintptr_t PDEIndex = 0;
			uintptr_t PTEIndex = 0;
			PageMapIndexer(uintptr_t VirtualAddress);
		};

		/**
		 * @brief Check if page has the specified flag.
		 *
		 * @param VirtualAddress Virtual address of the page
		 * @param Flag Flag to check
		 * @param Type Type of the page. Check MapType enum.
		 * @return true if page has the specified flag.
		 * @return false if page is has the specified flag.
		 */
		bool Check(void *VirtualAddress, PTFlag Flag = PTFlag::P, MapType Type = MapType::FourKiB);

		/**
		 * @brief Get physical address of the page.
		 * @param VirtualAddress Virtual address of the page.
		 * @return Physical address of the page.
		 */
		void *GetPhysical(void *VirtualAddress);

		/**
		 * @brief Get map type of the page.
		 * @param VirtualAddress Virtual address of the page.
		 * @return Map type of the page.
		 */
		MapType GetMapType(void *VirtualAddress);

#ifdef a64
		PageMapLevel5 *GetPML5(void *VirtualAddress, MapType Type = MapType::FourKiB);
		PageMapLevel4 *GetPML4(void *VirtualAddress, MapType Type = MapType::FourKiB);
		PageDirectoryPointerTableEntry *GetPDPTE(void *VirtualAddress, MapType Type = MapType::FourKiB);
#endif /* a64 */
		PageDirectoryEntry *GetPDE(void *VirtualAddress, MapType Type = MapType::FourKiB);
		PageTableEntry *GetPTE(void *VirtualAddress, MapType Type = MapType::FourKiB);

		/**
		 * @brief Map page.
		 *
		 * @param VirtualAddress Virtual address of the page.
		 * @param PhysicalAddress Physical address of the page.
		 * @param Flags Flags of the page. Check PTFlag enum.
		 * @param Type Type of the page. Check MapType enum.
		 */
		void Map(void *VirtualAddress, void *PhysicalAddress, uint64_t Flag = PTFlag::P, MapType Type = MapType::FourKiB);

		/**
		 * @brief Map multiple pages.
		 *
		 * @param VirtualAddress First virtual address of the page.
		 * @param PhysicalAddress First physical address of the page.
		 * @param Length Length to map.
		 * @param Flags Flags of the page. Check PTFlag enum.
		 * @param Type Type of the page. Check MapType enum.
		 */
		__always_inline inline void Map(void *VirtualAddress, void *PhysicalAddress, size_t Length, uint64_t Flags, MapType Type = MapType::FourKiB)
		{
			int PageSize = PAGE_SIZE_4K;

			if (Type == MapType::TwoMiB)
				PageSize = PAGE_SIZE_2M;
			else if (Type == MapType::FourMiB)
				PageSize = PAGE_SIZE_4M;
			else if (Type == MapType::OneGiB)
				PageSize = PAGE_SIZE_1G;

			for (uintptr_t i = 0; i < Length; i += PageSize)
				this->Map((void *)((uintptr_t)VirtualAddress + i), (void *)((uintptr_t)PhysicalAddress + i), Flags, Type);
		}

		/**
		 * @brief Map multiple pages efficiently.
		 *
		 * This function will detect the best page size to map the pages.
		 *
		 * @note This function will not check if PSE or 1GB pages are enabled or supported.
		 *
		 * @param VirtualAddress First virtual address of the page.
		 * @param PhysicalAddress First physical address of the page.
		 * @param Length Length of the pages.
		 * @param Flags Flags of the page. Check PTFlag enum.
		 * @param Fit If true, the function will try to fit the pages in the smallest page size.
		 * @param FailOnModulo If true, the function will return NoMapType if the length is not a multiple of the page size.
		 * @return The best page size to map the pages.
		 */
		__always_inline inline MapType OptimizedMap(void *VirtualAddress, void *PhysicalAddress, size_t Length, uint64_t Flags, bool Fit = false, bool FailOnModulo = false)
		{
			if (unlikely(Fit))
			{
				while (Length >= PAGE_SIZE_1G)
				{
					this->Map(VirtualAddress, PhysicalAddress, Length, Flags, Virtual::MapType::OneGiB);
					VirtualAddress = (void *)((uintptr_t)VirtualAddress + PAGE_SIZE_1G);
					PhysicalAddress = (void *)((uintptr_t)PhysicalAddress + PAGE_SIZE_1G);
					Length -= PAGE_SIZE_1G;
				}

				while (Length >= PAGE_SIZE_4M)
				{
					this->Map(VirtualAddress, PhysicalAddress, Length, Flags, Virtual::MapType::FourMiB);
					VirtualAddress = (void *)((uintptr_t)VirtualAddress + PAGE_SIZE_4M);
					PhysicalAddress = (void *)((uintptr_t)PhysicalAddress + PAGE_SIZE_4M);
					Length -= PAGE_SIZE_4M;
				}

				while (Length >= PAGE_SIZE_2M)
				{
					this->Map(VirtualAddress, PhysicalAddress, Length, Flags, Virtual::MapType::TwoMiB);
					VirtualAddress = (void *)((uintptr_t)VirtualAddress + PAGE_SIZE_2M);
					PhysicalAddress = (void *)((uintptr_t)PhysicalAddress + PAGE_SIZE_2M);
					Length -= PAGE_SIZE_2M;
				}

				while (Length >= PAGE_SIZE_4K)
				{
					this->Map(VirtualAddress, PhysicalAddress, Length, Flags, Virtual::MapType::FourKiB);
					VirtualAddress = (void *)((uintptr_t)VirtualAddress + PAGE_SIZE_4K);
					PhysicalAddress = (void *)((uintptr_t)PhysicalAddress + PAGE_SIZE_4K);
					Length -= PAGE_SIZE_4K;
				}

				return Virtual::MapType::FourKiB;
			}

			Virtual::MapType Type = Virtual::MapType::FourKiB;

			if (Length >= PAGE_SIZE_1G)
			{
				Type = Virtual::MapType::OneGiB;
				if (Length % PAGE_SIZE_1G != 0)
				{
					warn("Length is not a multiple of 1GB.");
					if (FailOnModulo)
						return Virtual::MapType::NoMapType;
				}
			}
			else if (Length >= PAGE_SIZE_4M)
			{
				Type = Virtual::MapType::FourMiB;
				if (Length % PAGE_SIZE_4M != 0)
				{
					warn("Length is not a multiple of 4MB.");
					if (FailOnModulo)
						return Virtual::MapType::NoMapType;
				}
			}
			else if (Length >= PAGE_SIZE_2M)
			{
				Type = Virtual::MapType::TwoMiB;
				if (Length % PAGE_SIZE_2M != 0)
				{
					warn("Length is not a multiple of 2MB.");
					if (FailOnModulo)
						return Virtual::MapType::NoMapType;
				}
			}

			this->Map(VirtualAddress, PhysicalAddress, Length, Flags, Type);
			return Type;
		}

		/**
		 * @brief Unmap page.
		 *
		 * @param VirtualAddress Virtual address of the page.
		 * @param Type Type of the page. Check MapType enum.
		 */
		void Unmap(void *VirtualAddress, MapType Type = MapType::FourKiB);

		/**
		 * @brief Unmap multiple pages.
		 *
		 * @param VirtualAddress First virtual address of the page.
		 * @param Length Length to map.
		 * @param Type Type of the page. Check MapType enum.
		 */
		__always_inline inline void Unmap(void *VirtualAddress, size_t Length, MapType Type = MapType::FourKiB)
		{
			int PageSize = PAGE_SIZE_4K;

			if (Type == MapType::TwoMiB)
				PageSize = PAGE_SIZE_2M;
			else if (Type == MapType::FourMiB)
				PageSize = PAGE_SIZE_4M;
			else if (Type == MapType::OneGiB)
				PageSize = PAGE_SIZE_1G;

			for (uintptr_t i = 0; i < Length; i += PageSize)
				this->Unmap((void *)((uintptr_t)VirtualAddress + i), Type);
		}

		/**
		 * @brief Remap page.
		 *
		 * @param VirtualAddress Virtual address of the page.
		 * @param PhysicalAddress Physical address of the page.
		 * @param Flags Flags of the page. Check PTFlag enum.
		 * @param Type Type of the page. Check MapType enum.
		 */
		__always_inline inline void Remap(void *VirtualAddress, void *PhysicalAddress, uint64_t Flags, MapType Type = MapType::FourKiB)
		{
			this->Unmap(VirtualAddress, Type);
			this->Map(VirtualAddress, PhysicalAddress, Flags, Type);
		}

		/**
		 * @brief Construct a new Virtual object
		 *
		 * @param Table Page table. If null, it will use the current page table.
		 */
		Virtual(PageTable *Table = nullptr);

		/**
		 * @brief Destroy the Virtual object
		 *
		 */
		~Virtual();
	};

	class StackGuard
	{
	private:
		struct AllocatedPages
		{
			void *PhysicalAddress;
			void *VirtualAddress;
		};

		void *StackBottom = nullptr;
		void *StackTop = nullptr;
		void *StackPhysicalBottom = nullptr;
		void *StackPhysicalTop = nullptr;
		uint64_t Size = 0;
		bool UserMode = false;
		bool Expanded = false;
		PageTable *Table = nullptr;
		std::vector<AllocatedPages> AllocatedPagesList;

	public:
		std::vector<AllocatedPages> GetAllocatedPages() { return AllocatedPagesList; }

		/** @brief Fork stack guard */
		void Fork(StackGuard *Parent);

		/** @brief For general info */
		uint64_t GetSize() { return Size; }

		/** @brief For general info */
		bool GetUserMode() { return UserMode; }

		/** @brief For general info */
		bool IsExpanded() { return Expanded; }

		/** @brief For general info */
		void *GetStackBottom() { return StackBottom; }

		/** @brief For RSP */
		void *GetStackTop() { return StackTop; }

		/** @brief For general info (avoid if possible)
		 * @note This can be used only if the stack was NOT expanded.
		 */
		void *GetStackPhysicalBottom()
		{
			if (Expanded)
				return nullptr;
			return StackPhysicalBottom;
		}

		/** @brief For general info (avoid if possible)
		 * @note This can be used only if the stack was NOT expanded.
		 */
		void *GetStackPhysicalTop()
		{
			if (Expanded)
				return nullptr;
			return StackPhysicalTop;
		}

		/** @brief Called by exception handler */
		bool Expand(uintptr_t FaultAddress);
		/**
		 * @brief Construct a new Stack Guard object
		 * @param User Stack for user mode?
		 */
		StackGuard(bool User, PageTable *Table);
		/**
		 * @brief Destroy the Stack Guard object
		 */
		~StackGuard();
	};

	class MemMgr
	{
	public:
		struct AllocatedPages
		{
			void *Address;
			std::size_t PageCount;
		};

		std::vector<AllocatedPages> GetAllocatedPagesList() { return AllocatedPagesList; }
		uint64_t GetAllocatedMemorySize();

		bool Add(void *Address, size_t Count);

		void *RequestPages(std::size_t Count, bool User = false);
		void FreePages(void *Address, size_t Count);

		void DetachAddress(void *Address);

		MemMgr(PageTable *Table = nullptr, VirtualFileSystem::Node *Directory = nullptr);
		~MemMgr();

	private:
		NewLock(MgrLock);
		Bitmap PageBitmap;
		PageTable *Table;
		VirtualFileSystem::Node *Directory;

		std::vector<AllocatedPages> AllocatedPagesList;
	};

	class SmartHeap
	{
	private:
		void *Object = nullptr;
		std::size_t ObjectSize = 0;

	public:
		auto GetObject() { return Object; }
		SmartHeap(std::size_t Size);
		~SmartHeap();

		void *operator->() { return Object; }
		void *operator*() { return Object; }
		operator void *() { return Object; }
		operator const char *()
		{
			return r_cst(const char *, Object);
		}
		void operator=(void *Address)
		{
			memcpy(Object, Address, ObjectSize);
		}
	};
}

void InitializeMemoryManagement();

void *operator new(std::size_t Size);
void *operator new[](std::size_t Size);
void *operator new(std::size_t Size, std::align_val_t Alignment);
void operator delete(void *Pointer);
void operator delete[](void *Pointer);
void operator delete(void *Pointer, long unsigned int Size);
void operator delete[](void *Pointer, long unsigned int Size);

extern Memory::Physical KernelAllocator;
extern Memory::PageTable *KernelPageTable;

#endif // __cplusplus

EXTERNC void *malloc(size_t Size);
EXTERNC void *calloc(size_t n, size_t Size);
EXTERNC void *realloc(void *Address, size_t Size);
EXTERNC void free(void *Address);

#define kmalloc(Size) malloc(Size)
#define kcalloc(n, Size) calloc(n, Size)
#define krealloc(Address, Size) realloc(Address, Size)
#define kfree(Address) free(Address)

#endif // !__FENNIX_KERNEL_INTERNAL_MEMORY_H__
