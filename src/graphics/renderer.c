#include "renderer.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void renderer_create_frames(VkDevice device, u32 graphics_family, Renderer* renderer) {
    renderer->frames = malloc(renderer->max_flight * sizeof(Frame*));
    for (u32 i = 0; i < renderer->max_flight; i++) {
        Frame* frame = malloc(sizeof(Frame));

        VkCommandPoolCreateInfo command_pool_info = {
          .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
          .pNext = NULL,
          .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
          .queueFamilyIndex = graphics_family
        };
    
        VkResult command_pool_create = vkCreateCommandPool(device, &command_pool_info, NULL, &frame->cmd_pool);
        if (command_pool_create != VK_SUCCESS) {
          fprintf(stderr, "Failed to create vulkan command pool! %d\n", command_pool_create);
          return;
        }
    
        VkSemaphoreCreateInfo semaphore_create_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0
          };
      
        VkResult image_available_semaphore_create = vkCreateSemaphore(device, &semaphore_create_info, NULL, &frame->image_available_semaphore);
        if (image_available_semaphore_create != VK_SUCCESS) {
          fprintf(stderr, "Failed to create vulkan image available semaphore for index %d! %d\n", i, image_available_semaphore_create);
          return;
        }
    
        VkResult render_finished_semaphore_create = vkCreateSemaphore(device, &semaphore_create_info, NULL, &frame->render_finished_semaphore);
        if (image_available_semaphore_create != VK_SUCCESS) {
          fprintf(stderr, "Failed to create vulkan render finished semaphore for index %d! %d\n", i, render_finished_semaphore_create);
          return;
        }
    
        VkFenceCreateInfo fence_info = {
          .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
          .pNext = NULL,
          .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };
    
        VkResult fence_create = vkCreateFence(device, &fence_info, NULL, &frame->fence);
        if (fence_create != VK_SUCCESS) {
          fprintf(stderr, "Failed to create vulkan fence for index %d! %d\n", i, fence_create);
          return;
        }
    
        VkCommandBufferAllocateInfo command_buffer_info = {
          .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
          .pNext = NULL,
          .commandPool = frame->cmd_pool,
          .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
          .commandBufferCount = 1
        };
    
        VkResult command_buffer_create = vkAllocateCommandBuffers(device, &command_buffer_info, &frame->cmd);
        if (command_buffer_create != VK_SUCCESS) {
          fprintf(stderr, "Failed to create vulkan command buffer for index %d! %d\n", i, command_buffer_create);
          return;
        }

        renderer->frames[i] = frame;
    }
}

static void renderer_free_frames(VkDevice device, Renderer* renderer) {
    vkDeviceWaitIdle(device);
    for (u32 i = 0; i < renderer->max_flight; i++) {
        Frame* frame = renderer->frames[i];
        vkDestroySemaphore(device, frame->image_available_semaphore, NULL);
        vkDestroySemaphore(device, frame->render_finished_semaphore, NULL);
        vkDestroyFence(device, frame->fence, NULL);
  
        vkDestroyCommandPool(device, frame->cmd_pool, NULL);
        free(frame);
    }
    free(renderer->frames);
}

Renderer* renderer_new(VkDevice device, u32 graphics_family, u32 max_frames_in_flight) {
    Renderer* renderer = malloc(sizeof(Renderer));
    renderer->max_flight = max_frames_in_flight;
    renderer->frame_index = 0;
    renderer->current_swapchain = NULL;
    renderer->current_image_index = 0;
    renderer->graphics_family = graphics_family;

    renderer_create_frames(device, graphics_family, renderer);
    return renderer;
}

void renderer_free(VkDevice device, Renderer* renderer) {
    renderer_free_frames(device, renderer);
    free(renderer);
}

const Frame* renderer_begin_rendering(VkDevice device, VkPhysicalDevice physical_device, Renderer* renderer, Swapchain* swapchain) {
    u32 frame_index = renderer->frame_index;
    Frame* frame = renderer->frames[frame_index];

    VkResult wait_for_fence = vkWaitForFences(device, 1, &frame->fence, VK_TRUE, INFINITY);
    if (wait_for_fence != VK_SUCCESS) {
      fprintf(stderr, "Failed to wait on fence for index %d! %d\n", frame_index, wait_for_fence);
      return NULL;
    }
    
    u32 image_index = UINT32_MAX;
    u32 get_next_image = vkAcquireNextImageKHR(device, swapchain->swapchain, INFINITY, frame->image_available_semaphore, NULL, &image_index);
    if (get_next_image == VK_SUBOPTIMAL_KHR || (int)get_next_image == VK_ERROR_OUT_OF_DATE_KHR) {
      printf("Resizing swapchain for index %d\n", frame_index);
      swapchain_resize(device, physical_device, swapchain);
      renderer_rebuild_resources(device, renderer);
      return NULL;
    } else if (get_next_image != VK_SUCCESS) {
      fprintf(stderr, "Failed to get next image for index %d! %d\n", frame_index, get_next_image);
      return NULL;
    }

    VkResult reset_fence = vkResetFences(device, 1, &frame->fence);
    if (reset_fence != VK_SUCCESS) {
      fprintf(stderr, "Failed to reset fence for index %d! %d\n", frame_index, reset_fence);
      return NULL;
    } 

    VkCommandBufferBeginInfo command_buffer_begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = NULL,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo = NULL
    };

    VkResult command_buffer_begin = vkBeginCommandBuffer(frame->cmd, &command_buffer_begin_info);
    if (command_buffer_begin != VK_SUCCESS) {
      fprintf(stderr, "Failed to start command buffer for index %d! %d\n", frame_index, command_buffer_begin);
      return NULL;
    }

    VkImageMemoryBarrier2 undefined_to_color_barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
      .srcAccessMask = VK_ACCESS_2_NONE,
      .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = swapchain->images[image_index],
      .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel = 0,
      .subresourceRange.levelCount = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount = 1
    };

    VkDependencyInfo undefined_to_color = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .pNext = NULL,
      .dependencyFlags = 0,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &undefined_to_color_barrier,
    };

    vkCmdPipelineBarrier2(frame->cmd, &undefined_to_color);

    renderer->current_swapchain = swapchain;
    renderer->current_image_index = image_index;
    return frame;
}

bool renderer_end_rendering(VkQueue graphics_queue, VkQueue present_queue, Renderer* renderer) {
    u32 frame_index = renderer->frame_index;
    Frame* frame = renderer->frames[frame_index];

    VkImageMemoryBarrier2 color_to_present_barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_2_NONE,
      .dstAccessMask = VK_ACCESS_2_NONE,
      .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = renderer->current_swapchain->images[renderer->current_image_index],
      .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel = 0,
      .subresourceRange.levelCount = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount = 1
    };

    VkDependencyInfo color_to_present = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .pNext = NULL,
      .dependencyFlags = 0,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &color_to_present_barrier,
    };

    vkCmdPipelineBarrier2(frame->cmd, &color_to_present);
    VkResult command_buffer_end = vkEndCommandBuffer(frame->cmd);
    if (command_buffer_end != VK_SUCCESS) {
      fprintf(stderr, "Failed to end vulkan command buffer for index %d! %d\n", frame_index, command_buffer_end);
      return false;
    }

    VkPipelineStageFlags wait_stages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submit_info = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = NULL,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &frame->image_available_semaphore,
      .pWaitDstStageMask = &wait_stages,
      .commandBufferCount = 1,
      .pCommandBuffers = &frame->cmd,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &frame->render_finished_semaphore
    };

    VkResult submit = vkQueueSubmit(graphics_queue, 1, &submit_info, frame->fence);
    if (submit != VK_SUCCESS) {
      fprintf(stderr, "Failed to submit graphics for index %d! %d\n", frame_index, submit);
      return false;
    }

    VkPresentInfoKHR present_info = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext = NULL,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &frame->render_finished_semaphore,
      .swapchainCount = 1,
      .pSwapchains = &renderer->current_swapchain->swapchain,
      .pImageIndices = &renderer->current_image_index,
      .pResults = NULL
    };

    VkResult present = vkQueuePresentKHR(present_queue, &present_info);
    if (present != VK_SUCCESS) {
      fprintf(stderr, "Failed to present graphics for index %d! %d\n", frame_index, present);
      return false;
    }

    renderer->frame_index = (renderer->frame_index + 1) % renderer->max_flight;
    renderer->current_swapchain = NULL;
    renderer->current_image_index = UINT32_MAX;
    return true;
}

void renderer_rebuild_resources(VkDevice device, Renderer* renderer) {
    renderer_free_frames(device, renderer);
    renderer_create_frames(device, renderer->graphics_family, renderer);
}
