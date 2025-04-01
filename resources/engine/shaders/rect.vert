#version 450

layout(push_constant) uniform constants {
    vec2 framebufferSize;
} PushConstants;

// Per vertex
layout(location = 0) in vec2 _edge;
// Per instance
layout(location = 1) in vec2 _inPosition;
layout(location = 2) in vec2 _dimensions;
layout(location = 3) in vec3 _inColor;
layout(location = 4) in vec2 _inTexturePos;
layout(location = 5) in vec2 _inTextureDimensions;
layout(location = 6) in uint _inTextureID;

layout(location = 0)      out vec3 _fragColor;
layout(location = 1)      out vec2 _texturePos;
layout(location = 2) flat out uint _textureID;

void main() {
    gl_Position = vec4(
        ( (_inPosition.x+_edge.x*_dimensions.x) /PushConstants.framebufferSize.x)*2-1, 
        ( (_inPosition.y+_edge.y*_dimensions.y) /PushConstants.framebufferSize.y)*2-1, 
        0.0,
        1.0
    );
    _fragColor = _inColor;
    _texturePos = vec2(_inTexturePos.x+_edge.x*_inTextureDimensions.x, _inTexturePos.y+_edge.y*_inTextureDimensions.y);
    _textureID = _inTextureID;
}