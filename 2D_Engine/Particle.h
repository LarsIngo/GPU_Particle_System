#pragma once

#include <glm/glm.hpp>

// An particle.
class Particle 
{
    public:
        // Constructor.
        Particle();

        // Destructor.
        ~Particle();

        // Ops.
        //Particle& operator=(const Particle& other);
        const bool operator<(const Particle& other) const
        {
            return this->mPosition.x < other.mPosition.x;
        }

        const bool operator>(const Particle& other) const
        {
            return other < *this;
        }

        const bool operator<=(const Particle& other) const
        {
            return !(*this > other);
        }

        const bool operator>=(const Particle& other) const
        {
            return !(*this < other);
        }

        // Position.
        glm::vec3 mPosition;

        // Scale.
        glm::vec2 mScale;

        // Color.
        glm::vec3 mColor;

        // Velocity.
        glm::vec3 mVelocity;

        // Lifetime.
        float mLifeTime;
};
