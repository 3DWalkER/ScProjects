scsa_add_static("Core" ${SCSA_CORE_PRO} "")

option(SC_ENABLE_DEBUG_OUTPUT   "Enable HYK Carrier Detection Library"            OFF)

# Add compile definitions
target_compile_definitions(${PROJECT_NAME} 
    PUBLIC 
    $<$<BOOL:${SC_ENABLE_DEBUG_OUTPUT}>:SC_ENABLE_DEBUG_OUTPUT>
)

find_package(ScUtils REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_LIBS} ScUtils::ScUtils)