#include "demo_scene.hpp"
#include <systems/geometry/geometry_system.hpp>
#include <systems/material/material_system.hpp>
#include <systems/resource/resource_system.hpp>
#include <renderer/renderer.hpp>

using namespace Engine;

DemoScene::DemoScene() {
    Initialize();
}

DemoScene::~DemoScene() {
    for (auto mesh : meshes)
        delete mesh;
}

bool DemoScene::Initialize() {
    CreateMeshes();
    return true;
}

void DemoScene::CreateMeshes() {
    GeometrySystem* gs = GeometrySystem::GetInstance();
    MaterialSystem* ms = MaterialSystem::GetInstance();

    GeometryConfig g_config = gs->GenerateCubeConfig(10.0f, 10.0f, 10.0f, 1.0f, 1.0f, "test_cube", "test");
    MeshCreateConfig cube_01{};
    cube_01.geometries.push_back(gs->AcquireGeometryFromConfig(g_config, true));
    cube_01.transform = new Transform();
            
    Mesh* parent = new Mesh(cube_01);
    meshes.push_back(parent);

    gs->DisposeConfig(g_config);
            
    GeometryConfig g_config2 = gs->GenerateCubeConfig(5.0f, 5.0f, 5.0f, 1.0f, 1.0f, "test_cube2", "test");
    MeshCreateConfig cube_02{};
    cube_02.geometries.push_back(gs->AcquireGeometryFromConfig(g_config2, true));
    cube_02.transform = new Transform(glm::vec3(15.0f, 0.0f, 0.0f), parent->transform);

    Mesh* second = new Mesh(cube_02);
    meshes.push_back(second);

    gs->DisposeConfig(g_config2);

    GeometryConfig g_config3 = gs->GenerateCubeConfig(3.0f, 3.0f, 3.0f, 1.0f, 1.0f, "test_cube3", "test");
    MeshCreateConfig cube_03{};
    cube_03.geometries.push_back(gs->AcquireGeometryFromConfig(g_config3, true));
    cube_03.transform = new Transform(glm::vec3(5.0f, 0.0f, 0.0f), second->transform);

    meshes.push_back(new Mesh(cube_03));

    gs->DisposeConfig(g_config3);

    MeshResource* mesh_resource = (MeshResource*)ResourceSystem::GetInstance()->LoadResource(ResourceType::MESH, "sponza");
    MeshCreateConfig sponza{};
    GeometryConfigs configs = mesh_resource->GetConfigs();
    for (u32 i = 0; i < configs.size(); ++i) {
        sponza.geometries.push_back(gs->AcquireGeometryFromConfig(configs[i], true));
    }
    sponza.transform = new Transform(glm::vec3(0,0,0), glm::identity<glm::quat>(), glm::vec3(0.05f, 0.05f, 0.05f));
    meshes.push_back(new Mesh(sponza));
    delete mesh_resource;

    MeshResource* mesh_resource2 = (MeshResource*)ResourceSystem::GetInstance()->LoadResource(ResourceType::MESH, "falcon");
    MeshCreateConfig falcon{};
    GeometryConfigs configs2 = mesh_resource2->GetConfigs();
    for (u32 i = 0; i < configs2.size(); ++i) {
        falcon.geometries.push_back(gs->AcquireGeometryFromConfig(configs2[i], true));
    }
    falcon.transform = new Transform(glm::vec3(30,0,0), glm::identity<glm::quat>());
    meshes.push_back(new Mesh(falcon));
    delete mesh_resource2;

    RendererFrontend::GetInstance()->meshes = meshes;

    // Load up some test UI geometry.
    // GeometryConfig ui_config;
    // ui_config.vertex_size = sizeof(Vertex2D);
    // ui_config.vertex_count = 4;
    // ui_config.index_size = sizeof(u32);
    // ui_config.index_count = 6;
    // ui_config.material_name = "test_ui";
    // ui_config.name = "test_ui_geometry";

    // const f32 width = 128.0f;
    // const f32 height = 36.0f;
    // std::vector<Vertex2D> uiverts(4);
    // uiverts[0].position.x = 0.0f;  // 0    3
    // uiverts[0].position.y = 0.0f;  //
    // uiverts[0].texcoord.x = 0.0f;  //
    // uiverts[0].texcoord.y = 0.0f;  // 2    1

    // uiverts[1].position.y = height;
    // uiverts[1].position.x = width;
    // uiverts[1].texcoord.x = 1.0f;
    // uiverts[1].texcoord.y = 1.0f;

    // uiverts[2].position.x = 0.0f;
    // uiverts[2].position.y = height;
    // uiverts[2].texcoord.x = 0.0f;
    // uiverts[2].texcoord.y = 1.0f;

    // uiverts[3].position.x = width;
    // uiverts[3].position.y = 0.0;
    // uiverts[3].texcoord.x = 1.0f;
    // uiverts[3].texcoord.y = 0.0f;
    // ui_config.vertices = uiverts.data();

    // // Indices - counter-clockwise
    // u32 uiindices[6] = {2, 1, 0, 3, 0, 1};
    // ui_config.indices = uiindices;

    // // Get UI geometry from config.
    // test_ui_geometry = gs->AcquireGeometryFromConfig(ui_config, true);
}