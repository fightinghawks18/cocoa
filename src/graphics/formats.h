#ifndef FORMATS_H
#define FORMATS_H

typedef enum ColorTypes {
    COLOR_RGBA_UNDEFINED,
    COLOR_RGBA8_UNORM,
    COLOR_BGRA8_UNORM,
    COLOR_BGRA8_SRGB,
    COLOR_RGBA8_SRGB,
    COLOR_RGBA16_SFLOAT,
    COLOR_RGBA32_SFLOAT
} ColorFormat;

typedef enum VertexTypes {
    VERTEX_FLOAT1,
    VERTEX_FLOAT2,
    VERTEX_FLOAT3,
    VERTEX_FLOAT4,
    VERTEX_INT1,
    VERTEX_INT2,
    VERTEX_INT3,
    VERTEX_INT4,
    VERTEX_UINT1,
    VERTEX_UINT2,
    VERTEX_UINT3,
    VERTEX_UINT4,
    VERTEX_UNORM4
} VertexFormat;

typedef enum DepthTypes {
    DEPTH_UNDEFINED,
    DEPTH32_SFLOAT,
    DEPTH24_UNORM_STENCIL8_UINT,
    DEPTH32_SFLOAT_STENCIL8_UINT
} DepthFormat;

void color_format_to_vk(ColorFormat format, int* vk_format);
void depth_format_to_vk(DepthFormat format, int* vk_format);
void vertex_format_to_vk(VertexFormat format, int* vk_format);

#endif // FORMATS_H
