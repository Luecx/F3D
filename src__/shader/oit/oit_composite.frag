#version 460 core

layout(location = 0) out vec4 frag_color;

in vec2 v_uv;

uniform sampler2D u_transparent_accum;
uniform sampler2D u_transparent_reveal;
uniform sampler2D u_opaque_color;

void main() {
    vec4 accum = texture(u_transparent_accum, v_uv);
    float reveal = texture(u_transparent_reveal, v_uv).r;
    vec3 opaque = texture(u_opaque_color, v_uv).rgb;

    float accum_alpha = max(accum.a, 1e-4);
    vec3 transparent = accum.rgb / accum_alpha;
    float trans_alpha = clamp(1.0 - reveal, 0.0, 1.0);
    vec3 premult_transparent = transparent * trans_alpha;

    vec3 color = premult_transparent + opaque * (1.0 - trans_alpha);
    frag_color = vec4(color, 1.0);
}
