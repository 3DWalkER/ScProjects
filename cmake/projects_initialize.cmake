#[[
	Function: sc_projects_add_target
	Description:
		Automatically collects source files and creates a target 
	(executable or library) with custom properties and dependencies.

	Arguments:
		- name			: Name of the target project.
		- target_type	: Target type. Valid options: EXECUTABLE, STATIC, SHARED, LIBRARY.
						(Note: LIBRARY falls back to CMake's default library type).
		- description   : Description of the project, stored in target property 'DESCRIPTION'.
		- src_dir		: Relative path to the source directory.
		- [ARGN]		: Optional list of libraries to link against.
]]
macro(sc_projects_add_target name target_type description src_dir)
    set(PROJECT_NAME "${name}")
    set(PROJECT_DESCRIPTION "${description}")
    set(PROJECT_LIBS "${ARGN}")

	# Validate target type parameter
    set(VALID_TYPES "EXECUTABLE" "STATIC" "SHARED" "LIBRARY")
    if(NOT ${target_type} IN_LIST VALID_TYPES)
        message(FATAL_ERROR "Invalid target_type '${target_type}'. Must be one of: EXECUTABLE, STATIC, SHARED, LIBRARY.")
    endif()

	# Specify the project suffix under Debug configuration
    if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
        set(CMAKE_DEBUG_POSTFIX "d")
    endif()

	# Collect source and header files
    sc_project_collect_files("${src_dir}" SC_PROJECT_HEADERS SC_PROJECT_SOURCES)
    
    # Check if files were successfully collected
    if(NOT SC_PROJECT_HEADERS AND NOT SC_PROJECT_SOURCES)
        message(FATAL_ERROR "No source or header files found for ${PROJECT_NAME} in '${src_dir}'.")
    endif()

	# Define target based on the chosen type
    if(${target_type} STREQUAL "EXECUTABLE")
        add_executable(${PROJECT_NAME} ${SC_PROJECT_SOURCES} ${SC_PROJECT_HEADERS})
    elseif(${target_type} STREQUAL "STATIC")
        add_library(${PROJECT_NAME} STATIC ${SC_PROJECT_SOURCES} ${SC_PROJECT_HEADERS})
    elseif(${target_type} STREQUAL "SHARED")
        add_library(${PROJECT_NAME} SHARED ${SC_PROJECT_SOURCES} ${SC_PROJECT_HEADERS})
    else()
        # Fallback to standard library (type determined by BUILD_SHARED_LIBS)
        add_library(${PROJECT_NAME} ${SC_PROJECT_SOURCES} ${SC_PROJECT_HEADERS})
    endif()

	# Store project description inside a target property
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        DESCRIPTION "${PROJECT_DESCRIPTION}"
    )
	
	sc_project_add_includes(${src_dir} PUBLIC)

    # Link dependencies if provided
    if(PROJECT_LIBS)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_LIBS})
    endif()
endmacro()


#[[
	Function: sc_projects_add_executable
	Description:
		Automatically collects source files and creates an executable 
	target with a custom description and dependencies.

	Arguments:
		- name			: Name of the executable target.
		- description   : Description of the project, stored in target property 'DESCRIPTION'.
		- src_dir		: Relative path to the source directory.
		- [ARGN]		: Optional list of libraries to link against.
]]
macro(sc_projects_add_executable name description src_dir)
	sc_projects_add_target(
        "${name}" 
        "EXECUTABLE" 
        "${description}" 
        "${src_dir}" 
        ${ARGN}
    )
endmacro()


#[[
	Function: sc_projects_add_static
	Description:
		Automatically collects source files and creates a static 
	target with a custom description and dependencies.

	Arguments:
		- name			: Name of the executable target.
		- description   : Description of the project, stored in target property 'DESCRIPTION'.
		- src_dir		: Relative path to the source directory.
		- [ARGN]		: Optional list of libraries to link against.
]]
macro(sc_projects_add_static name description src_dir)
	sc_projects_add_target(
        "${name}" 
        "STATIC" 
        "${description}" 
        "${src_dir}" 
        ${ARGN}
    )
endmacro()