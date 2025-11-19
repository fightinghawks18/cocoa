#include "swapchain.h"

#include <stdio.h>
#include <stdlib.h>

void swapchain_create_images(VkDevice device, Swapchain* swapchain) {
    u32 swapchain_image_count = 0;
    vkGetSwapchainImagesKHR(device, swapchain->swapchain, &swapchain_image_count, NULL);
    
    VkImage* swapchain_images = malloc(swapchain_image_count * sizeof(VkImage));
    vkGetSwapchainImagesKHR(device, swapchain->swapchain, &swapchain_image_count, swapchain_images);

    swapchain->images = swapchain_images;
    swapchain->image_count = swapchain_image_count;
    
    swapchain->image_views = malloc(swapchain_image_count * sizeof(VkImageView));
    for (u32 i = 0; i < swapchain_image_count; i++) {
      VkImageViewCreateInfo image_view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .image = swapchain->images[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapchain->format,
        .components = {
          .r = VK_COMPONENT_SWIZZLE_IDENTITY,
          .b = VK_COMPONENT_SWIZZLE_IDENTITY,
          .g = VK_COMPONENT_SWIZZLE_IDENTITY,
          .a = VK_COMPONENT_SWIZZLE_IDENTITY,
        },
        .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1,
        }
      };
    
      VkResult image_view_create = vkCreateImageView(device, &image_view_info, NULL, &swapchain->image_views[i]);
      if (image_view_create != VK_SUCCESS) {
        fprintf(stderr, "Failed to create vulkan image view for index %d! %d\n", i, image_view_create);
      }
    }
}

Swapchain* swapchain_new(VkDevice device, VkPhysicalDevice physical_device, SwapchainOptions options) {
    Swapchain* swapchain = malloc(sizeof(Swapchain));

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, options.surface, &surfaceCapabilities);

    u32 width = surfaceCapabilities.currentExtent.width;
    u32 height = surfaceCapabilities.currentExtent.height;

    // FORMAT:VK_FORMAT_B8G8R8A8_SRGB
    // COLORSPACE:VK_COLOR_SPACE_SRGB_NONLINEAR_KHR

    VkSwapchainCreateInfoKHR swapchain_info = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext = NULL,
      .flags = 0,
      .surface = options.surface,
      .minImageCount = options.min_image_count,
      .imageFormat = options.format,
      .imageColorSpace = options.color_space,
      .imageExtent = {width, height},
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = NULL,
      .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = VK_PRESENT_MODE_FIFO_KHR,
      .clipped = VK_TRUE,
      .oldSwapchain = options.oldSwapchain
    };

    VkResult swapchain_create = vkCreateSwapchainKHR(device, &swapchain_info, NULL, &swapchain->swapchain);
    if (swapchain_create != VK_SUCCESS) {
      fprintf(stderr, "Failed to create vulkan swapchain! %d\n", swapchain_create);
      free(swapchain);
      return NULL;
    }

    swapchain->surface = options.surface;
    swapchain->format = options.format;
    swapchain->color_space = options.color_space;
    swapchain->min_image_count = options.min_image_count;
    swapchain->extent = swapchain_info.imageExtent;

    swapchain_create_images(device, swapchain);
    return swapchain;
}

void swapchain_free(VkDevice device, Swapchain* swapchain) {
    for (u32 i = 0; i < swapchain->image_count; i++) {
        vkDestroyImageView(device, swapchain->image_views[i], NULL);
    }
    free(swapchain->image_views);
    free(swapchain->images);

    vkDestroySwapchainKHR(device, swapchain->swapchain, NULL);
    free(swapchain);
}

Swapchain* swapchain_resize(VkDevice device, VkPhysicalDevice physical_device, Swapchain* swapchain) {
    vkDeviceWaitIdle(device);

    Swapchain* new_swapchain = swapchain_new(device, physical_device, (SwapchainOptions){
        .oldSwapchain = swapchain->swapchain,
        .surface = swapchain->surface,
        .min_image_count = swapchain->min_image_count,
        .format = swapchain->format,
        .color_space = swapchain->color_space
    });
    if (new_swapchain == NULL) {
        fprintf(stderr, "Failed to create new swapchain!");
        return NULL;
    }
    
    swapchain_free(device, swapchain);
    return new_swapchain;
}