#include "Scene.h"

#include "DxHelp.h"

#include <random>
#include <time.h>
#include <vector>

Scene::Scene(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int maxNumParticles, unsigned int maxNumParticleClouds)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;

    mMaxNumParticles = maxNumParticles;
    mMaxNumParticleClouds = maxNumParticleClouds;

    // Populate particles array.
    std::vector<Particle> particles;
    std::vector<ParticleCloud> particleClouds;

    float space = 0.5f;
    int yAxis, xAxis;
    yAxis = xAxis = sqrt(mMaxNumParticleClouds);

    std::default_random_engine rng(time(0));
    std::uniform_int_distribution<int> dist(-1000, 1000);

    unsigned int index = 0;
    for (int y = 0; y < yAxis; ++y)
    {
        for (int x = 0; x < xAxis; ++x)
        {
            ParticleCloud particleCloud;
            particleCloud.mPosition = glm::vec3(x, 0.f, y) * space; // dist(rng) / 1000.f
            particleCloud.mRadius = 0.2f;
            particleCloud.mNumParticles = 16;
            particleCloud.mParticleStartID = index;
            index += particleCloud.mNumParticles;
            particleCloud.mVelocity = -glm::normalize(particleCloud.mPosition + glm::vec3(0.01f, 0.01f, 0.01f)) * 4.f;
            particleCloud.mColor = glm::vec3(0.f, 0.2f, 0.f);
            particleClouds.push_back(particleCloud);

            float random = (float)dist(rng) / 10000.f;
            glm::vec3 pos = particleCloud.mPosition;
            for (unsigned int i = 0; i < 16; ++i)
            {
                Particle particle;
                pos += glm::vec3(1.f, 0.f, 1.f) * random;
                particle.mPosition = pos + pos;
                particle.mPosition.y = 5.f * ((float)i / particleCloud.mNumParticles);
                particle.mScale = glm::vec2(0.2f, 0.2f) * 1.f;
                particle.mColor = glm::vec3(i % 3, i % 2, 1.f);
                //particle.mVelocity = glm::vec3(particleCloud.mVelocity.x, 0.f, particleCloud.mVelocity.z);
                particle.mLifetime = 100.f;
                particles.push_back(particle);
            }
        }
    }

    //float r = 5.f;
    //float s = r / std::cbrt(mMaxNumParticles) * 2; // Works-ish.
    //for (float z = -r; z <= r; z += s) 
    //{
    //    for (float y = -r; y <= r; y += s)
    //    {
    //        for (float x = -r; x <= r; x += s)
    //        {
    //            if (mParticles.Size() < mMaxNumParticles) {
    //                Particle particle = Particle();
    //                particle.mPosition = glm::vec3(x + 0.01f * y + 0.01f * z, y, z);
    //                particle.mScale = glm::vec2(0.2f, 0.2f);
    //                particle.mVelocity = glm::vec3(0.f, 0.f, 0.f);
    //                particle.mColor = glm::abs(glm::normalize(particle.mPosition));
    //                mParticles.Push(particle);
    //            }
    //        }
    //    }
    //}

    //for (int x = 0; x < mMaxNumParticles; ++x)
    //{
    //    Particle particle = Particle();
    //    particle.mPosition = glm::vec3(x, 0, (float)x / mMaxNumParticles * 12);
    //    particle.mScale = glm::vec2(0.45f, 0.45f);
    //    particle.mVelocity = glm::vec3(-((float)x/ mMaxNumParticles * (float)x / mMaxNumParticles), 0.f, 0.f);
    //    particle.mColor = glm::vec3(0.f, 0.2f, 0.f);
    //    mParticles.Push(particle);
    //}

    //for (int x = 0; x < mMaxNumParticles; ++x)
    //{
    //    Particle particle = Particle();
    //    particle.mPosition = glm::vec3(x, (float)x / mMaxNumParticles, 0);
    //    particle.mScale = glm::vec2(0.45f, 0.45f);
    //    particle.mVelocity = glm::vec3(0.f, 0.f, 0.f);
    //    particle.mColor = glm::vec3((float)x / mMaxNumParticles, 0.2f, 0.f);
    //    mParticles.Push(particle);
    //}

    // Create buffer and init particle data.
    if (particles.size() < mMaxNumParticles)
        for (unsigned int i = particles.size(); i < mMaxNumParticles; ++i)
            particles.push_back(Particle());
    //if (particleClouds.size() < mMaxNumParticleClouds)
    //    for (unsigned int i = 0; i < mMaxNumParticles - particleClouds.size(); ++i)
    //        particleClouds.push_back(ParticleCloud());
    assert(particles.size() == mMaxNumParticles);
    assert(particleClouds.size() == mMaxNumParticleClouds);
    mParticlesGPUSwapBuffer = new GPUSwapBuffer<Particle>(mpDevice, mpDeviceContext, mMaxNumParticles, particles.data());
    mParticleCloudsGPUSwapBuffer = new GPUSwapBuffer<ParticleCloud>(mpDevice, mpDeviceContext, mMaxNumParticleClouds, particleClouds.data());
}

Scene::~Scene()
{
    delete mParticlesGPUSwapBuffer;
    delete mParticleCloudsGPUSwapBuffer;
}
