struct VertexToPixel_Sky
{
	float4 screenPosition	: SV_POSITION;
	float3 sampleDir		: DIRECTION;
};

TextureCube SkyTexture		: register(t0);
SamplerState SkySampler		: register(s0);

struct PS_Output
{
	float4 colorNoAmbient	: SV_TARGET0;
	float4 ambient			: SV_TARGET1;
	float4 normals			: SV_TARGET2;
	float depths : SV_TARGET3;
};

PS_Output main(VertexToPixel_Sky input)
{

	PS_Output output;
	output.colorNoAmbient = SkyTexture.Sample(SkySampler, input.sampleDir);
	output.ambient = float4(0,0,0, 1);
	output.normals = float4(0,0,0, 1);
	output.depths = input.screenPosition.z;

	return output;
}