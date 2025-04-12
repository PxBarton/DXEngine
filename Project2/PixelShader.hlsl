
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
	//float3 inTex : TEXCOORD;
	//nointerpolation float3 inNormal : NORMAL;
	float3 inNormal : NORMAL;
	float3 inWorldPos : WORLD_POSITION;
};

float GridPattern(float2 position, float lineWidth, float cellSize)
{
	float2 gridPos = position / cellSize;
	float2 gridFrac = frac(gridPos);
	float2 gridDist = min(gridFrac, 1.0 - gridFrac);
	float minDist = min(gridDist.x, gridDist.y);
	return smoothstep(0.0, lineWidth, minDist);
}

//Texture2D meshTex : TEXTURE: register(t0);
//SamplerState meshSamplerState: SAMPLER: register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
	//float3 white = float3(1.0f, 1.0f, 1.0f);
	float3 ambientLight = ambientColor * ambientStrength;
	float3 pixelColor = input.inColor;
	float3 finalLight = ambientLight;

	float3 lightDirVec = normalize(lightPosition - input.inWorldPos);
	float3 diffuseIntensity = max(dot(lightDirVec, input.inNormal), 0);
	diffuseIntensity -= max(((1 - diffuseIntensity) * .3), 0);
	float3 diffuseLight = diffuseIntensity * lightStrength * lightColor;
	finalLight += diffuseLight;

	float3 outColor = pixelColor * finalLight;
	float3 baseColor = pixelColor * finalLight;

	// Grid parameters
	float gridWidth = 10.0;
	float gridLength = 10.0;
	float gridCellSize = 1.0;
	float gridLineWidth = 0.02;    // Width of grid lines
	float gridThickness = 0.02; // How thick the grid appears in the Y direction

	/*
	float planeDistance = abs(input.inWorldPos.y);
	// Only process grid if we're near the plane
	if (planeDistance < gridThickness)
	{
		// Check if we're within the grid bounds
		bool withinBounds =
			abs(input.inWorldPos.x) <= gridWidth * 0.5 &&
			abs(input.inWorldPos.z) <= gridLength * 0.5;

		if (withinBounds)
		{
			// Calculate grid pattern
			float gridValue = GridPattern(input.inWorldPos.xz, gridLineWidth, gridCellSize);

			// If we're on a grid line, show white
			if (gridValue < 0.5)
			{
				float fadeIntensity = 1.0 - (planeDistance / gridThickness);
				//return float4(lerp(baseColor, float3(1, 1, 1), fadeIntensity), 1.0);
				return float4(
					input.inWorldPos.y, // R channel shows height
					GridPattern(input.inWorldPos.xz, gridLineWidth, gridCellSize), // G channel shows grid pattern
					withinBounds ? 1.0 : 0.0, // B channel shows grid bounds
					1.0
					);
			}
		}
	}
	*/

	return float4(baseColor, 1.0f);

}