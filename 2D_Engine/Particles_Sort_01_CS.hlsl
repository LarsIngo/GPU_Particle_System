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
    uint numThreads;
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
    uint stepLen = metaData.step;

    if (tID < numParticles / 2)
    {
        uint setLen = 4 * stepLen;
        uint threadsPerSet = setLen / 2;
        uint setID = tID / threadsPerSet;
        uint setStart = setID * setLen;

        bool setRightSide = tID % threadsPerSet >= threadsPerSet / 2;
        uint tOffset = (tID % stepLen) + setRightSide * 2 * stepLen;
        uint selfID = setStart + tOffset;
        uint otherID = selfID + stepLen;

        if (setRightSide)
        {
            // Swap ID.
            uint tmp = selfID;
            selfID = otherID;
            otherID = tmp;
        }

        // Clamp IDs inside array.
        selfID = min(selfID, numParticles - 1);
        otherID = min(otherID, numParticles - 1);

        Particle self = g_Source[selfID];
        Particle other = g_Source[otherID];

        // Compare other < self.
        if (Compare(other, self))
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
        //self.color.z = (float)tOffset / 1;
        //other.color.z = (float)tOffset / 1;

        g_Target[selfID] = self;
        g_Target[otherID] = other;
    }
}

bool Compare(Particle lhs, Particle rhs)
{
    return lhs.position.x < rhs.position.x;
}
