#ifndef PIPELINE_H
#define PIPELINE_H

#include "device.h"
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

typedef struct Pipeline Pipeline;

Pipeline* pipeline_new(Device* device, ColorFormat format);
void pipeline_free(Device* device, Pipeline* pipeline);

VkPipeline pipeline_get_vk_pipeline(Pipeline* pipeline);

#endif // PIPELINE_H
