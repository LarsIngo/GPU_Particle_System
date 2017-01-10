#include "Particle.h"

Particle::Particle()
{
    mPosition = glm::vec3(0.f, 0.f, 0.f);
    mScale = glm::vec2(1.f, 1.f);
    mColor = glm::vec3(1.f, 1.f, 1.f);
    mVelocity = glm::vec3(0.f, 0.f, 0.f);
    mLifetime = -1.f;
}

Particle::~Particle() 
{

}
