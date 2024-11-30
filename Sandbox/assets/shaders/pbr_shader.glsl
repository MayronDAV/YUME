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
	vec3  WorldPos;
	vec3  Normal;
	vec2  TexCoord;
	vec4  Color;
	vec3  CameraPosition;
	vec4  WorldPosLightSpace;
};
layout(location = 0) out VertexOutput Output;


layout(set = 0, binding = 0) uniform u_Camera
{
	mat4 ViewProjection;
	vec3 Position;
} u_camera;

layout(set = 0, binding = 1) uniform u_ShadowBuffer
{
	mat4 LightSpaceMatrix;
} u_shadowBuffer;


layout(push_constant) uniform model
{
	mat4 Transform;
} Model;



void main()
{
	Output.TexCoord = a_TexCoord;
	Output.Color = a_Color;
	Output.WorldPos = vec3(Model.Transform * vec4(a_Position, 1.0));
	Output.Normal = a_Normal;
	Output.CameraPosition = u_camera.Position;
	Output.WorldPosLightSpace = (u_shadowBuffer.LightSpaceMatrix * Model.Transform) * vec4(a_Position, 1.0);

	gl_Position = u_camera.ViewProjection * vec4(Output.WorldPos, 1.0);
}


@type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

struct VertexOutput
{
	vec3  WorldPos;
	vec3  Normal;
	vec2  TexCoord;
	vec4  Color;
	vec3  CameraPosition;
	vec4  WorldPosLightSpace;
};
layout(location = 0) in VertexOutput Input;

layout(push_constant) uniform model
{
	mat4 Transform;
} Model;

#define LIGHT_TYPE_POINT 0.0
#define LIGHT_TYPE_DIRECTIONAL 1.0

#define MAX_LIGHTS 16
struct Light 
{
	vec4  Position;
	vec4  Color; // w is Intensity
	vec4  Direction;
	vec3  AttenuationProps; // x: const, y: linear, z: quadratic
	float Type;
};
layout(set = 0, binding = 2) uniform LightBuffer
{
	Light Lights[MAX_LIGHTS];
	int NumLights;
};

layout(set = 0, binding = 3) uniform sampler2D u_ShadowMap;


layout(set = 1, binding = 0) uniform sampler2D u_AlbedoTexture;
layout(set = 1, binding = 1) uniform sampler2D u_NormalTexture;
layout(set = 1, binding = 2) uniform sampler2D u_SpecularTexture;
layout(set = 1, binding = 3) uniform sampler2D u_RoughnessTexture;
layout(set = 1, binding = 4) uniform sampler2D u_MetallicTexture;
layout(set = 1, binding = 5) uniform sampler2D u_AoTexture;
layout(set = 1, binding = 6) uniform u_MaterialProperties
{
	vec4   AlbedoColor;
	int    SpecularMap;
	int    NormalMap;
	float  AlphaCutOff;
} u_Material;



#define PI 3.14159265359
#define EPSILON 0.0001

//PBR Calculations
float DistributionGGX(vec3 p_N, vec3 p_H, float p_Roughness);
float GeometrySchlickGGX(float p_NdotV, float p_Roughness);
float GeometrySmith(vec3 p_N, vec3 p_V, vec3 p_L, float p_Roughness);
vec3  FresnelSchlick(float p_CosTheta, vec3 p_F0);
vec3  GetNormalFromMap();
vec3  DeGamma(vec3 p_Color, float p_Gamma);
float ShadowCalculation(vec4 p_FragPos, vec3 p_Normal, vec3 p_Direction);




void main()
{
	vec4 albedoTex 	= texture(u_AlbedoTexture, Input.TexCoord);
	if (u_Material.AlbedoColor.a < u_Material.AlphaCutOff ||
		albedoTex.a < u_Material.AlphaCutOff)
	{
		discard;
	}

	vec3  albedo    = u_Material.AlbedoColor.rgb * pow(albedoTex.rgb, vec3(2.2));
	float metallic  = texture(u_MetallicTexture, Input.TexCoord).r;
	float roughness = texture(u_RoughnessTexture, Input.TexCoord).r;
	float ao        = texture(u_AoTexture, Input.TexCoord).r;

	vec3 N 			= Input.Normal;
	if (u_Material.NormalMap > 0)
	{
		N 			= GetNormalFromMap();
	}
	vec3 V          = normalize(Input.CameraPosition - Input.WorldPos);

	vec3 F0         = vec3(0.04);
	F0              = mix(F0, albedo, metallic);

	vec3 specular;
	if (u_Material.SpecularMap > 0)
	{
		specular 	= texture(u_SpecularTexture, Input.TexCoord).rgb;
	}

	// Reflectance
	vec3 Lo = vec3(0.0);
	vec3 light_direction = vec3(0.0);
	for (int i = 0; i < NumLights; ++i)
	{
		Light curLight = Lights[i];

		// calculate per-light radiance
		vec3 L;
		vec3 radiance;
		float attenuation = 1.0;
		if (curLight.Type == LIGHT_TYPE_POINT)
		{
			// Point light calculations
	
			L 				   = normalize(vec3(curLight.Position) - Input.WorldPos);
			float distance     = length(vec3(curLight.Position) - Input.WorldPos);

			float constant     = curLight.AttenuationProps.x;
			float linear 	   = curLight.AttenuationProps.y;
			float quadratic    = curLight.AttenuationProps.z;
			attenuation  	   = 1.0 / (constant + (linear * distance) + (quadratic * (distance * distance)));

			radiance 		   = vec3(curLight.Color) * attenuation * curLight.Color.w;
		}
		else if (curLight.Type == LIGHT_TYPE_DIRECTIONAL)
		{
			// Directional light calculations

			L 		  		    = normalize(-vec3(curLight.Direction));
			light_direction 	= normalize(vec3(curLight.Direction));
			radiance  		    = vec3(curLight.Color);
		}
		
		vec3  H   		      = normalize(V + L);
		vec3  F   		      = FresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3  kS   		      = F;
		vec3  kD   		      = (vec3(1.0) - kS) * (1.0 - metallic);

		if (u_Material.SpecularMap <= 0)
		{
			float NDF 		  = DistributionGGX(N, H, roughness);
			float G   		  = GeometrySmith(N, V, L, roughness);

			vec3  numerator   = NDF * G * F;
			float denominator = (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0)) + EPSILON;
			specular     	  = numerator / denominator;
		}

		specular 			 *= attenuation;
            
        // Add to outgoing radiance Lo
        float NdotL 	      = max(dot(N, L), 0.0);         
        Lo 				     += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	// Shadow
	float shadow = ShadowCalculation(Input.WorldPosLightSpace, N, light_direction);

	// Ambient lighting
    vec3 ambient = vec3(0.03) * albedo * ao;
	//vec3 color = ambient + Lo;
	vec3 color 	 = ambient + (1.0 - shadow) * Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
	// gamma correct
    color = DeGamma(color, 2.2);

	o_Color = vec4(color, 1.0);
}



float DistributionGGX(vec3 p_N, vec3 p_H, float p_Roughness)
{
	float a      = p_Roughness * p_Roughness;
	float a2     = a * a;
	float NdotH  = max(dot(p_N, p_H), 0.0);
	float NdotH2 = NdotH * NdotH;
	
	float num    = a2;
	float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
	denom        = PI * denom * denom;
	
	return num / denom;
}

float GeometrySchlickGGX(float p_NdotV, float p_Roughness)
{
	float r     = (p_Roughness + 1.0);
	float k     = (r*r) / 8.0;

	float num   = p_NdotV;
	float denom = p_NdotV * (1.0 - k) + k;
	
	return num / denom;
}

float GeometrySmith(vec3 p_N, vec3 p_V, vec3 p_L, float p_Roughness)
{
	float NdotV = max(dot(p_N, p_V), 0.0);
	float NdotL = max(dot(p_N, p_L), 0.0);
	float ggx2  = GeometrySchlickGGX(NdotV, p_Roughness);
	float ggx1  = GeometrySchlickGGX(NdotL, p_Roughness);
	
	return ggx1 * ggx2;
}

vec3 FresnelSchlick(float p_CosTheta, vec3 p_F0)
{
	return p_F0 + (1.0 - p_F0) * pow(clamp(1.0 - p_CosTheta, 0.0, 1.0), 5.0);
}  

vec3 GetNormalFromMap()
{
	vec3 tangentNormal = texture(u_NormalTexture, Input.TexCoord).xyz * 2.0 - 1.0;

	vec3 Q1  = dFdx(Input.WorldPos);
	vec3 Q2  = dFdy(Input.WorldPos);
	vec2 st1 = dFdx(Input.TexCoord);
	vec2 st2 = dFdy(Input.TexCoord);

	vec3 N   = normalize(Input.Normal);
	vec3 T   = normalize(Q1 * st2.t - Q2 * st1.t);
	vec3 B   = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}

vec3 DeGamma(vec3 p_Color, float p_Gamma)
{
	return pow(p_Color, vec3(1.0 / p_Gamma));
}

float ShadowCalculation(vec4 p_FragPos, vec3 p_Normal, vec3 p_Direction)
{
    vec3 projCoords    = p_FragPos.xyz / p_FragPos.w;
	projCoords		   = projCoords * 0.5 + 0.5; 

	// Outside
	if (projCoords.z > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;

    float shadow 	   = 0.0;
    vec2 texelSize 	   = 1.0 / textureSize(u_ShadowMap, 0);
	float cosTheta 	   = dot(p_Normal, p_Direction);
	float bias 		   = 0.005 * tan(acos(cosTheta));
	bias 			   = clamp(bias, 0, 0.01);

	int count 		   = 0;
	int range 		   = 1;
    for(int x = -range; x <= range; ++x)
    {
        for(int y = -range; y <= range; ++y)
        {
            float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
	
            shadow 		  += currentDepth - bias > pcfDepth  ? 0.9 : 0.0;
			count++;
        }
    }
    shadow 			/= count;

	return shadow;
}
