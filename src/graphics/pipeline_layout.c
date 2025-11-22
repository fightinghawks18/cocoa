#include "pipeline_layout.h"
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>


typedef struct PipelineLayout {
    VkPipelineLayout layout;
} PipelineLayout;


PipelineLayoutResult pipeline_layout_new(Device* device, PipelineLayout** out_layout) {
    void* device_handle = NULL;
    device_get_device(device, &device_handle);

    PipelineLayout* layout = malloc(sizeof(PipelineLayout));
    layout->layout = NULL;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = NULL,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL
    };

    VkResult create_pipeline_layout = vkCreatePipelineLayout(
        device_handle, 
        &pipeline_layout_info, 
        NULL, 
        &layout->layout
    );
    if (create_pipeline_layout != VK_SUCCESS) {
        fprintf(stderr, "Failed to create vulkan pipeline layout! %d\n", create_pipeline_layout);
        pipeline_layout_free(device, layout);
        return PIPELINE_LAYOUT_ERROR_CREATE_HANDLE_FAIL;
    }

    *out_layout = layout;
    return PIPELINE_LAYOUT_OK;
}

void pipeline_layout_free(Device* device, PipelineLayout* layout) {
    void* device_handle = NULL;
    device_get_device(device, &device_handle);

    if (layout->layout) {
        vkDestroyPipelineLayout(device_handle, layout->layout, NULL);
        layout->layout = NULL;
    }
    free(layout);
}

void pipeline_layout_get_layout(PipelineLayout* layout, void** out_layout) {
    *out_layout = layout->layout;
}