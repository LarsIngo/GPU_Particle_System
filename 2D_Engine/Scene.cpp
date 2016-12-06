#include "Scene.h"

Scene::Scene()
{
    unsigned int numParticles = 400000;
    mParticles = new DynamicArray<Particle>(numParticles);

    float r = 10.f;
    float s = r / std::cbrt(numParticles) * 2; // Works-ish.
    for (float z = -r; z <= r; z += s) 
    {
        for (float y = -r; y <= r; y += s)
        {
            for (float x = -r; x <= r; x += s)
            {
                if (mParticles->Size() < numParticles) {
                    Particle particle = Particle();
                    particle.mPosition = glm::vec3(x + 0.01f * y + 0.01f * z, y, z);
                    particle.mScale = glm::vec2(0.1f, 0.1f);
                    particle.mVelocity = glm::vec3(0.f, 1.f, 0.f);
                    particle.mColor = glm::normalize(particle.mPosition);
                    mParticles->Push(particle);
                }
            }
        }
    }
}

Scene::~Scene()
{
    //mParticles->Delete();
    delete mParticles;
}
