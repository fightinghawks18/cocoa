#include "geometry.h"
#include <stdlib.h>

Geometry* geometry_new(u32 vertex_count, u32 index_count) {
    Geometry* geometry = malloc(sizeof(Geometry));
    geometry->vertices = calloc(vertex_count, sizeof(Vertex));
    geometry->indices = calloc(index_count, sizeof(u32));

    geometry->vertex_count = vertex_count;
    geometry->index_count = index_count;
    return geometry;
}

void geometry_free(Geometry* geometry) {
    if (geometry->indices) {
        free(geometry->indices);
        geometry->indices = NULL;
    }

    if (geometry->vertices) {
        free(geometry->vertices);
        geometry->vertices = NULL;
    }

    free(geometry);
}


void geometry_set_vertex(Geometry* geometry, Vertex vertex, u32 vertex_slot) {
    geometry->vertices[vertex_slot] = vertex;
}

void geometry_set_index(Geometry* geometry, u32 index, u32 index_slot) {
    geometry->indices[index_slot] = index;
}

