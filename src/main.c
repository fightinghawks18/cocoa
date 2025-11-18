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
  VkResult result = vkCreateInstance(&instance_info, NULL, &instance);
  if (result != VK_SUCCESS) {
    fprintf(stderr, "Failed to create vulkan instance! %d\n", result);
    return -1;
  }

  while (game_is_alive(game)) {
    game_update(game);
  }
  vkDestroyInstance(instance, NULL);

  game_close(game);
  return 0;
}