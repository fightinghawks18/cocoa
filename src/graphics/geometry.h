#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "../int_types.h"

typedef struct {
    f32 pos[3];
    f32 col[4];
} Vertex;

typedef struct {
    Vertex* vertices;
    u32* indices;

    u32 vertex_count;
    u32 index_count;
} Geometry;

Geometry* geometry_new(u32 vertex_count, u32 index_count);
void geometry_free(Geometry* geometry);

void geometry_set_vertex(Geometry* geometry, Vertex vertex, u32 vertex_slot);
void geometry_set_index(Geometry* geometry, u32 index, u32 index_slot);

#endif // GEOMETRY_H
