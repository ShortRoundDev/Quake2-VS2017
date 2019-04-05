/* Replace LIBXML1 by the name of the DLL */

#ifndef __IMPORT_LIBXML1_DLL_H__
#define __IMPORT_LIBXML1_DLL_H__

#include <_dll_import.h>

#if defined(BUILD_LIBXML1_DLL) || !defined(__WIN32__)
#define DLL_LIBXML1_IMPORT extern
#else
#define DLL_LIBXML1_IMPORT __DLL_IMPORT 
#endif

#endif /* __IMPORT_LIBXML1_DLL_H__ */
