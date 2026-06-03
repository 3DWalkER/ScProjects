# ------------------------------------------------------------------------------
# Determine the current system platform
# ------------------------------------------------------------------------------
# Unified processor architecture name
if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(i.86)")
    set(SC_SYSTEM_ARCH "x86_32")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64)|(AMD64)|(amd64)")
    set(SC_SYSTEM_ARCH "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "(aarch64)|(arm64)")
    set(SC_SYSTEM_ARCH "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    set(SC_SYSTEM_ARCH "arm32")
endif()

# Splicing operating system prefixes
if(WIN32)
    set(SC_SYSTEM_ARCH "win_${SC_SYSTEM_ARCH}")
elseif(UNIX AND NOT APPLE)
    set(SC_SYSTEM_ARCH "linux_${SC_SYSTEM_ARCH}")
endif()

# 
message(STATUS "Detected SC_SYSTEM_ARCH: ${SC_SYSTEM_ARCH}")
