
cbuffer lightBuffer: register(b0)
{
	float3 ambientColor;
	float ambientStrength;
	float3 lightColor;
	float lightStrength;
	float3 lightPosition;
}

struct PS_INPUT
{
	float4 inPosition : SV_POSITION;
	float3 inColor : COLOR;
	nointerpolation float3 inNormal : NORMAL;
	float3 inWorldPos : WORLD_POSITION;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float3 ambientLight = ambientColor * ambientStrength;
	float3 lightDirVec = normalize(lightPosition - input.inWorldPos);
	
	//float3 pixelColor = input.inNormal;
	float3 pixelColor = input.inColor;
	
	float3 finalLight = ambientLight;
	float3 diffuseIntensity = max(dot(lightDirVec, input.inNormal), 0);
	float3 diffuseLight = diffuseIntensity * lightStrength * lightColor;
	finalLight += diffuseLight;
	float3 outColor = pixelColor * finalLight;
	float3 white = float3(1.0f, 1.0f, 1.0f);
	//return float4(input.inColor, 1.0f);
	return float4(input.inNormal, 1.0f);

}