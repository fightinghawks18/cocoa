#include "device.h"
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL_vulkan.h>

typedef struct Device {
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice physical_device;

    u32 graphics_family;

    VkQueue graphics_queue;
} Device;

Device* device_new() {
    Device* device = malloc(sizeof(Device));
    device->device = NULL;

    u32 api_version = VK_MAKE_API_VERSION(0, 1, 3, 0);
    u32 app_version = VK_MAKE_API_VERSION(0, 0, 1, 0);
    u32 engine_version = VK_MAKE_API_VERSION(0, 0, 1, 0);
    
    u32 sdl_extension_count = 0;
    const char* const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count);
    
    const char* instance_extensions[] = {};
    const char* instance_layers[] = {
      "VK_LAYER_KHRONOS_validation"
    };
  
    u32 instance_layer_count = sizeof(instance_layers) / sizeof(instance_layers[0]);
    u32 instance_extensions_count = sizeof(instance_extensions) / sizeof(instance_extensions[0]);
    u32 extension_total_count = instance_extensions_count + sdl_extension_count;
  
    const char** all_instance_extensions = malloc(extension_total_count * sizeof(char*));
    memcpy(all_instance_extensions, 
          sdl_extensions, sdl_extension_count * sizeof(char*));
    memcpy(all_instance_extensions + sdl_extension_count,
          instance_extensions, instance_extensions_count * sizeof(char*));
    
    VkApplicationInfo app_info = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = NULL,
      .pApplicationName = "Cocoa",
      .applicationVersion = app_version,
      .pEngineName = "Cocoa",
      .engineVersion = engine_version,
      .apiVersion = api_version
    };
  
    VkInstanceCreateInfo instance_info = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .pApplicationInfo = &app_info,
      .enabledLayerCount = instance_layer_count,
      .ppEnabledLayerNames = instance_layers,
      .enabledExtensionCount = extension_total_count,
      .ppEnabledExtensionNames = all_instance_extensions
    };
  
    #ifdef __APPLE__
      instance_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    #endif
  
    VkResult instance_create = vkCreateInstance(&instance_info, NULL, &device->instance);
    if (instance_create != VK_SUCCESS) {
      fprintf(stderr, "Failed to create vulkan instance! %d\n", instance_create);
      device_free(device);
      return NULL;
    }
  
    free(all_instance_extensions);
  
    u32 physical_device_count = 0;
    VkResult get_physical_devices = vkEnumeratePhysicalDevices(device->instance, &physical_device_count, NULL);
    if (physical_device_count == 0) {
      fprintf(stderr, "Failed to find any physical devices! %d\n", get_physical_devices);
      device_free(device);
      return NULL;
    }
  
    VkPhysicalDevice physical_devices[physical_device_count];
    vkEnumeratePhysicalDevices(device->instance, &physical_device_count, physical_devices);
  
    u32 best_score = 0;
    VkPhysicalDevice best_device = NULL;
    for (u32 i = 0; i < physical_device_count; i++) {
      u32 score = 0;
      
      VkPhysicalDeviceProperties properties;
      vkGetPhysicalDeviceProperties(physical_devices[i], &properties);
      VkPhysicalDeviceFeatures features;
      vkGetPhysicalDeviceFeatures(physical_devices[i], &features);
    
      switch (properties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
          score += 1000;
          break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
          score += 500;
          break;
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
        case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
          break;
      }
    
      score += properties.limits.maxImageDimension2D;
    
      if (score > best_score) {
        best_score = score;
        best_device = physical_devices[i];
      }
    }
  
    if (best_device == NULL) {
      fprintf(stderr, "Failed to find the best suitable physical device!\n");
      device_free(device);
      return NULL;
    }
    device->physical_device = best_device;
  
    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(best_device, &queue_family_count, NULL);
    if (queue_family_count == 0) {
      fprintf(stderr, "No queue families found from physical device!\n");
      device_free(device);
      return NULL;
    }
  
    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(best_device, &queue_family_count, queue_families);
  
    u32 graphics_family = UINT32_MAX;
  
    for (u32 i = 0; i < queue_family_count; i++) {
      VkQueueFamilyProperties queue_family = queue_families[i];
      if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        graphics_family = i;
        break;
      }
    }
  
    if (graphics_family == UINT32_MAX) {
          fprintf(stderr, "Failed to get graphics support!\n");
          device_free(device);
          return NULL;
    }

    device->graphics_family = graphics_family;
  
    f32 queue_priority = 1.0f;
    VkDeviceQueueCreateInfo graphics_queue_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .pQueuePriorities = &queue_priority,
      .queueFamilyIndex = graphics_family,
      .queueCount = 1,
    };
  
    const char* device_extensions[] = {
      "VK_KHR_swapchain",
      "VK_EXT_extended_dynamic_state",
      "VK_EXT_extended_dynamic_state2",
      "VK_EXT_extended_dynamic_state3"
    };
    const char* device_layers[] = {};
  
    u32 device_extension_count = sizeof(device_extensions) / sizeof(device_extensions[0]);
    u32 device_layer_count = sizeof(device_layers) / sizeof(device_extensions[0]);

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
      .extendedDynamicState = VK_TRUE
    };

    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
      .pNext = &extended_dynamic_state_features,
      .extendedDynamicState2 = VK_TRUE,
      .extendedDynamicState2LogicOp = VK_TRUE,
      .extendedDynamicState2PatchControlPoints = VK_TRUE
    };

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
      .pNext = &extended_dynamic_state2_features,
      .extendedDynamicState3PolygonMode = VK_TRUE,
      .extendedDynamicState3TessellationDomainOrigin = VK_TRUE
    };
  
    VkPhysicalDeviceVulkan13Features physical_device_vulkan_13_features = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .pNext = &extended_dynamic_state3_features,
      .dynamicRendering = VK_TRUE,
      .synchronization2 = VK_TRUE
    };
  
    VkDeviceCreateInfo device_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &physical_device_vulkan_13_features,
      .flags = 0,
      .pQueueCreateInfos = &graphics_queue_info,
      .queueCreateInfoCount = 1,
      .enabledExtensionCount = device_extension_count,
      .ppEnabledExtensionNames = device_extensions,
      .enabledLayerCount = device_layer_count,
      .ppEnabledLayerNames = device_layers
    };
    
    VkResult device_create = vkCreateDevice(best_device, &device_info, NULL, &device->device);
    if (device_create != VK_SUCCESS) {
      fprintf(stderr, "Failed to create vulkan device! %d\n", device_create);
      device_free(device);
      return NULL;
    }
  
    vkGetDeviceQueue(device->device, graphics_family, 0, &device->graphics_queue);

  
    return device;
}

void device_free(Device* device) {
    if (device->device != NULL) {
        vkDestroyDevice(device->device, NULL);
        device->device = NULL;
    }
    
    if (device->instance != NULL) {
        vkDestroyInstance(device->instance, NULL);
        device->instance = NULL;
    }

    device->physical_device = NULL;
    device->graphics_family = UINT32_MAX;
    device->graphics_queue = NULL;
    free(device);
}

void device_wait(Device* device) {
    vkDeviceWaitIdle(device->device);
}

VkInstance device_get_vk_instance(Device* device) {
    return device->instance;
}

VkDevice device_get_vk_device(Device* device) {
    return device->device;
}

VkPhysicalDevice device_get_vk_physical_device(Device* device) {
    return device->physical_device;
}

u32 device_get_graphics_family(Device* device) {
    return device->graphics_family;
}

VkQueue device_get_graphics_queue(Device* device) {
    return device->graphics_queue;
}
