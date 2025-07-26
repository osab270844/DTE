# Source files for Device Tree Explorer CLI
set(CLI_SOURCES
    main.cpp
    ../src/DeviceTree.cpp
    ../src/DeviceTreeParser.cpp
    ../src/DeviceTreeDiff.cpp
    CLIApp.cpp
)

# Create the CLI executable
add_executable(dte-cli ${CLI_SOURCES})

# Link JSON and YAML libraries if available
if(nlohmann_json_FOUND)
    target_link_libraries(dte-cli nlohmann_json::nlohmann_json)
endif()

if(yaml-cpp_FOUND)
    target_link_libraries(dte-cli yaml-cpp)
endif()

# Include directories
target_include_directories(dte-cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Set output directory
set_target_properties(dte-cli PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
) 