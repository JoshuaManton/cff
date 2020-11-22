#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "application.h"
#include "renderer.h"

void calculate_tangents_and_bitangents(Vertex *vert0, Vertex *vert1, Vertex *vert2) {
    Vector3 delta_pos1 = vert1->position - vert0->position;
    Vector3 delta_pos2 = vert2->position - vert0->position;

    Vector3 delta_uv1 = vert1->tex_coord - vert0->tex_coord;
    Vector3 delta_uv2 = vert2->tex_coord - vert0->tex_coord;

    float r = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x);
    Vector3 tangent   = (delta_pos1 * delta_uv2.y - delta_pos2 * delta_uv1.y) * r;
    Vector3 bitangent = (delta_pos2 * delta_uv1.x - delta_pos1 * delta_uv2.x) * r;

    // note(josh): we += here instead of = because in the case of indexing we could hit
    // the same vertex more than once, so we want an "average" of all the tangents/bitangents
    // for that vertex. explained here under the "Indexing" heading
    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/#indexing

    vert0->tangent += tangent;
    vert1->tangent += tangent;
    vert2->tangent += tangent;

    vert0->bitangent += bitangent;
    vert1->bitangent += bitangent;
    vert2->bitangent += bitangent;
}

void process_node(const aiScene *scene, aiNode *node, char *directory, Allocator allocator, Model *out_model) {
    Array<Vertex> vertices = make_array<Vertex>(allocator, 1024);
    defer(vertices.destroy());

    Array<u32> indices = make_array<u32>(allocator, 1024);
    defer(indices.destroy());

    for (int i = 0; i < node->mNumMeshes; i++) {
        vertices.clear();
        indices.clear();

        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

        for (int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex = {};
            vertex.position.x = mesh->mVertices[i].x;
            vertex.position.y = mesh->mVertices[i].y;
            vertex.position.z = mesh->mVertices[i].z;

            if (mesh->HasVertexColors(0)) {
                vertex.color.x = (float)mesh->mColors[0][i].r;
                vertex.color.y = (float)mesh->mColors[0][i].g;
                vertex.color.z = (float)mesh->mColors[0][i].b;
                vertex.color.w = (float)mesh->mColors[0][i].a;
            }
            else {
                vertex.color.x = 1;
                vertex.color.y = 1;
                vertex.color.z = 1;
                vertex.color.w = 1;
            }

            // todo(josh): vertex colors

            if (mesh->HasTextureCoords(0)) {
                vertex.tex_coord.x = (float)mesh->mTextureCoords[0][i].x;
                vertex.tex_coord.y = (float)mesh->mTextureCoords[0][i].y;
            }

            if (mesh->HasNormals()) {
                vertex.normal.x = (float)mesh->mNormals[i].x;
                vertex.normal.y = (float)mesh->mNormals[i].y;
                vertex.normal.z = (float)mesh->mNormals[i].z;
            }

            if (mesh->HasTangentsAndBitangents()) {
                vertex.tangent.x = (float)mesh->mTangents[i].x;
                vertex.tangent.y = (float)mesh->mTangents[i].y;
                vertex.tangent.z = (float)mesh->mTangents[i].z;

                vertex.bitangent.x = (float)mesh->mBitangents[i].x;
                vertex.bitangent.y = (float)mesh->mBitangents[i].y;
                vertex.bitangent.z = (float)mesh->mBitangents[i].z;
            }

            vertices.append(vertex);
        }

        for (int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (int j = 0; j < face.mNumIndices; j++) {
                indices.append(face.mIndices[j]);
            }
        }

        if (!mesh->HasTangentsAndBitangents()) {
            if (indices.count) {
                for (int i = 0; i < indices.count; i += 3) {
                    int index0 = indices[i+0];
                    int index1 = indices[i+1];
                    int index2 = indices[i+2];

                    Vertex *vert0 = &vertices[index0];
                    Vertex *vert1 = &vertices[index1];
                    Vertex *vert2 = &vertices[index2];
                    calculate_tangents_and_bitangents(vert0, vert1, vert2);
                }
            }
            else {
                for (int i = 0; i < vertices.count; i += 3) {
                    Vertex *vert0 = &vertices[i+0];
                    Vertex *vert1 = &vertices[i+1];
                    Vertex *vert2 = &vertices[i+2];
                    calculate_tangents_and_bitangents(vert0, vert1, vert2);
                }
            }
        }

        PBR_Material material = {};
        bool has_material = false;
        if (scene->mNumMaterials > 0) {
            has_material = true;
            assert(material.cbuffer_handle == nullptr);
            material.cbuffer_handle = create_pbr_material_cbuffer();
            aiMaterial *assimp_material = scene->mMaterials[mesh->mMaterialIndex];
            assert(assimp_material != nullptr);
            for (int prop_index = 0; prop_index < assimp_material->mNumProperties; prop_index++) {
                aiMaterialProperty *property = assimp_material->mProperties[prop_index];
                if (strcmp(property->mKey.data, "$tex.file") == 0) {
                    assert(property->mType == aiPTI_String);
                    char *cstr = ((aiString *)property->mData)->data;
                    String_Builder path_sb = make_string_builder(allocator);
                    defer(path_sb.destroy());
                    path_sb.printf("%s/%s", directory, cstr);
                    switch (property->mSemantic) {
                        // todo(josh): there is probably a material parameter for the wrap mode ???
                        // todo(josh): there is probably a material parameter for the wrap mode ???
                        // todo(josh): there is probably a material parameter for the wrap mode ???
                        // todo(josh): there is probably a material parameter for the wrap mode ???
                        // todo(josh): there is probably a material parameter for the wrap mode ???
                        // todo(josh): there is probably a material parameter for the wrap mode ???
                        case aiTextureType_DIFFUSE:           { if (!material.albedo_map.valid)    {  material.albedo_map    = create_texture_from_file(path_sb.string(), TF_R8G8B8A8_UINT_SRGB, TWM_LINEAR_WRAP); } break; }
                        case aiTextureType_NORMALS:           { if (!material.normal_map.valid)    {  material.normal_map    = create_texture_from_file(path_sb.string(), TF_R8G8B8A8_UINT,      TWM_LINEAR_WRAP); } break; }
                        case aiTextureType_BASE_COLOR:        { if (!material.albedo_map.valid)    {  material.albedo_map    = create_texture_from_file(path_sb.string(), TF_R8G8B8A8_UINT,      TWM_LINEAR_WRAP); } break; }
                        case aiTextureType_NORMAL_CAMERA:     { if (!material.normal_map.valid)    {  material.normal_map    = create_texture_from_file(path_sb.string(), TF_R8G8B8A8_UINT,      TWM_LINEAR_WRAP); } break; }
                        case aiTextureType_EMISSION_COLOR:    { if (!material.emission_map.valid)  {  material.emission_map  = create_texture_from_file(path_sb.string(), TF_R8G8B8A8_UINT,      TWM_LINEAR_WRAP); } break; }
                        case aiTextureType_METALNESS:         { if (!material.metallic_map.valid)  {  material.metallic_map  = create_texture_from_file(path_sb.string(), TF_R8G8B8A8_UINT,      TWM_LINEAR_WRAP); } break; }
                        case aiTextureType_DIFFUSE_ROUGHNESS: { if (!material.roughness_map.valid) {  material.roughness_map = create_texture_from_file(path_sb.string(), TF_R8G8B8A8_UINT,      TWM_LINEAR_WRAP); } break; }
                        case aiTextureType_AMBIENT_OCCLUSION: { if (!material.ao_map.valid)        {  material.ao_map        = create_texture_from_file(path_sb.string(), TF_R8G8B8A8_UINT,      TWM_LINEAR_WRAP); } break; }
                        case aiTextureType_LIGHTMAP:          { if (!material.ao_map.valid)        {  material.ao_map        = create_texture_from_file(path_sb.string(), TF_R8G8B8A8_UINT,      TWM_LINEAR_WRAP); } break; }
                        case aiTextureType_EMISSIVE:          { if (!material.emission_map.valid)  {  material.emission_map  = create_texture_from_file(path_sb.string(), TF_R8G8B8A8_UINT,      TWM_LINEAR_WRAP); } break; }
                        case aiTextureType_SPECULAR:          { printf("Unhandled: aiTextureType_SPECULAR: %s\n",     path_sb.string()); break; }
                        case aiTextureType_AMBIENT:           { printf("Unhandled: aiTextureType_AMBIENT: %s\n",      path_sb.string()); break; }
                        case aiTextureType_HEIGHT:            { printf("Unhandled: aiTextureType_HEIGHT: %s\n",       path_sb.string()); break; }
                        case aiTextureType_SHININESS:         { printf("Unhandled: aiTextureType_SHININESS: %s\n",    path_sb.string()); break; }
                        case aiTextureType_OPACITY:           { printf("Unhandled: aiTextureType_OPACITY: %s\n",      path_sb.string()); break; }
                        case aiTextureType_DISPLACEMENT:      { printf("Unhandled: aiTextureType_DISPLACEMENT: %s\n", path_sb.string()); break; }
                        case aiTextureType_REFLECTION:        { printf("Unhandled: aiTextureType_REFLECTION: %s\n",   path_sb.string()); break; }
                        case aiTextureType_NONE:              { assert(false); }
                        case aiTextureType_UNKNOWN: {
                            printf("Unknown texture type: %s\n", path_sb.string());
                            break;
                        }
                    }
                }
                // todo(josh): apparently property->mData can be an array (of floats, for example).
                //             we should use property->mDataLength to pull the right values out.
                //             the only one I've seen be an array is for ambient but all the values
                //             are the same and our current Material system only does scalar ambient.
                else if (strcmp(property->mKey.data, "$mat.gltf.pbrMetallicRoughness.baseColorFactor") == 0) {
                    assert(property->mType == aiPTI_Float);
                    material.ambient = *(float *)property->mData;
                }
                else if (strcmp(property->mKey.data, "$mat.gltf.pbrMetallicRoughness.metallicFactor") == 0) {
                    assert(property->mType == aiPTI_Float);
                    material.metallic = *(float *)property->mData;
                }
                else if (strcmp(property->mKey.data, "$mat.gltf.pbrMetallicRoughness.roughnessFactor") == 0) {
                    assert(property->mType == aiPTI_Float);
                    material.roughness = *(float *)property->mData;
                }
                else if (strcmp(property->mKey.data, "$mat.gltf.alphaMode") == 0) {
                    assert(property->mType == aiPTI_String);
                    char *cstr = ((aiString *)property->mData)->data;
                    if (strcmp(cstr, "MASK") == 0) {
                        material.has_transparency = true;
                    }
                    else {
                        if (strcmp(cstr, "OPAQUE") != 0) {
                            printf("%s\n", cstr);
                            assert(false);
                        }
                    }
                }
                else {
                    // switch (property->mType) {
                    //     case aiPTI_Float:   printf("%s -> %f\n", property->mKey.data, *(float *)property->mData);           break;
                    //     case aiPTI_Double:  printf("%s -> %f\n", property->mKey.data, *(double *)property->mData);          break;
                    //     case aiPTI_Integer: printf("%s -> %d\n", property->mKey.data, *(int *)property->mData);             break;
                    //     case aiPTI_Buffer:  printf("%s -> %p\n", property->mKey.data, property->mData);                     break;
                    //     case aiPTI_String:  printf("%s -> %s\n", property->mKey.data, ((aiString *)property->mData)->data); break;
                    // }
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
        out_model->meshes.append(loaded_mesh);
    }

    for (int i = 0; i < node->mNumChildren; i++) {
        process_node(scene, node->mChildren[i], directory, allocator, out_model);
    }
}

Model load_model_from_file(char *filename, Allocator allocator) {
    Assimp::Importer importer;

    const aiScene *scene = importer.ReadFile(filename,
        aiProcess_PreTransformVertices |
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs);

    if (scene == nullptr) {
        printf("Error loading mesh: %s\n", importer.GetErrorString());
        assert(false);
    }

    char *directory = path_directory(filename, allocator);
    assert(directory != nullptr);
    defer(free(allocator, directory));

    Model model = create_model(allocator);
    process_node(scene, scene->mRootNode, directory, allocator, &model);
    return model;
}
