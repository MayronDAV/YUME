// Set = 0 -> Global
// Set = 1 -> Per Material


@type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec4 a_Color;

struct VertexOutput
{
    vec2 TexCoord;
    vec4 Color;
};
layout(location = 0) out VertexOutput Output;


layout(set = 0, binding = 0) uniform u_Camera
{
	mat4 ViewProjection;
    vec3 Position;
} u_camera;

layout(push_constant) uniform model
{
    mat4 Transform;
} Model;



void main()
{
    Output.TexCoord = a_TexCoord;
    Output.Color = a_Color;

    gl_Position = u_camera.ViewProjection * Model.Transform * vec4(a_Position, 1.0f);
}


@type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

struct VertexOutput
{
    vec2 TexCoord;
    vec4 Color;
};
layout(location = 0) in VertexOutput Input;

layout(push_constant) uniform model
{
    mat4 Transform;
} Model;


layout(set = 1, binding = 0) uniform sampler2D u_Texture;


vec3 DeGamma(vec3 p_Color, float p_Gamma)
{
	return pow(p_Color, vec3(1.0 / p_Gamma));
}

void main()
{
    vec4 color = Input.Color * texture(u_Texture, Input.TexCoord);
	o_Color = vec4(DeGamma(color.rgb, 1.5f), color.a);
}
