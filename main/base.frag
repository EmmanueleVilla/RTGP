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
subroutine vec4 fragshader();
subroutine uniform fragshader fragShaderImpl;

/* CONSTANTS */
const float PI = 3.1415926535;

subroutine(fragshader)
vec4 textured() {
    vec2 repeatedUV = mod(interp_UV * repeat, 1.0f);
    return vec4(texture(tex, repeatedUV).xyz, 1.0f);
}

subroutine(fragshader)
vec4 pincushion() {
    vec2 repeatedUV = mod(interp_UV * repeat, 1.0f);
    float newX = interp_UV.x - 1.0f;
    float y = pow(newX, 2) + newX + 0.1;
    y = clamp(y, 0.0f, 1.0f);
    vec2 uv = interp_UV.xy - vec2(0.5, 0.5);
	float uva = atan(uv.x, uv.y);
    float uvd = sqrt(dot(uv, uv));
    uvd = uvd * (1.0 + distorsion * uvd * uvd);
    vec3 col = vec3(texture(tex, vec2(0.5) + vec2(sin(uva), cos(uva)) * uvd).xyz);
    return vec4(col + vec3(distorsion/20.0f) + vec3(distorsion * y), 1.0f);
}

subroutine(fragshader)
vec4 fixedColor() {
    return vec4(colorIn, 1.0f);
}

subroutine(fragshader)
vec4 redOutline() {
    return vec4(0.5f - distorsion, 0.0f, 0.0f, 1.0f);
}

void main() {
    color = fragShaderImpl();
}