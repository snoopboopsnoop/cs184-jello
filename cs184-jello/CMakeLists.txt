cmake_minimum_required(VERSION 3.16)
project(cs184-jello)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECT_NAME cs184-jello)

################################################################################
# Source groups (for IDE organization only)
################################################################################
set(Header_Files
        "../src/camera.h"
        "../src/mesh.h"
        "../src/model.h"
        "../src/shader.h"
        "../src/stb_image.h"
        #"../src/physobj.h"
        "../glad/include/glad/glad.h"        # Fixed: removed /src/
        "../glad/include/KHR/khrplatform.h" # Fixed: removed /src/
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
        "../glad/src/glad.c"
        "../src/main.cpp"
        "../src/stb_image.cpp"
)

################################################################################
# Add executable target with only source files
################################################################################
add_executable(${PROJECT_NAME} ${Source_Files})

################################################################################
# Add headers and shaders to the target for IDE visibility (not compiled)
################################################################################
target_sources(${PROJECT_NAME} PRIVATE
        ${Header_Files}
)

################################################################################
# Include directories so compiler can find glad and other headers
################################################################################
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../glad/include/glad  # Point directly to glad folder
        ${CMAKE_CURRENT_SOURCE_DIR}/../glad/include/KHR   # Point directly to KHR folder
        ${CMAKE_CURRENT_SOURCE_DIR}/../glad/include       # General glad include
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}/
)

# Add this line for macOS OpenGL deprecation warnings
target_compile_definitions(${PROJECT_NAME} PRIVATE GL_SILENCE_DEPRECATION)

################################################################################
# Platform-specific linking and definitions
################################################################################
if(MSVC)
    use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
    set(ROOT_NAMESPACE cs184jello)

    set_target_properties(${PROJECT_NAME} PROPERTIES
            VS_GLOBAL_KEYWORD "Win32Proj"
    )

    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64" OR "${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        set_target_properties(${PROJECT_NAME} PROPERTIES
                INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
        )
    endif()

    target_compile_definitions(${PROJECT_NAME} PRIVATE
            "$<$<CONFIG:Debug>:_DEBUG>"
            "$<$<CONFIG:Release>:NDEBUG>"
            "_CONSOLE"
            "UNICODE"
            "_UNICODE"
    )

    target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:/Oi;/Gy>
            /permissive-
            /sdl
            /W3
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )

    target_link_options(${PROJECT_NAME} PRIVATE
            /DEBUG
            /SUBSYSTEM:CONSOLE
    )

    set(ADDITIONAL_LIBRARY_DEPENDENCIES
            "$<$<CONFIG:Release>:assimp-vc143-mtd>"
            glfw3
            opengl32
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")
endif()

if(APPLE)
    find_package(OpenGL REQUIRED)
    find_package(glfw3 REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ASSIMP REQUIRED assimp)

    target_compile_options(${PROJECT_NAME} PRIVATE
            -Wno-deprecated-declarations
    )

    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            OpenGL::GL
            glfw
            ${ASSIMP_LIBRARIES}
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
    )

    target_link_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/Cellar/assimp/6.0.2/lib)
endif()