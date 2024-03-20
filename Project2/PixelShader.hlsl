
cbuffer lightBuffer: register(b0)
{
	float3 ambientColor;
	float ambientStrength;
}

struct PS_INPUT
{
	float4 inPosition : SV_POSITION;
	float3 inColor : COLOR;
	nointerpolation float3 inNormal : NORMAL;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float3 pixelColor = input.inNormal;
	//float3 pixelColor = input.inColor;
	float3 ambientLight = ambientColor * ambientStrength;
	float3 outColor = pixelColor * ambientStrength;
	float3 white = float3(1.0f, 1.0f, 1.0f);
	//return float4(input.inColor, 1.0f);
	return float4(outColor, 1.0f);

}