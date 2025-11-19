#ifndef RENDERER_H
#define RENDERER_H

#include "../int_types.h"
#include "swapchain.h"
#include "device.h"

typedef struct {
    VkCommandPool cmd_pool;
    VkSemaphore image_available_semaphore;
    VkSemaphore render_finished_semaphore;
    VkFence fence;
    VkCommandBuffer cmd;
} Frame;

typedef struct Renderer Renderer;

Renderer* renderer_new(Device* device, u32 max_frames_in_flight);
void renderer_free(Device* device, Renderer* renderer);

const Frame* renderer_begin_rendering(Device* device, Renderer* renderer, Swapchain* swapchain);
bool renderer_end_rendering(Device* device, Renderer* renderer);
void renderer_rebuild_resources(Device* device, Renderer* renderer);

Swapchain* renderer_get_swapchain(Renderer* renderer);
u32 renderer_get_image_index(Renderer* renderer);

#endif // RENDERER_H
