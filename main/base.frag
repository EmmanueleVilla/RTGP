#version 410 core

/* OUTPUT TO FRAME BUFFER */
layout(location = 0) out vec4 color;

/* INPUT FROM APP */
uniform sampler2D tex;
uniform float repeat;
uniform vec3 colorIn;
uniform float distorsion;
uniform float time;

/* INPUT FROM SHADERS */
in vec2 interp_UV;

/* SUBROUTINES */
subroutine vec4 fragshader();
subroutine uniform fragshader fragShaderImpl;

/* CONSTANTS */
const float PI = 3.1415926535;

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r) {
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v) {
    return 1.0f;
    /*
    const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
    const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

    vec3 i  = floor(v + dot(v, C.yyy) );
    vec3 x0 =   v - i + dot(i, C.xxx) ;
    
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min( g.xyz, l.zxy );
    vec3 i2 = max( g.xyz, l.zxy );

    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy;
    vec3 x3 = x0 - D.yyy;

    i = mod289(i);
    vec4 p = permute( permute( permute(
                i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
            + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
            + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

    float n_ = 0.142857142857;
    vec3  ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_ );

    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4( x.xy, y.xy );
    vec4 b1 = vec4( x.zw, y.zw );

    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

    vec3 p0 = vec3(a0.xy,h.x);
    vec3 p1 = vec3(a0.zw,h.y);
    vec3 p2 = vec3(a1.xy,h.z);
    vec3 p3 = vec3(a1.zw,h.w);

    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    m = m * m;
    return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3)));
    */
}

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
    /*
    float sinMin = -1.0f;
    float sinMax = 1.0f;

    float redSin = sin(time + 75 * interp_UV.x * interp_UV.y);
    float redMin = 0.6f;
    float redMax = 1.0f;

    float red = ((redSin - sinMin)/(sinMax - sinMin))*(redMax - redMin) + redMin;

    float alphaSin = sin(5*(time + 25 * interp_UV.x + 10 * interp_UV.y));
    float alphaMin = 0.25f;
    float alphaMax = 1.0f;

    float alpha = ((alphaSin - sinMin)/(sinMax - sinMin))*(alphaMax - alphaMin) + alphaMin;
    */
    float alpha = snoise(interp_UV);
    return vec4(1.0f, 0.0f, 0.0f, alpha);
}

void main() {
    color = fragShaderImpl();
}