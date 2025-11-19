#ifndef RENDERER_H
#define RENDERER_H

#include <vulkan/vulkan.h>
#include "../int_types.h"
#include "swapchain.h"

typedef struct {
    VkCommandPool cmd_pool;
    VkSemaphore image_available_semaphore;
    VkSemaphore render_finished_semaphore;
    VkFence fence;
    VkCommandBuffer cmd;
} Frame;

typedef struct {
    Frame** frames;
    Swapchain* current_swapchain;

    u32 current_image_index;
    u32 frame_index;
    u32 max_flight;
    u32 graphics_family;
} Renderer;

Renderer* renderer_new(VkDevice device, u32 graphics_family, u32 max_frames_in_flight);
void renderer_free(VkDevice device, Renderer* renderer);

const Frame* renderer_begin_rendering(VkDevice device, VkPhysicalDevice physical_device, Renderer* renderer, Swapchain* swapchain);
bool renderer_end_rendering(VkQueue graphics_queue, VkQueue present_queue, Renderer* renderer);
void renderer_rebuild_resources(VkDevice device, Renderer* renderer);

#endif // RENDERER_H
