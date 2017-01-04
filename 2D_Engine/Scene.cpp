#include "Scene.h"

#include "DxHelp.h"

#include <random>
#include <time.h>

Scene::Scene(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int maxNumParticles)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;

    mMaxNumParticles = maxNumParticles;
    mActiveNumParticles = maxNumParticles;

    // Populate particles array.
    DynamicArray<Particle> mParticles(mMaxNumParticles);
    float space = 0.5f;
    int yAxis, xAxis;
    yAxis = xAxis = sqrt(mMaxNumParticles);

    std::default_random_engine rng(time(0));
    std::uniform_int_distribution<int> dist(-1000, 1000);

    for (int y = 0; y < yAxis; ++y)
    {
        for (int x = 0; x < xAxis; ++x)
        {
            Particle particle = Particle();
            particle.mPosition = glm::vec3(x, 0.f, y) * space; // dist(rng) / 1000.f
            particle.mScale = glm::vec2(0.2f, 0.2f);
            particle.mColor = glm::vec3(0.f, 0.2f, 0.f);
            particle.mVelocity = -glm::normalize(particle.mPosition); // glm::vec3(dist(rng)/1000.f, dist(rng) / 1000.f, dist(rng) / 1000.f); // glm::normalize(particle.mPosition); /
            mParticles.Push(particle);
        }
    }
    mActiveNumParticles = yAxis * xAxis;

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
    mParticlesGPUSwapBuffer = new GPUSwapBuffer<Particle>(mpDevice, mpDeviceContext, mMaxNumParticles, mParticles.GetArrPointer());
    mParticles.Delete();
}

Scene::~Scene()
{
    delete mParticlesGPUSwapBuffer;
}
