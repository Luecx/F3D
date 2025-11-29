#version 450 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require

#include "lit_common.glsl"

layout(location = 0) out vec4 frag_color;

in VS_OUT {
    vec3 world_pos;
    vec3 normal;
    vec2 uv;
} fs_in;
flat in int v_material_id;

void main() {
    MaterialSample _sample = sample_material(v_material_id, fs_in.uv);

    const float tile_size = 0.05;
    vec2 grid = floor(fs_in.uv / tile_size);
    float checker = mod(grid.x + grid.y, 2.0);
    vec3 checker_color_light = vec3(0.92, 0.92, 0.92);
    vec3 checker_color_dark = vec3(0.12, 0.12, 0.12);
_sample.base_color = mix(checker_color_dark, checker_color_light, checker);

    vec3 N = normalize(fs_in.normal);
    vec3 V = normalize(u_camera_pos - fs_in.world_pos);
    if (length(V) < 1e-5) {
        V = vec3(0.0, 0.0, 1.0);
    }

    if (u_debug_mode == 1) {
        vec3 debug_normal = N * 0.5 + 0.5;
        frag_color = vec4(debug_normal, 1.0);
        return;
    } else if (u_debug_mode == 2) {
        vec3 uv_color = vec3(fract(fs_in.uv), 0.0);
        frag_color = vec4(uv_color, 1.0);
        return;
    } else if (u_debug_mode == 3) {
        vec3 pos_color = normalize(abs(fs_in.world_pos));
        frag_color = vec4(pos_color, 1.0);
        return;
    } else if (u_debug_mode == 4) {
        float id = max(float(v_material_id), 0.0);
        vec3 id_color = vec3(fract(id * 0.123), fract(id * 0.456), fract(id * 0.789));
        frag_color = vec4(id_color, 1.0);
        return;
    }

    vec3 color = evaluate_lit_color(_sample, N, V, fs_in.world_pos);
    frag_color = vec4(color, 1.0);
}
