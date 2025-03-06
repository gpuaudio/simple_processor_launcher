# Component name
set(component_name iir_launcher)

# Unit tests
set(exec_name ${component_name})

# process file executable
add_executable(${component_name})

# Source files
target_sources(${component_name} PRIVATE
    src/iir_launcher.cpp
)

# Include directories
target_include_directories(${component_name} PRIVATE
    ../include
    ../common/include
    include
)

target_compile_definitions(${component_name} PRIVATE
    ${win_common_private_compile_definitions}
    BUILD_TYPE="$<CONFIG>"
)

# Link libraries
target_link_libraries(${component_name} PRIVATE
    ProcLaunchLib
)

set_property(TARGET ${component_name} PROPERTY COMPILE_WARNING_AS_ERROR OFF)
