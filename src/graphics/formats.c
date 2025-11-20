#include "formats.h"


static const VkFormat color_format_to_vk_format[] = {
    [COLOR_RGBA_UNDEFINED] = VK_FORMAT_UNDEFINED,
    [COLOR_RGBA8_UNORM] = VK_FORMAT_R8G8B8A8_UNORM,
    [COLOR_BGRA8_UNORM] = VK_FORMAT_B8G8R8A8_UNORM,
    [COLOR_RGBA8_SRGB] = VK_FORMAT_R8G8B8A8_SRGB,
    [COLOR_RGBA16_SFLOAT] = VK_FORMAT_R16G16B16A16_SFLOAT,
    [COLOR_RGBA32_SFLOAT] = VK_FORMAT_R32G32B32A32_SFLOAT
};

static const VkFormat depth_format_to_vk_format[] = {
    [DEPTH_UNDEFINED] = VK_FORMAT_UNDEFINED,
    [DEPTH32_SFLOAT] = VK_FORMAT_D32_SFLOAT,
    [DEPTH24_UNORM_STENCIL8_UINT] = VK_FORMAT_D24_UNORM_S8_UINT,
    [DEPTH32_SFLOAT_STENCIL8_UINT] = VK_FORMAT_D32_SFLOAT_S8_UINT
};

static const VkFormat vertex_format_to_vk_format[] = {
    [VERTEX_FLOAT1] = VK_FORMAT_R32_SFLOAT,
    [VERTEX_FLOAT2] = VK_FORMAT_R32G32_SFLOAT,
    [VERTEX_FLOAT3] = VK_FORMAT_R32G32B32_SFLOAT,
    [VERTEX_FLOAT4] = VK_FORMAT_R32G32B32A32_SFLOAT,
    [VERTEX_INT1] = VK_FORMAT_R32_SINT,
    [VERTEX_INT2] = VK_FORMAT_R32G32_SINT,
    [VERTEX_INT3] = VK_FORMAT_R32G32B32_SINT,
    [VERTEX_INT4] = VK_FORMAT_R32G32B32A32_SINT,
    [VERTEX_UINT1] = VK_FORMAT_R32_UINT,
    [VERTEX_UINT2] = VK_FORMAT_R32G32_UINT,
    [VERTEX_UINT3] = VK_FORMAT_R32G32B32_UINT,
    [VERTEX_UINT4] = VK_FORMAT_R32G32B32A32_UINT,
    [VERTEX_UNORM4] = VK_FORMAT_R8G8B8A8_UNORM,
};

VkFormat color_format_to_vk(ColorFormat format) {
    return color_format_to_vk_format[format];
}

VkFormat depth_format_to_vk(DepthFormat format) {
    return depth_format_to_vk_format[format];
}

VkFormat vertex_format_to_vk(VertexFormat format) {
    return vertex_format_to_vk_format[format];
}

