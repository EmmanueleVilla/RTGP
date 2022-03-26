#version 410 core

/* OUTPUT TO FRAME BUFFER */
layout(location = 0) out vec4 color;

/* INPUT FROM APP */
uniform sampler2D tex;
uniform float repeat;
uniform vec3 colorIn;
uniform float distorsion;

/* INPUT FROM SHADERS */
in vec2 interp_UV;

/* SUBROUTINES */
subroutine vec3 fragshader();
subroutine uniform fragshader fragShaderImpl;

/* CONSTANTS */
const float PI = 3.1415926535;

subroutine(fragshader)
vec3 textured() {
    vec2 repeatedUV = mod(interp_UV * repeat, 1.0f);
    return vec3(texture(tex, repeatedUV).xyz);
}

subroutine(fragshader)
vec3 pincushion() {
    vec2 repeatedUV = mod(interp_UV * repeat, 1.0f);
    vec2 uv = interp_UV.xy - vec2(0.5, 0.5);
	float uva = atan(uv.x, uv.y);
    float uvd = sqrt(dot(uv, uv));
    uvd = uvd * (1.0 + distorsion * uvd * uvd);
    return vec3(texture(tex, vec2(0.5) + vec2(sin(uva), cos(uva)) * uvd).xyz);
}

subroutine(fragshader)
vec3 fixedColor() {
    return vec3(colorIn);
}

void main() {
    color = vec4(fragShaderImpl(), 1.0f);
}