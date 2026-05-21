#ifndef SCMALLOCIMPL_H
#define SCMALLOCIMPL_H

#include "scutils/utils/scconfig.h"
#include "scutils/utils/scprocessordetection.h"

extern "C" {

#if SC_HAS_WEAK_SYMBOLS
#	if !defined(SC_OS_FREEBSD)
	void* mallocx(size_t, int) __attribute__((__nothrow__, __weak__));
	void* rallocx(void*, size_t, int) __attribute__((__nothrow__, __weak__));
	size_t xallocx(void*, size_t, size_t, int) __attribute__((__nothrow__, __weak__));
	size_t sallocx(const void*, int) __attribute__((__nothrow__, __weak__));
	void dallocx(void*, int) __attribute__((__nothrow__, __weak__));
	void sdallocx(void*, size_t, int) __attribute__((__nothrow__, __weak__));
	size_t nallocx(size_t, int) __attribute__((__nothrow__, __weak__));
	int mallctl(const char*, void*, size_t*, void*, size_t) __attribute__((__nothrow__, __weak__));
	int mallctlnametomib(const char*, size_t*, size_t*) __attribute__((__nothrow__, __weak__));
	int mallctlbymib(const size_t*, size_t, void*, size_t*, void*, size_t) __attribute__((__nothrow__, __weak__));
#	endif
	bool MallocExtension_Internal_GetNumericProperty(const char*, size_t, size_t*) __attribute__((__weak__));
#else
#	if (!defined(USE_JEMALLOC) && !defined(SC_USE_JEMALLOC))
	extern void* (*mallocx)(size_t, int);
	extern void* (*rallocx)(void*, size_t, int);
	extern size_t(*xallocx)(void*, size_t, size_t, int);
	extern size_t(*sallocx)(const void*, int);
	extern void (*dallocx)(void*, int);
	extern void (*sdallocx)(void*, size_t, int);
	extern size_t(*nallocx)(size_t, int);
	extern int (*mallctl)(const char*, void*, size_t*, void*, size_t);
	extern int (*mallctlnametomib)(const char*, size_t*, size_t*);
	extern int (*mallctlbymib)(const size_t*, size_t, void*, size_t*, void*, size_t);
#	endif
	extern bool (*MallocExtension_Internal_GetNumericProperty)(const char*, size_t, size_t*);

#ifdef SC_CC_MSVC
#	if defined(_M_IX86)
#pragma comment(linker, "/alternatename:_mallocx=_mallocxWeak")
#pragma comment(linker, "/alternatename:_rallocx=_rallocxWeak")
#pragma comment(linker, "/alternatename:_xallocx=_xallocxWeak")
#pragma comment(linker, "/alternatename:_sallocx=_sallocxWeak")
#pragma comment(linker, "/alternatename:_dallocx=_dallocxWeak")
#pragma comment(linker, "/alternatename:_sdallocx=_sdallocxWeak")
#pragma comment(linker, "/alternatename:_nallocx=_nallocxWeak")
#pragma comment(linker, "/alternatename:_mallctl=_mallctlWeak")
#pragma comment(linker, "/alternatename:_mallctlnametomib=_mallctlnametomibWeak")
#pragma comment(linker, "/alternatename:_mallctlbymib=_mallctlbymibWeak")
#pragma comment(linker, "/alternatename:_MallocExtension_Internal_GetNumericProperty=_MallocExtension_Internal_GetNumericPropertyWeak")
#	else
#pragma comment(linker, "/alternatename:mallocx=mallocxWeak")
#pragma comment(linker, "/alternatename:rallocx=rallocxWeak")
#pragma comment(linker, "/alternatename:xallocx=xallocxWeak")
#pragma comment(linker, "/alternatename:sallocx=sallocxWeak")
#pragma comment(linker, "/alternatename:dallocx=dallocxWeak")
#pragma comment(linker, "/alternatename:sdallocx=sdallocxWeak")
#pragma comment(linker, "/alternatename:nallocx=nallocxWeak")
#pragma comment(linker, "/alternatename:mallctl=mallctlWeak")
#pragma comment(linker, "/alternatename:mallctlnametomib=mallctlnametomibWeak")
#pragma comment(linker, "/alternatename:mallctlbymib=mallctlbymibWeak")
#pragma comment(linker, "/alternatename:MallocExtension_Internal_GetNumericProperty=MallocExtension_Internal_GetNumericPropertyWeak")
#	endif
#endif
#endif
}

#endif // SCMALLOCIMPL_H