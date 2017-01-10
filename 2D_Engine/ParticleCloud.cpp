#include "ParticleCloud.h"

ParticleCloud::ParticleCloud()
{
    mPosition = glm::vec3(0.f, 0.f, 0.f);
    mRadius = 1.f;
    mVelocity = glm::vec3(0.f, 0.f, 0.f);
    mParticleStartID = 0;
    mNumParticles = 0;
}

ParticleCloud::~ParticleCloud()
{

}
