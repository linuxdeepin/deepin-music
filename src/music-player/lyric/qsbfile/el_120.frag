#version 120

struct buf
{
    mat4 qt_Matrix;
    float qt_Opacity;
    vec3 iColor;
    vec3 iResolution;
    float iTime;
    float iTimeDelta;
    int iFrame;
    float iFrameRate;
    float iChannelTime[4];
    vec3 iChannelResolution[4];
    vec4 iMouse;
    vec4 iDate;
    float iSampleRate;
};

uniform buf _70;

uniform sampler2D iChannel1;
uniform sampler2D iChannel0;
uniform sampler2D iChannel2;
uniform sampler2D iChannel3;

varying vec4 fragCoord;

vec2 rotate(inout vec2 point, vec2 center, float angle)
{
    float s = sin(radians(angle));
    float c = cos(radians(angle));
    point.x -= center.x;
    point.y -= center.y;
    float x = (point.x * c) - (point.y * s);
    float y = (point.x * s) + (point.y * c);
    point.x = x + center.x;
    point.y = y + center.y;
    return point;
}

vec4 capsule(vec4 color, vec4 background, vec4 region, vec2 uv)
{
    bool _329 = uv.x > (region.x - region.z);
    bool _340;
    if (_329)
    {
        _340 = uv.x < (region.x + region.z);
    }
    else
    {
        _340 = _329;
    }
    bool _352;
    if (_340)
    {
        _352 = uv.y > (region.y - region.w);
    }
    else
    {
        _352 = _340;
    }
    bool _363;
    if (_352)
    {
        _363 = uv.y < (region.y + region.w);
    }
    else
    {
        _363 = _352;
    }
    bool _378;
    if (!_363)
    {
        _378 = distance(uv, region.xy - vec2(0.0, region.w)) < region.z;
    }
    else
    {
        _378 = _363;
    }
    bool _393;
    if (!_378)
    {
        _393 = distance(uv, region.xy + vec2(0.0, region.w)) < region.z;
    }
    else
    {
        _393 = _378;
    }
    if (_393)
    {
        return color;
    }
    return background;
}

vec4 bar(vec4 color, vec4 background, vec2 position, vec2 diemensions, vec2 uv)
{
    vec4 param = color;
    vec4 param_1 = background;
    vec4 param_2 = vec4(position.x, position.y + (diemensions.y / 2.0), diemensions.x / 2.0, diemensions.y / 2.0);
    vec2 param_3 = uv;
    return capsule(param, param_1, param_2, param_3);
}

vec4 rays(inout vec4 color, inout vec4 background, vec2 position, float radius, float rays_1, float ray_length, sampler2D sound, vec2 uv)
{
    float inside = (1.0 - ray_length) * radius;
    float outside = radius - inside;
    float circle = 9.424777984619140625 * inside;
    for (int i = 1; float(i) <= rays_1; i++)
    {
        float len = outside * texture2D(sound, vec2(float(i) / rays_1, 0.0)).x;
        vec3 _251 = mix(vec3(0.83529412746429443359375, 0.94509804248809814453125, 0.650980412960052490234375), vec3(1.0, 0.839215695858001708984375, 1.0), vec3(float(i) / rays_1));
        color.x = _251.x;
        color.y = _251.y;
        color.z = _251.z;
        vec2 param = uv;
        vec2 param_1 = position;
        float param_2 = (360.0 / rays_1) * float(i);
        vec2 _283 = rotate(param, param_1, param_2);
        vec4 param_3 = color;
        vec4 param_4 = background;
        vec2 param_5 = vec2(position.x, position.y + inside);
        vec2 param_6 = vec2(circle / (rays_1 * 2.0), len);
        vec2 param_7 = _283;
        background = bar(param_3, param_4, param_5, param_6, param_7);
    }
    return background;
}

vec4 removeRing(inout vec4 t, vec2 fragCoord_1, float radis, float border)
{
    vec2 tc = fragCoord_1 / _70.iResolution.xy;
    tc -= vec2(0.5);
    tc.x *= (_70.iResolution.x / _70.iResolution.y);
    float grad = length(tc);
    float fClr = 1.0 - smoothstep(radis - border, radis, grad);
    if (border < 0.2599999904632568359375)
    {
        t = ((t * (((fClr < 1.0) && (fClr > 0.0)) ? 0.89999997615814208984375 : fClr)) * fClr) * 0.800000011920928955078125;
    }
    else
    {
        t *= fClr;
    }
    return t;
}

void mainImage(out vec4 fragColor, vec2 fragCoord_1)
{
    float aspect = _70.iResolution.x / _70.iResolution.y;
    vec2 uv = fragCoord_1 / _70.iResolution.xy;
    uv.x *= aspect;
    vec4 color = mix(vec4(0.66274511814117431640625, 0.752941191196441650390625, 0.862745106220245361328125, 1.0), vec4(0.525490224361419677734375, 0.9843137264251708984375, 0.9843137264251708984375, 1.0), vec4(distance(vec2(aspect / 2.0, 0.5), uv)));
    float RADIUS = 0.4600000083446502685546875;
    float RAY_LENGTH = 0.300000011920928955078125;
    vec4 param = vec4(0.83529412746429443359375, 0.94509804248809814453125, 0.650980412960052490234375, 1.0);
    vec4 param_1 = color;
    vec2 param_2 = vec2(aspect / 2.0, 0.5);
    float param_3 = RADIUS;
    float param_4 = 128.0;
    float param_5 = RAY_LENGTH;
    vec2 param_6 = uv;
    vec4 _190 = rays(param, param_1, param_2, param_3, param_4, param_5, iChannel1, param_6);
    color = _190;
    vec4 param_7 = color;
    vec2 param_8 = fragCoord_1;
    float param_9 = 0.5;
    float param_10 = 0.300000011920928955078125;
    vec4 _198 = removeRing(param_7, param_8, param_9, param_10);
    vec4 t = _198;
    vec4 param_11 = t;
    vec2 param_12 = fragCoord_1;
    float param_13 = 0.5;
    float param_14 = 0.20000000298023223876953125;
    vec4 _206 = removeRing(param_11, param_12, param_13, param_14);
    fragColor = _206;
}

void main()
{
    vec2 param_1 = vec2(fragCoord.x, _70.iResolution.y - fragCoord.y);
    vec4 param;
    mainImage(param, param_1);
    gl_FragData[0] = param;
}

