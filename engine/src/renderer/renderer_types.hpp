#pragma once

#include "defines.hpp"

namespace Engine {

    enum class BuiltinRenderpasses {
        WORLD = 0x01,
        UI = 0x02
    };

    
    struct Vertex3D {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoord;
        glm::vec4 color;
        glm::vec4 tangent;

        b8 const operator ==(const Vertex3D& v) const {
            return glm::all(glm::epsilonEqual(position, v.position, FLOAT_EPSILON)) &&
                    glm::all(glm::epsilonEqual(normal, v.normal, FLOAT_EPSILON)) &&
                    glm::all(glm::epsilonEqual(texcoord, v.texcoord, FLOAT_EPSILON)) &&
                    glm::all(glm::epsilonEqual(position, v.position, FLOAT_EPSILON)) &&
                    glm::all(glm::epsilonEqual(color, v.color, FLOAT_EPSILON)) &&
                    glm::all(glm::epsilonEqual(tangent, v.tangent, FLOAT_EPSILON));
        };
    };

    

    struct Vertex2D {
        glm::vec2 position;
        glm::vec2 texcoord;
    };

    struct GeometryRenderData {
        u32 object_id;
        glm::mat4 model;
        class Geometry* geometry;
    };
    
    struct RenderPacket {
        f32 delta_time;
        std::vector<GeometryRenderData> geometries;
        std::vector<GeometryRenderData> ui_geometries;
    };
};

namespace std {
    template<> struct hash<glm::vec4> {
        u64 operator()(glm::vec4 const& v) const {
            std::hash<f32> floatHasher;
            return floatHasher(v.x) ^ (floatHasher(v.y) << 1) ^ (floatHasher(v.z) << 2) ^ (floatHasher(v.w) << 3);
        }
    };

    template<> struct hash<glm::vec3> {
        u64 operator()(glm::vec3 const& v) const {
            std::hash<f32> floatHasher;
            return floatHasher(v.x) ^ (floatHasher(v.y) << 1) ^ (floatHasher(v.z) << 2);
        }
    };

    template<> struct hash<glm::vec2> {
        u64 operator()(glm::vec2 const& v) const {
            std::hash<f32> floatHasher;
            return floatHasher(v.x) ^ (floatHasher(v.y) << 1);
        }
    };

    template<> struct hash<Engine::Vertex3D> {
        u64 operator()(Engine::Vertex3D const& vertex) const {
            std::hash<glm::vec2> vec2Hasher;
            std::hash<glm::vec3> vec3Hasher;
            std::hash<glm::vec4> vec4Hasher;

            u64 hash = 17;
            hash = hash * 31 + vec3Hasher(vertex.position);
            hash = hash * 31 + vec3Hasher(vertex.normal);
            hash = hash * 31 + vec2Hasher(vertex.texcoord);
            hash = hash * 31 + vec4Hasher(vertex.color);
            hash = hash * 31 + vec4Hasher(vertex.tangent);
            
            return hash;
        }
    };
}