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
    vec2 p = (2.0*fragCoord.xy-iResolution.xy)/iResolution.y;
    float tau = 3.1415926535*2.0;
    float a = atan(p.x,p.y);
    float r = length(p)*0.75;
    vec2 uv = vec2(a/tau,r);
    vec2 tc = fragCoord/iResolution.xy;
    float colortex = 1.0*texture(iChannel0, vec2(0.0, 0.0)).x;
    //get the color
    float xCol = (uv.x - (colortex / 3.0)) * 3.0;
    xCol = mod(xCol, 3.0);
//    vec3 horColour = vec3(0.25);
    vec3 horColour = iColor;
    if (xCol < 1.0) {

        horColour.r += 1.0 - xCol;
        horColour.g += xCol;
    }
    else if (xCol < 2.0) {

        xCol -= 1.0;
        horColour.g += 1.0 - xCol;
        horColour.b += xCol;
    }
    else {

        xCol -= 2.0;
        horColour.b += 1.0 - xCol;
        horColour.r += xCol;
    }

    // draw color beam
    uv = (2.0 * uv) - 1.0;
    float beamWidth = (0.7+0.5*cos(uv.x*10.0*tau*0.15*clamp(floor(5.0 + 10.0*cos(iTime)), 0.0, 10.0))) * abs(1.0 / (30.0 * uv.y));
    vec3 horBeam = vec3(beamWidth);
    vec3 col = mix(iColor, (vec3(( horBeam) * horColour)), beamWidth);
    fragColor = removeRing(vec4(col, 1.), fragCoord, 0.5);
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
