#include "mesh_data.h"

#include "resource_manager.h"

#include "../logging/logging.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include <glad/glad.h>

using namespace logging;

MeshData::MeshData(std::string path) : ResourceData(std::move(path)) { set_label("Mesh"); }

namespace {
struct Float3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

Float3 operator-(const Float3& a, const Float3& b) { return Float3{a.x - b.x, a.y - b.y, a.z - b.z}; }

struct VertexKey {
    int position{-1};
    int texcoord{-1};
    int normal{-1};
    int material_slot{-1};

    bool operator==(const VertexKey& other) const {
        return position == other.position && texcoord == other.texcoord && normal == other.normal &&
               material_slot == other.material_slot;
    }
};

struct VertexKeyHash {
    std::size_t operator()(const VertexKey& key) const noexcept {
        std::size_t seed = 0;
        seed ^= std::hash<int>{}(key.position) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>{}(key.texcoord) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>{}(key.normal) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>{}(key.material_slot) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

Float3 cross(const Float3& a, const Float3& b) {
    return Float3{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

Float3 normalize(const Float3& v) {
    float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len < 1e-8f) {
        return Float3{0.0f, 1.0f, 0.0f};
    }
    return Float3{v.x / len, v.y / len, v.z / len};
}

} // namespace

bool MeshData::load_to_ram() {
    std::ifstream file(get_path());
    if (!file.is_open()) {
        log(1, ERROR, "Failed to open OBJ file: " + get_path());
        return false;
    }

    geometry_ = MeshGeometry{};
    material_slots_.clear();

    std::vector<Float3> positions;
    std::vector<Float3> normals;
    std::vector<Vec2f> texcoords;

    std::unordered_map<VertexKey, uint32_t, VertexKeyHash> vertex_map;

    std::filesystem::path base_dir = std::filesystem::path(get_path()).parent_path();
    std::vector<std::filesystem::path> material_libraries;

    std::shared_ptr<MaterialData> active_material;
    std::unordered_map<MaterialData*, int> material_slot_lookup;
    int current_material_slot = -1;

    auto slot_for_material = [&](const std::shared_ptr<MaterialData>& mat) -> int {
        if (!mat) {
            return -1;
        }
        auto* ptr = mat.get();
        if (auto it = material_slot_lookup.find(ptr); it != material_slot_lookup.end()) {
            return it->second;
        }
        int slot = static_cast<int>(material_slots_.size());
        material_slot_lookup.emplace(ptr, slot);
        material_slots_.push_back(mat);
        register_dependency(resources::ResourceState::Ram, mat, resources::ResourceState::Ram);
        register_dependency(resources::ResourceState::Gpu, mat, resources::ResourceState::Gpu);
        return slot;
    };

    auto resolve_material = [&](const std::string& name) -> std::shared_ptr<MaterialData> {
        if (name.empty()) {
            return nullptr;
        }
        auto* mgr = get_manager();
        if (!mgr) {
            return nullptr;
        }
        for (const auto& lib : material_libraries) {
            auto mat = mgr->get_material(lib.string(), name);
            if (mat) {
                return mat;
            }
        }
        log(1, WARNING, "Material '" + name + "' referenced but not defined in OBJ: " + get_path());
        return nullptr;
    };

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;
        if (keyword == "v") {
            Float3 p;
            iss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        } else if (keyword == "vn") {
            Float3 n;
            iss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        } else if (keyword == "vt") {
            float u, v;
            iss >> u >> v;
            texcoords.emplace_back(u, v);
        } else if (keyword == "mtllib") {
            std::string name;
            iss >> name;
            if (!name.empty()) {
                material_libraries.push_back(std::filesystem::weakly_canonical(base_dir / name));
            }
        } else if (keyword == "usemtl") {
            std::string mat_name;
            iss >> mat_name;
            active_material = resolve_material(mat_name);
            current_material_slot = slot_for_material(active_material);
        } else if (keyword == "f") {
            std::vector<std::string> tokens;
            std::string token;
            while (iss >> token) {
                tokens.push_back(token);
            }
            if (tokens.size() < 3) {
                continue;
            }

            auto parse_vertex = [&](const std::string& str) -> VertexKey {
                VertexKey key;
                std::stringstream ss(str);
                std::string item;
                int index = 0;
                while (std::getline(ss, item, '/')) {
                    if (item.empty()) {
                        ++index;
                        continue;
                    }
                    int value = std::stoi(item) - 1;
                    if (index == 0)
                        key.position = value;
                    else if (index == 1)
                        key.texcoord = value;
                    else if (index == 2)
                        key.normal = value;
                    ++index;
                }
                key.material_slot = current_material_slot;
                return key;
            };

            auto emit_vertex = [&](const VertexKey& key) {
                auto it = vertex_map.find(key);
                if (it != vertex_map.end()) {
                    geometry_.indices.push_back(it->second);
                    return;
                }
                uint32_t new_index = static_cast<uint32_t>(geometry_.positions.size() / 3);
                vertex_map[key] = new_index;
                if (key.position >= 0 && key.position < static_cast<int>(positions.size())) {
                    const auto& p = positions[key.position];
                    geometry_.positions.insert(geometry_.positions.end(), {p.x, p.y, p.z});
                } else {
                    geometry_.positions.insert(geometry_.positions.end(), {0.0f, 0.0f, 0.0f});
                }
                if (key.normal >= 0 && key.normal < static_cast<int>(normals.size())) {
                    const auto& n = normals[key.normal];
                    geometry_.normals.insert(geometry_.normals.end(), {n.x, n.y, n.z});
                } else {
                    geometry_.normals.insert(geometry_.normals.end(), {0.0f, 0.0f, 0.0f});
                }
                if (key.texcoord >= 0 && key.texcoord < static_cast<int>(texcoords.size())) {
                    const auto& t = texcoords[key.texcoord];
                    geometry_.texcoords.insert(geometry_.texcoords.end(), {t[0], t[1]});
                } else {
                    geometry_.texcoords.insert(geometry_.texcoords.end(), {0.0f, 0.0f});
                }
                geometry_.material_slots.push_back(key.material_slot);
                geometry_.indices.push_back(new_index);
            };

            VertexKey v0 = parse_vertex(tokens[0]);
            VertexKey v1 = parse_vertex(tokens[1]);
            VertexKey v2 = parse_vertex(tokens[2]);
            emit_vertex(v0);
            emit_vertex(v1);
            emit_vertex(v2);

            for (std::size_t i = 3; i < tokens.size(); ++i) {
                VertexKey vi = parse_vertex(tokens[i]);
                emit_vertex(v0);
                emit_vertex(v2);
                emit_vertex(vi);
                v2 = vi;
            }
        }
    }

    if (geometry_.positions.empty()) {
        log(1, WARNING, "OBJ contains no vertices: " + get_path());
        return false;
    }

    // generate normals if missing
    if (geometry_.normals.size() != geometry_.positions.size()) {
        geometry_.normals.resize(geometry_.positions.size());
    }
    std::vector<Float3> accum(vertex_count());
    for (std::size_t i = 0; i + 2 < geometry_.indices.size(); i += 3) {
        auto idx0 = geometry_.indices[i];
        auto idx1 = geometry_.indices[i + 1];
        auto idx2 = geometry_.indices[i + 2];
        Float3 p0{geometry_.positions[idx0 * 3 + 0], geometry_.positions[idx0 * 3 + 1],
                  geometry_.positions[idx0 * 3 + 2]};
        Float3 p1{geometry_.positions[idx1 * 3 + 0], geometry_.positions[idx1 * 3 + 1],
                  geometry_.positions[idx1 * 3 + 2]};
        Float3 p2{geometry_.positions[idx2 * 3 + 0], geometry_.positions[idx2 * 3 + 1],
                  geometry_.positions[idx2 * 3 + 2]};
        Float3 normal = normalize(cross(p1 - p0, p2 - p0));
        accum[idx0].x += normal.x;
        accum[idx0].y += normal.y;
        accum[idx0].z += normal.z;
        accum[idx1].x += normal.x;
        accum[idx1].y += normal.y;
        accum[idx1].z += normal.z;
        accum[idx2].x += normal.x;
        accum[idx2].y += normal.y;
        accum[idx2].z += normal.z;
    }
    for (std::size_t i = 0; i < vertex_count(); ++i) {
        Float3 n = normalize(accum[i]);
        geometry_.normals[i * 3 + 0] = n.x;
        geometry_.normals[i * 3 + 1] = n.y;
        geometry_.normals[i * 3 + 2] = n.z;
    }

    has_transparent_materials_ = false;
    has_opaque_materials_ = false;
    for (const auto& slot : material_slots_) {
        if (!slot) {
            has_opaque_materials_ = true;
            continue;
        }
        bool transparent = slot->is_transparent();
        has_transparent_materials_ = has_transparent_materials_ || transparent;
        has_opaque_materials_ = has_opaque_materials_ || !transparent;
    }

    log(1, INFO,
        "Loaded OBJ '" + get_path() + "' with " + std::to_string(vertex_count()) + " vertices and " +
            std::to_string(material_slots_.size()) + " material assignments");
    return true;
}

void MeshData::unload_from_ram() {
    geometry_.positions.clear();
    geometry_.normals.clear();
    geometry_.texcoords.clear();
    geometry_.indices.clear();
    geometry_.material_slots.clear();
    material_slots_.clear();
    has_transparent_materials_ = false;
    has_opaque_materials_ = false;
}

bool MeshData::load_to_gpu() {
    if (geometry_.positions.empty() || geometry_.indices.empty()) {
        log(1, WARNING, "Mesh has no geometry; cannot upload: " + get_path());
        return false;
    }

    gpu_.vao = std::make_unique<VAOData>();
    gpu_.vao->bind();

    gpu_.position_vbo = std::make_unique<VBOData>();
    gpu_.normal_vbo = std::make_unique<VBOData>();
    gpu_.uv_vbo = std::make_unique<VBOData>();
    gpu_.index_vbo = std::make_unique<VBOData>();

    glEnableVertexAttribArray(0);
    gpu_.position_vbo->store_data(0, 3, geometry_.positions);

    glEnableVertexAttribArray(1);
    gpu_.normal_vbo->store_data(1, 3, geometry_.normals);

    if (!geometry_.texcoords.empty()) {
        glEnableVertexAttribArray(2);
        gpu_.uv_vbo->store_data(2, 2, geometry_.texcoords);
    }

    if (!geometry_.material_slots.empty()) {
        std::vector<int> slot_to_gpu(material_slots_.size(), -1);
        for (std::size_t i = 0; i < material_slots_.size(); ++i) {
            if (material_slots_[i]) {
                slot_to_gpu[i] = material_slots_[i]->gpu_material_index();
            }
        }

        std::vector<int> vertex_material_ids;
        vertex_material_ids.reserve(geometry_.material_slots.size());
        for (int slot : geometry_.material_slots) {
            int gpu_index = (slot >= 0 && static_cast<std::size_t>(slot) < slot_to_gpu.size()) ? slot_to_gpu[slot] : -1;
            vertex_material_ids.push_back(gpu_index);
        }

        glEnableVertexAttribArray(3);
        gpu_.material_vbo = std::make_unique<VBOData>();
        gpu_.material_vbo->store_data(3, 1, vertex_material_ids);
    }

    std::vector<uint32_t> indices32(geometry_.indices.begin(), geometry_.indices.end());
    gpu_.index_vbo->store_indices(indices32);

    gpu_.vao->unbind();
    log(1, INFO, "Uploaded mesh to GPU: " + get_path());
    return true;
}

void MeshData::unload_from_gpu() {
    gpu_.index_vbo.reset();
    gpu_.material_vbo.reset();
    gpu_.uv_vbo.reset();
    gpu_.normal_vbo.reset();
    gpu_.position_vbo.reset();
    gpu_.vao.reset();
}

void MeshData::draw() const {
    if (!gpu_.vao) {
        logging::log(0, logging::WARNING, "MeshData::draw skipped: GPU buffers missing for " + get_path());
        return;
    }
    if (index_count() == 0) {
        logging::log(0, logging::WARNING, "MeshData::draw skipped: no indices for " + get_path());
        return;
    }
    logging::log(0, logging::DEBUG,
                 "MeshData::draw issuing GL draw for " + get_path() + " indices=" + std::to_string(index_count()));
    gpu_.vao->bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(index_count()), GL_UNSIGNED_INT, nullptr);
    gpu_.vao->unbind();
}

void MeshData::draw_instanced(GLsizei instance_count, GLuint base_instance) const {
    if (instance_count <= 0) {
        return;
    }
    if (!gpu_.vao) {
        logging::log(0, logging::WARNING, "MeshData::draw_instanced skipped: GPU buffers missing for " + get_path());
        return;
    }
    if (index_count() == 0) {
        logging::log(0, logging::WARNING, "MeshData::draw_instanced skipped: no indices for " + get_path());
        return;
    }
    gpu_.vao->bind();
    glDrawElementsInstancedBaseInstance(GL_TRIANGLES, static_cast<GLsizei>(index_count()), GL_UNSIGNED_INT, nullptr,
                                        instance_count, base_instance);
    gpu_.vao->unbind();
}
