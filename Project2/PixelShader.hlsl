
struct PS_INPUT
{
	float4 inPosition : SV_POSITION;
	float3 inColor : COLOR;
	float3 inNormal : NORMAL;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float3 pixelColor = input.inNormal;
	//return float4(input.inColor, 1.0f);
	return float4(pixelColor, 1.0f);

}