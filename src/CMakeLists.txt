# Source files for Device Tree Explorer GUI
set(GUI_SOURCES
    main.cpp
    DeviceTree.cpp
    DeviceTreeParser.cpp
    DeviceTreeDiff.cpp
    MainWindow.cpp
    DeviceTreeWidgetItem.cpp
    PropertyTableWidget.cpp
    DiffViewerWidget.cpp
)

# Header files for MOC
set(GUI_HEADERS
    ../include/MainWindow.h
    ../include/PropertyTableWidget.h
    ../include/DiffViewerWidget.h
)

# Create the main GUI executable
add_executable(DeviceTreeExplorer ${GUI_SOURCES} ${GUI_HEADERS})

# Link Qt libraries
target_link_libraries(DeviceTreeExplorer
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)

# Link JSON and YAML libraries if available
if(nlohmann_json_FOUND)
    target_link_libraries(DeviceTreeExplorer nlohmann_json::nlohmann_json)
endif()

if(yaml-cpp_FOUND)
    target_link_libraries(DeviceTreeExplorer yaml-cpp)
endif()

# Include directories
target_include_directories(DeviceTreeExplorer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Set output directory
set_target_properties(DeviceTreeExplorer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
) 