#version 330

// EmberGUI — rotating two-color gradient that sweeps around a rect's outline
// (the "Gemini text box" chasing-border look). Keeps the original alpha mask,
// same trick as rainbow.fs — pair with a border-only rect (fill alpha = 0) so
// only the outline reads the sweep.

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 u_color1;
uniform vec4 u_color2;
uniform float u_time;
uniform vec2 u_size; // rect size in pixels, keeps the sweep circular instead of skewed

void main() {
    vec4 texel = texture(texture0, fragTexCoord);

    // Angle around the rect's center, aspect-corrected so the gradient
    // travels evenly around the perimeter instead of racing along the long axis.
    vec2 p = (fragTexCoord - 0.5) * u_size;
    float angle = atan(p.y, p.x);

    float mix_amt = 0.5 + 0.5 * cos(angle - u_time * 2.0);
    vec3 col = mix(u_color2.rgb, u_color1.rgb, mix_amt);

    finalColor = vec4(col, texel.a);
}