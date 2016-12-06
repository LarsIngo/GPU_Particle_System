// Sampler State.
SamplerState sampClamp : register(s0);

// Input.
struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : UV;
    float3 color : COLOR;
    float3 worldPos : WORLDPOS;
};

// Output.
struct PSOutput
{
    float4 diffuse : SV_TARGET0;
};
 
PSOutput main(PSInput input) : SV_TARGET
{
    PSOutput output;

    float x = input.uv.x - 0.5f;
    float y = input.uv.y - 0.5f;
    float r = sqrt(x * x + y * y);
    float factor = max(1.f - r * 2.f, 0.f); //[1,0]
    float sinFactor = 1.f - sin(3.14159265f / 2.f * (factor + 1.f));

    //float4 atlasValue = txAtlas.Sample(sampClamp, input.uv);
    //float3 depthVector = lensPosition - input.worldPos;
    //float depth = length(depthVector);
    ////float linDepth = sqrt(depthVector.x * depthVector.x + depthVector.y * depthVector.y + depthVector.z * depthVector.z) / (farZ - nearZ);
    ////float expDepth = -((2.0f * nearZ) / linDepth - farZ - nearZ) / (farZ - nearZ);
    output.diffuse = float4(input.color, sinFactor);
    //output.refraction = float4(sinFactor, 0.f, 0.f, 1.f);
    //output.worldPos = float4(input.worldPos, depth);
    return output;
}
 