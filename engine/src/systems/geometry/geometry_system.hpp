#pragma once

#include "defines.hpp"
#include "resources/geometry/geometry.hpp"

namespace Engine {

    struct GeometryReference {
        Geometry* geometry;
        b8 auto_release;
        u32 ref_count;
    };

    class ENGINE_API GeometrySystem {
        public:
            GeometrySystem();
            ~GeometrySystem();

            static b8 Initialize();
            static void Shutdown();
            static GeometrySystem* GetInstance() { return instance; };

            Geometry* AcquireGeometryById(u32 id);
            void ReleaseGeometry(u32 id);

            u32 GetNewGeometryId();

            Geometry* GetDefaultGeometry();

            void CreateDefaultGeometries();
            void DestroyDefaultGeometries();

            GeometryConfig GeneratePlainConfig(
                f32 width, f32 height, 
                u32 x_segment_count, u32 y_segment_count, 
                f32 tile_x, f32 tile_y, 
                std::string name, std::string material_name);
            
            GeometryConfig GenerateCubeConfig(
                f32 width, f32 height, f32 depth,
                f32 tile_x, f32 tile_y, 
                std::string name, std::string material_name);

            void DisposeConfig(GeometryConfig& config);

            Geometry* AcquireGeometryFromConfig(GeometryConfig config, b8 auto_release);

        private:
            static GeometrySystem* instance;

            Geometry* default_geometry;
            std::vector<GeometryReference> registered_geometries;
    };
} 
