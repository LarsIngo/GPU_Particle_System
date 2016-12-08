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
    uint step;
    uint numParticles;
    uint len;
    float pad;
};
// Meta buffer.
StructuredBuffer<MetaData> g_MetaBuffer : register(t1);

// Output.
RWStructuredBuffer<Particle> g_Target : register(u0);

// Compare self < other
bool Compare(Particle self, Particle other);

// 16x16
[numthreads(256, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    uint tID = threadID.x;
    MetaData metaData = g_MetaBuffer[0];
    uint numParticles = metaData.numParticles;
    uint step = metaData.step;

    if (tID < numParticles / (2 * step))
    {
        uint offset = 2 * step * tID;
        uint selfID = offset;
        uint otherID = selfID + step;

        if (tID % 2 == 1)
        {
            // Swap ID.
            uint tmp = selfID;
            selfID = otherID;
            otherID = tmp;
        }

        Particle self = g_Source[selfID];
        Particle other = g_Source[otherID];

        // Compate other < self.
        if (Compare(other, self))
        {
            // Swap ID.
            uint tmp = selfID;
            selfID = otherID;
            otherID = tmp;

            // TMP
            self.color.z = 1.f;
            other.color.z = 1.f;
        }

        g_Target[selfID] = self;
        g_Target[otherID] = other;
    }
}

bool Compare(Particle lhs, Particle rhs)
{
    return lhs.color.x < rhs.color.x;
}
