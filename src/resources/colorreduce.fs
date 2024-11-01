//precision highp float;                // Precision required for OpenGL ES2 (WebGL)
varying vec2 fragTexCoord;
uniform sampler2D texture0;

bool mapGreenToRedBlue(float green, float match, vec2 rg, float delta, out vec3 col)
{
    if (green < match + delta && green > match - delta)
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

    // if (true)
    // {
    //     gl_FragColor = vec4(texelColor.rgb,1.0);
    //     return;
    // }

    vec3 color = vec3(0.0);
    float green = texelColor.g * 255.0;
    vec3 result;
    if (mapGreenToRedBlue(green, 55.0, vec2(85.0,95.0), 20.0, result)
    || mapGreenToRedBlue(green, 102.0, vec2(100.0,100.0), 12.0, result)
    || mapGreenToRedBlue(green, 115.0, vec2(215.0,85.0), 8.0, result)
    || mapGreenToRedBlue(green, 140.0, vec2(80.0,215.0), 20.0, result)
    || mapGreenToRedBlue(green, 185.0, vec2(100.0,100.0), 5.0, result)
    || mapGreenToRedBlue(green, 200.0, vec2(230.0,110.0), 10.0, result)
    || mapGreenToRedBlue(green, 245.0, vec2(220.0,255.0), 25.0, result)
    )
    {
        color = vec3(result) / 255.0;
    }
    else
    {
        // color = green < 90.0 ? vec3(0.0, 0.0, 0.0) : vec3(1.0, 1.0, 1.0);
    }

    gl_FragColor = vec4(color, 1.0);
}