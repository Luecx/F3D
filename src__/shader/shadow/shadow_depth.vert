#version 460 core

layout(location = 0) in vec3 in_position;

layout(std430, row_major, binding = 0) readonly buffer InstanceBuffer {
    mat4 instance_matrices[];
};

uniform mat4 u_light_vp;

out VS_OUT {
    vec3 world_pos;
} vs_out;

void main() {
    uint index = gl_BaseInstance + gl_InstanceID;
    mat4 model = instance_matrices[index];
    vec4 world_position = model * vec4(in_position, 1.0);
    vs_out.world_pos = world_position.xyz;
    gl_Position = u_light_vp * world_position;
}
