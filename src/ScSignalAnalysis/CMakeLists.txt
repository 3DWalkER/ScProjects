# Define root paths for dependency libraries
set(SCSA_LIBRARY_DIR "${CMAKE_CURRENT_LIST_DIR}/CommSrc/library")

# Register myself CMake config path to prefix path
list(APPEND CMAKE_PREFIX_PATH "${SCSA_LIBRARY_DIR}/GJS/lib/cmake")

# Append local custom cmake modules path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CommSrc/cmake")

# Include essential common utility and initialization scripts
include(scsa_project_common)
include(scsa_projects_initialize)

# Initialize tracking lists for unique projects and dependencies
set(SCSA_PROJECT_BUILD_LIST "")
set(SCSA_DEPENDENCY_BUILD_LIST "")

# Iterate through all discovered/defined projects to parse explicit metadata
foreach(TARGET IN LISTS SC_PROJECTS)
	message(STATUS "Considering target: ${TARGET}")

	scsa_get_dependencies(${TARGET} DEPENDENCY_LIST)
	if(DEPENDENCY_LIST)
		STRING(REPLACE "," ";" LIST_RESULT ${DEPENDENCY_LIST})
		foreach(DEPENDENCY IN LISTS LIST_RESULT)
			list(APPEND SCSA_DEPENDENCY_BUILD_LIST ${DEPENDENCY})
		endforeach()
	endif()

	scsa_get_project_dir(${TARGET} DEPENDENCY_LIST)
	if(DEPENDENCY_LIST)
		STRING(REPLACE "," ";" LIST_RESULT ${DEPENDENCY_LIST})
		foreach(DEPENDENCY IN LISTS LIST_RESULT)
			list(APPEND SCSA_PROJECT_BUILD_LIST ${DEPENDENCY})
		endforeach()
	endif()
endforeach()

# Safely remove duplicate entries to prevent redundant target definitions
if(SCSA_PROJECT_BUILD_LIST)
    list(REMOVE_DUPLICATES SCSA_PROJECT_BUILD_LIST)
endif()

if(SCSA_DEPENDENCY_BUILD_LIST)
    list(REMOVE_DUPLICATES SCSA_DEPENDENCY_BUILD_LIST)
endif()

# Process and add required companion dependency modules
if(SCSA_DEPENDENCY_BUILD_LIST)
    message(STATUS "Adding signal analysis dependent modules...")
    
    foreach(DEP_MODULE IN LISTS SCSA_DEPENDENCY_BUILD_LIST)
        # Include module subdirectory into the build system tree
        add_subdirectory("CommSrc/${DEP_MODULE}")
        
        # Organize targets within Visual Studio / CLion solution folders
        # Using the standard matching target name to avoid directory-property racing conditions
        if(TARGET ${DEP_MODULE})
            set_property(TARGET ${DEP_MODULE} PROPERTY FOLDER "${SC_PROJECT_SA}")
        else()
            # Fallback check for dynamic build-system target lookup if names mismatch
            get_directory_property(_SUB_TARGETS DIRECTORY "CommSrc/${DEP_MODULE}" BUILDSYSTEM_TARGETS)
            foreach(_TARGET IN LISTS _SUB_TARGETS)
                set_property(TARGET ${_TARGET} PROPERTY FOLDER "${SC_PROJECT_SA}")
            endforeach()
        endif()
    endforeach()
    
    message(STATUS "Signal analysis dependent modules added successfully.")
endif()

# Process and add core executable/library projects
if(SCSA_PROJECT_BUILD_LIST)
    message(STATUS "Adding signal analysis projects...")
    
    foreach(PROJ_DIR IN LISTS SCSA_PROJECT_BUILD_LIST)
        add_subdirectory(${PROJ_DIR})
    endforeach()
    
    message(STATUS "Signal analysis projects added successfully.")
else()
    message(WARNING "No signal analysis projects have been added.")
endif()