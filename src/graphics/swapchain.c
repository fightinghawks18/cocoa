#include "swapchain.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct Swapchain {
    VkSwapchainKHR swapchain;
    VkSurfaceKHR surface;
    VkImage* images;
    VkImageView* image_views;
    
    VkExtent2D extent;
    VkFormat format;
    VkColorSpaceKHR color_space;
    
    u32 min_image_count;
    u32 image_count;
} Swapchain;

static void swapchain_create_images(Device* device, Swapchain* swapchain) {
    u32 swapchain_image_count = 0;
    vkGetSwapchainImagesKHR(device_get_vk_device(device), swapchain->swapchain, &swapchain_image_count, NULL);
    
    VkImage* swapchain_images = malloc(swapchain_image_count * sizeof(VkImage));
    vkGetSwapchainImagesKHR(device_get_vk_device(device), swapchain->swapchain, &swapchain_image_count, swapchain_images);

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
    
      VkResult image_view_create = vkCreateImageView(device_get_vk_device(device), &image_view_info, NULL, &swapchain->image_views[i]);
      if (image_view_create != VK_SUCCESS) {
        fprintf(stderr, "Failed to create vulkan image view for index %d! %d\n", i, image_view_create);
      }
    }
}

static void swapchain_free_images(Device* device, Swapchain* swapchain) {
  for (u32 i = 0; i < swapchain->image_count; i++) {
      vkDestroyImageView(device_get_vk_device(device), swapchain->image_views[i], NULL);
  }
  free(swapchain->image_views);
  free(swapchain->images);
}

static void swapchain_free_resources(Device* device, Swapchain* swapchain) {
  swapchain_free_images(device, swapchain);
  vkDestroySwapchainKHR(device_get_vk_device(device), swapchain->swapchain, NULL);
}

Swapchain* swapchain_new(Device* device, SwapchainOptions options) {
    Swapchain* swapchain = malloc(sizeof(Swapchain));

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_get_vk_physical_device(device), options.surface, &surfaceCapabilities);

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

    VkResult swapchain_create = vkCreateSwapchainKHR(device_get_vk_device(device), &swapchain_info, NULL, &swapchain->swapchain);
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

void swapchain_free(Device* device, Swapchain* swapchain) {
    swapchain_free_resources(device, swapchain);
    free(swapchain);
}

void swapchain_resize(Device* device, Swapchain* swapchain) {
    device_wait(device);

    Swapchain* new_swapchain = swapchain_new(device, (SwapchainOptions){
        .oldSwapchain = swapchain->swapchain,
        .surface = swapchain->surface,
        .min_image_count = swapchain->min_image_count,
        .format = swapchain->format,
        .color_space = swapchain->color_space
    });
    if (new_swapchain == NULL) {
        fprintf(stderr, "Failed to create new swapchain!");
        return;
    }
    
    swapchain_free_resources(device, swapchain);
    *swapchain = *new_swapchain;
    free(new_swapchain);
}

VkSwapchainKHR swapchain_get_vk_swapchain(Swapchain* swapchain) {
  return swapchain->swapchain;
}

VkSurfaceKHR swapchain_get_vk_surface(Swapchain* swapchain) {
  return swapchain->surface;
}

VkImage* swapchain_get_vk_images(Swapchain* swapchain) {
  return swapchain->images;
}

VkImageView* swapchain_get_vk_image_views(Swapchain* swapchain) {
  return swapchain->image_views;
}

VkExtent2D swapchain_get_extent(Swapchain* swapchain) {
  return swapchain->extent;
}

VkFormat swapchain_get_format(Swapchain* swapchain) {
  return swapchain->format;
}

VkColorSpaceKHR swapchain_get_color_space(Swapchain* swapchain) {
  return swapchain->color_space;
}

u32 swapchain_get_image_count(Swapchain* swapchain) {
  return swapchain->image_count;
}
