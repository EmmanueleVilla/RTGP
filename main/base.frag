#version 410 core

//--- OUTPUT TO FRAME BUFFER
layout(location = 0) out vec4 color;

//--- INPUT FROM APP
uniform sampler2D tex;
uniform float repeat;
uniform vec3 colorIn;
uniform float distorsion;
uniform float time;

//--- INPUT FROM SHADERS
in vec2 TexCoords;

//--- SUBROUTINES
subroutine vec4 fragshader();
subroutine uniform fragshader fragShaderImpl;

//--- CONSTANTS
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
}

//--- SUBROUTINE THAT APPLIES THE GIVEN TEXTURE
subroutine(fragshader)
vec4 textured() {
    vec2 repeatedUV = mod(TexCoords * repeat, 1.0f);
    return vec4(texture(tex, repeatedUV).xyz, 1.0f);
}


//--- SUBROUTING THAT APPLIES A PINCUSHION DISTORSION
subroutine(fragshader)
vec4 pincushion() {
    vec2 repeatedUV = mod(TexCoords * repeat, 1.0f);
    float newX = TexCoords.x - 1.0f;
    float y = pow(newX, 2) + newX + 0.1;
    y = clamp(y, 0.0f, 1.0f);
    vec2 uv = TexCoords.xy - vec2(0.5, 0.5);
	float uva = atan(uv.x, uv.y);
    float uvd = sqrt(dot(uv, uv));
    uvd = uvd * (1.0 + distorsion * uvd * uvd);
    vec3 col = vec3(texture(tex, vec2(0.5) + vec2(sin(uva), cos(uva)) * uvd).xyz);
    return vec4(col + vec3(distorsion/20.0f) + vec3(distorsion * y), 1.0f);
}

//--- SUBROUTINE THAT ALWAYS RETURNS THE GIVEN COLOR
subroutine(fragshader)
vec4 fixedColor() {
    return vec4(colorIn, 1.0f);
}

//--- RETURNS THE COLOR OF THE TEXTURE
//--- BY PINCUSHION DISTORTING THE GIVEN UVs
vec3 distortedColorByUv(vec2 TexCoords) {
    float newX = TexCoords.x - 1.0f;
    float y = pow(newX, 2) + newX + 0.1;
    y = clamp(y, 0.0f, 1.0f);
    vec2 uv = TexCoords.xy - vec2(0.5, 0.5);
	float uva = atan(uv.x, uv.y);
    float uvd = sqrt(dot(uv, uv));
    uvd = uvd * (1.0 + distorsion * uvd * uvd);
    vec3 col = vec3(texture(tex, vec2(0.5) + vec2(sin(uva), cos(uva)) * uvd).xyz);
    return col + vec3(distorsion/10.0f) + vec3(distorsion * y);
}

//--- SUBROUTINE THAT COLORS THE TRACE OF THE GREEN/RED TRACE TEXTURE
subroutine(fragshader)
vec4 tracePlane() {

    //--- IF I'M NOT IN WITCHER SENSES MODE, DISCARD ALL
    if(distorsion == 0) {
        discard;
    }

    //--- GET CURRENT COLOR, GREEN OR RED
    vec3 color = distortedColorByUv(TexCoords);

    //--- DISTANCE FROM THE NEIGHBOURS TO BE CHECKED
    float texel = 1.0f / 1280.0f * 5.5f;

    //--- NEIGHBOURS
    vec2 topTexel = TexCoords + vec2(0.0f, texel);
    vec3 topColor = distortedColorByUv(topTexel);

    vec2 bottomTexel = TexCoords + vec2(0.0f, -texel);
    vec3 bottomColor = distortedColorByUv(bottomTexel);

    vec2 leftTexel = TexCoords + vec2(-texel, 0.0f);
    vec3 leftColor = distortedColorByUv(leftTexel);

    vec2 rightTexel = TexCoords + vec2(-texel, 0.0f);
    vec3 rightColor = distortedColorByUv(rightTexel);

    vec2 topLeftTexel = TexCoords + vec2(-texel, texel);
    vec3 topLeftColor = distortedColorByUv(topLeftTexel);

    vec2 topRightTexel = TexCoords + vec2(texel, texel);
    vec3 topRightColor = distortedColorByUv(topRightTexel);

    vec2 bottomLeftTexel = TexCoords + vec2(-texel, -texel);
    vec3 bottomLeftColor = distortedColorByUv(bottomLeftTexel);

    vec2 bottomRightTexel = TexCoords + vec2(texel, -texel);
    vec3 bottomRightColor = distortedColorByUv(bottomRightTexel);

    if(color.z > 0) {
        //--- IF IT'S A BLUE PIXEL SURROUNDED BY ONLY GREEN PIXELS, I DISCARD IT
        if(topColor.r < 0.01
            && bottomColor.r < 0.9
            && leftColor.r < 0.9
            && rightColor.r < 0.9
            && topRightColor.r < 0.9
            && topLeftColor.r < 0.9
            && bottomRightColor.r < 0.9
            && bottomLeftColor.r < 0.9) {
            discard;
        }
    } else {
        //--- IF IT'S A RED PIXEL SURROUNDED BY ONLY RED PIXELS, I DISCARD IT
        if(topColor.r > 0.1
            && bottomColor.r > 0.1
            && leftColor.r > 0.1
            && rightColor.r > 0.1
            && topRightColor.r > 0.1
            && topLeftColor.r > 0.1
            && bottomRightColor.r > 0.1
            && bottomLeftColor.r > 0.1) {
            discard;
        }
    }

    //--- NOW SAME, BUT WITH A STRICTER RANGE
    texel = 1.0f / 1280.0f * 1.0f;

    topTexel = TexCoords + vec2(0.0f, texel);
    topColor = distortedColorByUv(topTexel);

    bottomTexel = TexCoords + vec2(0.0f, -texel);
    bottomColor = distortedColorByUv(bottomTexel);

    leftTexel = TexCoords + vec2(-texel, 0.0f);
    leftColor = distortedColorByUv(leftTexel);

    rightTexel = TexCoords + vec2(-texel, 0.0f);
    rightColor = distortedColorByUv(rightTexel);

    topLeftTexel = TexCoords + vec2(-texel, texel);
    topLeftColor = distortedColorByUv(topLeftTexel);

    topRightTexel = TexCoords + vec2(texel, texel);
    topRightColor = distortedColorByUv(topRightTexel);

    bottomLeftTexel = TexCoords + vec2(-texel, -texel);
    bottomLeftColor = distortedColorByUv(bottomLeftTexel);

    bottomRightTexel = TexCoords + vec2(texel, -texel);
    bottomRightColor = distortedColorByUv(bottomRightTexel);

    //--- IF I'M IN THE INTERNAL BORDER, MY ALPHA WILL ALWAYS BE 1
    bool variableAlpha = false;

    if(color.z > 0) {
        if(topColor.r < 0.01
            && bottomColor.r < 0.9
            && leftColor.r < 0.9
            && rightColor.r < 0.9
            && topRightColor.r < 0.9
            && topLeftColor.r < 0.9
            && bottomRightColor.r < 0.9
            && bottomLeftColor.r < 0.9) {
            variableAlpha = true;
        }
    } else {
        if(topColor.r > 0.1
            && bottomColor.r > 0.1
            && leftColor.r > 0.1
            && rightColor.r > 0.1
            && topRightColor.r > 0.1
            && topLeftColor.r > 0.1
            && bottomRightColor.r > 0.1
            && bottomLeftColor.r > 0.1) {
            variableAlpha = true;
        }
    }

    float alpha = 0.8f;
    if(variableAlpha) {
        //--- OTHERWISE, I GET A NOISE FROM 0 TO 1
        alpha = snoise(vec3(TexCoords * 15, time));
        float noiseMin = 0.0f;
        float noiseMax = 1.0f;
        float alphaMin = 0.5f;
        float alphaMax = 0.8f;

        //--- AND MOVE IT TO THE ALPHA_MIN - ALPHA_MAX RANGE
        alpha = ((alpha - noiseMin) / (noiseMax - noiseMin)) * (alphaMax - alphaMin) + alphaMin;
    }

    //--- APPLY NEGATIVE DISTORSION (FROM 0 TO 1) TO FADE IN/OUT THE TRACE
    return vec4(colorIn, alpha * distorsion * -1);
}

void main() {
    color = fragShaderImpl();
}