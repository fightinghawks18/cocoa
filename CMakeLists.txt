cmake_minimum_required(VERSION 3.20)
project(cocoa C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

find_package(SDL3 CONFIG REQUIRED)

add_executable(Cocoa
        main.c
)

target_link_libraries(Cocoa
        PUBLIC
        SDL3::SDL3
)

if(APPLE)
    target_link_libraries(Cocoa 
            PRIVATE
            "-framework AppKit"
    )
endif()

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  add_custom_target(copy_compiled_commands ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
            ${CMAKE_CURRENT_BINARY_DIR}/../
            COMMENT "Copying compiled commands to ${CMAKE_CURRENT_BINARY_DIR}/../"
  )
endif()