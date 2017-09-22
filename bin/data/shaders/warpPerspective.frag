#version 400

uniform sampler2DRect tex0;

in mat3 fWarpMatrix;
in vec2 texCoordVarying;
out vec4 outputColor;

void main()
{
    vec3 result = fWarpMatrix * vec3(texCoordVarying, 1.0);
    outputColor = vec4(texture(tex0, result.xy/result.z).rgb, 1.0);
}
