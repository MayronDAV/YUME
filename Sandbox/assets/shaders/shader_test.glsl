// Test

@type vertex
#version 450


vec2 Position[4] = {
    vec2(0.0f, 0.0f),
    vec2(1.0f, 0.0f),
    vec2(1.0f, 1.0f),
    vec2(0.0f, 1.0f),
};

void main()
{
    gl_Position = vec4(Position[gl_VertexIndex], 0.0f, 1.0f);
}


@type fragment
#version 450

layout(location = 0) out vec4 o_FragColor;

void main()
{
    o_FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}


