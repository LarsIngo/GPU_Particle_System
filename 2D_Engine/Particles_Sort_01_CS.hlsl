#define FLT_MAX 3.402823466e+38F;
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
    bool init;
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
    uint stepLen = metaData.step;
    uint numParticles = metaData.numParticles;
    uint numThreads = metaData.numThreads;
    bool init = metaData.init;

    if (tID < numThreads)
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

        Particle self;// = g_Source[0];
        Particle other;// = g_Source[0];

        if (selfID >= numParticles && init)
        {
            self.position.x = FLT_MAX;
            self.color.z = 1.f;
        }
        else
        {
            self = g_Source[selfID];
        }
            
        if (otherID >= numParticles && init)
        {
            other.position.x = FLT_MAX;
            other.color.z = 1.f;
        }
        else
        {
            other = g_Source[otherID];
        }

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

        g_Target[selfID] = self;
        g_Target[otherID] = other;
    }
}

bool Compare(Particle lhs, Particle rhs)
{
    return lhs.position.x < rhs.position.x;
}
