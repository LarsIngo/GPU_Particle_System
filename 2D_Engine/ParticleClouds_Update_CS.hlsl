#define BATCHSIZE 256U
#define BIAS 0.1F

// Particle.
struct Particle
{
    float3 position;
    float2 scale;
    float3 color;
    float3 velocity;
    float lifetime;
};

// Particle cloud.
struct ParticleCloud
{
    float3 position;
    float radius;
    float3 color;
    float3 velocity;
    uint particleStartID;
    uint numParticles;
    uint firstID;
    uint lastID;
    float timer;
    float spawntime;
};

// Meta data.
struct MetaData
{
    float dt;
    uint numClouds;
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

// Input particle clouds.
StructuredBuffer<ParticleCloud> g_SourceClouds : register(t0);

// Input particles.
StructuredBuffer<Particle> g_SourceParticles : register(t1);

// Meta buffer.
StructuredBuffer<MetaData> g_MetaBuffer : register(t2);

// Output particle clouds.
RWStructuredBuffer<ParticleCloud> g_TargetClouds : register(u0);

// Output particles.
RWStructuredBuffer<Particle> g_TargetParticles : register(u1);

// Check if clouds overlaps in x-axis.
// selfPosX This cloud x-coordinate.
// selfRadius This cloud radius.
// otherPosX Other cloud x-coordinate.
// otherRadius Other cloud radius.
bool XInstersect(float selfPosX, float selfRadius, float otherPosX, float otherRadius);

// Check if clouds overlaps.
// Clouds needs to be sorted along the x-axis.
// selfPos This cloud position.
// selfRadius This cloud radius.
// otherPos Other cloud position.
// otherRadius Other cloud radius.
// Return whether cloud intersects.
float Instersect(float3 selfPos, float selfRadius, float3 otherPos, float otherRadius);

// Function called on clouds XIntersection.
// self This cloud.
// other Other cloud.
// metaData Meta data.
void OnXIntersection(inout ParticleCloud self, ParticleCloud other, MetaData metaData, inout BoidData boidData);

groupshared ParticleCloud gs_clouds[BATCHSIZE];
groupshared ParticleCloud gs_first;
groupshared ParticleCloud gs_last;

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
    uint numClouds = metaData.numClouds;
    uint pID = min(tID, numClouds - 1);

    // Boid.
    BoidData boidData;
    boidData.radius = 10.0f;
    boidData.center = float3(0.f, 0.f, 0.f);
    boidData.n = 0;
    boidData.separation = float3(0.f, 0.f, 0.f);
    boidData.velocity = float3(0.f, 0.f, 0.f);

    // Self.
    ParticleCloud self = g_SourceClouds[pID];
    self.color = float3(0.f, 0.2f, 0.f); // TMP

    gs_clouds[gtID] = self;
    if (gtID == 0)
        gs_first = self;
    if (gtID == BATCHSIZE - 1)
        gs_last = self;
    GroupMemoryBarrierWithGroupSync();

    // ----- Update ----- //
    // Collision.
    //[unroll(BATCHSIZE)]
    for (uint otherID = 0; otherID < BATCHSIZE; ++otherID)
        if (pID != gID * BATCHSIZE + otherID && tID < numClouds)
            OnXIntersection(self, gs_clouds[otherID], metaData, boidData);
    GroupMemoryBarrierWithGroupSync();

    for (uint batchID = gID + 1; batchID < numClouds / BATCHSIZE + 1; ++batchID)
    {
        uint batchStartID = min(batchID * BATCHSIZE, numClouds - 1);
        uint batchEndID = min(batchStartID + BATCHSIZE - 1, numClouds - 1);
        ParticleCloud batchStartCloud = g_SourceClouds[batchStartID];
        if (XInstersect(gs_last.position.x, gs_last.radius, batchStartCloud.position.x, boidData.radius))
        {
            uint gpID = min(gtID + batchStartID, numClouds - 1);
            gs_clouds[gtID] = g_SourceClouds[gpID];
            GroupMemoryBarrierWithGroupSync();

            //[unroll(BATCHSIZE)]
            for (uint otherID = 0; otherID < BATCHSIZE - 1; ++otherID)
                if (gpID + otherID < numClouds)
                    OnXIntersection(self, gs_clouds[otherID], metaData, boidData);
            GroupMemoryBarrierWithGroupSync();  
        }
    }

    for (int batchID = gID - 1; batchID >= 0; --batchID)
    {
        uint batchStartID = min(batchID * BATCHSIZE, numClouds - 1);
        uint batchEndID = min(batchStartID + BATCHSIZE - 1, numClouds - 1);
        ParticleCloud batchEndCloud = g_SourceClouds[batchEndID];
        if (XInstersect(batchEndCloud.position.x, boidData.radius, gs_first.position.x, gs_first.radius))
        {
            uint gpID = min(gtID + batchStartID, numClouds - 1);
            gs_clouds[gtID] = g_SourceClouds[gpID];
            GroupMemoryBarrierWithGroupSync();

            //[unroll(BATCHSIZE)]
            for (uint otherID = 0; otherID < BATCHSIZE; ++otherID)
                OnXIntersection(self, gs_clouds[otherID], metaData, boidData);
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

        self.velocity += 0.05f * boidData.separation;
        self.velocity += 0.001f * boidData.velocity / boidData.n;
        self.velocity += 0.01f * (boidData.center / boidData.n - self.position);

        //self.velocity = normalize(self.velocity);
    }

    // Update cloud.
    self.position += self.velocity * dt;
    self.color.z = (float)tID / numClouds;
    self.timer += dt;
    //self.velocity -= self.velocity * dt/4.f;

    // Update particles in cloud.
    for (uint i = self.particleStartID; i < self.particleStartID + self.numParticles; ++i)
    {
        Particle particle = g_SourceParticles[i];
        if (self.timer > self.spawntime && particle.lifetime < 0.f)
        {
            self.timer = 0.f;
            particle.lifetime = self.spawntime * self.numParticles;
            particle.position = self.position;
            particle.velocity = self.velocity;
            particle.scale = float2(self.radius, self.radius);
            particle.color = self.color;
        }
        //particle.color = float3(1.f, 1.f, 1.f) * particle.lifetime / self.spawntime * self.numParticles;
        g_TargetParticles[i] = particle;
    }

    // Final result.
    g_TargetClouds[pID] = self;
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

void OnXIntersection(inout ParticleCloud self, ParticleCloud other, MetaData metaData, inout BoidData boidData)
{
    float result = Instersect(self.position, self.radius, other.position, boidData.radius);
    if (result <= 0.f)
    {
        self.color.x += 0.1f; // TMP

        // ----- Boid ----- //
        boidData.center += other.position;
        boidData.velocity += other.velocity;
        ++boidData.n;
        // Steer to avoid crowding local flockmates.
        float distance = sqrt(-result);
        if (distance < self.radius * 2.f)
            boidData.separation -= other.position - self.position;
    }
}
