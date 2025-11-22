#ifndef DEVICE_H
#define DEVICE_H

#include "../int_types.h"

typedef enum {
    DEVICE_OK, // Successfully created a device
    DEVICE_ERROR_CREATE_HANDLE_FAIL, // Failed to create a handle for the device
    DEVICE_ERROR_NO_GPUS, // Failed to find any gpu
    DEVICE_ERROR_NO_SUITABLE_GPU, // Failed to find any gpu that would've been suitable
    DEVICE_ERROR_NO_QUEUE_FAMILIES, // Failed to find any queue family (graphics queue, present queue, compute queue, etc..)
} DeviceResult;

typedef struct Device Device;

DeviceResult device_new(Device** out_device);
void device_free(Device* device);

void device_wait(Device* device);

void device_get_instance(Device* device, void** out_instance);
void device_get_device(Device* device, void** out_device);
void device_get_physical_device(Device* device, void** out_physical_device);
void device_get_graphics_family(Device* device, u32* out_graphics_family);
void device_get_graphics_queue(Device* device, void** out_graphics_queue);

#endif // DEVICE_H
