#include "mesh.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <filesystem>

#include "../textures/texture_manager.h"
#include "../materials/material_manager.h"
#include "../materials/material.h"

Mesh::Mesh(std::string path,
           TextureManager* tex_mgr,
           MaterialManager* mat_mgr)
    : path_(std::move(path))
    , tex_mgr_(tex_mgr)
    , mat_mgr_(mat_mgr) {}

void Mesh::impl_load(ResourceState state) {
    switch (state) {
        case ResourceState::Cpu:
            load_cpu();
            break;

        case ResourceState::Gpu:
            // Mesh is CPU-only; using it as a GPU resource is an error in the design.
            logging::log(reslog::MESH, logging::ERROR,
                         "Mesh::impl_load(Gpu) called for '" + path_ +
                             "'. Mesh is CPU-only; use MeshGroup for GPU geometry.");
            break;

        case ResourceState::Drive:
        default:
            // Drive is conceptual only.
            break;
    }
}

void Mesh::impl_unload(ResourceState state) {
    switch (state) {
        case ResourceState::Cpu:
            unload_cpu();
            break;

        case ResourceState::Gpu:
            // No GPU state to unload.
            break;

        case ResourceState::Drive:
        default:
            break;
    }
}

void Mesh::load_cpu() {
    if (cpu_data_) {
        // Already loaded.
        return;
    }

    logging::log(reslog::MESH, logging::INFO,
                 "Loading mesh from '" + path_ + "'");

    cpu_data_ = std::make_unique<MeshCPUData>();

    // For now we support OBJ as the only format.
    // You can extend this later based on file extension.
    load_obj();

    if (!cpu_data_->is_valid()) {
        logging::log(reslog::MESH, logging::ERROR,
                     "Mesh '" + path_ + "' loaded with invalid data");
    } else {
        logging::log(reslog::MESH, logging::INFO,
                     "Loaded mesh '" + path_ + "' verts=" +
                         std::to_string(cpu_data_->vertex_count()) +
                         " tris=" + std::to_string(cpu_data_->index_count() / 3));
    }
}

void Mesh::unload_cpu() {
    if (!cpu_data_) {
        return;
    }

    cpu_data_.reset();
    logging::log(reslog::MESH, logging::INFO,
                 "Released CPU mesh '" + path_ + "'");
}

// --- OBJ/MTL loader --------------------------------------------------------

void Mesh::load_obj() {
    std::ifstream file(path_);
    if (!file.is_open()) {
        logging::log(reslog::MESH, logging::ERROR,
                     "Cannot open mesh file '" + path_ + "'");
        cpu_data_ = std::make_unique<MeshCPUData>(); // leave empty but non-null
        return;
    }

    auto& cpu = *cpu_data_;

    std::vector<float> obj_positions;
    std::vector<float> obj_normals;
    std::vector<float> obj_texcoords;

    struct Idx {
        int v{-1}, vt{-1}, vn{-1};
        bool operator==(const Idx& o) const {
            return v == o.v && vt == o.vt && vn == o.vn;
        }
    };

    struct Face {
        Idx a, b, c;
        std::string mtl;
    };

    std::vector<Face> faces;

    // material name -> material ptr
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;

    // Helper to load MTL file if managers are available.
    auto load_mtl = [&](const std::string& mtl_rel) {
        if (!mat_mgr_ || !tex_mgr_) {
            return;
        }

        std::filesystem::path base = std::filesystem::path(path_).parent_path();
        std::filesystem::path mtl_path = base / mtl_rel;

        std::ifstream mtl(mtl_path);
        if (!mtl.is_open()) {
            logging::log(reslog::MESH, logging::WARNING,
                         "Could not open MTL file '" + mtl_path.string() + "'");
            return;
        }

        std::string line;
        std::string cur_name;
        std::string map_kd;
        std::string map_bump;

        auto commit_current = [&]() {
            if (cur_name.empty()) return;
            auto mat = mat_mgr_->get(cur_name);

            if (!map_kd.empty()) {
                auto tex = tex_mgr_->get((base / map_kd).string());
                mat->properties().base_color.set_texture(tex);
            }
            if (!map_bump.empty()) {
                mat->properties().normal_map = tex_mgr_->get((base / map_bump).string());
            }

            materials[cur_name] = mat;
        };

        while (std::getline(mtl, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::istringstream iss(line);
            std::string tag;
            iss >> tag;

            if (tag == "newmtl") {
                // Commit previous material first.
                commit_current();
                iss >> cur_name;
                map_kd.clear();
                map_bump.clear();
            } else if (tag == "map_Kd") {
                iss >> map_kd;
            } else if (tag == "map_Bump" || tag == "map_bump") {
                iss >> map_bump;
            }
            // You can extend this with Ns, Ni, etc. if needed.
        }

        // Commit last material.
        commit_current();
    };

    std::string line;
    std::string current_mtl;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string tag;
        iss >> tag;

        if (tag == "v") {
            float x, y, z;
            if (iss >> x >> y >> z) {
                obj_positions.insert(obj_positions.end(), {x, y, z});
            }
        } else if (tag == "vt") {
            float u, v;
            if (iss >> u >> v) {
                obj_texcoords.insert(obj_texcoords.end(), {u, v});
            }
        } else if (tag == "vn") {
            float nx, ny, nz;
            if (iss >> nx >> ny >> nz) {
                obj_normals.insert(obj_normals.end(), {nx, ny, nz});
            }
        } else if (tag == "mtllib") {
            std::string mtlfile;
            if (iss >> mtlfile) {
                load_mtl(mtlfile);
            }
        } else if (tag == "usemtl") {
            iss >> current_mtl;
        } else if (tag == "f") {
            // Parse face with arbitrary polygon size, triangulate as fan.
            std::string vert;
            std::vector<Idx> vtx;

            while (iss >> vert) {
                Idx idx;
                // Replace '/' with ' ' to simplify parsing.
                std::replace(vert.begin(), vert.end(), '/', ' ');
                std::istringstream viss(vert);
                viss >> idx.v;
                if (!(viss >> idx.vt)) idx.vt = -1;
                if (!(viss >> idx.vn)) idx.vn = -1;

                // Convert from 1-based to 0-based indices.
                if (idx.v  > 0) idx.v  -= 1;
                if (idx.vt > 0) idx.vt -= 1;
                if (idx.vn > 0) idx.vn -= 1;

                vtx.push_back(idx);
            }

            // Triangulate polygon as triangle fan.
            for (std::size_t i = 1; i + 1 < vtx.size(); ++i) {
                faces.push_back({vtx[0], vtx[i], vtx[i + 1], current_mtl});
            }
        }
    }

    // Remap unique vertex triplets (v/vt/vn) to final vertex indices.
    struct Hash {
        std::size_t operator()(const Idx& i) const noexcept {
            // Simple hash; can be improved if necessary.
            std::size_t h = 0;
            h ^= std::hash<int>()(i.v)  + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<int>()(i.vt) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<int>()(i.vn) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };

    std::unordered_map<Idx, std::uint32_t, Hash> remap;

    auto add_vertex = [&](const Idx& idx) -> std::uint32_t {
        auto it = remap.find(idx);
        if (it != remap.end()) return it->second;

        std::uint32_t id = static_cast<std::uint32_t>(cpu.positions.size() / 3);

        // Position
        if (idx.v >= 0 && static_cast<std::size_t>(idx.v * 3 + 2) < obj_positions.size()) {
            const auto p = idx.v * 3;
            cpu.positions.push_back(obj_positions[p + 0]);
            cpu.positions.push_back(obj_positions[p + 1]);
            cpu.positions.push_back(obj_positions[p + 2]);
        } else {
            cpu.positions.insert(cpu.positions.end(), {0.f, 0.f, 0.f});
        }

        // Texcoords
        if (!obj_texcoords.empty()) {
            if (idx.vt >= 0 && static_cast<std::size_t>(idx.vt * 2 + 1) < obj_texcoords.size()) {
                const auto t = idx.vt * 2;
                cpu.texcoords.push_back(obj_texcoords[t + 0]);
                cpu.texcoords.push_back(obj_texcoords[t + 1]);
            } else {
                cpu.texcoords.insert(cpu.texcoords.end(), {0.f, 0.f});
            }
        }

        // Normals
        if (!obj_normals.empty()) {
            if (idx.vn >= 0 && static_cast<std::size_t>(idx.vn * 3 + 2) < obj_normals.size()) {
                const auto n = idx.vn * 3;
                cpu.normals.push_back(obj_normals[n + 0]);
                cpu.normals.push_back(obj_normals[n + 1]);
                cpu.normals.push_back(obj_normals[n + 2]);
            } else {
                cpu.normals.insert(cpu.normals.end(), {0.f, 0.f, 1.f});
            }
        }

        remap[idx] = id;
        return id;
    };

    // Group faces by material.
    struct SubAccum {
        std::vector<std::uint32_t> indices;
        std::shared_ptr<Material>  mat;
    };

    std::vector<SubAccum> subaccum;

    auto find_or_add_subaccum = [&](const std::string& mtl_name) -> SubAccum& {
        for (auto& s : subaccum) {
            if (s.mat && s.mat->name() == mtl_name) return s;
            if (!s.mat && mtl_name.empty()) return s;
        }

        subaccum.emplace_back();
        if (!mtl_name.empty()) {
            auto it = materials.find(mtl_name);
            if (it != materials.end()) {
                subaccum.back().mat = it->second;
            }
        }
        return subaccum.back();
    };

    for (const auto& f : faces) {
        auto& sub = find_or_add_subaccum(f.mtl);
        sub.indices.push_back(add_vertex(f.a));
        sub.indices.push_back(add_vertex(f.b));
        sub.indices.push_back(add_vertex(f.c));
    }

    // Build final index buffer and submesh table.
    std::size_t index_offset = 0;
    for (auto& sub : subaccum) {
        Submesh sm;
        sm.index_offset = index_offset;
        sm.index_count  = sub.indices.size();
        sm.material     = sub.mat;

        cpu.indices.insert(cpu.indices.end(), sub.indices.begin(), sub.indices.end());
        cpu.submeshes.push_back(sm);

        index_offset += sub.indices.size();
    }
}
