#pragma once

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <chrono>
#include <crtdbg.h>
#include <iostream>
#include <glm/glm.hpp>

#include "Particle.h"
#include "ParticleSorter.h"
#include "ParticleSystem.h"
#include "Scene.h"

#include "DxAssert.h"
#include "Profiler.h"
#include "Renderer.h"

glm::vec2 Arrowinput(float speed);
float ZXinput(float speed);

using namespace std::chrono;

void tonicMerge_init(int g_Source[], int g_Target[], int numParticles, int stepLen, int numThreads, bool init)
{
    for (int threadID = 0; threadID < numThreads; ++threadID)
    {
        unsigned int tID = threadID;

        if (tID < numThreads)
        {
            unsigned int setLen = 4 * stepLen;
            unsigned int threadsPerSet = setLen / 2;
            unsigned int setID = tID / threadsPerSet;
            unsigned int setStart = setID * setLen;

            bool setRightSide = tID % threadsPerSet >= threadsPerSet / 2;
            unsigned int tOffset = (tID % stepLen) + setRightSide * 2 * stepLen;
            unsigned int selfID = setStart + tOffset;
            unsigned int otherID = selfID + stepLen;

            if (setRightSide)
            {
                // Swap ID.
                unsigned int tmp = selfID;
                selfID = otherID;
                otherID = tmp;
            }

            int self;
            int other;
            if (selfID >= numParticles && init)
                self = 1000;
            else 
                self = g_Source[selfID];

            if (otherID >= numParticles && init)
                other = 1000;
            else
                other = g_Source[otherID];

            // Compare other < self.
            if (other < self)
            {
                // Swap ID.
                unsigned int tmp = selfID;
                selfID = otherID;
                otherID = tmp;
            }

            g_Target[selfID] = self;
            g_Target[otherID] = other;
        }
    }
}

void tonicMerge_swap(int g_Source[], int g_Target[], int numParticles, int stepLen, int numThreads)
{
    for (int threadID = 0; threadID < numThreads; ++threadID)
    {
        unsigned int tID = threadID;

        if (tID < numThreads)
        {
            bool setRightSide = tID >= numThreads / 2;
            unsigned int tOffset = (tID % stepLen) + (tID / stepLen) * (2 * stepLen);
            unsigned int selfID =  tOffset;
            unsigned int otherID = selfID + stepLen;

            if (setRightSide)
            {
                // Swap ID.
                unsigned int tmp = selfID;
                selfID = otherID;
                otherID = tmp;
            }

            int self = g_Source[selfID];
            int other = g_Source[otherID];

            // Compare other < self.
            if (other < self)
            {
                // Swap ID.
                unsigned int tmp = selfID;
                selfID = otherID;
                otherID = tmp;
            }

            g_Target[selfID] = self;
            g_Target[otherID] = other;
        }
    }
}

void tonicMerge_merge(int g_Source[], int g_Target[], int numParticles, int stepLen, int numThreads)
{
    for (int threadID = 0; threadID < numThreads; ++threadID)
    {
        unsigned int tID = threadID;

        if (tID < numThreads)
        {
            unsigned int tOffset = (tID % stepLen) + (tID / stepLen) * (2 * stepLen);
            unsigned int selfID = tOffset;
            unsigned int otherID = selfID + stepLen;

            int self = g_Source[selfID];
            int other = g_Source[otherID];

            // Compare other < self.
            if (other < self)
            {
                // Swap ID.
                unsigned int tmp = selfID;
                selfID = otherID;
                otherID = tmp;
            }

            g_Target[selfID] = self;
            g_Target[otherID] = other;
        }
    }
}

int main() 
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // TMP

    int g_Source[16];
    //int g_Target[16] = { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
    int g_Target[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

    unsigned int numParticles = 14;
    unsigned int v = numParticles;
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    unsigned int numThreads = v / 2;

    bool init = true;

    // TONIC INIT
    for (unsigned int step = 1; step <= numThreads / 4; step *= 2)
    {
        memcpy(g_Source, g_Target, sizeof(int) * numThreads * 2);
        tonicMerge_init(g_Source, g_Target, numParticles, step, numThreads, init);
        init = false;
    }

    // TONIC SWAP
    for (unsigned int step = numThreads / 2; step >= 1; step /= 2)
    {
        memcpy(g_Source, g_Target, sizeof(int) * numThreads * 2);
        tonicMerge_swap(g_Source, g_Target, numParticles, step, numThreads);
    }

    // TONIC MERGE
    for (unsigned int step = numThreads; step >= 1; step /= 2)
    {
        memcpy(g_Source, g_Target, sizeof(int) * numThreads * 2);
        tonicMerge_merge(g_Source, g_Target, numParticles, step, numThreads);
    }

    // ~TMP


    // Max number of particles.
    unsigned int maxNumParticles = pow(2, 16);

    // Create renderer.
    Renderer renderer(1024, 1024);

    // Create scene.
    Scene scene(renderer.mDevice, renderer.mDeviceContext, maxNumParticles);
    Camera& camera = scene.mCamera;
    camera.mPosition = glm::vec3(0.f, 0.f, -5.f);
    scene.mActiveNumParticles = maxNumParticles;

    // Create particle system.
    ParticleSystem particleSystem(renderer.mDevice, renderer.mDeviceContext);

    // Create particle sorter.
    ParticleSorter particleSorter(renderer.mDevice, renderer.mDeviceContext);

    // Set Frame Latency.
    IDXGIDevice1 * pDXGIDevice;
    DxAssert(renderer.mDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice), S_OK);
    DxAssert(pDXGIDevice->SetMaximumFrameLatency(1), S_OK);
    pDXGIDevice->Release();

    long long lastTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    float dt = 0.f;
    float duration = 0.f;
    unsigned int frameCounter = 0;
    while (renderer.Running()) {
        frameCounter++;
        { PROFILE("FRAME: " + std::to_string(scene.mActiveNumParticles), true);
            long long newTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            duration += dt = static_cast<float>(newTime - lastTime)/1000.f;
            lastTime = newTime;

            // Camera.
            camera.Update(20.f, dt, &renderer);

            // Particles sort.
            particleSorter.Sort(scene);

            // Particles update.
            particleSystem.Update(scene, dt);
        
            // Renderer.
            renderer.Render(scene);

            //MessageBox(NULL, "", "", 0);
        }
    }

    return 0;
}


glm::vec2 Arrowinput(float speed) {

    glm::vec2 input(0.f, 0.f);

    if (GetAsyncKeyState(VK_UP))
    {
        input.y += speed;
    }
    if (GetAsyncKeyState(VK_LEFT))
    {
        input.x -= speed;
    }
    if (GetAsyncKeyState(VK_DOWN))
    {
        input.y -= speed;
    }
    if (GetAsyncKeyState(VK_RIGHT))
    {
        input.x += speed;
    }

    return input;

}

float ZXinput(float speed) {

    float input = 0.f;

    if (GetAsyncKeyState('Z'))
    {
        input += speed;
    }
    if (GetAsyncKeyState('X'))
    {
        input -= speed;
    }

    return input;

}
