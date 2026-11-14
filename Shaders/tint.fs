#version 330

// EmberGUI — trivial post-process effect: multiplies a layer's texture by a
// tint color. Proves the offscreen-layer -> shader-pass -> composite round
// trip; real effects (glow, rainbow chase, blur) follow this same shape.

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 u_tint;

void main() {
    vec4 texel = texture(texture0, fragTexCoord);
    finalColor = texel * u_tint;
}