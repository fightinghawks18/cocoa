#include "pipeline.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct Pipeline {
    VkPipeline pipeline;
    ColorFormat format;
} Pipeline;

Pipeline* pipeline_new(Device* device, ColorFormat format) {
    Pipeline* pipeline = malloc(sizeof(Pipeline));
    pipeline->format = format;
    pipeline->pipeline = NULL;

    // TODO: Set up graphics pipeline info
    VkGraphicsPipelineCreateInfo graphics_pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .stageCount = 0,
        .pStages = NULL,
        .pVertexInputState = NULL,
        .pInputAssemblyState = NULL,
        .pTessellationState = NULL,
        .pViewportState = NULL,
        .pRasterizationState = NULL,
        .pMultisampleState = NULL,
        .pDepthStencilState = NULL,
        .pColorBlendState = NULL,
        .pDynamicState = NULL,
        .layout = NULL,
        .renderPass = NULL,
        .subpass = 0,
        .basePipelineHandle = NULL,
        .basePipelineIndex = -1
    };

    VkResult create_graphics_pipeline = vkCreateGraphicsPipelines(
        device_get_vk_device(device), 
        NULL, 
        1, 
        &graphics_pipeline_info, 
        NULL, 
        &pipeline->pipeline
    );
    if (create_graphics_pipeline != VK_SUCCESS) {
        fprintf(stderr, "Failed to create a vulkan graphics pipeline! %d\n", create_graphics_pipeline);
        pipeline_free(device, pipeline);
        return NULL;
    }
    return pipeline;
}

void pipeline_free(Device* device, Pipeline* pipeline) {
    if (pipeline->pipeline) {
        vkDestroyPipeline(device_get_vk_device(device), pipeline->pipeline, NULL);
    }
    free(pipeline);
}


VkPipeline pipeline_get_vk_pipeline(Pipeline* pipeline) {
    return pipeline->pipeline;
}
