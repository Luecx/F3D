#version 450 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;
layout(location = 3) in int  in_material_id;

uniform mat4 u_mvp;

out vec3 v_model_pos;
out vec3 v_normal;
out vec2 v_uv;
out vec4 v_clip_pos;
flat out int v_material_id;

void main() {
    gl_Position = u_mvp * vec4(in_position, 1.0);
    v_model_pos = in_position;
    v_normal = in_normal;
    v_uv = in_texcoord;
    v_clip_pos = gl_Position;
    v_material_id = in_material_id;
}
