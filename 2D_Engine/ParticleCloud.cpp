#include "ParticleCloud.h"

ParticleCloud::ParticleCloud()
{
    mPosition = glm::vec3(0.f, 0.f, 0.f);
    mRadius = 1.f;
    mVelocity = glm::vec3(0.f, 0.f, 0.f);
    mParticleStartID = 0;
    mNumParticles = 0;
    mFirstID = 0;
    mLastID = 0;
    mTimer = 0.f;
    mSpawntime = 5.f;
}

ParticleCloud::~ParticleCloud()
{

}
