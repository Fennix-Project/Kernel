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

#include <kconfig.hpp>

#include <convert.h>
#include <cargs.h>
#include <printf.h>
#include <targp.h>

#include "kernel.h"

static struct cag_option ConfigOptions[] = {
	{.identifier = 'a',
	 .access_letters = "aA",
	 .access_name = "alloc",
	 .value_name = "TYPE",
	 .description = "Memory allocator to use"},

	{.identifier = 'c',
	 .access_letters = "cC",
	 .access_name = "cores",
	 .value_name = "VALUE",
	 .description = "Number of cores to use (0 = all; 1 is the first code, not 0)"},

	{.identifier = 'p',
	 .access_letters = "pP",
	 .access_name = "ioapicirq",
	 .value_name = "VALUE",
	 .description = "Which core will be used for I/O APIC interrupts"},

	{.identifier = 't',
	 .access_letters = "tT",
	 .access_name = "tasking",
	 .value_name = "MODE",
	 .description = "Tasking mode (multi, single)"},

	{.identifier = 'd',
	 .access_letters = "dD",
	 .access_name = "drvdir",
	 .value_name = "PATH",
	 .description = "Directory to load drivers from"},

	{.identifier = 'i',
	 .access_letters = "iI",
	 .access_name = "init",
	 .value_name = "PATH",
	 .description = "Path to init program"},

	{.identifier = 'y',
	 .access_letters = "yY",
	 .access_name = "linux",
	 .value_name = "BOOL",
	 .description = "Use Linux syscalls by default"},

	{.identifier = 'l',
	 .access_letters = NULL,
	 .access_name = "udl",
	 .value_name = "BOOL",
	 .description = "Unlock the deadlock after 10 retries"},

	{.identifier = 'o',
	 .access_letters = NULL,
	 .access_name = "ioc",
	 .value_name = "BOOL",
	 .description = "Enable Interrupts On Crash. If enabled, the navigation keys will be enabled on crash"},

	{.identifier = 's',
	 .access_letters = NULL,
	 .access_name = "simd",
	 .value_name = "BOOL",
	 .description = "Enable SIMD instructions"},

	{.identifier = 'b',
	 .access_letters = NULL,
	 .access_name = "quiet",
	 .value_name = "BOOL",
	 .description = "Enable quiet boot"},

	{.identifier = 'h',
	 .access_letters = "h",
	 .access_name = "help",
	 .value_name = NULL,
	 .description = "Show help on screen and halt"}};

void ParseConfig(char *ConfigString, KernelConfig *ModConfig)
{
	if (ConfigString == NULL ||
		strlen(ConfigString) == 0)
	{
		KPrint("Empty kernel parameters!");
		return;
	}

	if (ModConfig == NULL)
	{
		KPrint("ModConfig is NULL!");
		return;
	}

	KPrint("Kernel parameters: %s", ConfigString);
	debug("Kernel parameters: %s", ConfigString);

	char *argv[32];
	int argc = 0;
	/* Not sure if the quotes are being parsed correctly. */
	targp_parse(ConfigString, argv, &argc);

#ifdef DEBUG
	for (int i = 0; i < argc; i++)
		debug("argv[%d] = %s", i, argv[i]);
	debug("argc = %d", argc);
#endif

	char identifier;
	const char *value;
	cag_option_context context;

	cag_option_prepare(&context, ConfigOptions,
					   CAG_ARRAY_SIZE(ConfigOptions), argc, argv);

	while (cag_option_fetch(&context))
	{
		identifier = cag_option_get(&context);
		switch (identifier)
		{
		case 'a':
		{
			value = cag_option_get_value(&context);
			if (strcmp(value, "xallocv1") == 0)
			{
				KPrint("\eAAFFAAUsing XallocV1 as memory allocator");
				ModConfig->AllocatorType = Memory::XallocV1;
			}
			else if (strcmp(value, "xallocv2") == 0)
			{
				KPrint("\eAAFFAAUsing XallocV2 as memory allocator");
				ModConfig->AllocatorType = Memory::XallocV2;
			}
			else if (strcmp(value, "liballoc11") == 0)
			{
				KPrint("\eAAFFAAUsing Liballoc11 as memory allocator");
				ModConfig->AllocatorType = Memory::liballoc11;
			}
			else if (strcmp(value, "pages") == 0)
			{
				KPrint("\eAAFFAAUsing Pages as memory allocator");
				ModConfig->AllocatorType = Memory::Pages;
			}
			break;
		}
		case 'c':
		{
			value = cag_option_get_value(&context);
			KPrint("\eAAFFAAUsing %s cores", atoi(value) ? value : "all");
			ModConfig->Cores = atoi(value);
			break;
		}
		case 'p':
		{
			value = cag_option_get_value(&context);
			KPrint("\eAAFFAARedirecting I/O APIC interrupts to %s%s",
				   atoi(value) ? "core " : "", atoi(value) ? value : "BSP");
			ModConfig->IOAPICInterruptCore = atoi(value);
			break;
		}
		case 't':
		{
			value = cag_option_get_value(&context);
			if (strcmp(value, "multi") == 0)
			{
				KPrint("\eAAFFAAUsing Multi-Tasking Scheduler");
				ModConfig->SchedulerType = Multi;
			}
			else if (strcmp(value, "single") == 0)
			{
				KPrint("\eAAFFAAUsing Single-Tasking Scheduler");
				ModConfig->SchedulerType = Mono;
			}
			else
			{
				KPrint("\eAAFFAAUnknown scheduler: %s", value);
				ModConfig->SchedulerType = Multi;
			}
			break;
		}
		case 'd':
		{
			value = cag_option_get_value(&context);
			strncpy(ModConfig->DriverDirectory, value, strlen(value));
			KPrint("\eAAFFAAUsing %s as module directory", value);
			break;
		}
		case 'i':
		{
			value = cag_option_get_value(&context);
			strncpy(ModConfig->InitPath, value, strlen(value));
			KPrint("\eAAFFAAUsing %s as init program", value);
			break;
		}
		case 'y':
		{
			value = cag_option_get_value(&context);
			strcmp(value, "true") == 0
				? ModConfig->UseLinuxSyscalls = true
				: ModConfig->UseLinuxSyscalls = false;
			KPrint("\eAAFFAAUse Linux syscalls by default: %s", value);
			break;
		}
		case 'o':
		{
			value = cag_option_get_value(&context);
			strcmp(value, "true") == 0
				? ModConfig->InterruptsOnCrash = true
				: ModConfig->InterruptsOnCrash = false;
			KPrint("\eAAFFAAInterrupts on crash: %s", value);
			break;
		}
		case 'l':
		{
			value = cag_option_get_value(&context);
			strcmp(value, "true") == 0
				? ModConfig->UnlockDeadLock = true
				: ModConfig->UnlockDeadLock = false;
			KPrint("\eAAFFAAUnlocking the deadlock after 10 retries");
			break;
		}
		case 's':
		{
			value = cag_option_get_value(&context);
			strcmp(value, "true") == 0 ? ModConfig->SIMD = true
									   : ModConfig->SIMD = false;
			KPrint("\eAAFFAASingle Instruction, Multiple Data (SIMD): %s", value);
			break;
		}
		case 'b':
		{
			value = cag_option_get_value(&context);
			strcmp(value, "true") == 0 ? ModConfig->Quiet = true
									   : ModConfig->Quiet = false;
			KPrint("\eAAFFAAQuiet boot: %s", value);
			break;
		}
		case 'h':
		{
			KPrint("\n---------------------------------------------------------------------------\nUsage: fennix.elf [OPTION]...\nKernel configuration.");
			cag_option_print(ConfigOptions, CAG_ARRAY_SIZE(ConfigOptions),
							 nullptr);
			KPrint("\eFF2200System Halted.");
			CPU::Stop();
		}
		default:
		{
			KPrint("\eFF2200Unknown option: %c", identifier);
			break;
		}
		}
	}
	debug("Config loaded");
}
