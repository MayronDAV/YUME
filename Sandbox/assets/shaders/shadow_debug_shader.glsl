@type vertex
#version 450 core

layout(location = 0) out vec2 o_TexCoord;


int Min(int a, int b)
{
    if (a < b)
        return a;

    return b;
}

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

    gl_Position       = vec4(positions[Min(int(gl_VertexIndex), 5)], 0.0f, 1.0f);
    o_TexCoord        = texCoords[Min(int(gl_VertexIndex), 5)];
}


@type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 inTexCoord;

layout (set = 0, binding = 0) uniform u_Buffer
{
    int   Mode; // 0: perspective, 1: ortho
    float zNear;
    float zFar;
};

layout (set = 0, binding = 1) uniform sampler2D u_ShadowMap;


float LinearizeDepth(float p_Depth)
{
    float z = p_Depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

void main()
{
    float depthValue  = texture(u_ShadowMap, inTexCoord).r;

    if (Mode == 0)
    {
        o_Color       = vec4(vec3(LinearizeDepth(depthValue) / zFar), 1.0);
    }
    else if (Mode == 1)
    {
        o_Color       = vec4(vec3(depthValue), 1.0);
    }
    else
    {
        o_Color       = vec4(0.5, 0.3, 0.8, 1.0);
    }
}
