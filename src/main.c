#include <stdio.h>

#include <SDL3/SDL_vulkan.h>

#include "game/game.h"
#include "graphics/buffer.h"
#include "graphics/geometry.h"
#include "graphics/pipeline.h"
#include "graphics/pipeline_layout.h"
#include "graphics/swapchain.h"
#include "graphics/renderer.h"
#include "graphics/device.h"

#define MAX_FRAMES_IN_FLIGHT 2

int main() {
  Game* game = game_new();
  if (!game_start(game)) {
    fprintf(stderr, "Failed to start game!\n");
    return -1;
  }

  Device* device = device_new();

  VkSurfaceKHR surface = NULL;
  SDL_Window* window = game_get_window(game);
  if (!SDL_Vulkan_CreateSurface(window, device_get_vk_instance(device), NULL, &surface)) {
    fprintf(stderr, "Failed to create vulkan surface from SDL3! %s\n ", SDL_GetError());
    return -1;
  }

  Swapchain* swapchain = swapchain_new(device, (SwapchainOptions){
    .oldSwapchain = NULL,
    .surface = surface,
    .min_image_count = MAX_FRAMES_IN_FLIGHT,
    .format = COLOR_BGRA8_SRGB,
    .color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
  });

  Renderer* renderer = renderer_new(device, MAX_FRAMES_IN_FLIGHT);

  Geometry* geometry = geometry_new(4, 6);
  geometry_set_vertex(geometry, (Vertex){
    .pos = {-1, -1, 0},
    .col = {1, 1,1, 1}
  }, 0);
  geometry_set_vertex(geometry, (Vertex){
    .pos = {1, -1, 0},
    .col = {1, 1,1, 1}
  }, 1);
  geometry_set_vertex(geometry, (Vertex){
    .pos = {1, 1, 0},
    .col = {1, 1,1, 1}
  }, 2);
  geometry_set_vertex(geometry, (Vertex){
    .pos = {-1, 1, 0},
    .col = {1, 1,1, 1}
  }, 3);
  geometry_set_index(geometry, 0, 0);
  geometry_set_index(geometry, 1, 1);
  geometry_set_index(geometry, 2, 2);

  geometry_set_index(geometry, 0, 3);
  geometry_set_index(geometry, 2, 4);
  geometry_set_index(geometry, 3, 5);

  Buffer* vertex_buffer = buffer_new(device, (BufferOptions){
    .size = geometry->vertex_count * sizeof(Vertex),
    .usage = BUFFER_VERTEX,
    .sharing = SHARING_EXCLUSIVE,
    .memory_access = MEMORY_ACCESS_CPU_TO_GPU,
    .initial_data = geometry->vertices
  });

  Buffer* index_buffer = buffer_new(device, (BufferOptions){
    .size = geometry->index_count * sizeof(u32),
    .usage = BUFFER_INDEX,
    .sharing = SHARING_EXCLUSIVE,
    .memory_access = MEMORY_ACCESS_CPU_TO_GPU,
    .initial_data = geometry->indices
  });

  Shader* vertex_shader = shader_from_file(device, (ShaderOptions){
    .shader = "content/object.vert.spv",
    .type = SHADER_VERTEX
  });

  Shader* fragment_shader = shader_from_file(device, (ShaderOptions){
    .shader = "content/object.frag.spv",
    .type = SHADER_FRAGMENT
  });

  Shader* shaders[2] = {vertex_shader, fragment_shader};

  PipelineLayout* layout = pipeline_layout_new(device);

  PipelineInputAttribute attributes[2] = {
    (PipelineInputAttribute){
      .binding = 0,
      .location = 0,
      .format = VERTEX_FLOAT3,
      .offset = offsetof(Vertex, pos)
    },

    (PipelineInputAttribute){
      .binding = 0,
      .location = 1,
      .format = VERTEX_FLOAT4,
      .offset = offsetof(Vertex, col)
    }
  };

  PipelineInputBinding binding = {
    .binding = 0,
    .rate = INPUT_VERTEX,
    .stride = sizeof(Vertex)
  };

  Pipeline* pipeline = pipeline_new(device, (PipelineOptions){
    .shader_stages = {
      .shaders = shaders,
      .shader_count = 2
    },
    .vertex_input = {
      .attributes = attributes,
      .attribute_count = 2,
      .binding_count = 1,
      .bindings = &binding
    },
    .input_assembly = {
      .primitive_restart = false,
      .topology = TOPOLOGY_TRIANGLE_LIST
    },
    .rasterization = {
      .depth_clamping = false,
      .discard_prims_until_rasterization = false,
      .polygon_mode = POLYGON_FILL,
      .cull_mode = CULL_BACK,
      .front_face_direction = FRONT_FACING_C_CLOCKWISE,
      .bias_fragment_depth = false,
      .depth_bias_factor = 0,
      .depth_bias_clamp = 0,
      .depth_bias_slope = 0,
      .line_width = 1
    },
    .multisampling = {
      .alpha_to_coverage = false,
      .alpha_to_one = false,
      .min_sample_shading = 0,
      .sample_shading = false,
      .sample_masks = NULL,
      .sample_flag = PIPELINE_SAMPLECOUNT1
    },
    .rendering = {
      .colors = &(ColorFormat){
        swapchain_get_color_format(swapchain)
      },
      .color_count = 1,
      .depth = DEPTH_UNDEFINED,
      .stencil = DEPTH_UNDEFINED
    },
    .depth_stencil = {
      .depth_bounds_test = false,
      .depth_test = true,
      .max_depth_bounds = 1,
      .min_depth_bounds = 0,
      .depth_write = true,
      .depth_compare_op = COMPARE_OP_LESS,
      .stencil_test = false,
    },
    .color_blending = {
      .blend = false,
      .logic_op_enable = false,
      .color_blend_op = LOGIC_OP_COPY,
      .attachment_count = 1,
      .color_blend_states = &(PipelineColorBlendState){
        .blend_enable = false,
        .color_write_mask = COLOR_COMPONENT_R | COLOR_COMPONENT_G | 
                           COLOR_COMPONENT_B | COLOR_COMPONENT_A
      }
    },
    .layout = layout
  });

  while (game_is_alive(game)) {
    game_update(game);

    const Frame* frame = renderer_begin_rendering(device, renderer, swapchain);
    if (frame == NULL) {
      continue;
    }

    VkRenderingAttachmentInfo rendering_attachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .pNext = NULL,
      .imageView = swapchain_get_vk_image_views(renderer_get_swapchain(renderer))[renderer_get_image_index(renderer)],
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .resolveMode = VK_RESOLVE_MODE_NONE,
      .resolveImageView = NULL,
      .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = {
        .color = {{0, 0, 1, 1} }
      }
    };

    VkRenderingInfo rendering_info = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .pNext = NULL,
      .flags = 0,
      .renderArea = {{0, 0}, swapchain_get_extent(swapchain)},
      .layerCount = 1,
      .viewMask = 0,
      .colorAttachmentCount = 1,
      .pColorAttachments = &rendering_attachment,
      .pStencilAttachment = NULL,
      .pDepthAttachment = NULL
    };

    vkCmdBeginRendering(frame->cmd, &rendering_info);

    VkViewport viewport = {
      0,
      0,
      800,
      600,
      0,
      1
    };
    VkRect2D scissor = {
      .offset = {0, 0},
      .extent = {800, 600}
    };

    VkBuffer vertex_buffer_handle = buffer_get_vk_buffer(vertex_buffer);
    VkBuffer index_buffer_handle = buffer_get_vk_buffer(index_buffer);
    VkDeviceSize offsets = {0};
    
    vkCmdSetViewport(frame->cmd, 0, 1, &viewport);
    vkCmdSetScissor(frame->cmd, 0, 1, &scissor);
    vkCmdBindPipeline(frame->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_get_vk_pipeline(pipeline));
    vkCmdBindVertexBuffers(frame->cmd, 0, 1, &vertex_buffer_handle, &offsets);
    vkCmdBindIndexBuffer(frame->cmd, index_buffer_handle, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(frame->cmd, geometry->index_count, 1, 0, 0, 0);

    vkCmdEndRendering(frame->cmd);
    renderer_end_rendering(device, renderer);
  }

  device_wait(device);

  geometry_free(geometry);
  shader_free(device, vertex_shader);
  shader_free(device, fragment_shader);
  buffer_free(device, vertex_buffer);
  buffer_free(device, index_buffer);
  pipeline_free(device, pipeline);
  pipeline_layout_free(device, layout);
  renderer_free(device, renderer);
  swapchain_free(device, swapchain);
  vkDestroySurfaceKHR(device_get_vk_instance(device), surface, NULL);
  device_free(device);

  game_close(game);
  return 0;
}
