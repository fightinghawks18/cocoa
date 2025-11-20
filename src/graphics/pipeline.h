#ifndef PIPELINE_H
#define PIPELINE_H

#include "device.h"
#include "shader.h"
#include "formats.h"
#include "../int_types.h"

typedef enum {
    VERTEX,
    INSTANCE
} PipelineInputRate;

typedef struct {
    u32 binding;
    u32 stride;
    PipelineInputRate rate;
} PipelineInputBinding;

typedef struct {
    u32 location;
    u32 binding;
    ColorFormat format;
    u32 offset;
} PipelineInputAttribute;

typedef struct {
    PipelineInputBinding* bindings;
    PipelineInputAttribute* attributes;
    u32 binding_count;
    u32 attribute_count;
} PipelineVertexInfo;

typedef struct {
    Shader* shaders;
    u32 shader_count;
} PipelineShaderInfo;

typedef enum PipelinePrimitive {
    TOPOLOGY_TRIANGLE_STRIP
} PipelineTopology;

typedef struct {
    PipelineTopology topology;
    bool primitive_restart;
} PipelineInputAssemblyInfo;

typedef enum PipelinePolygonMode {
    POLYGON_FILL,
    POLYGON_LINE,
    POLYGON_POINT
} PipelinePolygonMode;

typedef enum PipelineFrontFace {
    FRONT_FACING_C_CLOCKWISE,
    FRONT_FACING_CLOCKWISE
} PipelineFrontFaceDirection;

typedef struct {
    bool depth_clamping;
    bool discard_prims_until_rasterization;
    PipelinePolygonMode polygon_mode;
    PipelineFrontFaceDirection front_face_direction;
    bool bias_fragment_depth;
    f32 depth_bias_factor;
    f32 depth_bias_clamp;
    f32 depth_bias_slope;
    f32 line_width;
} PipelineRasterizationInfo;

typedef enum PipelineSampleCountFlags {
    PIPELINE_SAMPLECOUNT1 = 1,
    PIPELINE_SAMPLECOUNT2 = 2 << 0,
    PIPELINE_SAMPLECOUNT4 = 4 << 0,
    PIPELINE_SAMPLECOUNT8 = 8 << 0,
    PIPELINE_SAMPLECOUNT16 = 1 << 1,
    PIPELINE_SAMPLECOUNT32 = 2 << 1,
    PIPELINE_SAMPLECOUNT64 = 4 << 1,
} PipelineSamplingFlags;

typedef struct {
    PipelineSamplingFlags sample_flag;
} PipelineMultisampleInfo;

typedef struct Pipeline Pipeline;

Pipeline* pipeline_new(Device* device, ColorFormat format);
void pipeline_free(Device* device, Pipeline* pipeline);

VkPipeline pipeline_get_vk_pipeline(Pipeline* pipeline);

#endif // PIPELINE_H
