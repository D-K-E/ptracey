#pragma once

#include <bvh.hpp>
#include <external.hpp>
#include <hittable.hpp>
#include <hittable_list.hpp>
#include <material.hpp>
#include <matrix.hpp>
#include <ray.hpp>
#include <triangle.hpp>
#include <vec3.hpp>

color to_color(aiColor3D col) {
  auto x = static_cast<double>(col.r);
  auto y = static_cast<double>(col.g);
  auto z = static_cast<double>(col.b);
  return color(x, y, z);
}
class mesh : public hittable {
public:
  int mesh_id;
  mesh() {}
  shared_ptr<hittable> obj;
  hittable_list list;

  mesh(std::vector<shared_ptr<hittable>> hs, int mid)
      : mesh_id(mid), list(hittable_list(hs)) {
    obj = make_shared<bvh_node>(list, 0.0, 1.0);
  }
  bool hit(const ray &r, double t_min, double t_max,
           hit_record &rec) const override {
    bool res = obj->hit(r, t_min, t_max, rec);
    return res;
  }
  bool bounding_box(double time0, double time1,
                    aabb &output_box) const override {
    return obj->bounding_box(time0, time1, output_box);
  }
  double pdf_value(const vec3 &o,
                   const vec3 &v) const override {
    return list.pdf_value(o, v);
  }
  vec3 random(const vec3 &o) const override {
    return list.random(o);
  }
};
class model : public hittable {
public:
  shared_ptr<hittable> meshes;
  shared_ptr<material> mat = nullptr;
  hittable_list list;
  matrix transMat;

  model(std::string mpath) : transMat(identityMatrix()) {
    loadModel(mpath);
  }
  model(std::string mpath, const matrix &transM)
      : transMat(transM) {
    loadModel(mpath);
  }
  model(std::string mpath, const matrix &transM,
        shared_ptr<material> mt)
      : transMat(transM), mat(mt) {
    loadModel(mpath);
  }
  void loadModel(std::string mpath) {

    Assimp::Importer importer;
    const aiScene *scene =
        importer.ReadFile(mpath, aiProcess_Triangulate);
    if (!scene ||
        scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
      std::cout << "ERROR::ASSIMP::"
                << importer.GetErrorString() << std::endl;
      return;
    }
    // start processing from root node recursively
    std::vector<shared_ptr<hittable>> ms;
    processNode(scene->mRootNode, scene, 0, ms);
    list = hittable_list(ms);
    meshes = make_shared<bvh_node>(list, 0.0, 1.0);
  }
  void processNode(aiNode *node, const aiScene *scene,
                   int node_id,
                   std::vector<shared_ptr<hittable>> &ms) {
    // process the meshes of the given node on scene
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
      aiMesh *msh = scene->mMeshes[node->mMeshes[i]];
      ms.push_back(processMesh(msh, scene, i + node_id));
    }
    // now all meshes of this node has been processed
    // we should continue to meshes of child nodes
    for (unsigned int k = 0; k < node->mNumChildren; k++) {
      processNode(node->mChildren[k], scene, k + node_id,
                  ms);
    }
  }
  std::vector<shared_ptr<hittable>>
  processTriangles(aiMesh *msh,
                   shared_ptr<material> mat_ptr) {
    // iterating on vertices of the mesh
    std::vector<shared_ptr<hittable>> triangles;
    for (unsigned int j = 0; j < msh->mNumFaces; j++) {
      aiFace triangle_face = msh->mFaces[j];
      std::vector<point3> tri_points;
      for (unsigned int i = 0;
           i < triangle_face.mNumIndices; i++) {
        unsigned int index = triangle_face.mIndices[i];
        point3 vec;
        vec[0] = msh->mVertices[index].x;
        vec[1] = msh->mVertices[index].y;
        vec[2] = msh->mVertices[index].z;
        vec = transMat * vec;
        tri_points.push_back(vec);
      }
      shared_ptr<hittable> tri;
      if (!mat) {
        tri = make_shared<triangle>(tri_points[0],
                                    tri_points[1],
                                    tri_points[2], mat_ptr);
      } else {
        tri = make_shared<triangle>(tri_points[0],
                                    tri_points[1],
                                    tri_points[2], mat);
      }
      triangles.push_back(tri);
    }
    return triangles;
  }
  shared_ptr<material> processMaterial(aiMaterial *mat) {
    float ref_idx;
    mat->Get(AI_MATKEY_REFRACTI, ref_idx);
    if (ref_idx != 1.0f) {
      shared_ptr<material> ret =
          make_shared<dielectric>(ref_idx);
      return ret;
    }
    aiColor3D col;
    mat->Get(AI_MATKEY_COLOR_EMISSIVE, col);
    color em = to_color(col);
    bool res =
        em.x() == 0.0 && em.y() == 0.0 && em.z() == 0.0;
    if (!res) {
      shared_ptr<material> ret =
          make_shared<diffuse_light>(em);
      return ret;
    }
    float fuzz;
    mat->Get(AI_MATKEY_REFLECTIVITY, fuzz);
    if (fuzz != 0.0) {
      mat->Get(AI_MATKEY_COLOR_DIFFUSE, col);
      color c = to_color(col);
      res = c.x() == 0.0 && c.y() == 0.0 && c.z() == 0.0;
      if (!res) {
        shared_ptr<material> ret =
            make_shared<metal>(c, fuzz);
        return ret;
      }
      mat->Get(AI_MATKEY_COLOR_REFLECTIVE, col);
      c = to_color(col);
      res = c.x() == 0.0 && c.y() == 0.0 && c.z() == 0.0;
      if (!res) {
        shared_ptr<material> ret =
            make_shared<metal>(c, fuzz);
        return ret;
      }
    }
    mat->Get(AI_MATKEY_COLOR_DIFFUSE, col);
    color diffuse = to_color(col);
    shared_ptr<material> ret =
        make_shared<lambertian>(diffuse);
    return ret;
  }
  shared_ptr<hittable> processMesh(aiMesh *msh,
                                   const aiScene *scene,
                                   int mesh_id) {
    aiMaterial *mat =
        scene->mMaterials[msh->mMaterialIndex];
    shared_ptr<material> mat_ptr = processMaterial(mat);

    std::vector<shared_ptr<hittable>> triangles =
        processTriangles(msh, mat_ptr);

    shared_ptr<hittable> m;
    if (!mat) {
      m = make_shared<mesh>(triangles, mesh_id);
    } else {
      m = make_shared<mesh>(triangles, mesh_id);
    }
    return m;
  }
  bool hit(const ray &r, double t_min, double t_max,
           hit_record &rec) const override {
    bool res = meshes->hit(r, t_min, t_max, rec);
    return res;
  }
  bool bounding_box(double time0, double time1,
                    aabb &output_box) const override {
    return meshes->bounding_box(time0, time1, output_box);
  }
  double pdf_value(const vec3 &o,
                   const vec3 &v) const override {
    return list.pdf_value(o, v);
  }
  vec3 random(const vec3 &o) const override {
    return list.random(o);
  }
};
