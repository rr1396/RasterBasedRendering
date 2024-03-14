#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float roughness;
	float3 cameraPos;
	float3 ambient;
	Light directionalLight1;
}

Texture2D Albedo			: register(t0);
Texture2D NormalMap			: register(t1);
Texture2D RoughnessMap	 	: register(t2);
Texture2D MetalnessMap		: register(t3);
Texture2D ShadowMap			: register(t4);
SamplerState BasicSampler	: register(s0);
SamplerComparisonState ShadowSampler : register(s1);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Perform the perspective divide (divide by W) ourselves
	input.shadowMapPos /= input.shadowMapPos.w;

	// Convert the normalized device coordinates to UVs for sampling
	float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
	shadowUV.y = 1 - shadowUV.y; // Flip the Y

	// Grab the distances we need: light-to-pixel and closest-surface
	float distToLight = input.shadowMapPos.z;
	
	// Get a ratio of comparison results using SampleCmpLevelZero()
	float shadowAmount = ShadowMap.SampleCmpLevelZero(
		ShadowSampler,
		shadowUV,
		distToLight).r;



	float3 surfaceColor = pow(Albedo.Sample(BasicSampler, input.uv).rgb, 2.2f);

	float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
	unpackedNormal = normalize(unpackedNormal);

	float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
	float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;

	float3 specularColor = lerp(F0_NON_METAL, surfaceColor.rgb, metalness);

	float3 N = normalize(input.normal);
	float3 T = normalize(input.tangent);
	T = normalize(T - N * dot(T, N));;
	float3 B = cross(T,N);
	float3x3 TBN = float3x3(T, B, N);

	input.normal = mul(unpackedNormal, TBN);

	float3 total = surfaceColor * ambient;

	float3 dirToLight1 = normalize(-directionalLight1.Direction);

	float3 diffuse = Diffuse(input.normal, dirToLight1);
	float3 F;
	float3 specular = MicrofacetBRDF(input.normal, dirToLight1, normalize(cameraPos), roughness, specularColor, F);

	float3 balanceDiff = DiffuseEnergyConserve(diffuse, F, metalness);

	float3 lightResult = (balanceDiff * surfaceColor + specular) * directionalLight1.Intensity * directionalLight1.Color;

	lightResult *= shadowAmount;

	total += lightResult;

	return float4(pow(total, 1.0f/2.2f), 1);
}