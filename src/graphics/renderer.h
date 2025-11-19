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

typedef struct Renderer Renderer;

Renderer* renderer_new(VkDevice device, u32 graphics_family, u32 max_frames_in_flight);
void renderer_free(VkDevice device, Renderer* renderer);

const Frame* renderer_begin_rendering(VkDevice device, VkPhysicalDevice physical_device, Renderer* renderer, Swapchain* swapchain);
bool renderer_end_rendering(VkQueue graphics_queue, VkQueue present_queue, Renderer* renderer);
void renderer_rebuild_resources(VkDevice device, Renderer* renderer);

Swapchain* renderer_get_swapchain(Renderer* renderer);
u32 renderer_get_image_index(Renderer* renderer);

#endif // RENDERER_H
