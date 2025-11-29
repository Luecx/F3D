// Color component now has an enabled flag and uses a 64‐bit texture handle for bindless access.
struct GPU_ColorComponent {
    bool enabled;             // Whether this color component is enabled.
    vec3 color;               // Base color.
    uint64_t texture_handle;  // Bindless texture handle.
};

// Scalar component with an enabled flag.
struct GPU_ScalarComponent {
    bool enabled;             // Whether this scalar component is enabled.
    float value;              // Scalar value.
    uint64_t texture_handle;  // Bindless texture handle.
};

// Texture component with an enabled flag.
// Padding is provided to satisfy std430 alignment.
struct GPU_TextureComponent {
    bool enabled;             // Whether this texture component is enabled.
    uint64_t texture_handle;  // Bindless texture handle.
    int padding1;
    int padding2;
    int padding3;
};

// The complete material structure with several sub-components.
// Note: The individual enabled flags now control each sub-component.
struct GPU_Material {
    GPU_ColorComponent base_color;
    GPU_ColorComponent emission;
    GPU_ColorComponent sheen_color;

    GPU_ScalarComponent roughness;
    GPU_ScalarComponent metallic;
    GPU_ScalarComponent specular;
    GPU_ScalarComponent specular_tint;
    GPU_ScalarComponent transmission;
    GPU_ScalarComponent transmission_roughness;
    GPU_ScalarComponent clearcoat;
    GPU_ScalarComponent clearcoat_roughness;
    GPU_ScalarComponent subsurface;
    GPU_ScalarComponent sheen;
    GPU_ScalarComponent sheen_tint;
    GPU_ScalarComponent anisotropy;

    float ior;
    float anisotropy_rotation;
    float padding1;
    float padding2;

    GPU_TextureComponent normal_map;
    GPU_TextureComponent tangent_map;
};

// To use these in your shader, bind a Shader Storage Buffer Object (SSBO) with an array of GPU_Material at your chosen binding point.
layout(std430, binding = 3) buffer MaterialBuffer {
    GPU_Material materials[];
};
