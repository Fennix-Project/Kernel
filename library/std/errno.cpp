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

#include <errno.h>

#include <types.h>
#include <debug.h>
#include <atomic>

#include "../../kernel.h"

__aligned(16) static int errno_value = 0;

EXTERNC int *__errno_location(void)
{
	if (unlikely(!TaskManager || !thisThread))
		return &errno_value;
	return &thisThread->ErrorNumber;
}

const char *strerror(int errnum)
{
	if (errnum < 0)
		errnum = -errnum;

	switch (errnum)
	{
	case 0:
		return "Success";
	case EPERM:
		return "Operation not permitted";
	case ENOENT:
		return "No such file or directory";
	case ESRCH:
		return "No such process";
	case EINTR:
		return "Interrupted system call";
	case EIO:
		return "Input/output error";
	case ENXIO:
		return "No such device or address";
	case E2BIG:
		return "Argument list too long";
	case ENOEXEC:
		return "Exec format error";
	case EBADF:
		return "Bad file descriptor";
	case ECHILD:
		return "No child processes";
	case EAGAIN:
		return "Resource temporarily unavailable";
	case ENOMEM:
		return "Cannot allocate memory";
	case EACCES:
		return "Permission denied";
	case EFAULT:
		return "Bad address";
	case ENOTBLK:
		return "Block device required";
	case EBUSY:
		return "Device or resource busy";
	case EEXIST:
		return "File exists";
	case EXDEV:
		return "Invalid cross-device link";
	case ENODEV:
		return "No such device";
	case ENOTDIR:
		return "Not a directory";
	case EISDIR:
		return "Is a directory";
	case EINVAL:
		return "Invalid argument";
	case ENFILE:
		return "Too many open files in system";
	case EMFILE:
		return "Too many open files";
	case ENOTTY:
		return "Inappropriate ioctl for device";
	case ETXTBSY:
		return "Text file busy";
	case EFBIG:
		return "File too large";
	case ENOSPC:
		return "No space left on device";
	case ESPIPE:
		return "Illegal seek";
	case EROFS:
		return "Read-only file system";
	case EMLINK:
		return "Too many links";
	case EPIPE:
		return "Broken pipe";
	case EDOM:
		return "Numerical argument out of domain";
	case ERANGE:
		return "Numerical result out of range";
	case EDEADLK:
		return "Resource deadlock avoided";
	case ENAMETOOLONG:
		return "File name too long";
	case ENOLCK:
		return "No locks available";
	case ENOSYS:
		return "Function not implemented";
	case ENOTEMPTY:
		return "Directory not empty";
	case ELOOP:
		return "Too many levels of symbolic links";
	case ENOMSG:
		return "No message of desired type";
	case EIDRM:
		return "Identifier removed";
	case ECHRNG:
		return "Channel number out of range";
	case EL2NSYNC:
		return "Level 2 not synchronized";
	case EL3HLT:
		return "Level 3 halted";
	case EL3RST:
		return "Level 3 reset";
	case ELNRNG:
		return "Link number out of range";
	case EUNATCH:
		return "Protocol driver not attached";
	case ENOCSI:
		return "No CSI structure available";
	case EL2HLT:
		return "Level 2 halted";
	case EBADE:
		return "Invalid exchange";
	case EBADR:
		return "Invalid request descriptor";
	case EXFULL:
		return "Exchange full";
	case ENOANO:
		return "No anode";
	case EBADRQC:
		return "Invalid request code";
	case EBADSLT:
		return "Invalid slot";
	case EBFONT:
		return "Bad font file format";
	case ENOSTR:
		return "Device not a stream";
	case ENODATA:
		return "No data available";
	case ETIME:
		return "Timer expired";
	case ENOSR:
		return "Out of streams resources";
	case ENONET:
		return "Machine is not on the network";
	case ENOPKG:
		return "Package not installed";
	case EREMOTE:
		return "Object is remote";
	case ENOLINK:
		return "Link has been severed";
	case EADV:
		return "Advertise error";
	case ESRMNT:
		return "Srmount error";
	case ECOMM:
		return "Communication error on send";
	case EPROTO:
		return "Protocol error";
	case EMULTIHOP:
		return "Multihop attempted";
	case EDOTDOT:
		return "RFS specific error";
	case EBADMSG:
		return "Bad message";
	case EOVERFLOW:
		return "Value too large for defined data type";
	case ENOTUNIQ:
		return "Name not unique on network";
	case EBADFD:
		return "File descriptor in bad state";
	case EREMCHG:
		return "Remote address changed";
	case ELIBACC:
		return "Can not access a needed shared library";
	case ELIBBAD:
		return "Accessing a corrupted shared library";
	case ELIBSCN:
		return ".lib section in a.out corrupted";
	case ELIBMAX:
		return "Attempting to link in too many shared libraries";
	case ELIBEXEC:
		return "Cannot exec a shared library directly";
	case EILSEQ:
		return "Illegal byte sequence";
	case ERESTART:
		return "Interrupted system call should be restarted";
	case ESTRPIPE:
		return "Streams pipe error";
	case EUSERS:
		return "Too many users";
	case ENOTSOCK:
		return "Socket operation on non-socket";
	case EDESTADDRREQ:
		return "Destination address required";
	case EMSGSIZE:
		return "Message too long";
	case EPROTOTYPE:
		return "Protocol wrong type for socket";
	case ENOPROTOOPT:
		return "Protocol not available";
	case EPROTONOSUPPORT:
		return "Protocol not supported";
	case ESOCKTNOSUPPORT:
		return "Socket type not supported";
	case EOPNOTSUPP:
		return "Operation not supported";
	case EPFNOSUPPORT:
		return "Protocol family not supported";
	case EAFNOSUPPORT:
		return "Address family not supported by protocol";
	case EADDRINUSE:
		return "Address already in use";
	case EADDRNOTAVAIL:
		return "Cannot assign requested address";
	case ENETDOWN:
		return "Network is down";
	case ENETUNREACH:
		return "Network is unreachable";
	case ENETRESET:
		return "Network dropped connection on reset";
	case ECONNABORTED:
		return "Software caused connection abort";
	case ECONNRESET:
		return "Connection reset by peer";
	case ENOBUFS:
		return "No buffer space available";
	case EISCONN:
		return "Transport endpoint is already connected";
	case ENOTCONN:
		return "Transport endpoint is not connected";
	case ESHUTDOWN:
		return "Cannot send after transport endpoint shutdown";
	case ETOOMANYREFS:
		return "Too many references: cannot splice";
	case ETIMEDOUT:
		return "Connection timed out";
	case ECONNREFUSED:
		return "Connection refused";
	case EHOSTDOWN:
		return "Host is down";
	case EHOSTUNREACH:
		return "No route to host";
	case EALREADY:
		return "Operation already in progress";
	case EINPROGRESS:
		return "Operation now in progress";
	case ESTALE:
		return "Stale file handle";
	case EUCLEAN:
		return "Structure needs cleaning";
	case ENOTNAM:
		return "Not a XENIX named type file";
	case ENAVAIL:
		return "No XENIX semaphores available";
	case EISNAM:
		return "Is a named type file";
	case EREMOTEIO:
		return "Remote I/O error";
	case EDQUOT:
		return "Quota exceeded";
	case ENOMEDIUM:
		return "No medium found";
	case EMEDIUMTYPE:
		return "Wrong medium type";
	case ECANCELED:
		return "Operation Canceled";
	case ENOKEY:
		return "Required key not available";
	case EKEYEXPIRED:
		return "Key has expired";
	case EKEYREVOKED:
		return "Key has been revoked";
	case EKEYREJECTED:
		return "Key was rejected by service";
	case EOWNERDEAD:
		return "Owner died";
	case ENOTRECOVERABLE:
		return "State not recoverable";
	default:
		return "Unknown error";
	}
}