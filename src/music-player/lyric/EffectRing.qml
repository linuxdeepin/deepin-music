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
#define SOUND_MULTIPLIER 1.0

float drawCircle(float r, float polarRadius, float thickness)
{
    return 	smoothstep(r, r + thickness, polarRadius) -
            smoothstep(r + thickness, r + 2.0 * thickness, polarRadius);
}

float sin01(float v)
{
    return 0.5 + 0.5 * sin(v);
}
vec4 removeRing(in vec4 t, in vec2 fragCoord, float radis)
{
    vec2 tc = fragCoord/iResolution.xy;
    tc -= 0.5;
    tc.x *= iResolution.x / iResolution.y;

    float grad = length(tc);
    t = t * (1.0-smoothstep(radis-0.2, radis, grad));
    return t;
}
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord.xy / iResolution.xy;

    float rstandard = SOUND_MULTIPLIER * texture( iChannel0, vec2(0.0, 0.0) ).x;

    // Center the coordinates and apply the aspect ratio
    vec2 p = uv - vec2(0.5)/* + vec2(0.05, 0.05) * rstandard*/;
    p.x *= iResolution.x / iResolution.y;

    // Calculate polar coordinates
    float pr = length(p);
    float pa = atan(p.y, p.x); // * 3.0 / 3.14;

    // Retrieve the information from the texture
    float idx = (pa/3.1415 + 1.0) / 2.0;   // 0 to 1
    float idx2 = idx * 3.1415;             // 0 to PI

    // Get the data from the microphone
    vec2 react = sin(idx2) * SOUND_MULTIPLIER * texture( iChannel0, vec2(8.0, 0.0) ).xy;

    // Draw the circles
    float o = 0.0;
    float inc = 0.0;
    float fValue = 0/* * texture( iChannel0, vec2(idx, 0.0) ).x*/;

    for( float i = 1.0 ; i < 8.0 ; i += 1.0 )
    {
        float baseradius = 0.3 * ( 0.3 + sin01(rstandard + fValue * 0.2) );
        float radius = baseradius + inc;

        radius += 0.01 * ( sin01(pa * i + idx * (i - 1.0) ) );

        o += drawCircle(radius, pr, 0.008 * (1.0 + react.x * (i - 1.0)));

        inc += 0.007;
    }

    // Calculate the background color
    vec3 col = mix(vec3(0), iColor, o);
    vec4 t = mix(vec4(0.0), vec4(col, 0.2), 1);
    fragColor = removeRing(t, fragCoord, 0.45);
}
"
    iChannel0: View_data {
        id : _image
        visible: true
        anchors.centerIn: parent
        width: 1024
        height: 1
        presenter: Presenter
    }
    iColor: Qt.vector3d(Presenter.getMainColorByKmeans().r, Presenter.getMainColorByKmeans().g, Presenter.getMainColorByKmeans().b)

}
