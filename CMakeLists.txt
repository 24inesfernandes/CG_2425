cmake_minimum_required(VERSION 3.10)
project(Engine3D)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files for the engine
set(ENGINE_SOURCES
    engine/engine.cpp
    engine/camera.cpp
    engine/parser.cpp
)

# Add source file for the generator
set(GENERATOR_SOURCES
    generator/generator.cpp
)

set(OpenGL_GL_PREFERENCE GLVND)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

# Include directories
include_directories(
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add TinyXML2 library source directly
add_library(tinyxml2 STATIC engine/tinyxml2.cpp)

# Create the engine executable
add_executable(engine ${ENGINE_SOURCES})
target_link_libraries(engine
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    tinyxml2
)

# Create the generator executable
add_executable(generator ${GENERATOR_SOURCES})

# Copy models directory to build directory
add_custom_command(
    TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    ${CMAKE_CURRENT_BINARY_DIR}/files3d
    COMMENT "Creating 3D files directory"
)

# Set working directory for running the engine
set_target_properties(engine PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

# Define installation rules
install(TARGETS engine generator
    RUNTIME DESTINATION bin
)

# Add an option to enable/disable debugging
option(DEBUG_MODE "Enable debugging" OFF)
if(DEBUG_MODE)
    add_definitions(-DDEBUG)
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

# Print some configuration information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "OPENGL_LIBRARIES: ${OPENGL_LIBRARIES}")
message(STATUS "GLUT_LIBRARIES: ${GLUT_LIBRARIES}")