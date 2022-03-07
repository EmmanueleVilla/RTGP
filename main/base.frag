#version 410 core

out vec4 color;

//uniform vec3 colorIn;

in vec3 N;
in vec2 interp_UV;

void main() {
    //color = vec4(colorIn, 1.0f);
    color = vec4(vec3(N), 1.0f);
}