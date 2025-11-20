#ifndef PIPELINE_LAYOUT_H
#define PIPELINE_LAYOUT_H

#include "device.h"

typedef struct PipelineLayout PipelineLayout;

PipelineLayout* pipeline_layout_new(Device* device);
void pipeline_layout_free(Device* device, PipelineLayout* layout);

VkPipelineLayout pipeline_layout_get_vk_layout(PipelineLayout* layout);

#endif // PIPELINE_LAYOUT_H
