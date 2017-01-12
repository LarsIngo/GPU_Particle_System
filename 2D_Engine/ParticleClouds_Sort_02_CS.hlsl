// Particle cloud.
struct ParticleCloud
{
    float3 position;
    float radius;
    float3 color;
    float3 velocity;
    float particleStartID;
    uint numParticles;
    uint firstID;
    uint lastID;
    float timer;
    float spawntime;
};

// Input.
StructuredBuffer<ParticleCloud> g_Source : register(t0);

// Meta data.
struct MetaData 
{
    uint step;
    uint numClouds;
    uint numThreads;
    float pad;
};
// Meta buffer.
StructuredBuffer<MetaData> g_MetaBuffer : register(t1);

// Output.
RWStructuredBuffer<ParticleCloud> g_Target : register(u0);

// 16x16
[numthreads(256, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    uint tID = threadID.x;
    MetaData metaData = g_MetaBuffer[0];
    uint stepLen = metaData.step;
    uint numClouds = metaData.numClouds;
    uint numThreads = metaData.numThreads;

    if (tID < numThreads)
    {
        bool setRightSide = tID >= (numThreads / 2);
        uint tOffset = (tID % stepLen) + (tID / stepLen) * (2 * stepLen);
        uint selfID = tOffset;
        uint otherID = selfID + stepLen;

        if (setRightSide)
        {
            // Swap ID.
            uint tmp = selfID;
            selfID = otherID;
            otherID = tmp;
        }

        ParticleCloud self = g_Source[selfID];
        ParticleCloud other = g_Source[otherID];

        // Compare other < self.
        if (other.position.x < self.position.x)
        {
            // Swap ID.
            uint tmp = selfID;
            selfID = otherID;
            otherID = tmp;
        }

        //self.position.y = (float)selfID / numParticles;
        //other.position.y = (float)otherID / numParticles;
        //self.color.x = (float)selfID / numParticles;
        //other.color.x = (float)otherID / numParticles;

        g_Target[selfID] = self;
        g_Target[otherID] = other;
    }
}
