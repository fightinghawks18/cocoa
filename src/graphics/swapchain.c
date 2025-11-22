#include "swapchain.h"

#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

typedef struct Swapchain {
    VkSwapchainKHR swapchain;
    VkSurfaceKHR surface;
    VkImage* images;
    VkImageView* image_views;
    
    Extent extent;
    ColorFormat color_format;
    ColorSpace color_space;
    
    u32 min_image_count;
    u32 image_count;
} Swapchain;

static VkColorSpaceKHR color_space_to_vk[] = {
  [COLOR_SPACE_SRGB_NLINEAR] = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
};

static bool swapchain_create_images(Device* device, Swapchain* swapchain) {
    void* device_handle = NULL;
    device_get_device(device, &device_handle);

    u32 swapchain_image_count = 0;
    vkGetSwapchainImagesKHR(device_handle, swapchain->swapchain, &swapchain_image_count, NULL);
    
    VkImage* swapchain_images = malloc(swapchain_image_count * sizeof(VkImage));
    vkGetSwapchainImagesKHR(device_handle, swapchain->swapchain, &swapchain_image_count, swapchain_images);

    swapchain->images = swapchain_images;
    swapchain->image_count = swapchain_image_count;

    int color_format = 0;
    color_format_to_vk(swapchain->color_format, &color_format);
    
    swapchain->image_views = malloc(swapchain_image_count * sizeof(VkImageView));
    for (u32 i = 0; i < swapchain_image_count; i++) {
      VkImageViewCreateInfo image_view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .image = swapchain->images[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = color_format,
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
    
      VkResult image_view_create = vkCreateImageView(device_handle, &image_view_info, NULL, &swapchain->image_views[i]);
      if (image_view_create != VK_SUCCESS) {
        fprintf(stderr, "Failed to create vulkan image view for index %d! %d\n", i, image_view_create);
        return false;
      }
    }
    return true;
}

static void swapchain_free_images(Device* device, Swapchain* swapchain) {
  void* device_handle = NULL;
  device_get_device(device, &device_handle);
  
  for (u32 i = 0; i < swapchain->image_count; i++) {
      vkDestroyImageView(device_handle, swapchain->image_views[i], NULL);
  }
  free(swapchain->image_views);
  free(swapchain->images);
}

static void swapchain_free_resources(Device* device, Swapchain* swapchain) {
  void* device_handle = NULL;
  device_get_device(device, &device_handle);

  swapchain_free_images(device, swapchain);
  vkDestroySwapchainKHR(device_handle, swapchain->swapchain, NULL);
}

SwapchainResult swapchain_new(Device* device, SwapchainOptions options, Swapchain** out_swapchain) {
    void* device_handle = NULL;
    device_get_device(device, &device_handle);

    void* physical_device = NULL;
    device_get_physical_device(device, &physical_device);

    Swapchain* swapchain = malloc(sizeof(Swapchain));

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, options.surface, &surfaceCapabilities);

    u32 width = surfaceCapabilities.currentExtent.width;
    u32 height = surfaceCapabilities.currentExtent.height;

    // FORMAT:VK_FORMAT_B8G8R8A8_SRGB
    // COLORSPACE:VK_COLOR_SPACE_SRGB_NONLINEAR_KHR

    int color_format = 0;
    color_format_to_vk(options.format, &color_format);

    VkSwapchainCreateInfoKHR swapchain_info = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext = NULL,
      .flags = 0,
      .surface = options.surface,
      .minImageCount = options.min_image_count,
      .imageFormat = color_format,
      .imageColorSpace = color_space_to_vk[options.color_space],
      .imageExtent = {width, height},
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = NULL,
      .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = VK_PRESENT_MODE_FIFO_KHR,
      .clipped = VK_TRUE
    };

    if (options.oldSwapchain != NULL) {
      swapchain_info.oldSwapchain = options.oldSwapchain->swapchain;
    }

    VkResult swapchain_create = vkCreateSwapchainKHR(device_handle, &swapchain_info, NULL, &swapchain->swapchain);
    if (swapchain_create != VK_SUCCESS) {
      fprintf(stderr, "Failed to create vulkan swapchain! %d\n", swapchain_create);
      free(swapchain);
      return SWAPCHAIN_ERROR_CREATE_HANDLE_FAIL;
    }

    Extent extent = {width, height};

    swapchain->surface = options.surface;
    swapchain->color_format = options.format;
    swapchain->color_space = options.color_space;
    swapchain->min_image_count = options.min_image_count;
    swapchain->extent = extent;

    if (!swapchain_create_images(device, swapchain)) {
      swapchain_free(device, swapchain);
      return SWAPCHAIN_ERROR_IMAGE_VIEW_FAIL;
    }
    *out_swapchain = swapchain;
    return SWAPCHAIN_OK;
}

void swapchain_free(Device* device, Swapchain* swapchain) {
    swapchain_free_resources(device, swapchain);
    free(swapchain);
}

void swapchain_resize(Device* device, Swapchain* swapchain) {
    device_wait(device);

    Swapchain* new_swapchain = NULL;
    SwapchainResult recreate_swapchain = swapchain_new(device, (SwapchainOptions){
        .oldSwapchain = swapchain,
        .surface = swapchain->surface,
        .min_image_count = swapchain->min_image_count,
        .format = swapchain->color_format,
        .color_space = swapchain->color_space
    }, &new_swapchain);
    if (recreate_swapchain != SWAPCHAIN_OK) {
        fprintf(stderr, "Failed to create new swapchain! %d", recreate_swapchain);
        return;
    }
    
    swapchain_free_resources(device, swapchain);
    *swapchain = *new_swapchain;
    free(new_swapchain);
}

void swapchain_get_swapchain(Swapchain* swapchain, void** out_swapchain) {
  *out_swapchain = swapchain->swapchain;
}

void swapchain_get_surface(Swapchain* swapchain, void** out_surface) {
  *out_surface = swapchain->surface;
}

void swapchain_get_images(Swapchain* swapchain, void** out_images) {
  *out_images = swapchain->images;
}

void swapchain_get_image_views(Swapchain* swapchain, void** out_image_views) {
  *out_image_views = swapchain->image_views;
}

void swapchain_get_extent(Swapchain* swapchain, Extent* out_extent) {
  *out_extent = swapchain->extent;
}

void swapchain_get_color_format(Swapchain* swapchain, ColorFormat* out_color_format) {
  *out_color_format = swapchain->color_format;
}

void swapchain_get_color_space(Swapchain* swapchain, ColorSpace* out_color_space) {
  *out_color_space = swapchain->color_space;
}

void swapchain_get_image_count(Swapchain* swapchain, u32* out_image_count) {
  *out_image_count = swapchain->image_count;
}

