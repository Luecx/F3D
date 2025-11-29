#version 460 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require

#include "lit_common.glsl"

layout(location = 0) out vec4 out_accum;
layout(location = 1) out vec4 out_reveal;

in VS_OUT {
    vec3 world_pos;
    vec3 normal;
    vec2 uv;
} fs_in;
flat in int v_material_id;

void main() {
    if (u_debug_mode != 0) {
        out_accum = vec4(0.0);
        out_reveal = vec4(1.0);
        return;
    }

    MaterialSample _sample = sample_material(v_material_id, fs_in.uv);
    vec3 N = normalize(fs_in.normal);
    vec3 V = normalize(u_camera_pos - fs_in.world_pos);
    if (length(V) < 1e-5) {
        V = vec3(0.0, 0.0, 1.0);
    }

    vec3 color = evaluate_lit_color(_sample, N, V, fs_in.world_pos);
    float alpha = clamp(1.0 - _sample.transmission, 0.0, 0.99);
    if (alpha <= 0.0001) {
        discard;
    }

    float weight = max(alpha, 0.01) * 8.0 + 0.01;
    vec3 weighted_color = color * alpha * weight;
    out_accum = vec4(weighted_color, alpha);
    out_reveal = vec4(alpha);
}
