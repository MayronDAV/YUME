@type vertex
#version 450 core

layout(location = 0) out vec2 a_TexCoord;

void main()
{
    vec2 positions[6] = vec2[6](
        vec2(-1.0f, -1.0f),
        vec2( 1.0f, -1.0f),
        vec2(-1.0f,  1.0f),

        vec2( 1.0f, -1.0f),
        vec2( 1.0f,  1.0f),
        vec2(-1.0f,  1.0f)
    );

    vec2 texCoords[6] = vec2[6](
        vec2(0.0f, 0.0f),
        vec2(1.0f, 0.0f),
        vec2(0.0f, 1.0f),

        vec2(1.0f, 0.0f),
        vec2(1.0f, 1.0f),
        vec2(0.0f, 1.0f) 
    );

    gl_Position = vec4(positions[gl_VertexIndex], 0.0f, 1.0f);
	a_TexCoord = texCoords[gl_VertexIndex];
}


@type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 a_TexCoord;


layout (set = 0, binding = 0) uniform sampler2D u_Texture;


void main()
{
	o_Color = texture(u_Texture, a_TexCoord);
}
