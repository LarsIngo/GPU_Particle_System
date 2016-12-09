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

// Steer to avoid crowding local flockmates.
float3 Separation(BoidData boidData);

// Steer towards the average heading of local flockmates.
float3 Alignment(BoidData boidData);

// Steer to move toward the average position(center of mass) of local flockmates.
float3 Cohesion(BoidData boidData);

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
    boidData.radius = 50000000.f;
    boidData.center = float3(0.f, 0.f, 0.f);
    boidData.n = 0;
    boidData.separation = float3(0.f, 0.f, 0.f);
    boidData.velocity = float3(0.f, 0.f, 0.f);

    // ----- Update ----- //
    // Collision.
    for (int otherID = selfID + 1; otherID < numParticles; ++otherID)
    {
        Particle other = g_Source[otherID];
        if (!XInstersect(self, other))
            break;
        OnXIntersection(self, other, metaData, boidData);
    }
    for (int otherID = selfID - 1; otherID >= 0; --otherID)
    {
        Particle other = g_Source[otherID];
        if (!XInstersect(other, self))
            break;
        OnXIntersection(self, other, metaData, boidData);
    }

    // ----- Result ----- //
    // Boid.
    if (boidData.n > 0)
    {
        // Steer to move toward the average position(center of mass) of local flockmates.
        boidData.center /= (boidData.n * 100.f); 
        // Steer towards the average heading of local flockmates.
        boidData.velocity /= (boidData.n * 8.f);
        // Update velocity.
        //0.001f * (boidData.center + boidData.velocity + boidData.separation);
        self.velocity += boidData.center + boidData.velocity + boidData.separation;

        self.velocity = normalize(self.velocity);
    }

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
    float result = Instersect(self.position, boidData.radius, other.position, 0.f);
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

float3 Separation(BoidData boidData)
{
    return float3(0.f, 0.f, 0.f);
}

float3 Alignment(BoidData boidData)
{
    return float3(0.f, 0.f, 0.f);
}

float3 Cohesion(BoidData boidData)
{
    if (boidData.n == 0)
        return float3(0.f, 0.f, 0.f);

    float factor = 100.f;
    return boidData.center /= (boidData.n * factor);
}
