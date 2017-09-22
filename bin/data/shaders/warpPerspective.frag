#version 410

uniform sampler2DRect tex0;
uniform mat3 warpMatrix;

in vec2 texCoordVarying;
out vec4 outputColor;

void main()
{
    vec3 result = warpMatrix * vec3(texCoordVarying, 1.0);
    outputColor = vec4(texture(tex0, result.xy/result.z).rgb, 1.0);
}
