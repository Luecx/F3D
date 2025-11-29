#version 450 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require

#include "../material/mat.glsl"

in vec3 v_model_pos;
in vec3 v_normal;
in vec2 v_uv;
in vec4 v_clip_pos;
flat in int v_material_id;

out vec4 frag_color;

// Hard-coded directional light that approximates a simple overhead key light.
const vec3 kLightDirection = normalize(vec3(-0.4, -0.9, -0.6));
const vec3 kLightColor     = vec3(1.0, 0.98, 0.95);
const vec3 kAmbientColor   = vec3(0.12, 0.14, 0.17);

bool fetch_material(out GPU_Material mat) {
    if (v_material_id < 0 || v_material_id >= materials.length()) {
        return false;
    }
    mat = materials[v_material_id];
    return true;
}

vec3 sample_base_color(const GPU_Material mat, vec2 uv, bool has_material) {
    if (!has_material) {
        return vec3(0.7, 0.7, 0.72);
    }
    if (mat.base_color.enabled) {
        sampler2D tex = sampler2D(mat.base_color.texture_handle);
        return texture(tex, uv).rgb;
    }
    return mat.base_color.color;
}

void main() {
    GPU_Material mat;
    bool has_material = fetch_material(mat);

    vec3 normal = normalize(v_normal);
    vec3 light_dir = normalize(-kLightDirection);
    float NdotL = max(dot(normal, light_dir), 0.0);

    vec3 albedo = sample_base_color(mat, v_uv, has_material);
    vec3 diffuse = albedo * kLightColor * NdotL;
    vec3 ambient = albedo * kAmbientColor;

    // Cheap specular using a fixed eye vector pointing along -Z; enough for simple highlights.
    vec3 view_dir = normalize(vec3(0.0, 0.0, 1.0));
    vec3 half_vec = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, half_vec), 0.0), 32.0);
    vec3 specular = spec * vec3(0.2);

    vec3 color = ambient + diffuse + specular;
    frag_color = vec4(color, 1.0);
}
