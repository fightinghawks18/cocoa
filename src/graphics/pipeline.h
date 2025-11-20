#ifndef PIPELINE_H
#define PIPELINE_H

#include "device.h"
#include "shader.h"
#include "formats.h"
#include "../int_types.h"

typedef enum {
    INPUT_VERTEX,
    INPUT_INSTANCE
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
} PipelineVertexOptions;

typedef struct {
    Shader** shaders;
    u32 shader_count;
} PipelineShaderOptions;

typedef enum PipelinePrimitive {
    TOPOLOGY_TRIANGLE_STRIP
} PipelineTopology;

typedef struct {
    PipelineTopology topology;
    bool primitive_restart;
} PipelineInputAssemblyOptions;

typedef enum PipelinePolygonMode {
    POLYGON_FILL,
    POLYGON_LINE,
    POLYGON_POINT
} PipelinePolygonMode;

typedef enum PipelineCullMode {
    CULL_NONE,
    CULL_FRONT,
    CULL_BACK,
    CULL_BOTH
} PipelineCullMode;

typedef enum PipelineFrontFace {
    FRONT_FACING_C_CLOCKWISE,
    FRONT_FACING_CLOCKWISE
} PipelineFrontFaceDirection;

typedef struct {
    bool depth_clamping;
    bool discard_prims_until_rasterization;
    PipelinePolygonMode polygon_mode;
    PipelineCullMode cull_mode;
    PipelineFrontFaceDirection front_face_direction;
    bool bias_fragment_depth;
    f32 depth_bias_factor;
    f32 depth_bias_clamp;
    f32 depth_bias_slope;
    f32 line_width;
} PipelineRasterizationOptions;

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
    bool sample_shading;
    f32 min_sample_shading;
    void* sample_masks;
    bool alpha_to_coverage;
    bool alpha_to_one;
} PipelineMultisampleOptions;

typedef enum PipelineStencilOperation {
    STENCIL_OP_KEEP,
    STENCIL_OP_ZERO,
    STENCIL_OP_REPLACE,
    STENCIL_OP_INCREMENT_CLAMP,
    STENCIL_OP_DECREMENT_CLAMP,
    STENCIL_OP_INVERT,
    STENCIL_OP_INCREMENT_WRAP,
    STENCIL_OP_DECREMENT_WRAP
} PipelineStencilOp;

typedef enum PipelineCompareOperation {
    COMPARE_OP_NEVER,
    COMPARE_OP_LESS,
    COMPARE_OP_EQUAL,
    COMPARE_OP_LESS_EQUAL,
    COMPARE_OP_GREATER,
    COMPARE_OP_NOT_EQUAL,
    COMPARE_OP_GREATER_EQUAL,
    COMPARE_OP_ALWAYS
} PipelineCompareOp;

typedef struct {
    PipelineStencilOp fail_op;
    PipelineStencilOp pass_op;
    PipelineStencilOp depth_fail;
    PipelineCompareOp compare_op;
    u32 compare_mask;
    u32 write_mask;
} PipelineStencilOpState;

typedef struct {
    bool depth_test;
    bool depth_write;
    PipelineCompareOp depth_compare_op;
    bool depth_bounds_test;
    bool stencil_test;
    PipelineStencilOpState front;
    PipelineStencilOpState back;
    f32 min_depth_bounds;
    f32 max_depth_bounds;
} PipelineDepthStencilOptions;

typedef enum PipelineLogicOperation {
    LOGIC_OP_CLEAR,
    LOGIC_OP_AND,
    LOGIC_OP_AND_REVERSE,
    LOGIC_OP_COPY,
    LOGIC_OP_AND_INVERTED,
    LOGIC_OP_NO_OP,
    LOGIC_OP_XOR,
    LOGIC_OP_OR,
    LOGIC_OP_NOR,
    LOGIC_OP_EQUIVALENT,
    LOGIC_OP_INVERT,
    LOGIC_OP_OR_REVERSE,
    LOGIC_OP_COPY_INVERTED,
    LOGIC_OP_OR_INVERTED,
    LOGIC_OP_NAND,
    LOGIC_OP_SET
} PipelineLogicOp;

typedef enum PipelineBlendOperation {
    BLEND_OP_ADD,
    BlEND_OP_SUBTRACT,
    BLEND_OP_REVERSE_SUBTRACT,
    BLEND_OP_MIN,
    BLEND_OP_MAX,
} PipelineBlendOp;

typedef enum PipelineBlendFactor {
    BLEND_FACTOR_ZERO,
    BLEND_FACTOR_ONE,
    BLEND_FACTOR_SRC_COLOR,
    BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
    BLEND_FACTOR_DST_COLOR,
    BLEND_FACTOR_ONE_MINUS_DST_COLOR,
    BLEND_FACTOR_SRC_ALPHA,
    BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    BLEND_FACTOR_DST_ALPHA,
    BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
    BLEND_FACTOR_CONSTANT_COLOR,
    BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
    BLEND_FACTOR_CONSTANT_ALPHA,
    BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
    BLEND_FACTOR_SRC_ALPHA_SATURATE,
    BLEND_FACTOR_SRC1_COLOR,
    BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
    BLEND_FACTOR_SRC1_ALPHA,
    BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA
} PipelineBlendFactor;

typedef enum PipelineColorComponentFlagBits {
    COLOR_COMPONENT_R = 1 << 0,
    COLOR_COMPONENT_G = 2 << 0,
    COLOR_COMPONENT_B = 4 << 0,
    COLOR_COMPONENT_A = 8 << 0
} PipelineColorComponentFlags;

typedef struct {
    bool blend_enable;
    PipelineBlendFactor src_color_factor;
    PipelineBlendFactor dst_color_factor;
    PipelineBlendOp color_blend_op;
    PipelineBlendFactor src_alpha_factor;
    PipelineBlendFactor dst_alpha_factor;
    PipelineBlendOp alpha_blend_op;
    PipelineColorComponentFlags color_write_mask;
} PipelineColorBlendState;

typedef struct {
    bool blend;
    bool logic_op_enable;
    PipelineLogicOp color_blend_op;
    PipelineColorBlendState* color_blend_states;
    u32 attachment_count;
} PipelineColorBlendOptions;

typedef struct {
    PipelineShaderOptions shader_stages;
    PipelineVertexOptions vertex_input;
    PipelineInputAssemblyOptions input_assembly;
    PipelineRasterizationOptions rasterization;
    PipelineMultisampleOptions multisampling;
    PipelineDepthStencilOptions depth_stencil;
    PipelineColorBlendOptions color_blending;
} PipelineOptions;

typedef struct Pipeline Pipeline;

Pipeline* pipeline_new(Device* device, PipelineOptions options);
void pipeline_free(Device* device, Pipeline* pipeline);

VkPipeline pipeline_get_vk_pipeline(Pipeline* pipeline);

#endif // PIPELINE_H
