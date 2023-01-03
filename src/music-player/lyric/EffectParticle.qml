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
#define NUM_PARTICLES 200.0
#define GLOW 0.5

vec3 Orb(vec2 uv, vec3 color, float radius, float offset)
{
    vec2 position = vec2(sin(offset * (iTime+30.)),
                         cos(offset * (iTime+30.)));

    position *= sin((iTime ) - offset) * cos(offset);
    radius = radius * offset;
    float dist = radius / distance(uv, position);
    return color * pow(dist, 1.0 / GLOW);
}

vec4 removeRing(in vec4 t, in vec2 fragCoord, float radis)
{
    vec2 tc = fragCoord/iResolution.xy;
    tc -= 0.5;
    tc.x *= iResolution.x / iResolution.y;

    float grad = length(tc);
    t = t * (1.0-smoothstep(radis-0.4, radis, grad));
    return t;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv =  vec2(fragCoord.xy - 0.5 * iResolution.xy) / iResolution.y*1.5;
    vec3 pixel = vec3(0.0, 0.0, 0.0);
    vec3 color = vec3(0.0, 0.0, 0.0);
    vec2 smuv = fragCoord/iResolution.xy;
    float colortex = texture(iChannel0, vec2(smuv.x, 0.0)).x;
    color.r = ((sin(((iTime)) * 0.55) + 1.5) * 0.4);
    color.g = ((sin(((iTime)) * 0.34) + 2.0) * 0.4);
    color.b = ((sin(((iTime)) * 0.31) + 4.5) * 0.3);

    float radius = 0.025;

    for	(float i = 0.0; i < NUM_PARTICLES; i++)
        pixel += Orb(uv, iColor, radius*(0.2+texture(iChannel0, vec2(float(i)/NUM_PARTICLES, 0.0)).x), i / NUM_PARTICLES);

    vec4 t = mix(vec4(0.), vec4(pixel, 0.), 1.);
    fragColor = removeRing(t, fragCoord, 0.5);
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
