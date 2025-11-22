#ifndef RENDERER_H
#define RENDERER_H

#include "../int_types.h"
#include "swapchain.h"
#include "device.h"

typedef struct Frame Frame;

typedef struct Renderer Renderer;

typedef enum {
    RENDERER_OK, // Successfully created renderer
    RENDERER_ERROR_CREATE_FRAME_FAIL, // Failed to create a frame for the renderer
} RendererResult;

typedef enum {
    RENDER_BEGIN_OK, // Successfully started rendering
    RENDER_BEGIN_REBUILD_SWAPCHAIN, // Swapchain is suboptimal/out-of-date, must rebuild before rendering (DON'T TREAT AS AN ERROR!!)
    RENDER_BEGIN_ERROR_FENCE_WAIT_FAIL, // Failed to wait for the frame to finish rendering before reusing
    RENDER_BEGIN_ERROR_IMAGE_ACQUIRE_NEXT_FAIL, // Failed to acquire the next image in the swapchain to render to
    RENDER_BEGIN_ERROR_FENCE_RESET_FAIL, // Failed to reset the signal that tells us when rendering is finished
    RENDER_BEGIN_ERROR_RECORD_START_FAIL // Failed to start recording commands
} RenderBeginResult;

typedef enum {
    RENDER_END_OK, // Successfully stopped rendering and presented data to swapchain
    RENDER_END_ERROR_RECORD_STOP_FAIL,  // Failed to stop recording commands
    RENDER_END_ERROR_SUBMIT_FAIL, // Failed to submit commands
    RENDER_END_ERROR_PRESENT_FAIL // Failed to present data to swapchain
} RenderEndResult;

RendererResult renderer_new(Device* device, u32 max_frames_in_flight, Renderer** out_renderer);
void renderer_free(Device* device, Renderer* renderer);

RenderBeginResult renderer_begin_rendering(Device* device, Renderer* renderer, Swapchain* swapchain, Frame** out_frame);
RenderEndResult renderer_end_rendering(Device* device, Renderer* renderer);
void renderer_rebuild_resources(Device* device, Renderer* renderer);

void renderer_get_swapchain(Renderer* renderer, Swapchain** out_swapchain);
void renderer_get_image_index(Renderer* renderer, u32* out_image_index);

void renderer_get_frame_cmd(Frame* frame, void** out_cmd);

#endif // RENDERER_H
