#pragma once
#include <resources/mesh/mesh.hpp>
#include <vector>

class DemoScene {
public:
    DemoScene();
    ~DemoScene();
    bool Initialize();

private:
    std::vector<Engine::Mesh*> meshes;
    Engine::Geometry* test_ui_geometry;
    void CreateMeshes();
};
