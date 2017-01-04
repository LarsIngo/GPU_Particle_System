#define BATCHSIZE 32

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
// self This particle.
// other Other particle.
// Make sure self.position.x < other.position.x.
bool XInstersect(Particle self, Particle other);

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

// 16x16
[numthreads(256, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{

    // ----- Init ----- //
    // Meta.
    MetaData metaData = g_MetaBuffer[0];
    float dt = metaData.dt;
    uint numParticles = metaData.numParticles;

    // Self.
    uint selfID = threadID.x;
    Particle self = g_Source[selfID];
    self.color.x = 0.f; // TMP

    // Boid.
    BoidData boidData;
    boidData.radius = 1.0f;
    boidData.center = float3(0.f, 0.f, 0.f);
    boidData.n = 0;
    boidData.separation = float3(0.f, 0.f, 0.f);
    boidData.velocity = float3(0.f, 0.f, 0.f);

    // ----- Update ----- //
    // Collision.
    for (int batchID = 0; batchID < numParticles / BATCHSIZE + 1; ++batchID)
    {
        uint startID = min((selfID + 1) + batchID * BATCHSIZE, numParticles - 1);
        Particle other = g_Source[startID];
        if (!XInstersect(self, other))
            break;
        uint endID = min(startID + BATCHSIZE, numParticles - 1);
        [unroll(BATCHSIZE)]
        for (int otherID = startID; otherID < endID; ++otherID)
        {
            other = g_Source[otherID];
            OnXIntersection(self, other, metaData, boidData);
        }
    }
    for (int batchID = 0; batchID < numParticles / BATCHSIZE + 1; ++batchID)
    {
        uint startID = max((selfID - 1) - batchID * BATCHSIZE, 0);
        Particle other = g_Source[startID];
        if (!XInstersect(other, self))
            break;
        uint endID = max(startID - BATCHSIZE, 0);
        [unroll(BATCHSIZE)]
        for (int otherID = startID; otherID > endID; --otherID)
        {
            other = g_Source[otherID];
            OnXIntersection(self, other, metaData, boidData);
        }
    }


    // ----- Result ----- //
    // Boid.
    //if (boidData.n > 0)
    //{
    //    // Steer to move toward the average position(center of mass) of local flockmates.
    //    boidData.center /= (boidData.n * 100.f); 
    //    // Steer towards the average heading of local flockmates.
    //    boidData.velocity /= (boidData.n * 8.f);
    //    // Update velocity.
    //    //0.001f * (boidData.center + boidData.velocity + boidData.separation);
    //    self.velocity += boidData.center + boidData.velocity + boidData.separation;

    //    self.velocity = normalize(self.velocity);
    //}

    // Self.
    self.position += self.velocity * dt;
    self.color.z = (float)selfID / numParticles;

    // Final result.
    g_Target[selfID] = self;
}

// Assert self.pos.x < other.pos.x
bool XInstersect(Particle self, Particle other)
{
    float selfRadius = self.scale.x;
    float otherRadius = other.scale.x;
    float selfLen = selfRadius * 2.f;
    float otherLen = otherRadius * 2.f;
    float selfLeft = self.position.x - selfRadius;
    float otherLeft = other.position.x - otherRadius;

    return selfLeft + selfLen > otherLeft;
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

        //// ----- Boid ----- //
        //boidData.center += self.position;
        //boidData.velocity += self.velocity;
        //++boidData.n;
        //// Steer to avoid crowding local flockmates.
        //float distance = sqrt(-result);
        //if (distance < 1.0f)
        //    boidData.separation -= other.position - self.position;
    }
}
