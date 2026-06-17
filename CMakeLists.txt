cmake_minimum_required(VERSION 3.30)
project(LearningOpenGLInAWeek
    LANGUAGES CXX C
)

# Libraries linked via full path != linker search path error
cmake_policy(SET CMP0003 NEW)
# Syntax problems are errors
cmake_policy(SET CMP0010 NEW)

# Removes unnecessary cmake files when generating MSVC project files
set(CMAKE_SUPPRESS_REGENERATION TRUE)

# =================================================================================================
# Build
# =================================================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

set(BINARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${BINARY_OUTPUT_DIRECTORY}/Archive")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BINARY_OUTPUT_DIRECTORY}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BINARY_OUTPUT_DIRECTORY}")

set(ROOT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "Root Directory")

# =================================================================================================
# Executable
# =================================================================================================

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

add_executable(LearningOpenGL ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(LearningOpenGL
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (MSVC)
    target_compile_options(LearningOpenGL
    PUBLIC
        /Zc:preprocessor # Needed for __VA_OPT__() macro
    )
endif()

# =================================================================================================
# Dependencies
# =================================================================================================

add_subdirectory(thirdparty/fmt)
add_subdirectory(thirdparty/SDL)
add_subdirectory(thirdparty/SDL_image)
add_subdirectory(thirdparty/glad)
add_subdirectory(thirdparty/glm)

target_link_libraries(LearningOpenGL
PRIVATE
    fmt::fmt
    SDL3::SDL3
    SDL3_image::SDL3_image
    glad
    glm::glm
)
