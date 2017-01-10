#define BATCHSIZE 256U
#define BIAS 0.1F

// Particle.
struct Particle
{
    float3 position;
    float2 scale;
    float3 color;
    float3 velocity;
    float lifeTime;
};

// Meta data.
struct MetaData
{
    float dt;
    uint numParticles;
    float3 worldPos;
    bool active;
    float2 pad;
};

// Boid data.
struct BoidData
{
    float radius;
    float3 center;
    uint n;
    float3 separation;
    float3 velocity;
};

// Input.
StructuredBuffer<Particle> g_Source : register(t0);

// Meta buffer.
StructuredBuffer<MetaData> g_MetaBuffer : register(t1);

// Output.
RWStructuredBuffer<Particle> g_Target : register(u0);

// Check if particles overlaps in x-axis.
// selfPosX This particle x-coordinate.
// selfRadius This particle radius.
// otherPosX Other particle x-coordinate.
// otherRadius Other particle radius.
bool XInstersect(float selfPosX, float selfRadius, float otherPosX, float otherRadius);

// Check if particles overlaps.
// Particles needs to be sorted along the x-axis.
// selfPos This particle position.
// selfRadius This particle radius.
// otherPos Other particle position.
// otherRadius Other particle radius.
// Return whether particles intersects.
float Instersect(float3 selfPos, float selfRadius, float3 otherPos, float otherRadius);

// Function called on particle XIntersection.
// self This particle.
// other Other particle.
// metaData Meta data.
// Return updated particle.
void OnXIntersection(inout Particle self, Particle other, MetaData metaData, inout BoidData boidData);

groupshared Particle gs_particles[BATCHSIZE];
groupshared Particle gs_first;
groupshared Particle gs_last;

// 16x16
[numthreads(BATCHSIZE, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID, uint3 groupThreadID : SV_GroupThreadID, uint3 groupID : SV_GroupID)
{
    // Thread info.
    uint tID = threadID.x;
    uint gtID = groupThreadID.x;
    uint gID = groupID.x;

    // ----- Init ----- //
    // Meta.
    MetaData metaData = g_MetaBuffer[0];
    float dt = metaData.dt;
    uint numParticles = metaData.numParticles;
    uint pID = min(tID, numParticles - 1);

    // Boid.
    BoidData boidData;
    boidData.radius = 1.0f;
    boidData.center = float3(0.f, 0.f, 0.f);
    boidData.n = 0;
    boidData.separation = float3(0.f, 0.f, 0.f);
    boidData.velocity = float3(0.f, 0.f, 0.f);

    // Self.
    Particle self = g_Source[pID];
    self.color.x = 0.f; // TMP
    gs_particles[gtID] = self;
    if (gtID == 0)
        gs_first = self;
    if (gtID == BATCHSIZE - 1)
        gs_last = self;
    GroupMemoryBarrierWithGroupSync();

    // ----- Update ----- //
    // Collision.
    //[unroll(BATCHSIZE)]
    for (uint otherID = 0; otherID < BATCHSIZE; ++otherID)
        if (pID != gID * BATCHSIZE + otherID && tID < numParticles)
            OnXIntersection(self, gs_particles[otherID], metaData, boidData);
    GroupMemoryBarrierWithGroupSync();

    for (uint batchID = gID + 1; batchID < numParticles / BATCHSIZE + 1; ++batchID)
    {
        uint batchStartID = min(batchID * BATCHSIZE, numParticles - 1);
        uint batchEndID = min(batchStartID + BATCHSIZE - 1, numParticles - 1);
        Particle batchStartParticle = g_Source[batchStartID];
        if (XInstersect(gs_last.position.x, gs_last.scale.x, batchStartParticle.position.x, batchStartParticle.scale.x))
        {
            uint gpID = min(gtID + batchStartID, numParticles - 1);
            gs_particles[gtID] = g_Source[gpID];
            GroupMemoryBarrierWithGroupSync();

            //[unroll(BATCHSIZE)]
            for (uint otherID = 0; otherID < BATCHSIZE - 1; ++otherID)
                if (gpID + otherID < numParticles)
                    OnXIntersection(self, gs_particles[otherID], metaData, boidData);
            GroupMemoryBarrierWithGroupSync();  
        }
    }

    for (int batchID = gID - 1; batchID >= 0; --batchID)
    {
        uint batchStartID = min(batchID * BATCHSIZE, numParticles - 1);
        uint batchEndID = min(batchStartID + BATCHSIZE - 1, numParticles - 1);
        Particle batchEndParticle = g_Source[batchEndID];
        if (XInstersect(batchEndParticle.position.x, batchEndParticle.scale.x, gs_first.position.x, gs_first.scale.x))
        {
            uint gpID = min(gtID + batchStartID, numParticles - 1);
            gs_particles[gtID] = g_Source[gpID];
            GroupMemoryBarrierWithGroupSync();

            //[unroll(BATCHSIZE)]
            for (uint otherID = 0; otherID < BATCHSIZE; ++otherID)
                OnXIntersection(self, gs_particles[otherID], metaData, boidData);
            GroupMemoryBarrierWithGroupSync();
        }
    }

    //uint selfID = pID;
    //for (int batchID = 0; batchID < numParticles / BATCHSIZE + 1; ++batchID)
    //{
    //    uint startID = min((selfID + 1) + batchID * BATCHSIZE, numParticles - 1);
    //    Particle other = g_Source[startID];
    //    if (!XInstersect(self, other))
    //        break;
    //    uint endID = min(startID + BATCHSIZE, numParticles - 1);
    //    [unroll(BATCHSIZE)]
    //    for (int otherID = startID; otherID < endID; ++otherID)
    //    {
    //        other = g_Source[otherID];
    //        OnXIntersection(self, other, metaData, boidData);
    //    }
    //}
    //for (int batchID = 0; batchID < numParticles / BATCHSIZE + 1; ++batchID)
    //{
    //    uint startID = max((selfID - 1) - batchID * BATCHSIZE, 0);
    //    Particle other = g_Source[startID];
    //    if (!XInstersect(other, self))
    //        break;
    //    uint endID = max(startID - BATCHSIZE, 0);
    //    [unroll(BATCHSIZE)]
    //    for (int otherID = startID; otherID > endID; --otherID)
    //    {
    //        other = g_Source[otherID];
    //        OnXIntersection(self, other, metaData, boidData);
    //    }
    //}


    // ----- Result ----- //
    // Boid.
    if (boidData.n > 0)
    {
        // Steer to move toward the average position(center of mass) of local flockmates.
        //boidData.center /= (boidData.n * 100.f); 
        // Steer towards the average heading of local flockmates.
        //boidData.velocity /= (boidData.n * 8.f);
        // Update velocity.
        //0.001f * (boidData.center + boidData.velocity + boidData.separation);
        //self.velocity += boidData.center + boidData.velocity + boidData.separation;

        //self.velocity += boidData.separation + 0.01f * boidData.velocity;

        //self.velocity = normalize(self.velocity);
    }

    // Self.
    self.position += self.velocity * dt;
    self.color.z = (float)tID / numParticles;

    // Final result.
    g_Target[pID] = self;
}

bool XInstersect(float selfPosX, float selfRadius, float otherPosX, float otherRadius)
{
    return abs(otherPosX - selfPosX) < selfRadius + otherRadius + BIAS;
}

float Instersect(float3 selfPos, float selfRadius, float3 otherPos, float otherRadius)
{
    float len = selfRadius + otherRadius;
    float3 selfToOtherVec = otherPos - selfPos;
    float distance = selfToOtherVec.x * selfToOtherVec.x + selfToOtherVec.y * selfToOtherVec.y + selfToOtherVec.z * selfToOtherVec.z;
    
    return distance - len * len;
}

void OnXIntersection(inout Particle self, Particle other, MetaData metaData, inout BoidData boidData)
{
    float result = Instersect(self.position, self.scale.x, other.position, other.scale.x);
    if (result <= 0.f)
    {
        self.color.x += 0.1f; // TMP

        // ----- Boid ----- //
        boidData.center += self.position;
        boidData.velocity += self.velocity;
        ++boidData.n;
        // Steer to avoid crowding local flockmates.
        float distance = sqrt(-result);
        if (distance < 1.0f)
            boidData.separation -= other.position - self.position;
    }
}
