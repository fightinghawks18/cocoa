#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

typedef struct Shader {
    VkShaderModule module;
    ShaderType type;
} Shader;

typedef struct {
    char* source;
    int size;
} ShaderSource;

static ShaderResult shader_read_shader_file(const char* path, ShaderSource* out_source) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open a valid shader file at %s!\n", path);
        return SHADER_ERROR_FILE_OPEN_FAIL;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (file_size < 0) {
        fprintf(stderr, "Failed to acquire file size from shader file %s!\n", path);
        fclose(file);
        return SHADER_ERROR_FILE_SIZE_READ;
    }

    char* code = (char*)malloc(file_size);
    if (code == NULL) {
        fprintf(stderr, "Failed to allocate buffer to read code size from shader file %s!\n", path);
        fclose(file);
        return SHADER_ERROR_FILE_READ_SIZE_BUFFER_ALLOC_FAIL;
    }

    usize bytes_read = fread(code, 1, file_size, file);
    fclose(file);

    if (bytes_read != (usize)file_size) {
        fprintf(stderr, "Bytes being read doesn't match file size from shader file %s!\n", path);
        free(code);
        return SHADER_ERROR_BYTES_AND_FILE_SIZE_MISMATCH;
    }

    ShaderSource source = {
        .source = code,
        .size = bytes_read
    };
    *out_source = source;
    return SHADER_OK;
}

ShaderResult shader_new(Device* device, ShaderOptions options, Shader** out_shader) {
    void* device_handle = NULL;
    device_get_device(device, &device_handle);

    Shader* shader = malloc(sizeof(Shader));
    shader->type = options.type;
    shader->module = NULL;

    ShaderSource shader_source;
    ShaderResult shader_file_read = shader_read_shader_file(options.shader, &shader_source);
    if (shader_file_read != SHADER_OK) {
        fprintf(stderr, "Failed to read shader file to source!\n");
        shader_free(device, shader);
        return shader_file_read;
    }

    VkShaderModuleCreateInfo shader_module_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .codeSize = shader_source.size,
        .pCode = (const u32*)shader_source.source
    };
    
    VkResult shader_create = vkCreateShaderModule(device_handle, &shader_module_info, NULL, &shader->module);
    if (shader_create != VK_SUCCESS) {
        fprintf(stderr, "Failed to create vulkan shader module from shader file %s! %d\n", options.shader, shader_create);
        free(shader_source.source);
        shader_free(device, shader);
        return SHADER_ERROR_CREATE_HANDLE_FAIL;
    }
    free(shader_source.source);

    *out_shader = shader;
    return SHADER_OK;
}

void shader_free(Device* device, Shader* shader) {
    void* device_handle = NULL;
    device_get_device(device, &device_handle);

    if (shader->module) {
        vkDestroyShaderModule(device_handle, shader->module, NULL);
        shader->module = NULL;
    }
    free(shader);
}


void shader_get_module(Shader* shader, void** out_module) {
    *out_module = shader->module;
}

void shader_get_type(Shader* shader, ShaderType* out_type) {
    *out_type = shader->type;
}
