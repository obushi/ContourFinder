#version 410

uniform mat4 projectionMatrix;
uniform mat4 modelViewProjectionMatrix;

in vec4 position;
in vec2 texcoord;
out vec2 texCoordVarying;

void main()
{
    vec4 p = modelViewProjectionMatrix * position;
    texCoordVarying = texcoord;
    gl_Position = p;
}
