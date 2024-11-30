@type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec4 a_Color;

layout(set = 0, binding = 0) uniform u_LightBuffer
{
	mat4 LightSpaceMatrix;
} u_lightBuffer;

layout(push_constant) uniform model
{
	mat4 Transform;
} Model;


out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main()
{
    gl_Position = u_lightBuffer.LightSpaceMatrix * Model.Transform * vec4(a_Position, 1.0);
}

@type fragment
#version 450 core

layout(push_constant) uniform model
{
	mat4 Transform;
} Model;



void main()
{
}