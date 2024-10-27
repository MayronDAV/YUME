// Quad Texture Shader

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

    switch (gl_VertexIndex)
    {
        case 0:
        	gl_Position = vec4(positions[0], 0.0f, 1.0f);
	        a_TexCoord = texCoords[0];
            break;
        case 1:
            gl_Position = vec4(positions[1], 0.0f, 1.0f);
	        a_TexCoord = texCoords[1];
            break;
        case 2:
        	gl_Position = vec4(positions[2], 0.0f, 1.0f);
	        a_TexCoord = texCoords[2];
            break;
        case 3:
        	gl_Position = vec4(positions[3], 0.0f, 1.0f);
	        a_TexCoord = texCoords[3];
            break;
        case 4:
        	gl_Position = vec4(positions[4], 0.0f, 1.0f);
	        a_TexCoord = texCoords[4];
            break;
        case 5:
        	gl_Position = vec4(positions[5], 0.0f, 1.0f);
	        a_TexCoord = texCoords[5];
            break;
    }
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
