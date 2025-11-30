#include "mesh.h"

#include <utility>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include "../textures/texture_manager.h"
#include "../textures/texture.h"
#include "../materials/material_manager.h"
#include "../materials/material.h"
#include "../resource_globals.h"
#include "../resource_logging.h"
#include "../../logging/logging.h"

Mesh::Mesh(std::string path)
    : path_(std::move(path)) {}

void Mesh::request(resources::ResourceState state) {
    using S = resources::ResourceState;

    logging::log(reslog::MESH, logging::INFO, "Requesting mesh '" + path_ + "' to be in " +
                                               std::string(resources::to_string(state)));

    if (state == S::Drive) {
        // Drive is implicit; nothing to do.
        logging::log(reslog::MESH, logging::DEBUG, "Drive state implicit for '" + path_ + "'");
        return;
    }

    if (state == S::Ram) {
        const int prev = ram_refcount_.fetch_add(1, std::memory_order_acq_rel);
        if (prev == 0 && !has_ram_) {
            if (!load_from_drive()) {
                logging::log(reslog::MESH, logging::ERROR, "Mesh::request(Ram) failed for file: " + path_);
                return;
            }
            has_ram_ = true;
            logging::log(reslog::MESH, logging::INFO, "Mesh '" + path_ + "' loaded into RAM");
        } else {
            logging::log(reslog::MESH, logging::DEBUG, "Mesh '" + path_ + "' RAM already present/requested");
        }
        return;
    }
}

void Mesh::release(resources::ResourceState state) {
    using S = resources::ResourceState;

    logging::log(reslog::MESH, logging::INFO, "Releasing mesh '" + path_ + "' from " +
                                               std::string(resources::to_string(state)));

    if (state == S::Drive) {
        return;
    }

    if (state == S::Ram) {
        const int prev = ram_refcount_.fetch_sub(1, std::memory_order_acq_rel);
        if (prev <= 0) {
            ram_refcount_.store(0, std::memory_order_relaxed);
            return;
        }

        if (prev == 1 && has_ram_) {
            if (cpu_data_) {
                // Preserve submeshes (materials) but drop heavy geometry.
                cpu_data_->positions.clear();
                cpu_data_->positions.shrink_to_fit();
                cpu_data_->normals.clear();
                cpu_data_->normals.shrink_to_fit();
                cpu_data_->texcoords.clear();
                cpu_data_->texcoords.shrink_to_fit();
                cpu_data_->indices.clear();
                cpu_data_->indices.shrink_to_fit();
            }
            has_ram_ = false;
            logging::log(reslog::MESH, logging::INFO, "Mesh '" + path_ + "' RAM released");
        }
        return;
    }
}

bool Mesh::is_in_state(resources::ResourceState state) const {
    using S = resources::ResourceState;

    if (state == S::Drive) {
        return true;
    }
    if (state == S::Ram) {
        return has_ram_;
    }
    return false;
}

bool Mesh::load_from_drive() {
    logging::log(reslog::MESH, logging::INFO, "Loading OBJ mesh from '" + path_ + "'");

    std::ifstream file(path_);
    if (!file.is_open()) {
        logging::log(reslog::MESH, logging::ERROR, "Failed to open OBJ file '" + path_ + "'");
        return false;
    }

    std::vector<float> obj_positions;
    std::vector<float> obj_normals;
    std::vector<float> obj_texcoords;

    struct Idx { int v{-1}, vt{-1}, vn{-1}; bool operator==(const Idx& o) const { return v==o.v && vt==o.vt && vn==o.vn; } };
    struct Face { Idx a,b,c; std::string material; };
    std::vector<Face> faces;

    // Material parsing
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    auto& tex_mgr = global_texture_manager();
    auto& mat_mgr = global_material_manager();

    auto load_mtl = [&](const std::string& mtl_path) {
        std::ifstream mtl(mtl_path);
        if (!mtl.is_open()) {
            logging::log(reslog::MESH, logging::WARNING, "Could not open MTL '" + mtl_path + "'");
            return;
        }
        std::string line, cur;
        std::string map_kd, map_bump;
        while (std::getline(mtl, line)) {
            if (line.empty() || line[0]=='#') continue;
            std::istringstream iss(line);
            std::string tag; iss >> tag;
            if (tag == "newmtl") {
                if (!cur.empty()) {
                    auto mat = std::make_shared<Material>(cur);
                    if (!map_kd.empty()) mat->properties().base_color.set_texture(tex_mgr.get(map_kd));
                    if (!map_bump.empty()) mat->properties().normal_map = tex_mgr.get(map_bump);
                    mat_mgr.add_material(mat);
                    materials[cur] = mat;
                }
                iss >> cur;
                map_kd.clear(); map_bump.clear();
            } else if (tag == "map_Kd") {
                iss >> map_kd;
            } else if (tag == "map_Bump" || tag == "map_bump") {
                iss >> map_bump;
            }
        }
        if (!cur.empty()) {
            auto mat = std::make_shared<Material>(cur);
            if (!map_kd.empty()) mat->properties().base_color.set_texture(tex_mgr.get(map_kd));
            if (!map_bump.empty()) mat->properties().normal_map = tex_mgr.get(map_bump);
            mat_mgr.add_material(mat);
            materials[cur] = mat;
        }
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
                load_mtl((std::filesystem::path(path_).parent_path() / mtlfile).string());
            }
        } else if (tag == "usemtl") {
            iss >> current_mtl;
        } else if (tag == "f") {
            std::string vert;
            std::vector<Idx> face;
            while (iss >> vert) {
                Idx idx;
                std::replace(vert.begin(), vert.end(), '/', ' ');
                std::istringstream viss(vert);
                viss >> idx.v;
                if (!(viss >> idx.vt)) idx.vt = -1;
                if (!(viss >> idx.vn)) idx.vn = -1;
                if (idx.v > 0)  idx.v  -= 1;
                if (idx.vt > 0) idx.vt -= 1;
                if (idx.vn > 0) idx.vn -= 1;
                face.push_back(idx);
            }
            for (std::size_t i = 1; i + 1 < face.size(); ++i) {
                faces.push_back({face[0], face[i], face[i + 1], current_mtl});
            }
        }
    }

    if (faces.empty() || obj_positions.empty()) {
        logging::log(reslog::MESH, logging::ERROR, "OBJ parse produced no geometry for '" + path_ + "'");
        return false;
    }

    cpu_data_ = std::make_shared<MeshCPUData>();
    auto& cpu = *cpu_data_;

    // Deduplicate by (v, vt, vn)
    struct KeyHash {
        std::size_t operator()(const Idx& i) const noexcept {
            return (static_cast<std::size_t>(i.v + 1) * 73856093u) ^
                   (static_cast<std::size_t>(i.vt + 1) * 19349663u) ^
                   (static_cast<std::size_t>(i.vn + 1) * 83492791u);
        }
    };
    struct KeyEq {
        bool operator()(const Idx& a, const Idx& b) const noexcept {
            return a.v == b.v && a.vt == b.vt && a.vn == b.vn;
        }
    };

    std::unordered_map<Idx, std::uint32_t, KeyHash, KeyEq> remap;

    auto add_vertex = [&](const Idx& idx) -> std::uint32_t {
        auto it = remap.find(idx);
        if (it != remap.end()) return it->second;
        std::uint32_t new_index = static_cast<std::uint32_t>(cpu.positions.size() / 3);
        // positions
        if (idx.v >= 0) {
            std::size_t pos_i = static_cast<std::size_t>(idx.v) * 3;
            cpu.positions.push_back(obj_positions[pos_i]);
            cpu.positions.push_back(obj_positions[pos_i + 1]);
            cpu.positions.push_back(obj_positions[pos_i + 2]);
        } else {
            cpu.positions.insert(cpu.positions.end(), {0.f, 0.f, 0.f});
        }
        // texcoords
        if (!obj_texcoords.empty()) {
            if (idx.vt >= 0) {
                std::size_t t_i = static_cast<std::size_t>(idx.vt) * 2;
                cpu.texcoords.push_back(obj_texcoords[t_i]);
                cpu.texcoords.push_back(obj_texcoords[t_i + 1]);
            } else {
                cpu.texcoords.insert(cpu.texcoords.end(), {0.f, 0.f});
            }
        }
        // normals
        if (!obj_normals.empty()) {
            if (idx.vn >= 0) {
                std::size_t n_i = static_cast<std::size_t>(idx.vn) * 3;
                cpu.normals.push_back(obj_normals[n_i]);
                cpu.normals.push_back(obj_normals[n_i + 1]);
                cpu.normals.push_back(obj_normals[n_i + 2]);
            } else {
                cpu.normals.insert(cpu.normals.end(), {0.f, 0.f, 0.f});
            }
        }
        remap[idx] = new_index;
        return new_index;
    };

    // Build indices grouped by material to form submeshes.
    struct SubInfo { std::vector<std::uint32_t> idx; std::string mtl; };
    std::vector<SubInfo> subinfos;
    for (const auto& f : faces) {
        if (subinfos.empty() || subinfos.back().mtl != f.material) {
            subinfos.push_back(SubInfo{{}, f.material});
        }
        auto& v = subinfos.back().idx;
        v.push_back(add_vertex(f.a));
        v.push_back(add_vertex(f.b));
        v.push_back(add_vertex(f.c));
    }

    std::size_t offset = 0;
    for (auto& si : subinfos) {
        Submesh sm;
        sm.index_offset = offset;
        sm.index_count  = si.idx.size();
        auto mit = materials.find(si.mtl);
        if (mit != materials.end()) {
            sm.material = mit->second;
        }
        cpu.indices.insert(cpu.indices.end(), si.idx.begin(), si.idx.end());
        cpu.submeshes.push_back(sm);
        offset += si.idx.size();
    }

    logging::log(reslog::MESH, logging::INFO, "Loaded OBJ '" + path_ + "' with " +
                                             std::to_string(cpu.vertex_count()) + " vertices and " +
                                             std::to_string(cpu.index_count() / 3) + " triangles");
    return true;
}
