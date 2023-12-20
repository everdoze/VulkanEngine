#pragma once

#include "defines.hpp"
#include "resources/material/material.hpp"
#include "renderer/renderer_types.inl"

namespace Engine
{
    
    #define DEFAULT_GEOMETRY_NAME "default_geometry"

    enum class GeometryType {
        GEOMETRY_2D,
        GEOMETRY_3D
    };

    struct GeometryConfig {
        u32 vertex_size;
        u32 vertex_count;
        void* vertices;
        GeometryType type;
        u32 index_size;
        u32 index_count;
        void* indices;
        std::string name;
        std::string material_name;
    };

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
            Material* GetMaterial() { return material; };

            void SetInternalId(u32 id) { interanal_id = id; };
            void SetGeneration(u32 gen) { generation = gen; };
            void SetId(u32 id) { id = id; };

        protected:
            std::string name;
            u32 id;
            u32 generation;
            u32 interanal_id;
            Material* material;
    };

};
