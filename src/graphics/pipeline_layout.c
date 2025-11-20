#include "pipeline_layout.h"
#include <stdio.h>
#include <stdlib.h>


typedef struct PipelineLayout {
    VkPipelineLayout layout;
} PipelineLayout;


PipelineLayout* pipeline_layout_new(Device* device) {
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
        device_get_vk_device(device), 
        &pipeline_layout_info, 
        NULL, 
        &layout->layout
    );
    if (create_pipeline_layout != VK_SUCCESS) {
        fprintf(stderr, "Failed to create vulkan pipeline layout! %d\n", create_pipeline_layout);
        pipeline_layout_free(device, layout);
        return NULL;
    }
    return layout;

}

void pipeline_layout_free(Device* device, PipelineLayout* layout) {
    if (layout->layout) {
        vkDestroyPipelineLayout(device_get_vk_device(device), layout->layout, NULL);
        layout->layout = NULL;
    }
    free(layout);
}

VkPipelineLayout pipeline_layout_get_vk_layout(PipelineLayout* layout) {
    return layout->layout;
}