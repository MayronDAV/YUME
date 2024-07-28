// Quad Shader

@type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

layout(location = 0) out vec4 o_FragColor;



void main()
{
    o_FragColor = a_Color;
    gl_Position = vec4(a_Position, 1.0f);
}


@type fragment
#version 450 core

layout(location = 0) in vec4 o_FragColor;

layout(location = 0) out vec4 outColor;


void main()
{
    outColor = o_FragColor;
}


