@vs vs
in vec2 pos;
in vec3 color;
out vec3 vcolor;

void main() {
    gl_Position = vec4(pos, 0.0, 1.0);
    vcolor = color;
}
@end

@fs fs
in vec3 vcolor;
out vec4 frag_color;

void main() {
    frag_color = vec4(vcolor, 1.0);
}
@end

@program triangle vs fs
