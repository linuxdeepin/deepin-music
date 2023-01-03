// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import audio.image 1.0
import "effectpublic"
import QtQml.Models 2.3

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
#define M_PI 3.14159265359

vec4 rectangle(vec4 color, vec4 background, vec4 region, vec2 uv);
vec4 capsule(vec4 color, vec4 background, vec4 region, vec2 uv);
vec2 rotate(vec2 point, vec2 center, float angle);
vec4 bar(vec4 color, vec4 background, vec2 position, vec2 diemensions, vec2 uv);
vec4 rays(vec4 color, vec4 background, vec2 position, float radius, float rays, float ray_length, sampler2D sound, vec2 uv);

vec4 removeRing(in vec4 t, in vec2 fragCoord, float radis, float  border)
{
    vec2 tc = fragCoord/iResolution.xy;
    tc -= 0.5;
    tc.x *= iResolution.x / iResolution.y;

    float grad = length(tc);
    float fClr = (1.0-smoothstep(radis- border, radis, grad));
    if(border < 0.26)
        t = t * ((fClr<1.0&&fClr>0.)?0.9:fClr)*fClr*0.8;
    else
        t = t * fClr;
    return t;
}
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    //Prepare UV and background
    float aspect = iResolution.x / iResolution.y;
    vec2 uv = fragCoord/iResolution.xy;
    uv.x *= aspect;
    vec4 color = mix(vec4(vec3(169, 192, 220)/255.0, 1), vec4(vec3(134, 251, 251)/255.0, 1), distance(vec2(aspect/2.0, 0.5), uv));
    //VISUALIZER PARAMETERS
    const float RAYS = 128.0; //number of rays //Please, decrease this value if shader is working too slow
    float RADIUS = 0.46; //max circle radius
    float RAY_LENGTH = 0.3; //ray's max length //increased by 0.1
    color = rays(vec4(vec3(213, 241, 166)/255.0, 1), color, vec2(aspect/2.0, 1.0/2.0), RADIUS, RAYS, RAY_LENGTH, iChannel1, uv);

    vec4 t = removeRing(color, fragCoord, 0.5, 0.3);
    fragColor = removeRing(t, fragCoord, 0.5, 0.2);
}

vec4 rays(vec4 color, vec4 background, vec2 position, float radius, float rays, float ray_length, sampler2D sound, vec2 uv)
{
    float inside = (1.0 - ray_length) * radius; //empty part of circle
    float outside = radius - inside; //rest of circle
    float circle = 3.0*M_PI*inside; //circle lenght
    for(int i = 1; float(i) <= rays; i++)
    {
        float len = outside * texture(sound, vec2(float(i)/rays, 0.0)).x/* + 0.022 * abs(sin(float(i)* iTime))*/; //length of actual ray
        color.rgb = mix(vec3(213, 241, 166)/255.0, vec3(255, 214, 255)/255.0, float(i)/rays);
        background = bar(color, background, vec2(position.x, position.y+inside), vec2(circle/(rays*2.0), len), rotate(uv, position, 360.0/rays*float(i))); //Added capsules
    }
    return background; //output
}

vec4 bar(vec4 color, vec4 background, vec2 position, vec2 diemensions, vec2 uv)
{
    return capsule(color, background, vec4(position.x, position.y+diemensions.y/2.0, diemensions.x/2.0, diemensions.y/2.0), uv); //Just transform rectangle a little
}

vec4 capsule(vec4 color, vec4 background,  vec4 region, vec2 uv) //capsule
{
    if(uv.x > (region.x-region.z) && uv.x < (region.x+region.z) &&
       uv.y > (region.y-region.w) && uv.y < (region.y+region.w) ||
       distance(uv, region.xy - vec2(0.0, region.w)) < region.z ||
       distance(uv, region.xy + vec2(0.0, region.w)) < region.z)
        return color;
    return background;
}

vec2 rotate(vec2 point, vec2 center, float angle) //rotating point around the center
{
    float s = sin(radians(angle));
    float c = cos(radians(angle));

    point.x -= center.x;
    point.y -= center.y;

    float x = point.x * c - point.y * s;
    float y = point.x * s + point.y * c;

    point.x = x + center.x;
    point.y = y + center.y;

    return point;
}"

    iChannel1: View_data {
        id : _image
        visible: true
        anchors.centerIn: parent
        width: 1024
        height: 1
        presenter: Presenter
    }
    iColor: Qt.vector3d(Presenter.getMainColorByKmeans().r, Presenter.getMainColorByKmeans().g, Presenter.getMainColorByKmeans().b)
}
