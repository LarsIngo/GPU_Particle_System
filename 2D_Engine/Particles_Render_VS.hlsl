// Input.
struct VSInput
{
    float3 position : POSITION;
    float2 scale : SCALE;
    float3 color : COLOR;
    float3 velocity : VELOCITY;
    float lifetime : LIFETIME;
};

// Output.
struct VSOutput
{
    float4 position : SV_POSITION;
    float2 scale : SCALE;
    float3 color : COLOR;
    float lifetime : LIFETIME;
};
 
VSOutput main(VSInput input)
{
    VSOutput output;

    output.position = float4(input.position, 1.f);
    output.scale = input.scale;
    output.color = input.color;
    output.lifetime = input.lifetime;

    return output;
}
