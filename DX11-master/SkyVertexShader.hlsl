cbuffer ExternalData : register(b0)
{
	matrix view;
	matrix projection;
}

struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 localPosition	: POSITION;     // XYZ position
	float3 normal           : NORMAL;       // Normal
	float2 uv				: TEXCOORD;		// UV
	float3 tangent			: TANGENT;
};

struct VertexToPixel_Sky
{
	float4 screenPosition	: SV_POSITION;
	float3 sampleDir		: DIRECTION;
};

VertexToPixel_Sky main(VertexShaderInput input)
{
	VertexToPixel_Sky output;
	matrix viewNoTranslation = view;
	viewNoTranslation._14 = 0;
	viewNoTranslation._24 = 0;
	viewNoTranslation._34 = 0;

	matrix viewProj = mul(projection, viewNoTranslation);
	output.screenPosition = mul(viewProj, float4(input.localPosition, 1.0f));
	output.screenPosition.z = output.screenPosition.w;

	output.sampleDir = input.localPosition;

	return output;
}