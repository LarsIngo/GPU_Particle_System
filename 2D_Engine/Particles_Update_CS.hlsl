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

// Meta data.
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
[numthreads(256, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    MetaData metaData = g_MetaBuffer[0];
    float dt = metaData.dt;
    uint numParticles = metaData.numParticles;

    uint tID = threadID.x;
    Particle self = g_Source[tID];

    //for (uint i = 0; i < 100; ++i) 
    //{
    //    self.position += normalize(g_Source[threadID.x + i].position + float3(0.01, 0.01, 0.01)) * 0.001f + normalize(self.velocity) * 0.0001f;
    //}

    //self.color = float3(tID / 32.f, 0.5f, 0.f);

    g_Target[tID] = self;
}

// ! Ths < otr ! //
bool Instersect(Particle self, Particle other)
{
    float selfLen = self.scale.x * 2.f;
    float otherLen = other.scale.x * 2.f;
    float selfLeft = self.position.x - self.scale.x;
    float otherLeft = other.position.x - other.scale.x;
    //if (thsLeft)
    //{

    //}
    return true;
}
