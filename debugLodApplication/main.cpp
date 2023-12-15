#include "DebugLodApplication.h"
#include "Encode.h"
#include "Mesh.h"
#include "VirtualMesh.h"
#include "timer.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "test.h"

int main()
{
    std::cerr << "This application is used for debug lod of virtual mesh.\n";

    Vk::RenderConfig config {};
    config.width = 1920;
    config.height = 1080;
    config.isWireFrame = false;
    config.useInstance = true;

    bool isRebuildVirtualMesh = true;

    Util::Timer timer;
    const std::string modelFileName = "../assets/models/LNWDMEPB1.ifc.midfile";
    std::string packedfileName = modelFileName.substr(0, modelFileName.find_last_of('.')) + ".txt";
    std::vector<uint32_t> packedData;

    ifstream is(modelFileName.c_str(), std::ios::binary);
    vector<vector<uint32_t>> the_c_edge_indices;
    vector<float> the_face_area;
    vector<vector<uint32_t>> the_f_ver_indices;
    vector<uint32_t> the_face_nums_in_comp;
    vector<uint32_t> the_collision_pairs;
    Parser t;
    Parser::Datas2OpenGL ge = t.read_datas2OpenGL_from_binary(is, the_c_edge_indices, the_face_area, the_f_ver_indices, the_face_nums_in_comp, the_collision_pairs);
    
    timer.reset();
    auto& compIndices = ge.search_m;
    auto& storyCompIndices = ge.storeys_component_id;

    /*// Gen total mesh lod ----------------------------------------------------------------
    Core::Mesh mesh;
    std::cout << ge.verts.size() << " " << ge.vert_indices.size() << "\n";
    mesh.vertices.resize(ge.verts.size() / 3);
    for (int i = 0; i < ge.verts.size() / 3; i++) {
        mesh.vertices[i] = glm::vec3(ge.verts[i * 3 + 0], ge.verts[i * 3 + 1], ge.verts[i * 3 + 2]);
    }
    mesh.indices.swap(ge.vert_indices);
    mesh.normals.resize(ge.vert_normals2.size() / 3);
    for (int i = 0; i < ge.vert_normals2.size() / 3; i++) {
        mesh.normals[i] = glm::vec3(ge.vert_normals2[i * 3 + 0], ge.vert_normals2[i * 3 + 1], ge.vert_normals2[i * 3 + 2]);
    }
    vmeshes[0].Build(mesh);*/

    /*// Gen lod with stories ----------------------------------------------------------------
    std::vector <Core::VirtualMesh> vmeshes(ge.storeys_component_id.size());
    for (auto storyId = 0; storyId < vmeshes.size(); storyId++) {
        Core::Mesh mesh;
        int cnt = 0;
        for (int i = 0; i < storyCompIndices[storyId].size(); i++) {
            auto compId = storyCompIndices[storyId][i];
            for (auto id : compIndices[compId]) {
                mesh.vertices.emplace_back(ge.verts[id * 3 + 0], ge.verts[id * 3 + 1], ge.verts[id * 3 + 2]);
                mesh.indices.push_back(cnt++);
            }
        }
        std::cout << mesh.vertices.size() << " " << mesh.indices.size() << "\n";

        vmeshes[storyId].Build(mesh);
    }*/

    // Gen lod with component ----------------------------------------------------------------
    uint32_t triThreshold = 64;
    std::vector<uint32_t> largeCompId;
    for (auto compId = 0; compId < compIndices.size() && largeCompId.size() < 1000; compId++) {
        if (compIndices[compId].size() > triThreshold * 3) largeCompId.push_back(compId);
    }
    std::vector <Core::VirtualMesh> vmeshes(100);
    for (int i = 900; i < largeCompId.size(); i++) {
        auto compId = largeCompId[i];
        Core::Mesh mesh;
        int cnt = 0;
        for (auto id : compIndices[compId]) {
            mesh.vertices.emplace_back(ge.verts[id * 3 + 0], ge.verts[id * 3 + 1], ge.verts[id * 3 + 2]);
            mesh.indices.push_back(cnt++);
        }
        std::cout << mesh.vertices.size() << " " << mesh.indices.size() << "\n";

        vmeshes[i - 900].Build(mesh);
    }

    timer.log("Success generate virtual mesh");
    Core::Encode::PackingMeshData(modelFileName, vmeshes, packedData);

    // Render -----------------------------------------
#pragma region render
    timer.reset();
    Vk::DebugLodApplication renderer(config);
    timer.log("Success init vulkan");

    // renderer.Run(mesh, vmesh);
    renderer.Run(packedData, vmeshes.size());
#pragma endregion
    return 0;
}