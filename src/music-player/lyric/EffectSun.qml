// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import audio.image 1.0
import "effectpublic"

CusShaderToy {
    id: r

    //step 6: around
    property real wave
    property real speed
    property real ringWidth
    property real rotationSpeed
    property real holeSmooth
    property real holeSize
    property real colorSaturation
    property real colorValue
    pixelShader: "
#define TAU 6.2831852
#define MOD3 vec3(.1031,.11369,.13787)
#define BLACK_COL vec3(16,21,25)/255.

vec3 hash33(vec3 p3)
{
    p3 = fract(p3 * MOD3);
    p3 += dot(p3, p3.yxz+19.19);
    return -1.0 + 2.0 * fract(vec3((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y, (p3.y+p3.z)*p3.x));
}

float simplex_noise(vec3 p)
{
    const float K1 = 0.333333333;
    const float K2 = 0.166666667;

    vec3 i = floor(p + (p.x + p.y + p.z) * K1);
    vec3 d0 = p - (i - (i.x + i.y + i.z) * K2);

    vec3 e = step(vec3(0.0), d0 - d0.yzx);
    vec3 i1 = e * (1.0 - e.zxy);
    vec3 i2 = 1.0 - e.zxy * (1.0 - e);

    vec3 d1 = d0 - (i1 - 1.0 * K2);
    vec3 d2 = d0 - (i2 - 2.0 * K2);
    vec3 d3 = d0 - (1.0 - 3.0 * K2);

    vec4 h = max(0.6 - vec4(dot(d0, d0), dot(d1, d1), dot(d2, d2), dot(d3, d3)), 0.0);
    vec4 n = h * h * h * h * vec4(dot(d0, hash33(i)), dot(d1, hash33(i + i1)), dot(d2, hash33(i + i2)), dot(d3, hash33(i + 1.0)));

    return dot(vec4(31.316), n);
}
//渐变色
vec4 removeRing(in vec4 t, in vec2 fragCoord, float radis, float  border)
{
    vec2 tc = fragCoord/iResolution.xy;
    tc -= 0.5;
    tc.x *= iResolution.x / iResolution.y;

    float grad = length(tc);
    float fClr = (1.0-smoothstep(radis- border, radis, grad));
    if(border < 0.26)
        t = t * ((fClr<1.&&fClr>0.)?0.9:fClr)*fClr*0.8;
    else
        t = t * fClr;
    return t;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = (fragCoord.xy-iResolution.xy*0.5)/iResolution.y;

    float a = sin(atan(uv.y, uv.x));
    float am = abs(a-.5)/4.;
    float l = length(uv);
    float colortex = texture(iChannel0, vec2(0.0, 0.0)).x/1.2;

    float m1 = clamp(.1/smoothstep(.0, 1.75, l), 0., 1.);
    float m2 = clamp(.1/smoothstep(.42, 0., l), 0., 1.);
    float s1 = (simplex_noise(vec3(uv*2., 1. + colortex*.525))*(max(1.0 - l*1.75, 0.)) + .9);
    float s2 = (simplex_noise(vec3(uv*1., 15. + colortex*.525))*(max(.0 + l*1., .025)) + 1.25);
    float s3 = (simplex_noise(vec3(vec2(am, am*100. + colortex*3.)*.15, 30. + colortex*.525))*(max(.0 + l*1., .25)) + 1.5);
    s3 *= smoothstep(0.0, .3345, l);

    float sh = smoothstep(0.15, .35, l);


    float m = m1*m1*m2 * ((s1*s2*s3) * (1.-l)) * sh;

    vec3 col = mix(iColor, (0.5 + 0.5*cos(iTime+uv.xyx*3.+vec3(0,2,4))), m*2.);
    vec4 t = removeRing(vec4(col, 1.), fragCoord, 0.5, 0.4);
    fragColor = removeRing(t, fragCoord, 0.5, 0.2);
}
"

    iChannel0: View_data {
        id : _image
        visible: true
        width: 1024
        height: 1
        anchors.centerIn: parent
        presenter: Presenter
    }
    iColor: Qt.vector3d(Presenter.getMainColorByKmeans().r, Presenter.getMainColorByKmeans().g, Presenter.getMainColorByKmeans().b)
}
