#ifndef DEVICE_H
#define DEVICE_H

#include <vulkan/vulkan.h>
#include "../int_types.h"

typedef struct Device Device;

Device* device_new();
void device_free(Device* device);

void device_wait(Device* device);

VkInstance device_get_vk_instance(Device* device);
VkDevice device_get_vk_device(Device* device);
VkPhysicalDevice device_get_vk_physical_device(Device* device);
u32 device_get_graphics_family(Device* device);
VkQueue device_get_graphics_queue(Device* device);

#endif // DEVICE_H
