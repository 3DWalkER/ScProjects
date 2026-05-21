#ifndef SCGLOBAL_H
#   include "scutils/scglobal.h"
#endif

#ifndef SCSYSTEMDETECTION_H
#define SCSYSTEMDETECTION_H

/*
   The operating system, must be one of: (SC_OS_x)

     DARWIN   - Any Darwin system (macOS, iOS, watchOS, tvOS)
     MACOS    - macOS
     IOS      - iOS
     WATCHOS  - watchOS
     TVOS     - tvOS
     WIN32    - Win32 (Windows 2000/XP/Vista/7 and Windows Server 2003/2008)
     WINRT    - WinRT (Windows Runtime)
     CYGWIN   - Cygwin
     SOLARIS  - Sun Solaris
     HPUX     - HP-UX
     LINUX    - Linux [has variants]
     FREEBSD  - FreeBSD [has variants]
     NETBSD   - NetBSD
     OPENBSD  - OpenBSD
     INTERIX  - Interix
     AIX      - AIX
     HURD     - GNU Hurd
     QNX      - QNX [has variants]
     QNX6     - QNX RTP 6.1
     LYNX     - LynxOS
     BSD4     - Any BSD 4.4 system
     UNIX     - Any UNIX BSD/SYSV system
     ANDROID  - Android platform
     HAIKU    - Haiku

   The following operating systems have variants:
     LINUX    - both SC_OS_LINUX and SC_OS_ANDROID are defined when building for Android
              - only SC_OS_LINUX is defined if building for other Linux systems
     FREEBSD  - SC_OS_FREEBSD is defined only when building for FreeBSD with a BSD userland
              - SC_OS_FREEBSD_KERNEL is always defined on FreeBSD, even if the userland is from GNU
*/

#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#  include <TargetConditionals.h>
#  if defined(TARGET_OS_MAC) && TARGET_OS_MAC
#    define SC_OS_DARWIN
#    define SC_OS_BSD4
#    ifdef __LP64__
#      define SC_OS_DARWIN64
#    else
#      define SC_OS_DARWIN32
#    endif
#    if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#      if defined(TARGET_OS_WATCH) && TARGET_OS_WATCH
#        define SC_OS_WATCHOS
#      elif defined(TARGET_OS_TV) && TARGET_OS_TV
#        define SC_OS_TVOS
#      else
#        // TARGET_OS_IOS is only available in newer SDKs,
#        // so assume any other iOS-based platform is iOS for now
#        define SC_OS_IOS
#      endif
#    else
#      // TARGET_OS_OSX is only available in newer SDKs,
#      // so assume any non iOS-based platform is macOS for now
#      define SC_OS_MACOS
#    endif
#  else
#    error "Qt has not been ported to this Apple platform - see http://www.qt.io/developers"
#  endif
#elif defined(__ANDROID__) || defined(ANDROID)
#  define SC_OS_ANDROID
#  define SC_OS_LINUX
#elif defined(__CYGWIN__)
#  define SC_OS_CYGWIN
#elif !defined(SAG_COM) && (!defined(WINAPI_FAMILY) || WINAPI_FAMILY==WINAPI_FAMILY_DESKTOP_APP) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#  define SC_OS_WIN32
#  define SC_OS_WIN64
#elif !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#  if defined(WINAPI_FAMILY)
#    ifndef WINAPI_FAMILY_PC_APP
#      define WINAPI_FAMILY_PC_APP WINAPI_FAMILY_APP
#    endif
#    if defined(WINAPI_FAMILY_PHONE_APP) && WINAPI_FAMILY==WINAPI_FAMILY_PHONE_APP
#      define SC_OS_WINRT
#    elif WINAPI_FAMILY==WINAPI_FAMILY_PC_APP
#      define SC_OS_WINRT
#    else
#      define SC_OS_WIN32
#    endif
#  else
#    define SC_OS_WIN32
#  endif
#elif defined(__sun) || defined(sun)
#  define SC_OS_SOLARIS
#elif defined(hpux) || defined(__hpux)
#  define SC_OS_HPUX
#elif defined(__native_client__)
#  define SC_OS_NACL
#elif defined(__EMSCRIPTEN__)
#  define SC_OS_WASM
#elif defined(__linux__) || defined(__linux)
#  define SC_OS_LINUX
#elif defined(__FreeBSD__) || defined(__DragonFly__) || defined(__FreeBSD_kernel__)
#  ifndef __FreeBSD_kernel__
#    define SC_OS_FREEBSD
#  endif
#  define SC_OS_FREEBSD_KERNEL
#  define SC_OS_BSD4
#elif defined(__NetBSD__)
#  define SC_OS_NETBSD
#  define SC_OS_BSD4
#elif defined(__OpenBSD__)
#  define SC_OS_OPENBSD
#  define SC_OS_BSD4
#elif defined(__INTERIX)
#  define SC_OS_INTERIX
#  define SC_OS_BSD4
#elif defined(_AIX)
#  define SC_OS_AIX
#elif defined(__Lynx__)
#  define SC_OS_LYNX
#elif defined(__GNU__)
#  define SC_OS_HURD
#elif defined(__QNXNTO__)
#  define SC_OS_QNX
#elif defined(__INTEGRITY)
#  define SC_OS_INTEGRITY
#elif defined(__rtems__)
#  define SC_OS_RTEMS
#elif defined(VXWORKS) /* there is no "real" VxWorks define - this has to be set in the mkspec! */
#  define SC_OS_VXWORKS
#elif defined(__HAIKU__)
#  define SC_OS_HAIKU
#elif defined(__MAKEDEPEND__)
#else
#  error "Qt has not been ported to this OS - see http://www.qt-project.org/"
#endif

#if defined(SC_OS_WIN32) || defined(SC_OS_WIN64) || defined(SC_OS_WINRT)
#  define SC_OS_WINDOWS
#  define SC_OS_WIN
#endif

#if defined(SC_OS_WIN)
#  undef SC_OS_UNIX
#elif !defined(SC_OS_UNIX)
#  define SC_OS_UNIX
#endif

// Compatibility synonyms
#ifdef SC_OS_DARWIN
#define SC_OS_MAC
#endif
#ifdef SC_OS_DARWIN32
#define SC_OS_MAC32
#endif
#ifdef SC_OS_DARWIN64
#define SC_OS_MAC64
#endif
#ifdef SC_OS_MACOS
#define SC_OS_MACX
#define SC_OS_OSX
#endif

#ifdef SC_OS_DARWIN
#  include <Availability.h>
#  include <AvailabilityMacros.h>
#
#  ifdef SC_OS_MACOS
#    if !defined(__MAC_OS_X_VERSION_MIN_REQUIRED) || __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_6
#       undef __MAC_OS_X_VERSION_MIN_REQUIRED
#       define __MAC_OS_X_VERSION_MIN_REQUIRED __MAC_10_6
#    endif
#    if !defined(MAC_OS_X_VERSION_MIN_REQUIRED) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_6
#       undef MAC_OS_X_VERSION_MIN_REQUIRED
#       define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_6
#    endif
#  endif
#
#  // Numerical checks are preferred to named checks, but to be safe
#  // we define the missing version names in case Qt uses them.
#
#  if !defined(__MAC_10_11)
#       define __MAC_10_11 101100
#  endif
#  if !defined(__MAC_10_12)
#       define __MAC_10_12 101200
#  endif
#  if !defined(__MAC_10_13)
#       define __MAC_10_13 101300
#  endif
#  if !defined(__MAC_10_14)
#       define __MAC_10_14 101400
#  endif
#  if !defined(__MAC_10_15)
#       define __MAC_10_15 101500
#  endif
#  if !defined(MAC_OS_X_VERSION_10_11)
#       define MAC_OS_X_VERSION_10_11 __MAC_10_11
#  endif
#  if !defined(MAC_OS_X_VERSION_10_12)
#       define MAC_OS_X_VERSION_10_12 __MAC_10_12
#  endif
#  if !defined(MAC_OS_X_VERSION_10_13)
#       define MAC_OS_X_VERSION_10_13 __MAC_10_13
#  endif
#  if !defined(MAC_OS_X_VERSION_10_14)
#       define MAC_OS_X_VERSION_10_14 __MAC_10_14
#  endif
#  if !defined(MAC_OS_X_VERSION_10_15)
#       define MAC_OS_X_VERSION_10_15 __MAC_10_15
#  endif
#
#  if !defined(__IPHONE_10_0)
#       define __IPHONE_10_0 100000
#  endif
#  if !defined(__IPHONE_10_1)
#       define __IPHONE_10_1 100100
#  endif
#  if !defined(__IPHONE_10_2)
#       define __IPHONE_10_2 100200
#  endif
#  if !defined(__IPHONE_10_3)
#       define __IPHONE_10_3 100300
#  endif
#  if !defined(__IPHONE_11_0)
#       define __IPHONE_11_0 110000
#  endif
#  if !defined(__IPHONE_12_0)
#       define __IPHONE_12_0 120000
#  endif
#endif

#ifdef __LSB_VERSION__
#  if __LSB_VERSION__ < 40
#    error "This version of the Linux Standard Base is unsupported"
#  endif
#endif

#endif // SCSYSTEMDETECTION_H
