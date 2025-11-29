#ifndef LIT_COMMON_GLSL
#define LIT_COMMON_GLSL

#include "../../material/mat.glsl"

#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_SPOT_LIGHTS 4
#define MAX_POINT_LIGHTS 8

struct DirectionalLight {
    vec3 color;
    float intensity;
    vec3 direction;
    int casts_shadows;
    mat4 light_view_projection;
};

struct PointLight {
    vec3 color;
    float intensity;
    vec3 position;
    float radius;
    int casts_shadows;
    float shadow_far;
};

struct SpotLight {
    vec3 color;
    float intensity;
    vec3 position;
    float range;
    vec3 direction;
    float cos_inner;
    float cos_outer;
    int casts_shadows;
    mat4 light_view_projection;
};

struct MaterialSample {
    vec3 base_color;
    float metallic;
    float roughness;
    vec3 emission;
    float transmission;
};

uniform DirectionalLight u_directional_lights[MAX_DIRECTIONAL_LIGHTS];
uniform int u_directional_light_count;
uniform sampler2D u_directional_shadow_maps[MAX_DIRECTIONAL_LIGHTS];

uniform SpotLight u_spot_lights[MAX_SPOT_LIGHTS];
uniform int u_spot_light_count;
uniform sampler2D u_spot_shadow_maps[MAX_SPOT_LIGHTS];

uniform PointLight u_point_lights[MAX_POINT_LIGHTS];
uniform int u_point_light_count;
uniform samplerCube u_point_shadow_maps[MAX_POINT_LIGHTS];
uniform vec3 u_camera_pos;
uniform int u_debug_mode;

const float PI = 3.14159265359;
const vec3 kDefaultBaseColor = vec3(0.82, 0.78, 0.74);
const float kDefaultMetallic = 0.0;
const float kDefaultRoughness = 0.5;

bool fetch_material_data(int material_id, out GPU_Material mat) {
    if (material_id < 0 || material_id >= materials.length()) {
        return false;
    }
    mat = materials[material_id];
    return true;
}

vec3 sample_color_component(const GPU_ColorComponent comp, vec2 uv) {
    if (comp.enabled) {
        sampler2D tex = sampler2D(comp.texture_handle);
        return texture(tex, uv).rgb;
    }
    return comp.color;
}

float sample_scalar_component(const GPU_ScalarComponent comp, vec2 uv) {
    if (comp.enabled) {
        sampler2D tex = sampler2D(comp.texture_handle);
        return texture(tex, uv).r;
    }
    return comp.value;
}

float distribution_ggx(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float geometry_schlick_ggx(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = geometry_schlick_ggx(NdotV, roughness);
    float ggx2 = geometry_schlick_ggx(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnel_schlick(float cos_theta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cos_theta, 5.0);
}

float directional_shadow(int light_index, vec3 world_pos, vec3 N, vec3 L) {
    if (u_directional_lights[light_index].casts_shadows == 0) {
        return 1.0;
    }
    vec4 light_space = u_directional_lights[light_index].light_view_projection * vec4(world_pos, 1.0);
    vec3 proj = light_space.xyz / light_space.w;
    proj = proj * 0.5 + 0.5;
    if (proj.z > 1.0 || proj.z < 0.0) {
        return 1.0;
    }

    vec2 uv = proj.xy;
    float current_depth = proj.z;
    float bias = max(0.0005 * (1.0 - dot(N, L)), 0.00005);
    float visibility = 0.0;
    vec2 texel_size = 1.0 / textureSize(u_directional_shadow_maps[light_index], 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(x, y) * texel_size;
            float depth_sample = texture(u_directional_shadow_maps[light_index], uv + offset).r;
            visibility += current_depth - bias <= depth_sample ? 1.0 : 0.0;
        }
    }
    visibility /= 9.0;
    return visibility;
}

float spot_shadow(int light_index, vec3 world_pos, vec3 N, vec3 L) {
    if (u_spot_lights[light_index].casts_shadows == 0) {
        return 1.0;
    }
    vec4 light_space = u_spot_lights[light_index].light_view_projection * vec4(world_pos, 1.0);
    vec3 proj = light_space.xyz / light_space.w;
    proj = proj * 0.5 + 0.5;
    if (proj.z > 1.0 || proj.z < 0.0) {
        return 1.0;
    }

    vec2 uv = proj.xy;
    float current_depth = proj.z;
    float bias = max(0.001 * (1.0 - dot(N, L)), 0.0001);
    float visibility = 0.0;
    vec2 texel_size = 1.0 / textureSize(u_spot_shadow_maps[light_index], 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(x, y) * texel_size;
            float depth_sample = texture(u_spot_shadow_maps[light_index], uv + offset).r;
            visibility += current_depth - bias <= depth_sample ? 1.0 : 0.0;
        }
    }
    visibility /= 9.0;
    return visibility;
}

float point_shadow(int light_index, float distance_to_light, vec3 direction_to_fragment, vec3 N, vec3 L) {
    if (u_point_lights[light_index].casts_shadows == 0) {
        return 1.0;
    }
    float far_plane = u_point_lights[light_index].shadow_far;
    float bias = 0.003 * (1.0 - dot(N, L));
    vec3 dir = normalize(direction_to_fragment);
    float closest_depth = texture(u_point_shadow_maps[light_index], dir).r * far_plane;
    return distance_to_light - bias <= closest_depth ? 1.0 : 0.0;
}

MaterialSample sample_material(int material_id, vec2 uv) {
    GPU_Material mat;
    bool has_material = fetch_material_data(material_id, mat);

    MaterialSample _sample;
    _sample.base_color = has_material ? sample_color_component(mat.base_color, uv) : kDefaultBaseColor;
    _sample.metallic = has_material ? clamp(sample_scalar_component(mat.metallic, uv), 0.0, 1.0) : kDefaultMetallic;
    _sample.roughness = has_material ? clamp(sample_scalar_component(mat.roughness, uv), 0.04, 1.0) : kDefaultRoughness;
    _sample.emission = has_material ? sample_color_component(mat.emission, uv) : vec3(0.0);
    _sample.transmission = has_material ? clamp(sample_scalar_component(mat.transmission, uv), 0.0, 1.0) : 0.0;
    return _sample;
}

vec3 evaluate_lit_color(MaterialSample _sample, vec3 N, vec3 V, vec3 world_pos) {
    vec3 F0 = mix(vec3(0.04), _sample.base_color, _sample.metallic);
    vec3 ambient = _sample.base_color * 0.03;
    vec3 color = ambient;

    for (int i = 0; i < u_directional_light_count && i < MAX_DIRECTIONAL_LIGHTS; ++i) {
        vec3 L = normalize(-u_directional_lights[i].direction);
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) {
            continue;
        }

        vec3 H = normalize(V + L);
        float NDF = distribution_ggx(N, H, _sample.roughness);
        float G = geometry_smith(N, V, L, _sample.roughness);
        vec3 F = fresnel_schlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - _sample.metallic);
        vec3 diffuse = kD * _sample.base_color / PI;

        float shadow = directional_shadow(i, world_pos, N, L);
        vec3 radiance = u_directional_lights[i].color * u_directional_lights[i].intensity;
        color += (diffuse + specular) * radiance * NdotL * shadow;
    }

    for (int i = 0; i < u_spot_light_count && i < MAX_SPOT_LIGHTS; ++i) {
        vec3 to_light = u_spot_lights[i].position - world_pos;
        float distance = length(to_light);
        if (distance <= 1e-4 || distance > u_spot_lights[i].range) {
            continue;
        }
        vec3 L = to_light / distance;
        vec3 light_dir = normalize(u_spot_lights[i].direction);
        float theta = dot(light_dir, -L);
        float epsilon = max(u_spot_lights[i].cos_inner - u_spot_lights[i].cos_outer, 0.0001);
        float intensity = clamp((theta - u_spot_lights[i].cos_outer) / epsilon, 0.0, 1.0);
        if (intensity <= 0.0) {
            continue;
        }

        float attenuation = intensity * (1.0 - clamp(distance / u_spot_lights[i].range, 0.0, 1.0));
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) {
            continue;
        }

        vec3 H = normalize(V + L);
        float NDF = distribution_ggx(N, H, _sample.roughness);
        float G = geometry_smith(N, V, L, _sample.roughness);
        vec3 F = fresnel_schlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - _sample.metallic);
        vec3 diffuse = kD * _sample.base_color / PI;

        float shadow = spot_shadow(i, world_pos, N, L);
        vec3 radiance = u_spot_lights[i].color * u_spot_lights[i].intensity;
        color += (diffuse + specular) * radiance * NdotL * attenuation * shadow;
    }

    for (int i = 0; i < u_point_light_count && i < MAX_POINT_LIGHTS; ++i) {
        vec3 to_light = u_point_lights[i].position - world_pos;
        float distance = length(to_light);
        if (distance <= 1e-4 || distance > u_point_lights[i].radius) {
            continue;
        }
        vec3 L = to_light / distance;
        float attenuation = 1.0 - clamp(distance / u_point_lights[i].radius, 0.0, 1.0);
        attenuation *= attenuation;
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) {
            continue;
        }

        vec3 H = normalize(V + L);
        float NDF = distribution_ggx(N, H, _sample.roughness);
        float G = geometry_smith(N, V, L, _sample.roughness);
        vec3 F = fresnel_schlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - _sample.metallic);
        vec3 diffuse = kD * _sample.base_color / PI;

        vec3 radiance = u_point_lights[i].color * u_point_lights[i].intensity * attenuation;
        float shadow = point_shadow(i, distance, world_pos - u_point_lights[i].position, N, L);
        color += (diffuse + specular) * radiance * NdotL * shadow;
    }

    color += _sample.emission;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    return color;
}

#endif
