#pragma once

#include "defines.hpp"
#include "resources/material/material.hpp"
#include "renderer/renderer_types.hpp"
#include "systems/resource/resources/mesh/mesh_resource.hpp"

namespace Engine
{
    
    #define DEFAULT_GEOMETRY_NAME "default_geometry"

    struct GeometryCreateInfo {
        u32 vertex_count;
        u32 vertex_element_size;
        void* vertices;
        u32 index_count;
        u32 index_element_size;
        void* indices;
        std::string name;
        Material* material;
        u32 id;
    };

    class Geometry {
        public:
            Geometry(GeometryCreateInfo& info);
            virtual ~Geometry();

            std::string& GetName() { return name; };
            u32 GetId() { return id; };
            u32 GetInternalId() { return interanal_id; };
            u32 GetGeneration() { return generation; };
            void UpdateGeneration () { 
                if (generation == INVALID_ID) {
                    generation = 0;
                    return;
                }
                generation++;
            };

            void SetMaterial(Material* mat) { material = mat; };
            Material* GetMaterial() { return material; };

            void SetInternalId(u32 id) { interanal_id = id; };
            void SetGeneration(u32 gen) { generation = gen; };
            void SetId(u32 id) { id = id; };

            static void GenerateTangents(u32 vertex_count, Vertex3D* vertices, u32 index_count, u32* indices);
            static void GenerateNormals(u32 vertex_count, Vertex3D* vertices, u32 index_count, u32* indices);
            static u32 DeduplicateVertices(u32 vertex_count, Vertex3D* vertices, u32 index_count, u32* indices, Vertex3D** out_vertices);

        protected:
            static void ReassignIndex(u32 index_count, u32* indices, u32 from, u32 to);

            std::string name;
            u32 id;
            u32 generation;
            u32 interanal_id;
            Material* material;
    };

};
