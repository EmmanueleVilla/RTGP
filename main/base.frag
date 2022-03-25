#version 410 core

/* OUTPUT TO FRAME BUFFER */
layout(location = 0) out vec4 color;

/* INPUT FROM APP */
uniform sampler2D tex;
uniform float repeat;
uniform vec3 colorIn;

/* INPUT FROM SHADERS */
in vec2 interp_UV;

/* SUBROUTINES */
subroutine vec3 fragshader();
subroutine uniform fragshader fragShaderImpl;

subroutine(fragshader)
vec3 textured() {
    vec2 repeatedUV = mod(interp_UV * repeat, 1.0f);
    return vec3(texture(tex, repeatedUV).xyz);
}

subroutine(fragshader)
vec3 fixedColor() {
    return vec3(colorIn);
}

void main() {
    color = vec4(fragShaderImpl(), 1.0f);
}