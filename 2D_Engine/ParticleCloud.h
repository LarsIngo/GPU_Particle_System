#pragma once

#include <glm/glm.hpp>
#include "Particle.h"

// A particle cloud.
class ParticleCloud
{
    public:
        // Constructor.
        ParticleCloud();

        // Destructor.
        ~ParticleCloud();

        // Position.
        glm::vec3 mPosition;

        // Radius.
        float mRadius;

        // Color.
        glm::vec3 mColor;

        // Velocity.
        glm::vec3 mVelocity;

        // Index of first particle in particle array.
        unsigned int mParticleStartID;

        // Number of particles.
        unsigned int mNumParticles;

        // First particle index.
        unsigned int mFirstID;

        // Last particle index.
        unsigned int mLastID;

        // Timer.
        float mTimer;

        // Timestamp to spawn particles.
        float mSpawntime;
};
