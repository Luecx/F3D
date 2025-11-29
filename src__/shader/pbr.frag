#version 450 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require

#include "../material/mat.glsl"

layout(location = 0) out vec4 frag_color;

in VS_OUT {
    vec3 world_pos;
    vec3 normal;
    vec2 uv;
} fs_in;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

uniform int u_material_index = 0;
uniform DirectionalLight u_light;
uniform vec3 u_camera_pos;

vec3 sample_color(GPU_ColorComponent comp, vec2 uv) {
    if (comp.enabled) {
        sampler2D tex = sampler2D(comp.texture_handle);
        return texture(tex, uv).rgb;
    }
    return comp.color;
}

float sample_scalar(GPU_ScalarComponent comp, vec2 uv) {
    if (comp.enabled) {
        sampler2D tex = sampler2D(comp.texture_handle);
        return texture(tex, uv).r;
    }
    return comp.value;
}

vec3 apply_normal_map(GPU_TextureComponent comp, vec3 normal, vec2 uv) {
    if (comp.enabled) {
        sampler2D tex = sampler2D(comp.texture_handle);
        vec3 tangent_normal = texture(tex, uv).xyz * 2.0 - 1.0;
        vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 1.0, 0.0);
        vec3 tangent = normalize(cross(up, normal));
        vec3 bitangent = cross(normal, tangent);
        mat3 TBN = mat3(tangent, bitangent, normal);
        return normalize(TBN * tangent_normal);
    }
    return normalize(normal);
}

vec3 fresnel_schlick(float cos_theta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cos_theta, 5.0);
}

float distribution_ggx(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (3.14159265 * denom * denom);
}

float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
    float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
    return ggx1 * ggx2;
}

void main() {
    if (u_material_index < 0 || u_material_index >= materials.length()) {
        frag_color = vec4(1.0, 0.0, 1.0, 1.0);
        return;
    }

    GPU_Material mat = materials[u_material_index];

    vec3 base_color = sample_color(mat.base_color, fs_in.uv);
    float metallic = sample_scalar(mat.metallic, fs_in.uv);
    float roughness = clamp(sample_scalar(mat.roughness, fs_in.uv), 0.04, 1.0);
    float ao = 1.0;
    if (mat.ambient_occlusion_map.enabled) {
        sampler2D tex = sampler2D(mat.ambient_occlusion_map.texture_handle);
        ao = texture(tex, fs_in.uv).r;
    }

    vec3 N = apply_normal_map(mat.normal_map, normalize(fs_in.normal), fs_in.uv);
    vec3 V = normalize(u_camera_pos - fs_in.world_pos);
    vec3 L = normalize(-u_light.direction);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    if (NdotL <= 0.0) {
        frag_color = vec4(0.0);
        return;
    }

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, base_color, metallic);

    float NDF = distribution_ggx(N, H, roughness);
    float G = geometry_smith(N, V, L, roughness);
    vec3 F = fresnel_schlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 diffuse = base_color / 3.14159265;
    vec3 radiance = u_light.color;

    vec3 color = (kD * diffuse + specular) * radiance * NdotL;
    color *= ao;

    vec3 emission = sample_color(mat.emission, fs_in.uv);
    float emission_strength = sample_scalar(mat.emission_strength, fs_in.uv);
    color += emission * emission_strength;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    frag_color = vec4(color, 1.0);
}
