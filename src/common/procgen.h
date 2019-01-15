#ifndef _PROCGEN_2018_12_29_H_
#define _PROCGEN_2018_12_29_H_

#include "glm_usage.h"
#include <vector>

using index_t = unsigned int;

struct triangle_t
{
    index_t vertex_index[3];
};

struct vertex_t
{
    glm::vec3 p;
    glm::vec3 n;
    glm::vec2 uv; // TODO: 3 components for cubemap
};

using TriangleList = std::vector<triangle_t>;
using IndexList = std::vector<index_t>;
using VertexList = std::vector<vertex_t>;
using IndexedMesh = struct { VertexList vertices; IndexList indices; };

IndexedMesh make_icosphere(int subdivisions, float radius = 1.0f);
IndexedMesh make_uvsphere(unsigned int subdiv_lat=5, unsigned int subdiv_long=10, float radius = 1.0f);
IndexedMesh make_flat_cube(float width = 1.0f, float height = 1.0f, float depth = 1.0f);
IndexedMesh make_hexagon(float width, float height, glm::vec3 normal = glm::vec3(0, 0, 1));

struct loaded_image
{
    uint32_t width;
    uint32_t height;
    uint32_t size;
    void *data;
};

void create_checker_base_image(loaded_image *);
void create_checker_spec_image(loaded_image *);
void create_neutral_base_image(loaded_image *);
void create_neutral_metal_spec_image(loaded_image *);
void create_neutral_dielectric_spec_image(loaded_image *);

#endif // !_PROCGEN_2018_12_29_H_
