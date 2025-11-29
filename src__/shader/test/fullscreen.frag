#include "../../material/mat.glsl"

out vec4 frag_color;

void main() {
    // Simple gradient or color
    vec2 uv = gl_FragCoord.xy / vec2(1280.0, 720.0); // adjust or pass resolution
    frag_color = vec4(uv, 0.5 + 0.5 * sin(uv.x * 10.0), 1.0);
}
