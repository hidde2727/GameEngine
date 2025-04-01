#version 450

layout (set = 0, binding = 0) uniform texture2D _texture[16];
layout (set = 0, binding = 1) uniform sampler _sampler[2];

layout(location = 0)      in vec3 _fragColor;
layout(location = 1)      in vec2 _texturePos;
layout(location = 2) flat in uint _textureID;
layout(location = 3)      in float _pxRange;

layout(location = 0) out vec4 outColor;


float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}
void main() {
    vec4 msd;
    if(_textureID == 0 ) { msd = texture(sampler2D(_texture[0 ], _sampler[1]), _texturePos); }
    if(_textureID == 1 ) { msd = texture(sampler2D(_texture[1 ], _sampler[1]), _texturePos); }
    if(_textureID == 2 ) { msd = texture(sampler2D(_texture[2 ], _sampler[1]), _texturePos); }
    if(_textureID == 3 ) { msd = texture(sampler2D(_texture[3 ], _sampler[1]), _texturePos); }
    if(_textureID == 4 ) { msd = texture(sampler2D(_texture[4 ], _sampler[1]), _texturePos); }
    if(_textureID == 5 ) { msd = texture(sampler2D(_texture[5 ], _sampler[1]), _texturePos); }
    if(_textureID == 6 ) { msd = texture(sampler2D(_texture[6 ], _sampler[1]), _texturePos); }
    if(_textureID == 7 ) { msd = texture(sampler2D(_texture[7 ], _sampler[1]), _texturePos); }
    if(_textureID == 8 ) { msd = texture(sampler2D(_texture[8 ], _sampler[1]), _texturePos); }
    if(_textureID == 9 ) { msd = texture(sampler2D(_texture[9 ], _sampler[1]), _texturePos); }
    if(_textureID == 10) { msd = texture(sampler2D(_texture[10], _sampler[1]), _texturePos); }
    if(_textureID == 11) { msd = texture(sampler2D(_texture[11], _sampler[1]), _texturePos); }
    if(_textureID == 12) { msd = texture(sampler2D(_texture[12], _sampler[1]), _texturePos); }
    if(_textureID == 13) { msd = texture(sampler2D(_texture[13], _sampler[1]), _texturePos); }
    if(_textureID == 14) { msd = texture(sampler2D(_texture[14], _sampler[1]), _texturePos); }
    if(_textureID == 15) { msd = texture(sampler2D(_texture[15], _sampler[1]), _texturePos); }

    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = _pxRange*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    outColor = mix(vec4(0,0,0,0), vec4(_fragColor.rgb, 1), opacity);
}