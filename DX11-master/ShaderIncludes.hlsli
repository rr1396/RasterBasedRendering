#ifndef __GGP_SHADER_INCLUDES__ // Each .hlsli file needs a unique identifier!
#define __GGP_SHADER_INCLUDES__
#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2
#define MAX_SPECULAR_EXPONENT	256.0f

//fresnel value for non-metals
static const float F0_NON_METAL = 0.04f;

//Min Roughness for when spec distribution function denominator goes to zero
static const float MIN_ROUGHNESS = 0.0000001f; // 6 zeros after decimal

//PI
static const float PI = 3.14159265359f;

// ALL of your code pieces (structs, functions, etc.) go here!
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;
	float2 uv				: TEXCOORD;
	float3 normal			: NORMAL;
	float3 worldPos			: POSITION;
	float3 tangent			: TANGENT;
	float4 shadowMapPos		: SHADOW_POSITION;
};

struct Light
{
	int Type; // Which kind of light? 0, 1 or 2 (see above)
	float3 Direction; // Directional and Spot lights need a direction
	float Range; // Point and Spot lights have a max range for attenuation
	float3 Position; // Point and Spot lights have a position in space
	float Intensity; // All lights need an intensity
	float3 Color; // All lights need a color
	float SpotFalloff; // Spot lights need a value to define their “cone” size
	float3 Padding;  // Purposefully padding to hit the 16-byte boundaryectX::XMFLOAT3 Color
};

float3 GetNormalizedDirection(Light light)
{
	return normalize(-light.Direction);
}

float Specular(float3 cameraPos, float3 worldPos, float3 dirToLight, float3 normal, float roughness)
{
	float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
	float3 V = normalize(cameraPos - worldPos);
	float3 R = reflect(-dirToLight, normal);
	return pow(saturate(dot(R, V)), specExponent);
}
float Attenuate(Light light, float3 worldPos)
{
	float dist = distance(light.Position, worldPos);
	float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
	return att * att;
}

//PBR Functions: 
//lamber diffuse BRDF
float3 Diffuse(float3 normal, float3 dirToLight)
{
	return saturate(dot(normal, dirToLight));
}

//calculate diffuse amount based on energy conservation

float3 DiffuseEnergyConserve(float3 diffuse, float3 F, float metalness)
{
	return diffuse * (1 - F) * (1 - metalness);
}

//Normal Distribution Function
float D_GGX(float3 n, float3 h, float roughness)
{
	// Pre-calculations
	float NdotH = saturate(dot(n, h));
	float NdotH2 = NdotH * NdotH;
	float a = roughness * roughness;
	float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!

	// ((n dot h)^2 * (a^2 - 1) + 1)
	// Can go to zero if roughness is 0 and NdotH is 1
	float denomToSquare = NdotH2 * (a2 - 1) + 1;

	// Final value
	return a2 / (PI * denomToSquare * denomToSquare);
}


//Fresnel term - schlick approx
float3 F_Schlick(float3 v, float3 h, float3 f0)
{
	// Pre-calculations
	float VdotH = saturate(dot(v, h));

	// Final value
	return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

//Geometric Shadowing - Schlick-GGX
float G_SchlickGGX(float3 n, float3 v, float roughness)
{
	// End result of remapping:
	float k = pow(roughness + 1, 2) / 8.0f;
	float NdotV = saturate(dot(n, v));

	// Final value
	// Note: Numerator should be NdotV (or NdotL depending on parameters).
	// However, these are also in the BRDF's denominator, so they'll cancel!
	// We're leaving them out here AND in the BRDF function as the
	// dot products can get VERY small and cause rounding errors.
	return 1 / (NdotV * (1 - k) + k);
}

//Cook-Torrence Microfacet BRDF
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 f0, out float3 F_out)
{
	// Other vectors
	float3 h = normalize(v + l);

	// Run numerator functions
	float  D = D_GGX(n, h, roughness);
	float3 F = F_Schlick(v, h, f0);
	float  G = G_SchlickGGX(n, v, roughness) * G_SchlickGGX(n, l, roughness);

	// Pass F out of the function for diffuse balance
	F_out = F;

	// Final specular formula
	// Note: The denominator SHOULD contain (NdotV)(NdotL), but they'd be
	// canceled out by our G() term.  As such, they have been removed
	// from BOTH places to prevent floating point rounding errors.
	float3 specularResult = (D * F * G) / 4;

	// One last non-obvious requirement: According to the rendering equation,
	// specular must have the same NdotL applied as diffuse!  We'll apply
	// that here so that minimal changes are required elsewhere.
	return specularResult * max(dot(n, l), 0);
}
#endif