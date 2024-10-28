// outline shader;
// sample 4 neighboring pixels and determine if the current pixel is on the edge.
// edge pixels are identified by the following conditions:
// - the uv.y value differs - since the object's texture is simple, the color is selected in rows.
//   Due to this, we can use the y value as distinction indicator.
// - Z value is used to prioritize which side the edge is drawn on.
// - if the uv.y value is greater than 1.0, no edge detection is run, IF the current pixel is behind it.
//   This is done to have no outline behind FX objects but still having the outline when in front.
// The texture is 32bit float, which allows us to store the color in a single float.
precision highp float;                // Precision required for OpenGL ES2 (WebGL)
varying vec2 fragTexCoord;
varying vec4 fragColor;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 resolution;

uniform float depthOutlineEnabled;
uniform float uvOutlineEnabled;

float decode16bit(vec2 encoded) {
    // Reconstruct the 16-bit value from two 8-bit components
    float high = encoded.x * 255.0;
    float low = encoded.y * 255.0;
    float f = ((high * 256.0 + low) / 256.0);
    return f;
}

bool mapGreenToRedBlue(float green, float match, vec2 rg, out vec3 col)
{
    if (green < match + 1.0 && green > match - 1.0)
    {
        col.g = match;
        col.r = rg.x;
        col.b = rg.y;
        return true;
    }
    return false;
}

void main() {
    vec4 texelColor = texture2D(texture0, fragTexCoord.xy);
    float z = decode16bit(texelColor.rb);
    // float zg = z * 0.1;
    // gl_FragColor = vec4(zg, zg, zg, 1.0);
    // return;
    // gl_FragColor = vec4(texelColor.bbb*10.0, 1.0);
    vec4 texelColorN = texture2D(texture0, fragTexCoord.xy + vec2(0.0, 1.0 / resolution.y));
    vec4 texelColorE = texture2D(texture0, fragTexCoord.xy + vec2(1.0 / resolution.x, 0.0));
    vec4 texelColorS = texture2D(texture0, fragTexCoord.xy - vec2(0.0, 1.0 / resolution.y));
    vec4 texelColorW = texture2D(texture0, fragTexCoord.xy - vec2(1.0 / resolution.x, 0.0));
    
    float zN = decode16bit(texelColorN.rb);
    float zE = decode16bit(texelColorE.rb);
    float zS = decode16bit(texelColorS.rb);
    float zW = decode16bit(texelColorW.rb);
    
    // unpack color from 32bit but potential 16bit red channel
    float f = texelColor.r;
    vec3 color = vec3(0.0);
    color.g = texelColor.a;
    float green = color.g * 255.0;
    vec3 result;
    if (mapGreenToRedBlue(green, 65.0, vec2(85.0,95.0), result)
    || mapGreenToRedBlue(green, 105.0, vec2(100.0,100.0), result)
    || mapGreenToRedBlue(green, 185.0, vec2(100.0,100.0), result)
    || mapGreenToRedBlue(green, 140.0, vec2(80.0,215.0), result)
    || mapGreenToRedBlue(green, 115.0, vec2(215.0,85.0), result)
    || mapGreenToRedBlue(green, 200.0, vec2(230.0,110.0), result)
    || mapGreenToRedBlue(green, 245.0, vec2(220.0,255.0), result)
    )
    {
        color = result;
    }

    color.rgb /= 255.0;

    if (texelColor == vec4(1.0, 1.0, 1.0, 1.0))
    {
        color = vec3(1.0);
    }

    if (texelColor.g >= 1.0 
        || (texelColorW.g >= 1.0 && z + 0.5 > zW)
        || (texelColorE.g >= 1.0 && z + 0.5 > zE)
        || (texelColorS.g >= 1.0 && z + 0.5 > zS)
        || (texelColorN.g >= 1.0 && z + 0.5 > zN)
        )
    {
        gl_FragColor = vec4(color, 1.0);
        return;
    }
    // gl_FragColor = vec4(screenPos.x * 1000.0, screenPos.y * 0.001, 0.0, 1.0);
    float isEdge = 1.0;
    float v = texelColor.g;
    float vN = texelColorN.g;
    float vE = texelColorE.g;
    float vS = texelColorS.g;
    float vW = texelColorW.g;
    z += 0.0001;
    // v is the y coordinate of the UVs. If it differs, we want to draw a line (this is manually
    // defined when creating the 3d models). We only want a single pixel border. 
    if (
        // In the first checks here
        // we only use the v-difference to mark the edge, if the current pixel is in front of our neighbor.
        // without the z comparison, we would have 2 pixel wide edges
        (z < zE && abs(v - vE) > 0.00001 && uvOutlineEnabled > 0.0) || 
        (z < zW && abs(v - vW) > 0.00001 && uvOutlineEnabled > 0.0) || 
        (z < zS && abs(v - vS) > 0.00001 && uvOutlineEnabled > 0.0) || 
        (z < zN && abs(v - vN) > 0.00001 && uvOutlineEnabled > 0.0) ||
        // This 2nd check does z based edge detection; 
        // if the z value is closer than the assumed z value calculated using the
        // neighboring z values, we want an edge. It "sticks out" of the plane of neighboring pixels 
        // if this test succeeds, it means we found an edge due to a distance difference. Adding
        // a small value to compensate for rounding errors.
        (z + 0.75 < (zE + zW + zN + zS) * 0.25 && depthOutlineEnabled > 0.0)
        )
    {
        isEdge = 0.0;
    }
    else {
        z -= 0.0001;
        // the z comparison prevents another double edge
        // the v comparison is signed and is therefore also going only to one side
        if ((v - vW <= -0.000001 && z <= zW && uvOutlineEnabled > 0.0) ||
            (v - vS <= -0.000001 && z <= zS && uvOutlineEnabled > 0.0) ||
            (v - vN <= -0.000001 && z <= zN && uvOutlineEnabled > 0.0) ||
            (v - vE <= -0.000001 && z <= zE && uvOutlineEnabled > 0.0))
        {
            isEdge = 0.0;
        }
    }
    gl_FragColor = vec4(isEdge * color, 1.0);
}