layout(location = 0) out vec4 o_Accum;
layout(location = 1) out float o_Reveal;



void WritePixel(vec4 color, vec3 transmit, float csZ)
{
    //color.a *= 1.0 - clamp((transmit.r + transmit.g + transmit.b) * (1.0 / 3.0), 0, 1);
    //float a = min(1.0, color.a) * 8.0 + 0.01;
    //float b = -gl_FragCoord.z * 0.95 + 1.0;
    //b      /= sqrt(1e4 * abs(csZ));
    //float w   = clamp(a * a * a * 1e8 * b * b * b, 1e-2, 3e2);
    //o_Accum   = color * w;
    //o_Reveal  = color.a;

    float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);
	o_Accum = vec4(color.rgb * color.a, color.a) * weight;
	o_Reveal = color.a;
}