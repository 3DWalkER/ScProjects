function(sc_project_collect_files target_dir SC_PROJECT_HEADERS SC_PROJECT_SOURCES) 
    file(GLOB_RECURSE collected_header_files  
        LIST_DIRECTORIES false  
        "${target_dir}/*.h"  
    )  

    file(GLOB_RECURSE collected_source_files  
        LIST_DIRECTORIES false  
        "${target_dir}/*.cpp"  
    )  

    set(SC_PROJECT_HEADERS ${collected_header_files} PARENT_SCOPE)  
    set(SC_PROJECT_SOURCES ${collected_source_files} PARENT_SCOPE)

    if(MSVC)
        foreach(header_file ${collected_header_files})
            get_filename_component(header_dir ${header_file} DIRECTORY)
            file(RELATIVE_PATH relative_dir ${target_dir} ${header_dir})
            source_group("Header Files\\${relative_dir}" FILES ${header_file})
        endforeach()

        foreach(source_file ${collected_source_files})
            get_filename_component(source_dir ${source_file} DIRECTORY)
            file(RELATIVE_PATH relative_dir ${target_dir} ${source_dir})
            source_group("Source Files\\${relative_dir}" FILES ${source_file})
        endforeach()
    endif()
endfunction()

macro(sc_project_collect_dirs target_dir output_var)
	set(${output_var} "")

    file(GLOB_RECURSE _all_entries
        LIST_DIRECTORIES true
        RELATIVE ${target_dir}
        ${target_dir}/*
    )

    foreach(_entry ${_all_entries})
        set(_full_path ${target_dir}/${_entry})
        if(IS_DIRECTORY ${_full_path})
            list(APPEND ${output_var} ${_full_path})
        endif()
    endforeach()

    if(${output_var})
        list(REMOVE_DUPLICATES ${output_var})
        list(SORT ${output_var})
    endif()
endmacro()

macro(sc_project_add_includes target_dir scope)
    sc_project_collect_dirs(${target_dir} include_dirs)

    target_include_directories(${PROJECT_NAME}
        ${scope}
		${target_dir}
		${include_dirs}
    )
endmacro()