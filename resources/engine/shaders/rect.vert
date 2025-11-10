#version 450

layout(push_constant) uniform constants {
    vec2 framebufferSize;
    vec2 cameraPos;
} PushConstants;

// Per vertex
layout(location = 0) in uint _edge;
// Per instance
layout(location = 1) in vec2 _inTopLeft;
layout(location = 2) in vec2 _inBottomRight;
layout(location = 3) in vec2 _inDeltaPosition;
layout(location = 4) in vec3 _inColor;
layout(location = 5) in vec2 _inTexturePos;
layout(location = 6) in vec2 _inTextureDimensions;
layout(location = 7) in uint _inTextureID;

layout(location = 0)      out vec3 _fragColor;
layout(location = 1)      out vec2 _texturePos;
layout(location = 2) flat out uint _textureID;

void main() {
    _fragColor = _inColor;
    _textureID = _inTextureID;
    if(_edge == 0) {
        gl_Position = vec4(
            ( (_inTopLeft.x                           - PushConstants.cameraPos.x ) /PushConstants.framebufferSize.x)*2-1, 
            ( (_inTopLeft.y                           - PushConstants.cameraPos.y ) /PushConstants.framebufferSize.y)*2-1, 
            0.0,
            1.0
        );
        _texturePos = _inTexturePos.xy;
    } else if(_edge == 1) {
        gl_Position = vec4(
            ( (_inTopLeft.x + _inDeltaPosition.x      - PushConstants.cameraPos.x ) /PushConstants.framebufferSize.x)*2-1, 
            ( (_inTopLeft.y + _inDeltaPosition.y      - PushConstants.cameraPos.y ) /PushConstants.framebufferSize.y)*2-1, 
            0.0,
            1.0
        );
        _texturePos = vec2(_inTexturePos.x + _inTextureDimensions.x, _inTexturePos.y);
    } else if(_edge == 2) {
        gl_Position = vec4(
            ( (_inBottomRight.x - _inDeltaPosition.x - PushConstants.cameraPos.x ) /PushConstants.framebufferSize.x)*2-1, 
            ( (_inBottomRight.y - _inDeltaPosition.y - PushConstants.cameraPos.y ) /PushConstants.framebufferSize.y)*2-1, 
            0.0,
            1.0
        );
        _texturePos = vec2(_inTexturePos.x, _inTexturePos.y + _inTextureDimensions.y);
    } else if(_edge == 3) {
        gl_Position = vec4(
            ( (_inBottomRight.x                      - PushConstants.cameraPos.x ) /PushConstants.framebufferSize.x)*2-1, 
            ( (_inBottomRight.y                      - PushConstants.cameraPos.y ) /PushConstants.framebufferSize.y)*2-1, 
            0.0,
            1.0
        );
        _texturePos = vec2(_inTexturePos.x + _inTextureDimensions.x, _inTexturePos.y + _inTextureDimensions.y);
    }
}