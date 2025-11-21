#ifndef BUFFER_H
#define BUFFER_H

#include "device.h"
#include "../int_types.h"

typedef enum BufferUsageFlags {
    BUFFER_NO_USE = 0,
    BUFFER_TRANSFER_SRC = 1 << 0,
    BUFFER_TRANSFER_DST = 2 << 0,
    BUFFER_UNIFORM_TEXEL = 4 << 0,
    BUFFER_STORAGE_TEXEL = 8 << 0,
    BUFFER_UNIFORM = 1 << 1,
    BUFFER_STORAGE = 2 << 1,
    BUFFER_INDEX = 4 << 1,
    BUFFER_VERTEX = 8 << 1,
    BUFFER_INDIRECT = 1 << 2
} BufferUsage;

typedef enum SharingMode {
    SHARING_EXCLUSIVE,
    SHARING_CONCURRENT
} SharingMode;

typedef enum MemoryAccessMode {
    MEMORY_ACCESS_GPU,
    MEMORY_ACCESS_CPU_TO_GPU,
    MEMORY_ACCESS_GPU_TO_CPU,
    MEMORY_ACCESS_BOTH
} MemAccessMode;

typedef struct {
    u64 size;
    BufferUsage usage;
    SharingMode sharing;
    MemAccessMode memory_access;
    void* initial_data;
} BufferOptions;

typedef struct Buffer Buffer;

Buffer* buffer_new(Device* device, BufferOptions options);
void buffer_free(Device* device, Buffer* buffer);

void buffer_map(Device* device, Buffer* buffer, u64 size, void* data);
VkBuffer buffer_get_vk_buffer(Buffer* buffer);

#endif // BUFFER_H
