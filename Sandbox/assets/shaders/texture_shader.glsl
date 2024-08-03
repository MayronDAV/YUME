// Quad Shader

@type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;

layout(location = 0) out vec4 o_FragColor;
layout(location = 1) out vec2 o_FragTexCoord;


layout(set = 0, binding = 0) uniform camera
{
    mat4 Projection;
    mat4 View;
} u_Camera;

layout(push_constant) uniform player {
    vec3 Position;
    vec4 Color;
} Player;



void main()
{
    o_FragColor = a_Color;
    o_FragTexCoord = a_TexCoord;
    gl_Position = u_Camera.Projection * u_Camera.View * vec4(a_Position + Player.Position, 1.0f);
}


@type fragment
#version 450 core

layout(location = 0) in vec4 o_FragColor;
layout(location = 1) in vec2 o_FragTexCoord;

layout(location = 0) out vec4 outColor;


layout(push_constant) uniform player {
    vec3 Position;
    vec4 Color;
} Player;

layout(binding = 1) uniform sampler2D u_TexSampler;



void main()
{
    outColor = texture(u_TexSampler, o_FragTexCoord);
}


