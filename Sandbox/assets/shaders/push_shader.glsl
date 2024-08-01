// Quad Shader

@type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

layout(location = 0) out vec4 o_FragColor;

layout(push_constant) uniform player {
    mat4 Matrix;
    vec3 Position;
    vec4 Color;
} Player;



void main()
{
    o_FragColor = a_Color;
    gl_Position = Player.Matrix * vec4(a_Position + Player.Position, 1.0f);
}


@type fragment
#version 450 core

layout(location = 0) in vec4 o_FragColor;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform player {
    mat4 Matrix;
    vec3 Position;
    vec4 Color;
} Player;

void main()
{
    outColor = Player.Color;
}


