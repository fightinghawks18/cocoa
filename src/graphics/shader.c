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

static bool shader_read_shader_file(const char* path, ShaderSource* out_source) {
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open a valid shader file at %s!\n", path);
        return false;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (file_size < 0) {
        fprintf(stderr, "Failed to acquire file size from shader file %s!\n", path);
        fclose(file);
        return false;
    }

    char* code = (char*)malloc(file_size);
    if (code == NULL) {
        fprintf(stderr, "Failed to allocate buffer to read code size from shader file %s!\n", path);
        fclose(file);
        return false;
    }

    usize bytes_read = fread(code, 1, file_size, file);
    fclose(file);

    if (bytes_read != (usize)file_size) {
        fprintf(stderr, "Bytes being read doesn't match file size from shader file %s!\n", path);
        free(code);
        return false;
    }

    ShaderSource source = {
        .source = code,
        .size = bytes_read
    };
    *out_source = source;
    return true;
}

Shader* shader_from_file(Device* device, ShaderOptions options) {
    Shader* shader = malloc(sizeof(Shader));
    shader->type = options.type;
    shader->module = NULL;

    ShaderSource shader_source;
    bool shader_file_read = shader_read_shader_file(options.shader, &shader_source);
    if (!shader_file_read) {
        fprintf(stderr, "Failed to read shader file to source!\n");
        shader_free(device, shader);
        return NULL;
    }

    VkShaderModuleCreateInfo shader_module_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .codeSize = shader_source.size,
        .pCode = (const u32*)shader_source.source
    };
    
    VkResult shader_create = vkCreateShaderModule(device_get_vk_device(device), &shader_module_info, NULL, &shader->module);
    if (shader_create != VK_SUCCESS) {
        fprintf(stderr, "Failed to create vulkan shader module from shader file %s! %d\n", options.shader, shader_create);
        free(shader_source.source);
        shader_free(device, shader);
        return NULL;
    }
    free(shader_source.source);
    return shader;
}

Shader* shader_from_code(Device* device, ShaderOptions options) {
    Shader* shader = malloc(sizeof(Shader));
    shader->type = options.type;
    shader->module = NULL;

    VkShaderModuleCreateInfo shader_module_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .codeSize = strlen(options.shader),
        .pCode = (const u32*)options.shader,
    };

    VkResult shader_create = vkCreateShaderModule(device_get_vk_device(device), &shader_module_info, NULL, &shader->module);
    if (shader_create != VK_SUCCESS) {
        fprintf(stderr, "Failed to create vulkan shader module from shader source! %d\n", shader_create);
        shader_free(device, shader);
        return NULL;
    }
    return shader;
}

void shader_free(Device* device, Shader* shader) {
    if (shader->module) {
        vkDestroyShaderModule(device_get_vk_device(device), shader->module, NULL);
        shader->module = NULL;
    }
    free(shader);
}


VkShaderModule shader_get_vk_module(Shader* shader) {
    return shader->module;
}

ShaderType shader_get_type(Shader* shader) {
    return shader->type;
}
