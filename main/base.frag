#version 410 core

/* OUTPUT */
out vec4 color;

/* INPUT FROM APP */
uniform sampler2D tex;
uniform float repeat;

/* INPUT FROM SHADERS */
in vec2 interp_UV;

/* SUBROUTINES */
subroutine vec3 fragshader();
subroutine uniform fragshader fragShaderImpl;

subroutine(fragshader)
vec3 red() {
    return vec3(1.0f, 0.0f, 0.0f);
}

void main() {
    vec2 repeatedUV = mod(interp_UV * repeat, 1.0f);
    vec4 surfaceColor = texture(tex, repeatedUV);
    vec3 test = fragShaderImpl();
    //color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
    color = vec4(0.0f, sin(interp_UV.x * interp_UV.y), 0.0f, 1.0f);
}