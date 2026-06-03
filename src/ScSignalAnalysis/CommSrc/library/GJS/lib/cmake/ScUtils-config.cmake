if(ScUtils_FOUND)
	return()
endif()

if(NOT TARGET ScUtils::ScUtils)
	include("${CMAKE_CURRENT_LIST_DIR}/ScUtils-targets.cmake")
endif()

# Compute the installation prefix relative to this file.
get_filename_component(SC_SDK_DEFAULT_ROOT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(SC_SDK_DEFAULT_ROOT_DIR "${SC_SDK_DEFAULT_ROOT_DIR}" PATH)
get_filename_component(SC_SDK_DEFAULT_ROOT_DIR "${SC_SDK_DEFAULT_ROOT_DIR}" PATH)
get_filename_component(SC_SDK_DEFAULT_ROOT_DIR "${SC_SDK_DEFAULT_ROOT_DIR}" PATH)
if(SC_SDK_DEFAULT_ROOT_DIR STREQUAL "/")
	seet(SC_SDK_DEFAULT_ROOT_DIR "")
endif()

set(ScUtils_FOUND TRUE)
set(ScUtils_INCLUDE_DIRS "${SC_SDK_DEFAULT_ROOT_DIR}/include")

set(ScUtils_TARGETS ScUtils::ScUtils)
