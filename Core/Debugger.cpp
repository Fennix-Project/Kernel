#include <debug.h>

#include <uart.hpp>
#include <printf.h>
#include <lock.hpp>

NewLock(DebuggerLock);

using namespace UniversalAsynchronousReceiverTransmitter;

static inline __no_instrument_function void uart_wrapper(char c, void *unused)
{
    UART(COM1).Write(c);
    UNUSED(unused);
}

static inline __no_instrument_function void WritePrefix(DebugLevel Level, const char *File, int Line, const char *Function)
{
    const char *DbgLvlString;
    switch (Level)
    {
    case DebugLevelError:
        DbgLvlString = "ERROR";
        break;
    case DebugLevelWarning:
        DbgLvlString = "WARN ";
        break;
    case DebugLevelInfo:
        DbgLvlString = "INFO ";
        break;
    case DebugLevelDebug:
        DbgLvlString = "DEBUG";
        break;
    case DebugLevelTrace:
        DbgLvlString = "TRACE";
        break;
    case DebugLevelFixme:
        DbgLvlString = "FIXME";
        break;
    case DebugLevelUbsan:
    {
        DbgLvlString = "UBSAN";
        fctprintf(uart_wrapper, nullptr, "%s|%s: ", DbgLvlString, Function);
        return;
    }
    default:
        DbgLvlString = "UNKNW";
        break;
    }
    fctprintf(uart_wrapper, nullptr, "%s|%s->%s:%d: ", DbgLvlString, File, Function, Line);
}

namespace SysDbg
{
    __no_instrument_function void Write(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...)
    {
        WritePrefix(Level, File, Line, Function);
        va_list args;
        va_start(args, Format);
        vfctprintf(uart_wrapper, nullptr, Format, args);
        va_end(args);
    }

    __no_instrument_function void WriteLine(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...)
    {
        // SmartLock(DebuggerLock);
        WritePrefix(Level, File, Line, Function);
        va_list args;
        va_start(args, Format);
        vfctprintf(uart_wrapper, nullptr, Format, args);
        va_end(args);
        uart_wrapper('\n', nullptr);
    }
}

// C compatibility
extern "C" __no_instrument_function void SysDbgWrite(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...)
{
    WritePrefix(Level, File, Line, Function);
    va_list args;
    va_start(args, Format);
    vfctprintf(uart_wrapper, nullptr, Format, args);
    va_end(args);
}

// C compatibility
extern "C" __no_instrument_function void SysDbgWriteLine(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...)
{
    WritePrefix(Level, File, Line, Function);
    va_list args;
    va_start(args, Format);
    vfctprintf(uart_wrapper, nullptr, Format, args);
    va_end(args);
    uart_wrapper('\n', nullptr);
}
