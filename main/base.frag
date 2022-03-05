#version 410 core

out vec4 color;

uniform vec3 colorIn;

void main() {
    color = vec4(colorIn, 1.0f);
}