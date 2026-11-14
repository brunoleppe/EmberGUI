#version 330

// EmberGUI — animated rainbow fill effect. Replaces a layer's color with a
// hue that sweeps diagonally across it and cycles over time, keeping the
// original alpha mask (so border/fill shape is unchanged, only its color
// animates). Same shape as TintEffect — a Shader + a uniform setter — just
// with a continuously-updated time uniform instead of a static color.

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float u_time;

vec3 hue2rgb(float h) {
    vec3 rgb = clamp(abs(mod(h * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
    return rgb;
}

void main() {
    vec4 texel = texture(texture0, fragTexCoord);

    float hue = fract(u_time * 0.25 + (fragTexCoord.x + fragTexCoord.y) * 0.5);
    vec3 rainbow = hue2rgb(hue);

    finalColor = vec4(rainbow, texel.a);
}