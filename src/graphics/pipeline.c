#include "pipeline.h"
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

typedef struct Pipeline {
    VkPipeline pipeline;
} Pipeline;

static VkShaderStageFlagBits shader_stage_to_vk[] = {
    [SHADER_VERTEX] = VK_SHADER_STAGE_VERTEX_BIT,
    [SHADER_FRAGMENT] = VK_SHADER_STAGE_FRAGMENT_BIT
};

static VkVertexInputRate input_rate_to_vk[] = {
    [INPUT_INSTANCE] = VK_VERTEX_INPUT_RATE_INSTANCE,
    [INPUT_VERTEX] = VK_VERTEX_INPUT_RATE_VERTEX
};

static VkPrimitiveTopology topology_to_vk[] = {
    [TOPOLOGY_TRIANGLE_STRIP] = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
    [TOPOLOGY_TRIANGLE_LIST] = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
};

static VkPolygonMode polygon_mode_to_vk[] = {
    [POLYGON_FILL] = VK_POLYGON_MODE_FILL,
    [POLYGON_LINE] = VK_POLYGON_MODE_LINE,
    [POLYGON_POINT] = VK_POLYGON_MODE_POINT
};

static VkCullModeFlags cull_mode_to_vk[] = {
    [CULL_NONE] = VK_CULL_MODE_NONE,
    [CULL_BACK] = VK_CULL_MODE_BACK_BIT,
    [CULL_FRONT] = VK_CULL_MODE_FRONT_BIT,
    [CULL_BOTH] = VK_CULL_MODE_FRONT_AND_BACK
};

static VkCompareOp compare_op_to_vk[] = {
    [COMPARE_OP_NEVER] = VK_COMPARE_OP_NEVER,
    [COMPARE_OP_LESS] = VK_COMPARE_OP_LESS,
    [COMPARE_OP_EQUAL] = VK_COMPARE_OP_EQUAL,
    [COMPARE_OP_LESS_EQUAL] = VK_COMPARE_OP_LESS_OR_EQUAL,
    [COMPARE_OP_GREATER] = VK_COMPARE_OP_GREATER,
    [COMPARE_OP_NOT_EQUAL] = VK_COMPARE_OP_NOT_EQUAL,
    [COMPARE_OP_GREATER_EQUAL] = VK_COMPARE_OP_GREATER_OR_EQUAL,
    [COMPARE_OP_ALWAYS] = VK_COMPARE_OP_ALWAYS
};

static VkStencilOp stencil_op_to_vk[] = {
    [STENCIL_OP_KEEP] = VK_STENCIL_OP_KEEP,
    [STENCIL_OP_ZERO] = VK_STENCIL_OP_ZERO,
    [STENCIL_OP_REPLACE] = VK_STENCIL_OP_REPLACE,
    [STENCIL_OP_INCREMENT_CLAMP] = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    [STENCIL_OP_DECREMENT_CLAMP] = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    [STENCIL_OP_INVERT] = VK_STENCIL_OP_INVERT,
    [STENCIL_OP_INCREMENT_WRAP] = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    [STENCIL_OP_DECREMENT_WRAP] = VK_STENCIL_OP_DECREMENT_AND_WRAP
};

static VkBlendOp blend_op_to_vk[] = {
    [BLEND_OP_ADD] = VK_BLEND_OP_ADD,
    [BlEND_OP_SUBTRACT] = VK_BLEND_OP_SUBTRACT,
    [BLEND_OP_REVERSE_SUBTRACT] = VK_BLEND_OP_REVERSE_SUBTRACT,
    [BLEND_OP_MIN] = VK_BLEND_OP_MIN,
    [BLEND_OP_MAX] = VK_BLEND_OP_MAX,
};

static VkBlendFactor blend_factor_to_vk[] = {
    [BLEND_FACTOR_ZERO] = VK_BLEND_FACTOR_ZERO,
    [BLEND_FACTOR_ONE] = VK_BLEND_FACTOR_ONE,
    [BLEND_FACTOR_SRC_COLOR] = VK_BLEND_FACTOR_SRC_COLOR,
    [BLEND_FACTOR_ONE_MINUS_SRC_COLOR] = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
    [BLEND_FACTOR_DST_COLOR] = VK_BLEND_FACTOR_DST_COLOR,
    [BLEND_FACTOR_ONE_MINUS_DST_COLOR] = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
    [BLEND_FACTOR_SRC_ALPHA] = VK_BLEND_FACTOR_SRC_ALPHA,
    [BLEND_FACTOR_ONE_MINUS_SRC_ALPHA] = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    [BLEND_FACTOR_DST_ALPHA] = VK_BLEND_FACTOR_DST_ALPHA,
    [BLEND_FACTOR_ONE_MINUS_DST_ALPHA] = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
    [BLEND_FACTOR_CONSTANT_COLOR] = VK_BLEND_FACTOR_CONSTANT_COLOR,
    [BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR] = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
    [BLEND_FACTOR_CONSTANT_ALPHA] = VK_BLEND_FACTOR_CONSTANT_ALPHA,
    [BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA] = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
    [BLEND_FACTOR_SRC_ALPHA_SATURATE] = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
    [BLEND_FACTOR_SRC1_COLOR] = VK_BLEND_FACTOR_SRC1_COLOR,
    [BLEND_FACTOR_ONE_MINUS_SRC1_COLOR] = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
    [BLEND_FACTOR_SRC1_ALPHA] = VK_BLEND_FACTOR_SRC1_ALPHA,
    [BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA] = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA
};

static VkLogicOp logic_op_to_vk[] = {
    [LOGIC_OP_CLEAR] = VK_LOGIC_OP_CLEAR,
    [LOGIC_OP_AND] = VK_LOGIC_OP_AND,
    [LOGIC_OP_AND_REVERSE] = VK_LOGIC_OP_AND_REVERSE,
    [LOGIC_OP_COPY] = VK_LOGIC_OP_COPY,
    [LOGIC_OP_AND_INVERTED] = VK_LOGIC_OP_AND_INVERTED,
    [LOGIC_OP_NO_OP] = VK_LOGIC_OP_NO_OP,
    [LOGIC_OP_XOR] = VK_LOGIC_OP_XOR,
    [LOGIC_OP_OR] = VK_LOGIC_OP_OR,
    [LOGIC_OP_NOR] = VK_LOGIC_OP_NOR,
    [LOGIC_OP_EQUIVALENT] = VK_LOGIC_OP_EQUIVALENT,
    [LOGIC_OP_INVERT] = VK_LOGIC_OP_INVERT,
    [LOGIC_OP_OR_REVERSE] = VK_LOGIC_OP_OR_REVERSE,
    [LOGIC_OP_COPY_INVERTED] = VK_LOGIC_OP_COPY_INVERTED,
    [LOGIC_OP_OR_INVERTED] = VK_LOGIC_OP_OR_INVERTED,
    [LOGIC_OP_NAND] = VK_LOGIC_OP_NAND,
    [LOGIC_OP_SET] = VK_LOGIC_OP_SET
};


static VkFrontFace front_face_to_vk[] = {
    [FRONT_FACING_C_CLOCKWISE] = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    [FRONT_FACING_CLOCKWISE] = VK_FRONT_FACE_CLOCKWISE,
};

static VkSampleCountFlags sample_count_to_vk(PipelineSamplingFlags sampling_flags) {
    VkSampleCountFlags sample_count_flags = 0;

    if (sampling_flags & PIPELINE_SAMPLECOUNT1) sample_count_flags |= VK_SAMPLE_COUNT_1_BIT;
    if (sampling_flags & PIPELINE_SAMPLECOUNT2) sample_count_flags |= VK_SAMPLE_COUNT_2_BIT;
    if (sampling_flags & PIPELINE_SAMPLECOUNT4) sample_count_flags |= VK_SAMPLE_COUNT_4_BIT;
    if (sampling_flags & PIPELINE_SAMPLECOUNT8) sample_count_flags |= VK_SAMPLE_COUNT_8_BIT;
    if (sampling_flags & PIPELINE_SAMPLECOUNT16) sample_count_flags |= VK_SAMPLE_COUNT_16_BIT;
    if (sampling_flags & PIPELINE_SAMPLECOUNT32) sample_count_flags |= VK_SAMPLE_COUNT_32_BIT;
    if (sampling_flags & PIPELINE_SAMPLECOUNT64) sample_count_flags |= VK_SAMPLE_COUNT_64_BIT;

    return sample_count_flags;
}

static VkPipeline pipeline_build(Device* device, PipelineOptions options) {
    void* device_handle = NULL;
    device_get_device(device, &device_handle);

    VkPipelineShaderStageCreateInfo stages[options.shader_stages.shader_count];
    for (u32 i = 0; i < options.shader_stages.shader_count; i++) {
        Shader* shader = options.shader_stages.shaders[i];

        ShaderType type;
        shader_get_type(shader, &type);

        void* module = NULL;
        shader_get_module(shader, &module);

        stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[i].pNext = NULL;
        stages[i].flags = 0;
        stages[i].stage = shader_stage_to_vk[type];
        stages[i].module = module;
        stages[i].pName = "main";
        stages[i].pSpecializationInfo = NULL;
    }

    VkVertexInputBindingDescription bindings[options.vertex_input.binding_count];
    VkVertexInputAttributeDescription attributes[options.vertex_input.attribute_count];

    for (u32 i = 0; i < options.vertex_input.binding_count; i++) {
        PipelineInputBinding binding = options.vertex_input.bindings[i];
        bindings[i].binding = binding.binding;
        bindings[i].inputRate = input_rate_to_vk[binding.rate];
        bindings[i].stride = binding.stride;
    }

    for (u32 i = 0; i < options.vertex_input.attribute_count; i++) {
        PipelineInputAttribute attribute = options.vertex_input.attributes[i];
        int vertex_format = 0;
        vertex_format_to_vk(attribute.format, &vertex_format);

        attributes[i].binding = attribute.binding;
        attributes[i].format = vertex_format;
        attributes[i].location = attribute.location;
        attributes[i].offset = attribute.offset;
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .vertexBindingDescriptionCount = options.vertex_input.binding_count,
        .pVertexBindingDescriptions = bindings,
        .vertexAttributeDescriptionCount = options.vertex_input.attribute_count,
        .pVertexAttributeDescriptions = attributes,
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .primitiveRestartEnable = options.input_assembly.primitive_restart,
        .topology = topology_to_vk[options.input_assembly.topology]
    };

    VkPipelineRasterizationStateCreateInfo rasterization_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .depthClampEnable = options.rasterization.depth_clamping,
        .rasterizerDiscardEnable = options.rasterization.discard_prims_until_rasterization,
        .polygonMode = polygon_mode_to_vk[options.rasterization.polygon_mode],
        .cullMode = cull_mode_to_vk[options.rasterization.cull_mode],
        .frontFace = front_face_to_vk[options.rasterization.front_face_direction],
        .depthBiasEnable = options.rasterization.bias_fragment_depth,
        .depthBiasConstantFactor = options.rasterization.depth_bias_factor,
        .depthBiasClamp = options.rasterization.depth_bias_clamp,
        .depthBiasSlopeFactor = options.rasterization.depth_bias_slope,
        .lineWidth = options.rasterization.line_width
    };

    VkPipelineMultisampleStateCreateInfo multisample_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .rasterizationSamples = sample_count_to_vk(options.multisampling.sample_flag),
        .sampleShadingEnable = options.multisampling.sample_shading,
        .minSampleShading = options.multisampling.min_sample_shading,
        .pSampleMask = options.multisampling.sample_masks,
        .alphaToCoverageEnable = options.multisampling.alpha_to_coverage,
        .alphaToOneEnable = options.multisampling.alpha_to_one
    };

    VkStencilOpState front_state = {
        .failOp = stencil_op_to_vk[options.depth_stencil.front.fail_op],
        .passOp = stencil_op_to_vk[options.depth_stencil.front.pass_op],
        .depthFailOp = stencil_op_to_vk[options.depth_stencil.front.depth_fail],
        .compareOp = compare_op_to_vk[options.depth_stencil.front.compare_op],
        .compareMask = options.depth_stencil.front.compare_mask,
        .writeMask = options.depth_stencil.front.write_mask,
        .reference = 0
    };

    VkStencilOpState back_state = {
        .failOp = stencil_op_to_vk[options.depth_stencil.back.fail_op],
        .passOp = stencil_op_to_vk[options.depth_stencil.back.pass_op],
        .depthFailOp = stencil_op_to_vk[options.depth_stencil.back.depth_fail],
        .compareOp = compare_op_to_vk[options.depth_stencil.back.compare_op],
        .compareMask = options.depth_stencil.back.compare_mask,
        .writeMask = options.depth_stencil.back.write_mask,
        .reference = 0
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .depthTestEnable = options.depth_stencil.depth_test,
        .depthWriteEnable = options.depth_stencil.depth_write,
        .depthCompareOp = compare_op_to_vk[options.depth_stencil.depth_compare_op],
        .depthBoundsTestEnable = options.depth_stencil.depth_bounds_test,
        .stencilTestEnable = options.depth_stencil.stencil_test,
        .front = front_state,
        .back = back_state,
        .minDepthBounds = options.depth_stencil.min_depth_bounds,
        .maxDepthBounds = options.depth_stencil.max_depth_bounds
    };

    VkPipelineColorBlendAttachmentState blend_states[options.color_blending.attachment_count];
    for (u32 i = 0; i < options.color_blending.attachment_count; i++) {
        PipelineColorBlendState blend_state = options.color_blending.color_blend_states[i];
        blend_states[i].blendEnable = blend_state.blend_enable;
        
        blend_states[i].colorBlendOp = blend_op_to_vk[blend_state.color_blend_op];
        blend_states[i].dstColorBlendFactor = blend_factor_to_vk[blend_state.dst_color_factor];
        blend_states[i].srcColorBlendFactor = blend_factor_to_vk[blend_state.src_color_factor];

        blend_states[i].colorWriteMask = blend_state.color_write_mask;
        blend_states[i].dstAlphaBlendFactor = blend_factor_to_vk[blend_state.dst_alpha_factor];
        blend_states[i].srcAlphaBlendFactor = blend_factor_to_vk[blend_state.src_alpha_factor];
        blend_states[i].alphaBlendOp = blend_op_to_vk[blend_state.alpha_blend_op];
    }

    VkPipelineColorBlendStateCreateInfo color_blend_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .logicOpEnable = options.color_blending.logic_op_enable,
        .logicOp = logic_op_to_vk[options.color_blending.color_blend_op],
        .attachmentCount = options.color_blending.attachment_count,
        .pAttachments = blend_states,
        .blendConstants = {0, 0, 0, 0}
    };

    VkPipelineViewportStateCreateInfo viewport_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = NULL,
        .scissorCount = 1,
        .pScissors = NULL
    };

    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .dynamicStateCount = sizeof(dynamic_states) / sizeof(dynamic_states[0]),
        .pDynamicStates = dynamic_states
    };

    VkFormat color_formats[options.rendering.color_count];
    for (u32 i = 0; i < options.rendering.color_count; i++) {
        int color_format = 0;
        color_format_to_vk(options.rendering.colors[i], &color_format);
        color_formats[i] = color_format;
    }

    int depth_format = 0;
    depth_format_to_vk(options.rendering.depth, &depth_format);

    int stencil_format = 0;
    depth_format_to_vk(options.rendering.stencil, &stencil_format);

    VkPipelineRenderingCreateInfo pipeline_rendering_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = NULL,
        .viewMask = 0,
        .colorAttachmentCount = options.rendering.color_count,
        .pColorAttachmentFormats = color_formats,
        .depthAttachmentFormat = depth_format,
        .stencilAttachmentFormat = stencil_format
    };

    void* layout = NULL;
    pipeline_layout_get_layout(options.layout, &layout);

    VkGraphicsPipelineCreateInfo graphics_pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &pipeline_rendering_info,
        .flags = 0,
        .stageCount = options.shader_stages.shader_count,
        .pStages = stages,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly_info,
        .pTessellationState = NULL,
        .pViewportState = &viewport_info,
        .pRasterizationState = &rasterization_info,
        .pMultisampleState = &multisample_info,
        .pDepthStencilState = &depth_stencil_info,
        .pColorBlendState = &color_blend_info,
        .pDynamicState = &dynamic_state_info,
        .layout = layout,
        .renderPass = NULL,
        .subpass = 0,
        .basePipelineHandle = NULL,
        .basePipelineIndex = -1
    };

    VkPipeline pipeline = NULL;
    VkResult create_graphics_pipeline = vkCreateGraphicsPipelines(
        device_handle, 
        NULL, 
        1, 
        &graphics_pipeline_info, 
        NULL, 
        &pipeline
    );
    if (create_graphics_pipeline != VK_SUCCESS) {
        fprintf(stderr, "Failed to create a vulkan graphics pipeline! %d\n", create_graphics_pipeline);
        vkDestroyPipeline(device_handle, pipeline, NULL);
        return NULL;
    }
    return pipeline;
}

PipelineResult pipeline_new(Device* device, PipelineOptions options, Pipeline** out_pipeline) {
    Pipeline* pipeline = malloc(sizeof(Pipeline));
    pipeline->pipeline = NULL;

    VkPipeline pip = pipeline_build(device, options);
    if (pip == NULL) {
        pipeline_free(device, pipeline);
        return PIPELINE_ERROR_CREATE_HANDLE_FAIL;
    }
    pipeline->pipeline = pip;
    
    *out_pipeline = pipeline;
    return PIPELINE_OK;
}

void pipeline_free(Device* device, Pipeline* pipeline) {
    void* device_handle = NULL;
    device_get_device(device, &device_handle);

    if (pipeline->pipeline) {
        vkDestroyPipeline(device_handle, pipeline->pipeline, NULL);
    }
    free(pipeline);
}


void pipeline_get_pipeline(Pipeline* pipeline, void** out_pipeline) {
    *out_pipeline = pipeline->pipeline;
}
