/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is MPEG4IP.
 * 
 * The Initial Developer of the Original Code is Cisco Systems Inc.
 * Portions created by Cisco Systems Inc. are
 * Copyright (C) Cisco Systems Inc. 2000, 2001.  All Rights Reserved.
 * 
 * Contributor(s): 
 *		Dave Mackie		dmackie@cisco.com
 *		Bill May		wmay@cisco.com
 */

#ifdef WIN32
#define HAVE_IN_PORT_T
#define HAVE_SOCKLEN_T
#else
#include <config.h>
#endif

#ifndef __SYSTEMS_H__
#define __SYSTEMS_H__



#ifdef WIN32

#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int64 u_int64_t;
typedef unsigned __int32 u_int32_t;
typedef unsigned __int16 u_int16_t;
typedef unsigned char u_int8_t;
typedef __int64 int64_t;
typedef __int32 int32_t;
typedef __int16 int16_t;
typedef __int8  int8_t;
typedef unsigned short in_port_t;
typedef unsigned int socklen_t;
typedef int ssize_t;
#define snprintf _snprintf
#define strncasecmp _strnicmp
#define strcasecmp _stricmp

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#define write _write
#define lseek _lseek
#define close _close
#define open _open
#define access _access
#define F_OK 0
#define O_RDWR _O_RDWR
#define O_CREAT _O_CREAT
#define O_RDONLY _O_RDONLY
#define srandom srand
#define random rand

#define IOSBINARY ios::binary

#ifdef __cplusplus
extern "C" {
#endif
char *strsep(char **strp, const char *delim); 
int gettimeofday(struct timeval *t, void *);

#ifdef __cplusplus
}
#endif

#define PATH_MAX MAX_PATH
#define MAX_UINT64 -1
#define LLD "%I64d"
#define LLU "%I64u"
#define LLX "%I64x"
#define M_LLU 1000i64
#define C_LLU 100i64
#define I_LLU 1i64

#define LOG_EMERG 0
#define LOG_ALERT 1
#define LOG_CRIT 2
#define LOG_ERR 3
#define LOG_WARNING 4
#define LOG_NOTICE 5
#define LOG_INFO 6
#define LOG_DEBUG 7

#if     !__STDC__ && _INTEGRAL_MAX_BITS >= 64
#define VAR_TO_FPOS(fpos, var) (fpos) = (var)
#define FPOS_TO_VAR(fpos, typed, var) (var) = (typed)(_FPOSOFF(fpos))
#else
#define VAR_TO_FPOS(fpos, var) (fpos).lopart = ((var) & UINT_MAX); (fpos).hipart = ((var) >> 32)
#define FPOS_TO_VAR(fpos, typed, var) (var) = (typed)((uint64_t)((fpos).hipart ) << 32 | (fpos).lopart)
#endif

#define __STRING(expr) #expr

#define FOPEN_READ_BINARY "rb"
#define FOPEN_WRITE_BINARY "wb"

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#else /* UNIX */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#else
#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#error "Don't have stdint.h or inttypes.h - no way to get uint8_t"
#endif
#endif

#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <syslog.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/param.h>

#define closesocket close
#define IOSBINARY ios::bin
#define MAX_UINT64 -1LLU
#define LLD "%lld"
#define LLU "%llu"
#define LLX "%llx"
#define M_LLU 1000LLU
#define C_LLU 100LLU
#define I_LLU 1LLU
#ifdef HAVE_FPOS_T_POS
#define FPOS_TO_VAR(fpos, typed, var) (var) = (typed)((fpos).__pos)
#define VAR_TO_FPOS(fpos, var) (fpos).__pos = (var)
#else
#define FPOS_TO_VAR(fpos, typed, var) (var) = (typed)(fpos)
#define VAR_TO_FPOS(fpos, var) (fpos) = (var)
#endif

#define FOPEN_READ_BINARY "r"
#define FOPEN_WRITE_BINARY "w"
#endif /* define unix */

#include <stdarg.h>
typedef void (*error_msg_func_t)(int loglevel,
				 const char *lib,
				 const char *fmt,
				 va_list ap);

#ifndef HAVE_IN_PORT_T
typedef uint16_t in_port_t;
#endif

#ifndef HAVE_SOCKLEN_T
typedef unsigned int socklen_t;
#endif


#endif /* __SYSTEMS_H__ */
