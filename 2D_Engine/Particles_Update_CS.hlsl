// Particle.
struct Particle
{
    float3 position;
    float2 scale;
    float3 color;
    float3 velocity;
    float lifeTime;
};

// Input.
StructuredBuffer<Particle> g_Source : register(t0);

// MetaData.
struct MetaData 
{
    float dt;
    uint numParticles;
    float3 worldPos;
    bool active;
    float2 pad;
};
// Meta buffer.
StructuredBuffer<MetaData> g_MetaBuffer : register(t1);

// Output.
RWStructuredBuffer<Particle> g_Target : register(u0);

// 16x16
[numthreads(128, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    Particle particle = g_Source[threadID.x];

    MetaData metaData = g_MetaBuffer[0];
    float dt = metaData.dt;

    for (uint i = 1; i < 1; ++i) {
        particle.position += normalize(g_Source[threadID.x + i].position + float3(0.01, 0.01, 0.01)) * 0.001f;
    }


    g_Target[threadID.x] = particle;
}

// ! Ths < otr ! //
bool Instersect(Particle ths, Particle otr)
{
    float thsLen = ths.scale.x * 2.f;
    float otrLen = otr.scale.x * 2.f;
    float thsLeft = ths.position.x - ths.scale.x;
    float otrLeft = otr.position.x - otr.scale.x;
    //if (thsLeft)
    //{

    //}
    return true;
}
