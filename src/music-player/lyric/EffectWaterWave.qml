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
float hash(float n) { return fract(sin(n) * 1e4); }
float hash(vec2 p) { return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); }

float noise(float x) {
    float i = floor(x);
    float f = fract(x);
    float u = f * f * (3.0 - 2.0 * f);
    return mix(hash(i), hash(i + 1.0), u);
}

float noise(vec2 x) {
    vec2 i = floor(x);
    vec2 f = fract(x);

    // Four corners in 2D of a tile
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));


    // Same code, with the clamps in smoothstep and common subexpressions
    // optimized away.
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}


vec3 palette( in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d )
{
    return a + b*cos( 6.28318*(c*t+d) );
}

vec3 pal(float x){
  return palette( x, vec3(0.5,0.5,0.5),vec3(0.5,0.5,0.5),vec3(1.0,0.7,0.4),vec3(0.0,0.15,0.20)   );
}

vec2 rotate(vec2 v, float a) {
    float s = sin(a);
    float c = cos(a);
    mat2 m = mat2(c, -s, s, c);
    return m * v;
}


float blendAdd(float base, float blend) {
    return min(base+blend,1.0);
}

vec3 blendAdd(vec3 base, vec3 blend) {
    return min(base+blend,vec3(1.0));
}

vec3 blendAdd(vec3 base, vec3 blend, float opacity) {
    return (blendAdd(base, blend) * opacity + base * (1.0 - opacity));
}

float blendReflect(float base, float blend) {
    return (blend==1.0)?blend:min(base*base/(1.0-blend),1.0);
}

vec3 blendReflect(vec3 base, vec3 blend) {
    return vec3(blendReflect(base.r,blend.r),blendReflect(base.g,blend.g),blendReflect(base.b,blend.b));
}

vec3 blendReflect(vec3 base, vec3 blend, float opacity) {
    return (blendReflect(base, blend) * opacity + base * (1.0 - opacity));
}
vec3 blendGlow(vec3 base, vec3 blend) {
    return blendReflect(blend,base);
}

vec3 blendGlow(vec3 base, vec3 blend, float opacity) {
    return (blendGlow(base, blend) * opacity + base * (1.0 - opacity));
}
#define STEPS 10.

//Note: Press pause and play at iChannel0 to start sound


vec3 circle(float n, vec2 uv, float scale){
    uv=rotate(uv, iTime*.3+n*(1./STEPS)*3.14159*2.);

    float k=sin(iTime*.5)+.5;

    uv+=noise(iTime+uv*4.)*.1*scale;

    float size= .3+(  n*(1./STEPS)*.1)*scale;

    float d = smoothstep(size  + .01 ,size,length(uv));

    vec3 c = pal( iTime*.1 +n*(1./STEPS) );
    return n-1.>=STEPS?vec3(d):c*d;

    return vec3(d);
}

vec3 particles(vec2 uv, float masterVel) {
    vec3 col=vec3(0.);
    float w=10.;
    uv+=.5 ;
    uv*=vec2(w,1.);
    float id=ceil(uv.x);
    uv=fract(uv);
       uv-=.5;

    float startY=fract(noise(id));
    float vel= (fract(id*.1)*.2 + .05);
    float size=fract(noise(id))*.5+.1;
    float l=length((uv - vec2(0. ,  -.7 + fract(startY  + (iTime )*vel )*1.4  ))/vec2(w,1.));
    l/=size*masterVel;
    float d=smoothstep(0.1,0.07,l)*(masterVel)*.4;

     //col=vec3(d);
     col=blendAdd(col, d*pal( iTime*.1 + 1. +0.1*id*(1./STEPS)  ));

     col+=pal( uv.y /6. -iTime*.1 + 1.  )*masterVel*.4;

    return col;
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
void mainImage( out vec4 fragColor, in vec2 fragCoord ){
    vec2 uv = fragCoord/iResolution.xy;
    uv-=.5;
    uv.x *= iResolution.x/iResolution.y;
    vec3 col=vec3(0.1);

    for(float i=0.; i< STEPS; i++){
        float fft  = texture(iChannel0, vec2(0.0, 0.0)).x;
        col=blendAdd(col,circle( i, uv, (.5+fft)), (1./STEPS)  *2.5);
    }
    float m = col.r;//smoothstep(0.1,1., col.r);

    vec4 t=texture(iChannel1, .5+ uv*(m)* (1. +.1*(sin(1.)*.5+.5)) );

    col+=particles(uv, texture(iChannel0, vec2(8.0, 0.0)).x );

    fragColor = removeRing(vec4( col, 0.5), fragCoord, 0.5);
}
"
    iChannel1: Image {
        source: "qrc:/dsg/img/bk.png"
    }
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
