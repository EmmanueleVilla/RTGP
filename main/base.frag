#version 410 core

out vec4 color;

//uniform vec3 colorIn;

in vec3 N;

void main() {
    //color = vec4(colorIn, 1.0f);
    color = vec4(N, 1.0f);
}