# Minimum CMake version required
cmake_minimum_required(VERSION 3.15)

# Project Name
project(node-based-image-processor LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

message(STATUS "Configuring ${PROJECT_NAME}...")

# --- Find Required Packages ---

# Find OpenCV
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs) # Add components you use
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found. Set OpenCV_DIR or ensure it's in PATH.")
else()
    message(STATUS "Found OpenCV ${OpenCV_VERSION} (Include: ${OpenCV_INCLUDE_DIRS})")
endif()

# Find GLFW
find_package(glfw3 REQUIRED)
if(NOT glfw3_FOUND)
    message(FATAL_ERROR "GLFW3 not found. Ensure it's installed and findable by CMake.")
else()
     message(STATUS "Found GLFW3 ${glfw3_VERSION_STRING}")
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)
if(NOT OpenGL_FOUND)
    message(FATAL_ERROR "OpenGL not found.")
else()
    message(STATUS "Found OpenGL ${OPENGL_GL_VERSION_STRING}")
endif()

# --- Define External Source Locations ---
set(EXTERNALS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals)
set(APP_FRAMEWORK_DIR ${EXTERNALS_DIR}/application)
set(IMGUI_DIR ${EXTERNALS_DIR}/imgui)
set(IMGUI_NODE_EDITOR_DIR ${EXTERNALS_DIR}/imgui-node-editor)
set(GLEW_DIR ${EXTERNALS_DIR}/glew)
set(STB_IMAGE_DIR ${EXTERNALS_DIR}/stb_image)

# --- Define Project Source Locations ---
set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/node-based-image-processor)
set(NODE_EDITOR_DIR ${PROJECT_ROOT_DIR}/node-editor)
set(NODE_IMPL_DIR ${NODE_EDITOR_DIR}/nodes) # Assuming implementations are directly in nodes/

# --- List Source Files ---
message(STATUS "Gathering source files...")
set(PROJECT_SOURCES
    # Main Project sources
    ${PROJECT_ROOT_DIR}/main.cpp
    ${PROJECT_ROOT_DIR}/ImageEditorApp.cpp

    # Node Editor Core sources
    ${NODE_EDITOR_DIR}/ImageDataManager.cpp
    ${NODE_EDITOR_DIR}/Node.cpp
    ${NODE_EDITOR_DIR}/NodeEditorManager.cpp

    # Node Implementations sources (List explicitly)
    ${NODE_IMPL_DIR}/BlendNode.cpp
    ${NODE_IMPL_DIR}/BlurNode.cpp
    ${NODE_IMPL_DIR}/BrightnessContrastNode.cpp
    ${NODE_IMPL_DIR}/ColorChannelSplitterNode.cpp
    ${NODE_IMPL_DIR}/ConvolutionFilterNode.cpp
    ${NODE_IMPL_DIR}/EdgeDetectionNode.cpp
    ${NODE_IMPL_DIR}/InputNode.cpp
    ${NODE_IMPL_DIR}/NoiseGenerationNode.cpp
    ${NODE_IMPL_DIR}/OutputNode.cpp
    ${NODE_IMPL_DIR}/ThresholdNode.cpp

    # Application Framework sources
    ${APP_FRAMEWORK_DIR}/application.cpp
    ${APP_FRAMEWORK_DIR}/entry_point.cpp
    ${APP_FRAMEWORK_DIR}/imgui_impl_glfw.cpp
    ${APP_FRAMEWORK_DIR}/imgui_impl_opengl3.cpp
    ${APP_FRAMEWORK_DIR}/imgui_impl_win32.cpp
    ${APP_FRAMEWORK_DIR}/platform_glfw.cpp
    ${APP_FRAMEWORK_DIR}/platform_win32.cpp
    ${APP_FRAMEWORK_DIR}/renderer_ogl3.cpp

    # ImGui sources
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp

    # ImGui-Node-Editor sources
    ${IMGUI_NODE_EDITOR_DIR}/crude_json.cpp
    ${IMGUI_NODE_EDITOR_DIR}/imgui_canvas.cpp
    ${IMGUI_NODE_EDITOR_DIR}/imgui_node_editor_api.cpp
    ${IMGUI_NODE_EDITOR_DIR}/imgui_node_editor.cpp
)

# --- Add Executable Target ---
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# --- Include Directories ---
message(STATUS "Setting include directories...")
target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_ROOT_DIR}
    ${NODE_EDITOR_DIR}
    ${NODE_IMPL_DIR}
    ${APP_FRAMEWORK_DIR}
    ${OpenCV_INCLUDE_DIRS}    # From find_package
    ${IMGUI_DIR}
    ${IMGUI_NODE_EDITOR_DIR}
    ${GLEW_DIR}/include       # Assuming standard include path for GLEW
    ${STB_IMAGE_DIR}         # For stb_image.h
    # GLFW include is usually handled by find_package(glfw3) target
)

# --- Link Libraries ---
message(STATUS "Linking libraries...")
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${OpenCV_LIBS}          # From find_package
    glfw                    # Target from find_package(glfw3)
    OpenGL::GL              # Target from find_package(OpenGL)
)

# --- Platform Specific (Windows) ---
if(WIN32)
    message(STATUS "Adding Windows specific libraries...")
    target_link_libraries(${PROJECT_NAME} PUBLIC
        gdi32               # Needed by GLFW/ImGui?
        imm32               # For IME support
        xinput9_1_0         # For XInput gamepad support (or try xinput)
    )
endif()

# --- Compile Definitions ---
message(STATUS "Setting compile definitions...")
target_compile_definitions(${PROJECT_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS
    # STB_IMAGE_IMPLEMENTATION should be defined in ONE .cpp file (e.g., application.cpp)
)

# --- Post-Build: Copy Data Directory (Example - uncomment and adjust path if needed) ---
# set(SOURCE_DATA_DIR ${PROJECT_ROOT_DIR}/data) # Adjust this path if your data folder is elsewhere
# set(OUTPUT_DATA_DIR $<TARGET_FILE_DIR:${PROJECT_NAME}>/data)
#
# if(EXISTS ${SOURCE_DATA_DIR})
#     message(STATUS "Adding post-build step to copy data directory from ${SOURCE_DATA_DIR} to ${OUTPUT_DATA_DIR}")
#     add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_directory "${SOURCE_DATA_DIR}" "${OUTPUT_DATA_DIR}"
#         COMMENT "Copying data directory (fonts, etc.)"
#     )
# else()
#     message(WARNING "Source data directory not found, skipping copy: ${SOURCE_DATA_DIR}")
# endif()

message(STATUS "CMake configuration finished for ${PROJECT_NAME}")
