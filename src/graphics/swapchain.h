#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <vulkan/vulkan.h>
#include "../int_types.h"

typedef struct {
    VkSwapchainKHR oldSwapchain;
    VkSurfaceKHR surface;
    u32 min_image_count;
    VkFormat format;
    VkColorSpaceKHR color_space;
} SwapchainOptions;

typedef struct {
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

Swapchain* swapchain_new(VkDevice device, VkPhysicalDevice physical_device, SwapchainOptions options);
void swapchain_free(VkDevice device, Swapchain* swapchain);
Swapchain* swapchain_resize(VkDevice device, VkPhysicalDevice physical_device, Swapchain* swapchain);

#endif // SWAPCHAIN_H