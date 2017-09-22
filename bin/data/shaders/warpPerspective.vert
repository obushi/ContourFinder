#version 400

uniform mat3 warpMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelViewProjectionMatrix;

in vec4 position;
in vec2 texcoord;
out mat3 fWarpMatrix;
out vec2 texCoordVarying;

void main()
{
    vec4 p = modelViewProjectionMatrix * position;
    fWarpMatrix = warpMatrix;
    texCoordVarying = texcoord;
    gl_Position = p;
}
