#ifndef SHADER_H
#define SHADER_H

#include "device.h"

typedef enum {
    SHADER_VERTEX,
    SHADER_FRAGMENT
} ShaderType;
typedef struct {
    const char* shader;
    ShaderType type;
} ShaderOptions;

typedef struct Shader Shader;

Shader* shader_from_file(Device* device, ShaderOptions options);
Shader* shader_from_code(Device* device, ShaderOptions options);
void shader_free(Device* device, Shader* shader);

VkShaderModule shader_get_vk_module(Shader* shader);
ShaderType shader_get_type(Shader* shader);

#endif // SHADER_H
