#version 450

layout (set = 0, binding = 0) uniform texture2D _texture[16];
layout (set = 0, binding = 1) uniform sampler _sampler[2];

layout(location = 0)      in vec3 _fragColor;
layout(location = 1)      in vec2 _texturePos;
layout(location = 2) flat in uint _textureID;

layout(location = 0) out vec4 outColor;

void main() {
    if(_textureID == 0 ) { outColor = texture(sampler2D(_texture[0 ], _sampler[0]), _texturePos); }
    if(_textureID == 1 ) { outColor = texture(sampler2D(_texture[1 ], _sampler[0]), _texturePos); }
    if(_textureID == 2 ) { outColor = texture(sampler2D(_texture[2 ], _sampler[0]), _texturePos); }
    if(_textureID == 3 ) { outColor = texture(sampler2D(_texture[3 ], _sampler[0]), _texturePos); }
    if(_textureID == 4 ) { outColor = texture(sampler2D(_texture[4 ], _sampler[0]), _texturePos); }
    if(_textureID == 5 ) { outColor = texture(sampler2D(_texture[5 ], _sampler[0]), _texturePos); }
    if(_textureID == 6 ) { outColor = texture(sampler2D(_texture[6 ], _sampler[0]), _texturePos); }
    if(_textureID == 7 ) { outColor = texture(sampler2D(_texture[7 ], _sampler[0]), _texturePos); }
    if(_textureID == 8 ) { outColor = texture(sampler2D(_texture[8 ], _sampler[0]), _texturePos); }
    if(_textureID == 9 ) { outColor = texture(sampler2D(_texture[9 ], _sampler[0]), _texturePos); }
    if(_textureID == 10) { outColor = texture(sampler2D(_texture[10], _sampler[0]), _texturePos); }
    if(_textureID == 11) { outColor = texture(sampler2D(_texture[11], _sampler[0]), _texturePos); }
    if(_textureID == 12) { outColor = texture(sampler2D(_texture[12], _sampler[0]), _texturePos); }
    if(_textureID == 13) { outColor = texture(sampler2D(_texture[13], _sampler[0]), _texturePos); }
    if(_textureID == 14) { outColor = texture(sampler2D(_texture[14], _sampler[0]), _texturePos); }
    if(_textureID == 15) { outColor = texture(sampler2D(_texture[15], _sampler[0]), _texturePos); }
}