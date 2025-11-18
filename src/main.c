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

  free(all_instance_extensions);

  VkSurfaceKHR surface = NULL;
  SDL_Window* window = game_get_window(game);
  if (!SDL_Vulkan_CreateSurface(window, instance, NULL, &surface)) {
    fprintf(stderr, "Failed to create vulkan surface from SDL3! %s\n ", SDL_GetError());
    return -1;
  }

  u32 physical_device_count = 0;
  VkResult get_physical_devices = vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL);
  if (physical_device_count == 0) {
    fprintf(stderr, "Failed to find any physical devices! %d\n", get_physical_devices);
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
    fprintf(stderr, "Failed to find the best suitable physical device!\n");
    return -1;
  }

  u32 queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(best_device, &queue_family_count, NULL);
  if (queue_family_count == 0) {
    fprintf(stderr, "No queue families found from physical device!\n");
    return -1;
  }

  VkQueueFamilyProperties queue_families[queue_family_count];
  vkGetPhysicalDeviceQueueFamilyProperties(best_device, &queue_family_count, queue_families);

  u32 graphics_family = UINT32_MAX;
  u32 present_family = UINT32_MAX;

  for (u32 i = 0; i < queue_family_count; i++) {
    VkQueueFamilyProperties queue_family = queue_families[i];
    if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT && graphics_family == UINT32_MAX) {
      graphics_family = i;
    }

    VkBool32 presentable = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(best_device, i, surface, &presentable);
    if (presentable == VK_TRUE && present_family == UINT32_MAX) {
      present_family = i;
    }

    if (graphics_family != UINT32_MAX 
        && present_family != UINT32_MAX) {
          break;
    }
  }

  if (graphics_family == UINT32_MAX
      || present_family == UINT32_MAX) {
        fprintf(stderr, "Failed to get graphics and present support!\n");
        return -1;
  }
  
  // FIXME: This looks ugly, use malloc or realloc later
  u32 device_queue_count = 0;
  VkDeviceQueueCreateInfo device_queue_info[2];

  f32 queue_priority = 1.0f;
  VkDeviceQueueCreateInfo graphics_queue_info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .pNext = NULL,
    .flags = 0,
    .pQueuePriorities = &queue_priority,
    .queueFamilyIndex = graphics_family,
    .queueCount = 1,
  };
  device_queue_info[0] = graphics_queue_info;
  device_queue_count++;

  if (present_family != graphics_family) {
    VkDeviceQueueCreateInfo present_queue_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .pQueuePriorities = &queue_priority,
      .queueFamilyIndex = present_family,
      .queueCount = 1,
    };
    device_queue_info[1] = present_queue_info;
    device_queue_count++;
  }

  const char* device_extensions[] = {
    "VK_KHR_swapchain",
    #ifdef __APPLE__
      "VK_KHR_portability_subset"
    #endif
  };
  const char* device_layers[] = {};

  u32 device_extension_count = sizeof(device_extensions) / sizeof(device_extensions[0]);
  u32 device_layer_count = sizeof(device_layers) / sizeof(device_extensions[0]);

  VkDeviceCreateInfo device_info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext = NULL,
    .flags = 0,
    .pQueueCreateInfos = device_queue_info,
    .queueCreateInfoCount = device_queue_count,
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

  int w, h;
  SDL_GetWindowSize(window, &w, &h);

  u32 min_image_count = 2;
  VkFormat swapchain_image_format = VK_FORMAT_B8G8R8A8_SRGB;
  VkSwapchainCreateInfoKHR swapchain_info = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .pNext = NULL,
    .flags = 0,
    .surface = surface,
    .minImageCount = min_image_count,
    .imageFormat = swapchain_image_format,
    .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    .imageExtent = {w, h},
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices = NULL,
    .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = VK_PRESENT_MODE_FIFO_KHR,
    .clipped = VK_TRUE,
    .oldSwapchain = NULL
  };

  VkSwapchainKHR swapchain = NULL;
  VkResult swapchain_create = vkCreateSwapchainKHR(device, &swapchain_info, NULL, &swapchain);
  if (swapchain_create != VK_SUCCESS) {
    fprintf(stderr, "Failed to create vulkan swapchain! %d\n", swapchain_create);
    return -1;
  }

  u32 swapchain_image_count = 0;
  vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, NULL);
  VkImage swapchain_images[swapchain_image_count];
  vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images);

  VkImageView swapchain_image_views[min_image_count];
  for (u32 i = 0; i < swapchain_image_count; i++) {
    VkImageViewCreateInfo image_view_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .image = swapchain_images[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = swapchain_image_format,
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

    VkResult image_view_create = vkCreateImageView(device, &image_view_info, NULL, &swapchain_image_views[i]);
    if (image_view_create != VK_SUCCESS) {
      fprintf(stderr, "Failed to create image view for index %d! %d\n", i, image_view_create);
      return -1;
    }
  }

  while (game_is_alive(game)) {
    game_update(game);
  }

  for (u32 i = 0; i < swapchain_image_count; i++) {
    vkDestroyImageView(device, swapchain_image_views[i], NULL);
  }

  vkDestroySwapchainKHR(device, swapchain, NULL);
  vkDestroySurfaceKHR(instance, surface, NULL);
  vkDestroyDevice(device, NULL);
  vkDestroyInstance(instance, NULL);

  game_close(game);
  return 0;
}