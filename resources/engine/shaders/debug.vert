#version 450

layout(push_constant) uniform constants {
    vec2 framebufferSize;
} PushConstants;

// Per vertex
layout(location = 0) in vec2 _inPos;
layout(location = 1) in vec3 _inColor;

layout(location = 0) out vec3 _outColor;

void main() {
    _outColor = _inColor;
    gl_Position = vec4(
        ( _inPos.x/PushConstants.framebufferSize.x )*2-1,
        ( _inPos.y/PushConstants.framebufferSize.y )*2-1,
        0.0,
        1.0
    );
}