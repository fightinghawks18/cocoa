#ifndef PIPELINE_LAYOUT_H
#define PIPELINE_LAYOUT_H

#include "device.h"

typedef struct PipelineLayout PipelineLayout;

typedef enum {
    PIPELINE_LAYOUT_OK, // Successfully created a pipeline layout
    PIPELINE_LAYOUT_ERROR_CREATE_HANDLE_FAIL, // Failed to create the handle for the pipeline layout
} PipelineLayoutResult;

PipelineLayoutResult pipeline_layout_new(Device* device, PipelineLayout** out_layout);
void pipeline_layout_free(Device* device, PipelineLayout* layout);

void pipeline_layout_get_layout(PipelineLayout* layout, void** out_layout);

#endif // PIPELINE_LAYOUT_H
