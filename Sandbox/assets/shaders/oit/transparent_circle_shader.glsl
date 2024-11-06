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


#include <weighted_blended_definition.glsl>



void main()
{
    // Calculate distance and fill circle with white
    float distance = 1.0 - length(Input.LocalPosition);
    float circle = smoothstep(0.0, Input.Fade, distance);
    circle *= smoothstep(Input.Thickness + Input.Fade, Input.Thickness, distance);

	if (circle == 0.0)
		discard;

	vec4 color = Input.Color;
	
	switch(TexIndex)
	{
		case  0: color *= texture(u_Textures[ 0], Input.TexCoord); break;
		case  1: color *= texture(u_Textures[ 1], Input.TexCoord); break;
		case  2: color *= texture(u_Textures[ 2], Input.TexCoord); break;
		case  3: color *= texture(u_Textures[ 3], Input.TexCoord); break;
		case  4: color *= texture(u_Textures[ 4], Input.TexCoord); break;
		case  5: color *= texture(u_Textures[ 5], Input.TexCoord); break;
		case  6: color *= texture(u_Textures[ 6], Input.TexCoord); break;
		case  7: color *= texture(u_Textures[ 7], Input.TexCoord); break;
		case  8: color *= texture(u_Textures[ 8], Input.TexCoord); break;
		case  9: color *= texture(u_Textures[ 9], Input.TexCoord); break;
		case 10: color *= texture(u_Textures[10], Input.TexCoord); break;
		case 11: color *= texture(u_Textures[11], Input.TexCoord); break;
		case 12: color *= texture(u_Textures[12], Input.TexCoord); break;
		case 13: color *= texture(u_Textures[13], Input.TexCoord); break;
		case 14: color *= texture(u_Textures[14], Input.TexCoord); break;
		case 15: color *= texture(u_Textures[15], Input.TexCoord); break;
		case 16: color *= texture(u_Textures[16], Input.TexCoord); break;
		case 17: color *= texture(u_Textures[17], Input.TexCoord); break;
		case 18: color *= texture(u_Textures[18], Input.TexCoord); break;
		case 19: color *= texture(u_Textures[19], Input.TexCoord); break;
		case 20: color *= texture(u_Textures[20], Input.TexCoord); break;
		case 21: color *= texture(u_Textures[21], Input.TexCoord); break;
		case 22: color *= texture(u_Textures[22], Input.TexCoord); break;
		case 23: color *= texture(u_Textures[23], Input.TexCoord); break;
		case 24: color *= texture(u_Textures[24], Input.TexCoord); break;
		case 25: color *= texture(u_Textures[25], Input.TexCoord); break;
		case 26: color *= texture(u_Textures[26], Input.TexCoord); break;
		case 27: color *= texture(u_Textures[27], Input.TexCoord); break;
		case 28: color *= texture(u_Textures[28], Input.TexCoord); break;
		case 29: color *= texture(u_Textures[29], Input.TexCoord); break;
		case 30: color *= texture(u_Textures[30], Input.TexCoord); break;
		case 31: color *= texture(u_Textures[31], Input.TexCoord); break;
	}

    // Set output color
  	color.a *= circle;

    WritePixel(color, mix(vec3(1.0), color.rgb, 1.0 - color.a), 0.0f);
}
