#version 450

layout(location = 0) in vec2 _inPosition;
layout(location = 1) in vec3 _inColor;
layout(location = 2) in vec2 _inTexturePos;
layout(location = 3) in uint _inTextureID;

layout(location = 0)      out vec3 _fragColor;
layout(location = 1)      out vec2 _texturePos;
layout(location = 2) flat out uint _textureID;

void main() {
    gl_Position = vec4(_inPosition, 0.0, 1.0);
    _fragColor = _inColor;
    _texturePos = _inTexturePos;
    _textureID = _inTextureID;
}