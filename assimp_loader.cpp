#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "renderer.h"
#include "render_backend.h"

void process_node(const aiScene *scene, aiNode *node, Array<Loaded_Mesh> *out_array) {
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

        Material material = {};
        bool has_material = false;
        if (scene->mNumMaterials > 0) {
            has_material = true;
            aiMaterial *assimp_material = scene->mMaterials[mesh->mMaterialIndex];
            assert(assimp_material != nullptr);
            for (int prop_index = 0; prop_index < assimp_material->mNumProperties; prop_index++) {
                aiMaterialProperty *property = assimp_material->mProperties[prop_index];
                if (strcmp(property->mKey.data, "$tex.file") == 0) {
                    switch (property->mType) {
                        case aiPTI_Float:   break;
                        case aiPTI_Double:  break;
                        case aiPTI_Integer: break;
                        case aiPTI_Buffer:  break;
                        case aiPTI_String: {
                            char *cstr = ((aiString *)property->mData)->data;
                            switch (property->mSemantic) {
                                case aiTextureType_NONE:                                                              break;
                                case aiTextureType_DIFFUSE: {
                                    String_Builder sb = make_string_builder(default_allocator());
                                    defer(destroy_string_builder(sb));
                                    sb.print("sponza/");
                                    sb.print(cstr);
                                    material.albedo = load_texture_from_file(sb.string());
                                    break;
                                }
                                case aiTextureType_SPECULAR:                                                          break;
                                case aiTextureType_AMBIENT:                                                           break;
                                case aiTextureType_EMISSIVE:                                                          break;
                                case aiTextureType_HEIGHT:                                                            break;
                                case aiTextureType_NORMALS:                                                           break;
                                case aiTextureType_SHININESS:                                                         break;
                                case aiTextureType_OPACITY:                                                           break;
                                case aiTextureType_DISPLACEMENT:                                                      break;
                                case aiTextureType_LIGHTMAP:                                                          break;
                                case aiTextureType_REFLECTION:                                                        break;
                                case aiTextureType_BASE_COLOR:                                                        break;
                                case aiTextureType_NORMAL_CAMERA:                                                     break;
                                case aiTextureType_EMISSION_COLOR:                                                    break;
                                case aiTextureType_METALNESS:                                                         break;
                                case aiTextureType_DIFFUSE_ROUGHNESS:                                                 break;
                                case aiTextureType_AMBIENT_OCCLUSION:                                                 break;
                                case aiTextureType_UNKNOWN:                                                           break;
                            }
                            break;
                        }
                    }
                }
            }
        }

        Buffer vertex_buffer = create_buffer(BT_VERTEX, vertices.data, vertices.count * sizeof(vertices[0]));
        Buffer index_buffer  = create_buffer(BT_INDEX,  indices.data,  indices.count  * sizeof(indices[0]));
        Loaded_Mesh loaded_mesh = {
            vertex_buffer,
            vertices.count,
            index_buffer,
            indices.count,
            material,
            has_material,
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
