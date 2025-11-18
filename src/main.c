#include <stdlib.h>

#include <vulkan/vulkan.h>
#include <SDL3/SDL_vulkan.h>

#include "game/game.h"
#include "int_types.h"

int main() {
  Game* game = game_new();
  if (!game_start(game)) {
    fprintf(stderr, "Failed to start game!\n");
    return -1;
  }

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

  VkInstance instance = NULL;
  VkResult instance_create = vkCreateInstance(&instance_info, NULL, &instance);
  if (instance_create != VK_SUCCESS) {
    fprintf(stderr, "Failed to create vulkan instance! %d\n", instance_create);
    return -1;
  }

  u32 physical_device_count = 0;
  vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL);
  if (physical_device_count == 0) {
    fprintf(stderr, "Failed to find any physical devices!");
    return -1;
  }

  VkPhysicalDevice physical_devices[physical_device_count];
  vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices);

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
    fprintf(stderr, "Failed to find the best suitable physical device!");
    return -1;
  }

  const char* device_extensions[] = {
    #ifdef __APPLE__
      "VK_KHR_portability_subset"
    #endif
  };
  const char* device_layers[] = {};

  u32 device_extension_count = sizeof(device_extensions) / sizeof(char*);
  u32 device_layer_count = sizeof(device_layers) / sizeof(char*);

  VkDeviceCreateInfo device_info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext = NULL,
    .flags = 0,
    .pQueueCreateInfos = NULL,
    .queueCreateInfoCount = 0,
    .enabledExtensionCount = device_extension_count,
    .ppEnabledExtensionNames = device_extensions,
    .enabledLayerCount = device_layer_count,
    .ppEnabledLayerNames = device_layers
  };
  
  VkDevice device = NULL;
  VkResult device_create = vkCreateDevice(best_device, &device_info, NULL, &device);
  if (device_create != VK_SUCCESS) {
    fprintf(stderr, "Failed to create vulkan device! %d\n", device_create);
    return -1;
  }

  while (game_is_alive(game)) {
    game_update(game);
  }
  vkDestroyDevice(device, NULL);
  vkDestroyInstance(instance, NULL);

  game_close(game);
  return 0;
}