//------------------------------------------------------------------------------
// Renderer2D Circle Shader
// ------------------------------------------------------------------------------

@type vertex
#version 450 core

layout(location = 0) in vec3 a_WorldPosition;
layout(location = 1) in vec3 a_LocalPosition;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in float a_Thickness;
layout(location = 4) in float a_Fade;
layout(location = 5) in int a_TexIndex;

layout(set = 0, binding = 0) uniform u_Camera
{
	mat4 ViewProjection;
	glm::vec3 Position;
} u_camera;

struct VertexOutput
{
	vec3 LocalPosition;
	vec4 Color;
	float Thickness;
	float Fade;
	vec2 TexCoord;
};
layout (location = 0) out VertexOutput Output;
layout (location = 6) out flat int TexIndex;


void main()
{
	Output.LocalPosition = a_LocalPosition;
	Output.Color = a_Color;
	Output.Thickness = a_Thickness;
	Output.Fade = a_Fade;
	Output.TexCoord = a_LocalPosition.xy * 0.5 + vec2(0.5);

	TexIndex = a_TexIndex;


	gl_Position = u_camera.ViewProjection * vec4(a_WorldPosition, 1.0);
}


@type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

struct VertexOutput
{
	vec3 LocalPosition;
	vec4 Color;
	float Thickness;
	float Fade;
	vec2 TexCoord;
};
layout (location = 0) in VertexOutput Input;
layout (location = 6) in flat int TexIndex;

#define MAX_TEXTURE_SLOTS 32
layout(set = 1, binding = 0) uniform sampler2D u_Textures[MAX_TEXTURE_SLOTS];


void main()
{
    // Calculate distance and fill circle with white
    float distance = 1.0 - length(Input.LocalPosition);
    float circle = smoothstep(0.0, Input.Fade, distance);
    circle *= smoothstep(Input.Thickness + Input.Fade, Input.Thickness, distance);

	if (circle == 0.0)
		discard;

	o_Color = Input.Color;
	
	switch(TexIndex)
	{
		case  0: o_Color *= texture(u_Textures[ 0], Input.TexCoord); break;
		case  1: o_Color *= texture(u_Textures[ 1], Input.TexCoord); break;
		case  2: o_Color *= texture(u_Textures[ 2], Input.TexCoord); break;
		case  3: o_Color *= texture(u_Textures[ 3], Input.TexCoord); break;
		case  4: o_Color *= texture(u_Textures[ 4], Input.TexCoord); break;
		case  5: o_Color *= texture(u_Textures[ 5], Input.TexCoord); break;
		case  6: o_Color *= texture(u_Textures[ 6], Input.TexCoord); break;
		case  7: o_Color *= texture(u_Textures[ 7], Input.TexCoord); break;
		case  8: o_Color *= texture(u_Textures[ 8], Input.TexCoord); break;
		case  9: o_Color *= texture(u_Textures[ 9], Input.TexCoord); break;
		case 10: o_Color *= texture(u_Textures[10], Input.TexCoord); break;
		case 11: o_Color *= texture(u_Textures[11], Input.TexCoord); break;
		case 12: o_Color *= texture(u_Textures[12], Input.TexCoord); break;
		case 13: o_Color *= texture(u_Textures[13], Input.TexCoord); break;
		case 14: o_Color *= texture(u_Textures[14], Input.TexCoord); break;
		case 15: o_Color *= texture(u_Textures[15], Input.TexCoord); break;
		case 16: o_Color *= texture(u_Textures[16], Input.TexCoord); break;
		case 17: o_Color *= texture(u_Textures[17], Input.TexCoord); break;
		case 18: o_Color *= texture(u_Textures[18], Input.TexCoord); break;
		case 19: o_Color *= texture(u_Textures[19], Input.TexCoord); break;
		case 20: o_Color *= texture(u_Textures[20], Input.TexCoord); break;
		case 21: o_Color *= texture(u_Textures[21], Input.TexCoord); break;
		case 22: o_Color *= texture(u_Textures[22], Input.TexCoord); break;
		case 23: o_Color *= texture(u_Textures[23], Input.TexCoord); break;
		case 24: o_Color *= texture(u_Textures[24], Input.TexCoord); break;
		case 25: o_Color *= texture(u_Textures[25], Input.TexCoord); break;
		case 26: o_Color *= texture(u_Textures[26], Input.TexCoord); break;
		case 27: o_Color *= texture(u_Textures[27], Input.TexCoord); break;
		case 28: o_Color *= texture(u_Textures[28], Input.TexCoord); break;
		case 29: o_Color *= texture(u_Textures[29], Input.TexCoord); break;
		case 30: o_Color *= texture(u_Textures[30], Input.TexCoord); break;
		case 31: o_Color *= texture(u_Textures[31], Input.TexCoord); break;
	}

    // Set output color
  	o_Color.a *= circle;
}
