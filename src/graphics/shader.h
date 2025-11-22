#ifndef SHADER_H
#define SHADER_H

#include "device.h"

typedef struct Shader Shader;

typedef enum {
    SHADER_VERTEX,
    SHADER_FRAGMENT
} ShaderType;

typedef struct {
    const char* shader;
    const char* name;
    const char* entry_point;
    ShaderType type;
} ShaderOptions;

typedef enum {
    SHADER_OK, // Successfully created a shader
    SHADER_ERROR_FILE_OPEN_FAIL, // Failed to open the shader file
    SHADER_ERROR_FILE_SIZE_READ, // Failed to read the shader file size
    SHADER_ERROR_FILE_READ_SIZE_BUFFER_ALLOC_FAIL, // Failed to allocate buffer that reads the shader's code size
    SHADER_ERROR_BYTES_AND_FILE_SIZE_MISMATCH, // The code size and file size do not match
    SHADER_ERROR_CREATE_HANDLE_FAIL, // Failed to create the handle for the shader
} ShaderResult;

ShaderResult shader_new(Device* device, ShaderOptions options, Shader** out_shader);
void shader_free(Device* device, Shader* shader);

void shader_get_module(Shader* shader, void** out_module);
void shader_get_type(Shader* shader, ShaderType* out_type);

#endif // SHADER_H
