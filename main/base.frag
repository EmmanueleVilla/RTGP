#version 410 core

out vec4 color;

//uniform vec3 colorIn;

in vec3 N;
in vec2 interp_UV;

void main() {
    //color = vec4(colorIn, 1.0f);
    color = vec4(interp_UV, 0.0f, 1.0f);
}