@type vertex
#version 450 core

layout(location = 0) out vec3 o_TexCoord;

layout(set = 0, binding = 0) uniform u_Buffer
{
    mat4 Projection;
    mat4 View;
} u_buffer;


int Min(int a, int b)
{
    if (a < b)
        return a;

    return b;
}

void main()
{
    vec3 skyboxVertices[36] = {
        vec3(-1.0f,  1.0f, -1.0f),
        vec3(-1.0f, -1.0f, -1.0f),
        vec3( 1.0f, -1.0f, -1.0f),
        vec3( 1.0f, -1.0f, -1.0f),
        vec3( 1.0f,  1.0f, -1.0f),
        vec3(-1.0f,  1.0f, -1.0f),

        vec3(-1.0f, -1.0f,  1.0f),
        vec3(-1.0f, -1.0f, -1.0f),
        vec3(-1.0f,  1.0f, -1.0f),
        vec3(-1.0f,  1.0f, -1.0f),
        vec3(-1.0f,  1.0f,  1.0f),
        vec3(-1.0f, -1.0f,  1.0f),

        vec3( 1.0f, -1.0f, -1.0f),
        vec3( 1.0f, -1.0f,  1.0f),
        vec3( 1.0f,  1.0f,  1.0f),
        vec3( 1.0f,  1.0f,  1.0f),
        vec3( 1.0f,  1.0f, -1.0f),
        vec3( 1.0f, -1.0f, -1.0f),

        vec3(-1.0f, -1.0f,  1.0f),
        vec3(-1.0f,  1.0f,  1.0f),
        vec3( 1.0f,  1.0f,  1.0f),
        vec3( 1.0f,  1.0f,  1.0f),
        vec3( 1.0f, -1.0f,  1.0f),
        vec3(-1.0f, -1.0f,  1.0f),

        vec3(-1.0f,  1.0f, -1.0f),
        vec3( 1.0f,  1.0f, -1.0f),
        vec3( 1.0f,  1.0f,  1.0f),
        vec3( 1.0f,  1.0f,  1.0f),
        vec3(-1.0f,  1.0f,  1.0f),
        vec3(-1.0f,  1.0f, -1.0f),

        vec3(-1.0f, -1.0f, -1.0f),
        vec3(-1.0f, -1.0f,  1.0f),
        vec3( 1.0f, -1.0f, -1.0f),
        vec3( 1.0f, -1.0f, -1.0f),
        vec3(-1.0f, -1.0f,  1.0f),
        vec3( 1.0f, -1.0f,  1.0f)
    };


    int index   = Min(int(gl_VertexIndex), 35);
    gl_Position = u_buffer.Projection * u_buffer.View * vec4(skyboxVertices[index], 1.0f);
	o_TexCoord  = skyboxVertices[index];
}


@type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec3 in_TexCoord;


layout(set = 0, binding = 1) uniform samplerCube u_Skybox;



void main()
{
	o_Color = texture(u_Skybox, in_TexCoord);
}
