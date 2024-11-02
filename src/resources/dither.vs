#ifdef GL_ES
precision highp float;                // Precision required for OpenGL ES2 (WebGL) (on some browsers)
#endif
attribute vec3 vertexPosition;
attribute vec3 vertexNormal;
attribute vec2 vertexTexCoord;
attribute vec4 vertexColor;
varying vec2 fragTexCoord;
varying vec4 fragColor;
varying vec3 fragPosition;
varying vec3 fragNormal;

uniform mat4 matView;
uniform mat4 matModel;

uniform mat4 mvp;
void main() {
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    gl_Position = mvp * vec4(vertexPosition, 1.0);
    fragPosition = (matView * matModel * vec4(vertexPosition, 1.0)).xyz;
    fragNormal = (matModel * vec4(vertexNormal, 0.0)).xyz;
    // fragPosition -= fragNormal * 0.5;
}
