#version 460 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;
layout(location = 3) in int  in_material_id;

layout(std430, row_major, binding = 0) readonly buffer InstanceBuffer {
    mat4 instance_matrices[];
};

uniform mat4 u_view;
uniform mat4 u_projection;

out VS_OUT {
    vec3 world_pos;
    vec3 normal;
    vec2 uv;
} vs_out;
flat out int v_material_id;

void main() {
    uint index = gl_BaseInstance + gl_InstanceID;
    mat4 model = instance_matrices[index];
    vec4 world = model * vec4(in_position, 1.0);
    vs_out.world_pos = world.xyz;
    vs_out.normal = mat3(transpose(inverse(model))) * in_normal;
    vs_out.uv = in_texcoord;
    gl_Position = u_projection * u_view * world;
    v_material_id = in_material_id;
}
