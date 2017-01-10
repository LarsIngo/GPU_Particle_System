// Input.
struct GSInput
{
    float4 position : SV_POSITION;
    float2 scale : SCALE;
    float3 color : COLOR;
    float lifetime : LIFETIME;
};

// Output.
struct GSOutput
{
    float4 position : SV_POSITION;
    float2 uv : UV;
    float3 color : COLOR;
    float3 worldPos : WORLDPOS;
};

// Meta data.
struct MetaData
{
    float4x4 vpMatrix;
    float3 lensPosition;
    float3 lensUpDirection;
    float2 pad;
};
// Meta buffer.
StructuredBuffer<MetaData> g_MetaBuffer : register(t0);

[maxvertexcount(4)]
void main(point GSInput input[1], inout TriangleStream<GSOutput> TriStream)
{
    if (input[0].lifetime >= 0.f)
    {
        MetaData metaData = g_MetaBuffer[0];
        float4x4 vpMatrix = metaData.vpMatrix;
        float3 lensPosition = metaData.lensPosition;
        float3 lensUpDirection = metaData.lensUpDirection;

        GSOutput output;

        //// ----- Generate billboarded particle ----- //
        float3 worldPosition = input[0].position.xyz;
        float2 scale = input[0].scale;
        float3 color = input[0].color;

        float3 particleFrontDirection = normalize(lensPosition - worldPosition);
        float3 paticleSideDirection = cross(particleFrontDirection, lensUpDirection);
        float3 paticleUpDirection = cross(paticleSideDirection, particleFrontDirection);

        for (uint i = 0; i < 4; ++i)
        {
            float x = i == 1 || i == 3;
            float y = i == 0 || i == 1;
            output.position.xyz = worldPosition + paticleSideDirection * (x * 2.f - 1.f) * scale.x + paticleUpDirection * (y * 2.f - 1.f) * scale.y;
            output.position.w = input[0].position.w;
            output.uv.x = x;
            output.uv.y = 1.f - y;
            output.worldPos = output.position.xyz;
            output.position = mul(output.position, vpMatrix);
            output.color = color;
    
            TriStream.Append(output);
        }
    }
}
