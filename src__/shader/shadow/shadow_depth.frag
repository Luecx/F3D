#version 450 core

in VS_OUT {
    vec3 world_pos;
} fs_in;

uniform vec3 u_light_pos;
uniform float u_far_plane;
uniform int u_is_point_light;

void main() {
    if (u_is_point_light == 1) {
        float distance_to_light = length(fs_in.world_pos - u_light_pos);
        float depth = distance_to_light / u_far_plane;
        gl_FragDepth = clamp(depth, 0.0, 1.0);
    }
}
