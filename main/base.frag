#version 410 core

out vec4 color;

subroutine vec3 fragshader();

subroutine uniform fragshader fragShaderImpl;

subroutine(fragshader)
vec3 textured() {
    return vec3(1.0f, 0.0f, 0.0f);
}

void main() {
    color = vec4(fragShaderImpl(), 1.0f);
}