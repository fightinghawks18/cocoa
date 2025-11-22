#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "../int_types.h"
#include "device.h"
#include "formats.h"

typedef struct Swapchain Swapchain;

typedef enum {
    SWAPCHAIN_OK, // Successfully created a swapchain
    SWAPCHAIN_ERROR_CREATE_HANDLE_FAIL, // Failed to create a handle for the swapchain
    SWAPCHAIN_ERROR_IMAGE_VIEW_FAIL, // Failed to create a view (how we can access and modify images) for the swapchain images
} SwapchainResult;

typedef enum {
    COLOR_SPACE_SRGB_NLINEAR
} ColorSpace;

typedef struct {
    Swapchain* oldSwapchain;
    void* surface;
    u32 min_image_count;
    ColorFormat format;
    ColorSpace color_space;
} SwapchainOptions;

typedef struct {
    u32 width;
    u32 height;
} Extent;

SwapchainResult swapchain_new(Device* device, SwapchainOptions options, Swapchain** out_swapchain);
void swapchain_free(Device* device, Swapchain* swapchain);
void swapchain_resize(Device* device, Swapchain* swapchain);

void swapchain_get_swapchain(Swapchain* swapchain, void** out_swapchain);
void swapchain_get_surface(Swapchain* swapchain, void** out_surface);
void swapchain_get_images(Swapchain* swapchain, void** out_images);
void swapchain_get_image_views(Swapchain* swapchain, void** out_image_views);
void swapchain_get_extent(Swapchain* swapchain, Extent* out_extent);
void swapchain_get_color_format(Swapchain* swapchain, ColorFormat* out_color_format);
void swapchain_get_color_space(Swapchain* swapchain, ColorSpace* out_color_space);
void swapchain_get_image_count(Swapchain* swapchain, u32* out_image_count);

#endif // SWAPCHAIN_H
