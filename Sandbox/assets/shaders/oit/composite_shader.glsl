// Quad Texture Shader

@type vertex
#version 450 core

layout(location = 0) out vec2 TexCoord;

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

    switch(gl_VertexIndex)
    {
        case 0: gl_Position = vec4(positions[0], 0.0f, 1.0f); TexCoord = texCoords[0]; break;
        case 1: gl_Position = vec4(positions[1], 0.0f, 1.0f); TexCoord = texCoords[1]; break;
        case 2: gl_Position = vec4(positions[2], 0.0f, 1.0f); TexCoord = texCoords[2]; break;
        case 3: gl_Position = vec4(positions[3], 0.0f, 1.0f); TexCoord = texCoords[3]; break;
        case 4: gl_Position = vec4(positions[4], 0.0f, 1.0f); TexCoord = texCoords[4]; break;
        case 5: gl_Position = vec4(positions[5], 0.0f, 1.0f); TexCoord = texCoords[5]; break;
    }
}


@type fragment
#version 450 core

layout (location = 0) out vec4 o_Color;

layout(location = 0) in vec2 TexCoord;

layout (set = 0, binding = 0) uniform sampler2D u_AccumTexture;
layout (set = 0, binding = 1) uniform sampler2D u_RevealTexture;
layout (set = 0, binding = 2) uniform sampler2D u_OpaqueTexture;



const float EPSILON = 0.00001f;

bool isApproximatelyEqual(float a, float b)
{
	return abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * EPSILON;
}

float max3(vec3 v) 
{
	return max(max(v.x, v.y), v.z);
}

void main()
{
    ivec2 coords = ivec2(gl_FragCoord.xy);

    float revealage = texelFetch(u_RevealTexture, coords, 0).r;
    
    vec4 opaqueColor = texelFetch(u_OpaqueTexture, coords, 0);

    if (isApproximatelyEqual(revealage, 1.0f))
    {
        o_Color = opaqueColor;
    }
    else
    {
        vec4 accumulation = texelFetch(u_AccumTexture, coords, 0);
        
        if (isinf(max3(abs(accumulation.rgb)))) 
            accumulation.rgb = vec3(accumulation.a);

        vec3 average_color = accumulation.rgb / max(accumulation.a, EPSILON);

        float transparentAlpha = 1.0f - revealage;
        vec3 finalColor = mix(opaqueColor.rgb, average_color, smoothstep(0.0f, 1.0f, transparentAlpha));

        o_Color = vec4(finalColor, 1.0);
    }
}
