#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Buffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
    void* mapped;
} Buffer;

static VkSharingMode sharing_mode_to_vk[] = {
    [SHARING_EXCLUSIVE] = VK_SHARING_MODE_EXCLUSIVE,
    [SHARING_CONCURRENT] = VK_SHARING_MODE_CONCURRENT
};

static VkMemoryPropertyFlags memory_mode_to_vk[] = {
    [MEMORY_ACCESS_GPU] = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    [MEMORY_ACCESS_CPU_TO_GPU] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    [MEMORY_ACCESS_GPU_TO_CPU] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                          VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
    [MEMORY_ACCESS_BOTH] = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
};

static VkBufferUsageFlags buffer_usage_to_vk(BufferUsage usage) {
    VkBufferUsageFlags buffer_usage_flags = 0;

    if (usage & BUFFER_TRANSFER_SRC) buffer_usage_flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (usage & BUFFER_TRANSFER_DST) buffer_usage_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (usage & BUFFER_UNIFORM_TEXEL) buffer_usage_flags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    if (usage & BUFFER_STORAGE_TEXEL) buffer_usage_flags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    if (usage & BUFFER_UNIFORM) buffer_usage_flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (usage & BUFFER_STORAGE) buffer_usage_flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (usage & BUFFER_INDEX) buffer_usage_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (usage & BUFFER_VERTEX) buffer_usage_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (usage & BUFFER_INDIRECT) buffer_usage_flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    return buffer_usage_flags;
}

static u32 find_memory_type(Device* device, u32 type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(device_get_vk_physical_device(device), &memory_properties);

    for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
        if (type_filter & (1 << i) && memory_properties.memoryTypes[i].propertyFlags & properties) {
            return i;
        }
    }
    fprintf(stderr, "Failed to find memory index type for properties %d!\n", properties);
    return UINT32_MAX;
}

Buffer* buffer_new(Device* device, BufferOptions options) {
    Buffer* buffer = malloc(sizeof(Buffer));
    buffer->buffer = NULL;
    buffer->mapped = NULL;
    buffer->memory = NULL;

    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .size = options.size,
        .usage = buffer_usage_to_vk(options.usage),
        .sharingMode = sharing_mode_to_vk[options.sharing],
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL
    };

    VkResult create_buffer = vkCreateBuffer(
        device_get_vk_device(device), 
        &buffer_info, 
        NULL, 
        &buffer->buffer
    );
    if (create_buffer != VK_SUCCESS) {
        fprintf(stderr, "Failed to create vulkan buffer! %d\n", create_buffer);
        buffer_free(device, buffer);
        return NULL;
    }

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(device_get_vk_device(device), buffer->buffer, &mem_requirements);

    VkMemoryAllocateInfo memory_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = options.size,
        .memoryTypeIndex = find_memory_type(device, mem_requirements.memoryTypeBits, memory_mode_to_vk[options.memory_access])
    };
    VkResult create_memory = vkAllocateMemory(device_get_vk_device(device), &memory_info, NULL, &buffer->memory);
    if (create_memory != VK_SUCCESS) {
        fprintf(stderr, "Failed to create vulkan buffer memory! %d\n", create_buffer);
        buffer_free(device, buffer);
        return NULL;
    }

    VkResult bind_buffer_to_memory = vkBindBufferMemory(
        device_get_vk_device(device), 
        buffer->buffer, 
        buffer->memory, 
        0
    );
    if (bind_buffer_to_memory != VK_SUCCESS) {
        fprintf(stderr, "Failed to bind vulkan buffer memory! %d\n", create_buffer);
        buffer_free(device, buffer);
        return NULL;
    }
    return buffer;
}

void buffer_free(Device* device, Buffer* buffer) {
    if (buffer->buffer) {
        vkDestroyBuffer(device_get_vk_device(device), buffer->buffer, NULL);
        buffer->buffer = NULL;
    }

    if (buffer->memory) {
        vkFreeMemory(device_get_vk_device(device), buffer->memory, NULL);
        buffer->memory = NULL;
    }

    buffer->mapped = NULL;
    free(buffer);
}

void buffer_map(Device* device, Buffer* buffer, u64 size, void* data) {
    vkMapMemory(device_get_vk_device(device), buffer->memory, 0, size, 0, &buffer->mapped);
    memcpy(buffer->mapped, data, size);
    vkUnmapMemory(device_get_vk_device(device), buffer->memory);
}

VkBuffer buffer_get_vk_buffer(Buffer* buffer) {
    return buffer->buffer;
}
