#include <stdio.h>

#include <vulkan/vulkan.h>
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
  Game* game = NULL; 
  game_new(&game);
  if (!game_start(game)) {
    fprintf(stderr, "Failed to start game!\n");
    return -1;
  }

  Device* device = NULL;
  DeviceResult device_result = device_new(&device);
  if (device_result != DEVICE_OK) {
    fprintf(stderr, "Failed to create device! %d\n", device_result);
    return -1;
  }

  void* instance = NULL;
  device_get_instance(device, &instance);

  VkSurfaceKHR surface = NULL;
  SDL_Window* window = game_get_window(game);
  if (!SDL_Vulkan_CreateSurface(window, instance, NULL, &surface)) {
    fprintf(stderr, "Failed to create vulkan surface from SDL3! %s\n ", SDL_GetError());
    return -1;
  }

  Swapchain* swapchain = NULL; 
  SwapchainResult swapchain_result = swapchain_new(device, (SwapchainOptions){
    .oldSwapchain = NULL,
    .surface = surface,
    .min_image_count = MAX_FRAMES_IN_FLIGHT,
    .format = COLOR_BGRA8_SRGB,
    .color_space = COLOR_SPACE_SRGB_NLINEAR
  }, &swapchain);
  if (swapchain_result != SWAPCHAIN_OK) {
    fprintf(stderr, "Failed to create swapchain! %d\n", swapchain_result);
    return -1;
  }

  Renderer* renderer = NULL;
  RendererResult renderer_result = renderer_new(device, MAX_FRAMES_IN_FLIGHT, &renderer);
  if (renderer_result != RENDERER_OK) {
    fprintf(stderr, "Failed to create renderer! %d\n", renderer_result);
    return -1;
  }

  Geometry* geometry = NULL;
  geometry_new(4, 6, &geometry);
  geometry_set_vertex(geometry, (Vertex){
    .pos = {-0.5, -0.5, 0},
    .col = {1, 1,1, 1}
  }, 0);
  geometry_set_vertex(geometry, (Vertex){
    .pos = {0.5, -0.5, 0},
    .col = {1, 1,1, 1}
  }, 1);
  geometry_set_vertex(geometry, (Vertex){
    .pos = {0.5, 0.5, 0},
    .col = {1, 1,1, 1}
  }, 2);
  geometry_set_vertex(geometry, (Vertex){
    .pos = {-0.5, 0.5, 0},
    .col = {1, 1,1, 1}
  }, 3);
  geometry_set_index(geometry, 0, 0);
  geometry_set_index(geometry, 1, 1);
  geometry_set_index(geometry, 2, 2);

  geometry_set_index(geometry, 0, 3);
  geometry_set_index(geometry, 2, 4);
  geometry_set_index(geometry, 3, 5);

  Buffer* vertex_buffer = NULL;
  BufferResult vertex_buffer_result = buffer_new(device, (BufferOptions){
    .size = geometry->vertex_count * sizeof(Vertex),
    .usage = BUFFER_VERTEX,
    .sharing = SHARING_EXCLUSIVE,
    .memory_access = MEMORY_ACCESS_CPU_TO_GPU,
    .initial_data = geometry->vertices
  }, &vertex_buffer);
  if (vertex_buffer_result != BUFFER_OK) {
    fprintf(stderr, "Failed to create vertex buffer! %d\n", vertex_buffer_result);
    return -1;
  }

  Buffer* index_buffer = NULL;
  BufferResult index_buffer_result = buffer_new(device, (BufferOptions){
    .size = geometry->index_count * sizeof(u32),
    .usage = BUFFER_INDEX,
    .sharing = SHARING_EXCLUSIVE,
    .memory_access = MEMORY_ACCESS_CPU_TO_GPU,
    .initial_data = geometry->indices
  }, &index_buffer);
  if (index_buffer_result != BUFFER_OK) {
    fprintf(stderr, "Failed to create index buffer! %d\n", index_buffer_result);
    return -1;
  }

  Shader* vertex_shader = NULL;
  ShaderResult vertex_shader_result = shader_new(device, (ShaderOptions){
    .shader = "content/object.vert.spv",
    .type = SHADER_VERTEX
  }, &vertex_shader);
  if (vertex_shader_result != SHADER_OK) {
    fprintf(stderr, "Failed to create vertex shader! %d\n", vertex_shader_result);
    return -1;
  }

  Shader* fragment_shader = NULL;
  ShaderResult fragment_shader_result = shader_new(device, (ShaderOptions){
    .shader = "content/object.frag.spv",
    .type = SHADER_FRAGMENT
  }, &fragment_shader);
  if (fragment_shader_result != SHADER_OK) {
    fprintf(stderr, "Failed to create fragment shader! %d\n", fragment_shader_result);
    return -1;
  }

  Shader* shaders[2] = {vertex_shader, fragment_shader};

  PipelineLayout* layout = NULL;
  PipelineLayoutResult layout_result = pipeline_layout_new(device, &layout);
  if (layout_result != PIPELINE_LAYOUT_OK) {
    fprintf(stderr, "Failed to create pipeline layout! %d\n", layout_result);
    return -1;
  }

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

  ColorFormat swapchain_color;
  swapchain_get_color_format(swapchain, &swapchain_color);

  Pipeline* pipeline = NULL;
  PipelineResult pipeline_result = pipeline_new(device, (PipelineOptions){
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
      .cull_mode = CULL_NONE,
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
        swapchain_color
      },
      .color_count = 1,
      .depth = DEPTH_UNDEFINED,
      .stencil = DEPTH_UNDEFINED
    },
    .depth_stencil = {
      .depth_bounds_test = false,
      .depth_test = false,
      .max_depth_bounds = 0,
      .min_depth_bounds = 0,
      .depth_write = false,
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
  }, &pipeline);
  if (pipeline_result != PIPELINE_OK) {
    fprintf(stderr, "Failed to create pipeline! %d\n", pipeline_result);
    return -1;
  }

  while (game_is_alive(game)) {
    game_update(game);
    
    Frame* frame = NULL;
    RenderBeginResult render_begin_result = renderer_begin_rendering(device, renderer, swapchain, &frame);
    if (render_begin_result == RENDER_BEGIN_REBUILD_SWAPCHAIN) {
      continue;
    } else if (render_begin_result != RENDER_BEGIN_OK) {
      fprintf(stderr, "Failed to begin rendering! %d\n", render_begin_result);
      return -1;
    }

    void* cmd = NULL;
    renderer_get_frame_cmd(frame, &cmd);

    Swapchain* current_swapchain = NULL;
    renderer_get_swapchain(renderer, &current_swapchain);

    u32 image_index = 0;
    renderer_get_image_index(renderer, &image_index);

    void* image_views = NULL;
    swapchain_get_image_views(current_swapchain, &image_views);
    VkImageView image_view = ((VkImageView*)image_views)[image_index];

    VkRenderingAttachmentInfo rendering_attachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .pNext = NULL,
      .imageView = image_view,
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

    Extent swapchain_extent;
    swapchain_get_extent(swapchain, &swapchain_extent);

    VkRenderingInfo rendering_info = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .pNext = NULL,
      .flags = 0,
      .renderArea = {{0, 0}, {swapchain_extent.width, swapchain_extent.height}},
      .layerCount = 1,
      .viewMask = 0,
      .colorAttachmentCount = 1,
      .pColorAttachments = &rendering_attachment,
      .pStencilAttachment = NULL,
      .pDepthAttachment = NULL
    };

    vkCmdBeginRendering(cmd, &rendering_info);

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

    void* vertex_buffer_handle = NULL;
    void* index_buffer_handle = NULL;

    buffer_get_buffer(vertex_buffer, &vertex_buffer_handle);
    buffer_get_buffer(index_buffer, &index_buffer_handle);

    void* pipeline_handle = NULL;
    pipeline_get_pipeline(pipeline, &pipeline_handle);

    VkDeviceSize offsets = 0;
    
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_handle);
    vkCmdBindVertexBuffers(cmd, 0, 1,(VkBuffer*)&vertex_buffer_handle, &offsets);
    vkCmdBindIndexBuffer(cmd, index_buffer_handle, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmd, geometry->index_count, 1, 0, 0, 0);

    vkCmdEndRendering(cmd);
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
  vkDestroySurfaceKHR(instance, surface, NULL);
  device_free(device);

  game_close(game);
  return 0;
}
