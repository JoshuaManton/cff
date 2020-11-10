#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "renderer.h"
#include "render_backend.h"

struct Loaded_Mesh {
    Buffer vertex_buffer;
    int num_vertices;
    Buffer index_buffer;
    int num_indices;
};

void process_node(const aiScene *scene, aiNode *node, Array<Loaded_Mesh> *out_array) {
    printf("%s\n", node->mName.data);

    for (int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

        Array<Vertex> vertices = {};
        vertices.allocator = default_allocator();
        defer(vertices.destroy());

        Array<u32> indices = {};
        indices.allocator = default_allocator();
        defer(indices.destroy());

        for (int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex = {};
            vertex.position.x = mesh->mVertices[i].x;
            vertex.position.y = mesh->mVertices[i].y;
            vertex.position.z = mesh->mVertices[i].z;

            if (mesh->mTextureCoords[0]) {
                vertex.tex_coord.x = (float)mesh->mTextureCoords[0][i].x;
                vertex.tex_coord.y = (float)mesh->mTextureCoords[0][i].y;
            }

            vertices.append(vertex);
        }

        for (int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (int j = 0; j < face.mNumIndices; j++) {
                indices.append(face.mIndices[j]);
            }
        }

        // Foreach (vertex, vertices) {
        //     printf("%f %f %f\n", vertex->position.x, vertex->position.y, vertex->position.z);
        // }

        Buffer vertex_buffer = create_buffer(BT_VERTEX, vertices.data, vertices.count * sizeof(vertices[0]));
        Buffer index_buffer  = create_buffer(BT_INDEX,  indices.data,  indices.count  * sizeof(indices[0]));
        Loaded_Mesh loaded_mesh = {
            vertex_buffer,
            vertices.count,
            index_buffer,
            indices.count,
        };
        out_array->append(loaded_mesh);
    }

    for (int i = 0; i < node->mNumChildren; i++) {
        process_node(scene, node->mChildren[i], out_array);
    }
}

void load_mesh(void *data, int len, Array<Loaded_Mesh> *out_array) {
    Assimp::Importer importer;

    const aiScene *scene = importer.ReadFileFromMemory(data, len,
        aiProcess_PreTransformVertices |
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs,
        "glb");

    assert(scene != nullptr);
    process_node(scene, scene->mRootNode, out_array);
}

void load_mesh_from_file(char *filename, Array<Loaded_Mesh> *out_array) {
    int len;
    char *data = read_entire_file(filename, &len);
    load_mesh(data, len, out_array);
}
