cmake_minimum_required(VERSION 3.20)
project(cocoa C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

find_package(SDL3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED COMPONENTS glslangValidator)

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
        src/graphics/geometry.c
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

file(GLOB_RECURSE SHADER_FILES
    "${CMAKE_SOURCE_DIR}/content/*.vert"
    "${CMAKE_SOURCE_DIR}/content/*.frag"
    "${CMAKE_SOURCE_DIR}/content/*.comp"
    "${CMAKE_SOURCE_DIR}/content/*.geom"
    "${CMAKE_SOURCE_DIR}/content/*.tesc"
    "${CMAKE_SOURCE_DIR}/content/*.tese"
)

set(SPV_SHADERS "")
foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SPV_FILE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/content/${SHADER_NAME}.spv")
    
    add_custom_command(
        OUTPUT ${SPV_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/content"
        COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V ${SHADER} -o ${SPV_FILE}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${SHADER_NAME}"
    )
    
    list(APPEND SPV_SHADERS ${SPV_FILE})
endforeach()
add_custom_target(compile_shaders ALL DEPENDS ${SPV_SHADERS})

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