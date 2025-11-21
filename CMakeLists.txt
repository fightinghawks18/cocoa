cmake_minimum_required(VERSION 3.20)
project(cocoa C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

find_package(SDL3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(Cocoa
        src/main.c
        src/game/game.c
        src/graphics/swapchain.c
        src/graphics/renderer.c
        src/graphics/device.c
        src/graphics/formats.c
        src/graphics/pipeline.c
        src/graphics/pipeline_layout.c
        src/graphics/shader.c
        src/graphics/buffer.c
)

target_link_libraries(Cocoa
        PUBLIC
        SDL3::SDL3
        Vulkan::Vulkan
)

if(APPLE)
   target_link_libraries(Cocoa 
        PRIVATE
        "-framework AppKit"
   )
endif()

add_custom_target(copy_content_to_binary ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/content
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/content
        COMMENT "Copying content from ${CMAKE_SOURCE_DIR}/content to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
   add_custom_target(copy_compiled_commands ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
                ${CMAKE_CURRENT_BINARY_DIR}/../
        COMMENT "Copying compiled commands to ${CMAKE_CURRENT_BINARY_DIR}/../"
   )
endif()