#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "../int_types.h"
#include "device.h"

typedef struct {
    VkSwapchainKHR oldSwapchain;
    VkSurfaceKHR surface;
    u32 min_image_count;
    VkFormat format;
    VkColorSpaceKHR color_space;
} SwapchainOptions;

typedef struct Swapchain Swapchain;

Swapchain* swapchain_new(Device* device, SwapchainOptions options);
void swapchain_free(Device* device, Swapchain* swapchain);
void swapchain_resize(Device* device, Swapchain* swapchain);

VkSwapchainKHR swapchain_get_vk_swapchain(Swapchain* swapchain);
VkSurfaceKHR swapchain_get_vk_surface(Swapchain* swapchain);
VkImage* swapchain_get_vk_images(Swapchain* swapchain);
VkImageView* swapchain_get_vk_image_views(Swapchain* swapchain);
VkExtent2D swapchain_get_extent(Swapchain* swapchain);
VkFormat swapchain_get_format(Swapchain* swapchain);
VkColorSpaceKHR swapchain_get_color_space(Swapchain* swapchain);
u32 swapchain_get_image_count(Swapchain* swapchain);

#endif // SWAPCHAIN_H
