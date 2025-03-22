//#pragma pack_matrix( row_major )

cbuffer cBuffer : register(b0)
{
	float xOffset;
	float yOffset;
	float zOffset;
	float4x4 wvpMatrix;
	float4x4 worldMatrix;
};

struct VS_INPUT
{
	float3 inPos : POSITION;
	float3 inColor : COLOR;
	float3 inNormal : NORMAL;
	float3 instancePos : INSTANCEDATA;
	uint instanceID : SV_InstanceID;
};

struct VS_OUTPUT
{
	float4 outPosition : SV_POSITION;
	float3 outColor : COLOR;
	float3 outNormal : NORMAL;
	float3 outWorldPos : WORLD_POSITION;
};

VS_OUTPUT main(VS_INPUT input) 
{
	VS_OUTPUT output;
	input.inPos.x += xOffset;
	input.inPos.y += yOffset;
	input.inPos.z += zOffset;
	//input.inPos += input.instanceID * input.instancePos;
	input.inPos.x += input.instancePos.x;
	
	//output.outPosition = float4(input.inPos, 1.0f);
	output.outPosition = mul(float4(input.inPos, 1.0f), wvpMatrix);
	output.outColor = input.inColor;
	//output.outNormal = input.inNormal;
	output.outWorldPos = mul(float4(input.inPos, 1.0f), worldMatrix);
	output.outNormal = normalize(mul(float4(input.inNormal, 1.0f), worldMatrix));
	return output;
}
