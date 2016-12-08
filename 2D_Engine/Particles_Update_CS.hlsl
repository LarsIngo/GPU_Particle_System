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

// Check if particles overlaps.
// Particles needs to be sorted along the x-axis.
// Make shore self.position.x < other.position.x.
// self This particle.
// other Other particle.
// Return whether particles intersects.
bool Instersect(Particle self, Particle other);

// Fucntion called on particle collision.
// self This particle.
// other Other particle.
// metaData Meta data.
// Return updated particle.
Particle OnCollision(Particle self, Particle other, MetaData metaData);

// 16x16
[numthreads(256, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    MetaData metaData = g_MetaBuffer[0];
    float dt = metaData.dt;
    uint numParticles = metaData.numParticles;

    uint selfID = threadID.x;
    Particle self = g_Source[selfID];

    //self.color = float3(0, 0.2f, 0);

    for (int otherID = selfID + 1; otherID < numParticles; ++otherID)
    {
        Particle other = g_Source[otherID];
        if (!Instersect(self, other))
            break;
        self = OnCollision(self, other, metaData);
    }

    for (int otherID = selfID - 1; otherID >= 0; --otherID)
    {
        Particle other = g_Source[otherID];
        if (!Instersect(other, self))
            break;
        self = OnCollision(self, other, metaData);
    }


    self.position += self.velocity * dt;
    self.velocity += self.velocity * dt / 2.f;
    g_Target[selfID] = self;
}

// Assert self.pos.x < other.pos.x
bool Instersect(Particle self, Particle other)
{
    float selfRadius = self.scale.x;
    float otherRadius = other.scale.x;
    float selfLen = selfRadius * 2.f;
    float otherLen = otherRadius * 2.f;
    float selfLeft = self.position.x - selfRadius;
    float otherLeft = other.position.x - otherRadius;

    if (selfLeft + selfLen > otherLeft) 
    {
        float len = selfRadius + otherRadius;
        float3 selfToOtherVec = other.position - self.position;
        float distance = selfToOtherVec.x * selfToOtherVec.x + selfToOtherVec.y * selfToOtherVec.y + selfToOtherVec.z * selfToOtherVec.z;
        if (distance < len * len)
            return true;
    }
    
    return false;
}

Particle OnCollision(Particle self, Particle other, MetaData metaData)
{
    //self.color = float3(1.f, 0.f, 0.f);
    return self;
}
