#include <stdlib.h>

#include <vulkan/vulkan.h>
#include <SDL3/SDL_vulkan.h>

#include "game/game.h"
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
    .format = VK_FORMAT_B8G8R8A8_SRGB,
    .color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
  });

  Renderer* renderer = renderer_new(device, MAX_FRAMES_IN_FLIGHT);

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
    vkCmdEndRendering(frame->cmd);
    renderer_end_rendering(device, renderer);
  }

  renderer_free(device, renderer);
  swapchain_free(device, swapchain);
  vkDestroySurfaceKHR(device_get_vk_instance(device), surface, NULL);
  device_free(device);

  game_close(game);
  return 0;
}
