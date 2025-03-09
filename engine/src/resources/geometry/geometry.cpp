#include "geometry.hpp"

#include "core/logger/logger.hpp"
#include "platform/platform.hpp"

namespace Engine {

    Geometry::Geometry(GeometryCreateInfo& info) {
        id = info.id;
        internal_id = info.id;
        generation = INVALID_ID;
        name = info.name;
        material = info.material;
    }

    Geometry::~Geometry() {

    }

    void Geometry::GenerateTangents(u32 vertex_count, Vertex3D* vertices, u32 index_count, u32* indices) {
         for (u32 i = 0; i < index_count; i += 3) {
            u32 i0 = indices[i + 0];
            u32 i1 = indices[i + 1];
            u32 i2 = indices[i + 2];

            glm::vec3 edge1 = vertices[i1].position - vertices[i0].position;
            glm::vec3 edge2 = vertices[i2].position - vertices[i0].position;

            f32 deltaU1 = vertices[i1].texcoord.x - vertices[i0].texcoord.x;
            f32 deltaV1 = vertices[i1].texcoord.y - vertices[i0].texcoord.y;

            f32 deltaU2 = vertices[i2].texcoord.x - vertices[i0].texcoord.x;
            f32 deltaV2 = vertices[i2].texcoord.y - vertices[i0].texcoord.y;

            f32 dividend = (deltaU1 * deltaV2 - deltaU2 * deltaV1);
            f32 fc = 1.0f / dividend;

            glm::vec3 tangent = (glm::vec3){
                (fc * (deltaV2 * edge1.x - deltaV1 * edge2.x)),
                (fc * (deltaV2 * edge1.y - deltaV1 * edge2.y)),
                (fc * (deltaV2 * edge1.z - deltaV1 * edge2.z))};

            tangent = glm::normalize(tangent);

            f32 sx = deltaU1, sy = deltaU2;
            f32 tx = deltaV1, ty = deltaV2;
            f32 handedness = ((deltaU1 * deltaV2 - deltaU2 * deltaV1) < 0.0f) ? -1.0f : 1.0f;
            glm::vec4 t4 = glm::vec4(tangent, handedness);
            vertices[i0].tangent = t4;
            vertices[i1].tangent = t4;
            vertices[i2].tangent = t4;
        }

    };

    void Geometry::GenerateNormals(u32 vertex_count, Vertex3D* vertices, u32 index_count, u32* indices) {
        for (u32 i = 0; i < index_count; i += 3) {
            i32 triangle_num = 1;
            if (i != 0) {
                triangle_num = (i / 3) + 1;
            }

            if (triangle_num == 7) {
                u32 kasd = 123;
                kasd++;
            }

            u32 i0 = indices[i + 0];
            u32 i1 = indices[i + 1];
            u32 i2 = indices[i + 2];

            glm::vec3 edge1 = vertices[i1].position - vertices[i0].position;
            glm::vec3 edge2 = vertices[i2].position - vertices[i0].position;

            glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

            // NOTE: This just generates a face normal. Smoothing out should be done in a separate pass if desired.
            vertices[i0].normal = normal;
            vertices[i1].normal = normal;
            vertices[i2].normal = normal;
        }
    };

    void Geometry::ReassignIndex(u32 index_count, u32* indices, u32 from, u32 to) {
        for (u32 i = 0; i < index_count; ++i) {
            if (indices[i] == from) {
                indices[i] = to;
            } else if (indices[i] > from) {
                indices[i]--;
            }
        }
    };

    u32 Geometry::DeduplicateVertices(u32 vertex_count, Vertex3D* vertices, u32 index_count, u32* indices, Vertex3D** out_vertices) {
        Vertex3D* unique_verts = (Vertex3D*)Platform::AllocMemory(sizeof(Vertex3D) * vertex_count);
        u32 unique_vertex_count = 0;

        u32 found_count = 0;
        for (u32 i = 0; i <  vertex_count; ++i) {
            b8 found = false;

            for (u32 j = 0; j < unique_vertex_count; ++j) {
                if (vertices[i] == vertices[j]) {
                    ReassignIndex(index_count, indices, i - found_count, j);
                    found = true;
                    found_count++;
                    break;
                }
            }

            if (!found) {
                unique_verts[unique_vertex_count] = vertices[i];
                unique_vertex_count++;
            }
        }

        *out_vertices = (Vertex3D*)Platform::AllocMemory(sizeof(Vertex3D) * unique_vertex_count);
        Platform::CpMemory(*out_vertices, unique_verts, sizeof(Vertex3D) * unique_vertex_count);

        Platform::FrMemory(unique_verts);

        u32 removed_count = vertex_count - unique_vertex_count;
        DEBUG("Geometry::DeduplicateVertices - removed %d duiplicates, before: %d -> after: %d", removed_count, vertex_count, unique_vertex_count);

        return unique_vertex_count;
    };

}