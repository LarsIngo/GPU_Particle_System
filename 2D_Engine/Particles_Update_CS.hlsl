#define BATCHSIZE 256U

// Particle.
struct Particle
{
    float3 position;
    float2 scale;
    float3 color;
    float3 velocity;
    float lifetime;
};

// Meta data.
struct MetaData
{
    float dt;
    uint numParticles;
    float2 pad;
};

// Input.
StructuredBuffer<Particle> g_Source : register(t0);

// Meta buffer.
StructuredBuffer<MetaData> g_MetaBuffer : register(t1);

// Output.
RWStructuredBuffer<Particle> g_Target : register(u0);

// 16x16
[numthreads(BATCHSIZE, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    MetaData metaData = g_MetaBuffer[0];
    float dt = metaData.dt;
    uint numParticles = metaData.numParticles;
    uint tID = threadID.x;

    if (tID < numParticles)
    {
        Particle self = g_Source[tID];
        self.position += self.velocity * dt * 0.5f;
        self.velocity -= self.velocity * dt;
        self.color -= self.color * dt;
        self.scale -= self.scale * dt;
        self.lifetime -= dt;

        g_Target[tID] = self;
    }
}
