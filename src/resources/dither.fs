// dithering shader;
// Idea is simple: Our texture is 128x128 in size and divided into 8x8 blocks.
// when we draw a pixel, we use the vertex UV position to determine which block
// should be used for drawing that texture on the screen. The texture is
// sampled in screen coordinates, so we need to convert the UV to screen.
// Since we use post processing to draw the outlines on top, we have to pass
// additional information to that shader:
// - Red: Packed RGB color from the dithering sampled texture.
// - Green: UV.y value, which is used to determine if the pixel is on the edge.
//          If UV.x is > 1.0, we add 1.0 to UV.y to indicate that the pixel belongs to FX (transparent)
// - Blue: Z value from the vertex position, used to prioritize the outline.
// - Alpha: Not used atm.
#ifdef GL_ES
precision highp float;                // Precision required for OpenGL ES2 (WebGL)
#endif
varying vec2 fragTexCoord;
varying vec4 fragColor;
varying vec3 fragPosition;
varying vec3 fragNormal;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform vec4 colDiffuse;
uniform float time;
uniform float drawInnerOutlines;
uniform vec2 uvOverride;
uniform float uvDitherBlockPosScale;
uniform vec2 texSize;
uniform vec4 uvTextureFrame;

vec2 encode16bit(float x) {
    // Ensure the value is within the 16-bit range
    x = (clamp(floor(x * 256.0), 0.0, 65535.0));

    // Split the value into two 8-bit components
    float low = mod(x, 256.0);
    float high = ((x - low) / 256.0);

    return vec2(high / 255.0, low / 255.0);
}

void main() {
    vec2 fogPoint = vec2(174.0/512.0, 200.0/512.0);
    float dist = length(fragPosition.xyz);
    float fogDist = max(0.0, pow(dist,0.4) - 1.5);
    float fogLevel = min(1.0,fogDist * 0.125);
    fogLevel = floor(fogLevel *64.0)/64.0;
    // gl_FragColor = vec4(fogLevel,0.0,0.0,1.0);
    // return;

    vec2 screenPos = gl_FragCoord.xy;
    vec2 uv = screenPos / texSize;
    vec2 fogBlockPos = mod(screenPos, 8.0) / 512.0;
    vec4 fogColor = texture2D(texture1,max(vec2(0,0), fogBlockPos + fogPoint - vec2(fogLevel, 0.0)));
    const float fogOutlineCutoff = 0.4;

    if (drawInnerOutlines == 0.0)
    {
        vec2 fragUv = fragTexCoord;
        if (uvTextureFrame.z > 0.0)
        {
            fragUv = fragUv * uvTextureFrame.zw + uvTextureFrame.xy;
        }
        vec4 color = texture2D(texture0, fragUv);
        if ((color.r > 0.9 && color.g < 0.5 && color.b > 0.9) || color.a < 0.1)
        {
            // pink transparent color
            discard;
        }


        gl_FragColor = vec4(0.0,0.0,0.0, color.g);
        float z = -fragPosition.z * 32.0;
        gl_FragColor.rb = encode16bit(z);
        if (fogColor.a > 0.5)
        {
            if (fogLevel > fogPoint.x * fogOutlineCutoff)
                gl_FragColor.g = 1.0;
            gl_FragColor.a = fogColor.g;
        }

        // gl_FragColor = fragColor;

        return;
    }
    vec2 fragUv = fragTexCoord;
    if (uvOverride.x > 0.0 || uvOverride.y > 0.0)
    {
        fragUv = uvOverride;
    }
    vec2 blockPos = floor(fract(fragUv) * uvDitherBlockPosScale) / uvDitherBlockPosScale;
    if (fragUv.x > 1.0)
    {
        uv.y += fract(time * -1.0) / uvDitherBlockPosScale;
        // uv.x += fract(time * -2.0) / uvDitherBlockPosScale;
    }
    uv.x = fract(uv.x * uvDitherBlockPosScale) / uvDitherBlockPosScale + blockPos.x;
    uv.y = fract(uv.y * uvDitherBlockPosScale) / uvDitherBlockPosScale + blockPos.y;
    vec4 color = texture2D(texture0, uv);
    if (color.r > 0.9 && color.g < 0.5 && color.b > 0.9)
    {
        // pink transparent color
        discard;
    }

    gl_FragColor.g = (fract(fragUv.y * 8.0) + (fragUv.x > 1.0 || fragUv.x < 0.0 ? 1.0 : 0.0));
    float z = (-fragPosition.z + 0.0 * fragNormal.z * (1.0 + fragUv.y)) * 32.0;
    gl_FragColor.rb = encode16bit(z);
    // use green channel value to reconstruct red / blue via lookup
    gl_FragColor.a = color.g;
    if (fogColor.a > 0.5)
    {
        if (fogLevel > fogPoint.x * fogOutlineCutoff)
                gl_FragColor.g = 1.0;
        gl_FragColor.a = fogColor.g;
        // gl_FragColor.g = 1.0;
    }
}