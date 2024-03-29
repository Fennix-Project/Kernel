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

#ifndef __FENNIX_KERNEL_UBSAN_H__
#define __FENNIX_KERNEL_UBSAN_H__

#include <types.h>

struct source_location
{
	const char *file;
	uint32_t line;
	uint32_t column;
};

struct type_descriptor
{
	uint16_t kind;
	uint16_t info;
	char name[];
};

struct type_mismatch_v1_data
{
	struct source_location location;
	struct type_descriptor *type;
	uint8_t alignment;
	uint8_t type_check_kind;
};

struct out_of_bounds_info
{
	struct source_location location;
	struct type_descriptor left_type;
	struct type_descriptor right_type;
};

struct overflow_data
{
	struct source_location location;
	struct type_descriptor *type;
};

struct negative_vla_data
{
	struct source_location location;
	struct type_descriptor *type;
};

struct invalid_value_data
{
	struct source_location location;
	struct type_descriptor *type;
};

struct nonnull_return_data
{
	struct source_location location;
};

struct nonnull_arg_data
{
	struct source_location location;
};

struct unreachable_data
{
	struct source_location location;
};

struct invalid_builtin_data
{
	struct source_location location;
	uint8_t kind;
};

struct array_out_of_bounds_data
{
	struct source_location location;
	struct type_descriptor *array_type;
	struct type_descriptor *index_type;
};

struct shift_out_of_bounds_data
{
	struct source_location location;
	struct type_descriptor *left_type;
	struct type_descriptor *right_type;
};

struct dynamic_type_cache_miss_data
{
	struct source_location location;
	struct type_descriptor *type;
};

#endif // !__FENNIX_KERNEL_UBSAN_H__
