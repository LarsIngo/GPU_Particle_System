#pragma once

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <chrono>
#include <crtdbg.h>
#include <iostream>
#include <glm/glm.hpp>

#include "Particle.h"
#include "ParticleCloud.h"
#include "ParticleCloudSorter.h"
#include "ParticleCloudSystem.h"
#include "ParticleSystem.h"
#include "Scene.h"

#include "DxAssert.h"
#include "Profiler.h"
#include "Renderer.h"

glm::vec2 Arrowinput(float speed);
float ZXinput(float speed);

using namespace std::chrono;

int main() 
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // Max number of particles.
    unsigned int maxNumParticleClouds = pow(2, 16);
    unsigned int maxNumParticles = maxNumParticleClouds * 8;

    // Create renderer.
    Renderer renderer(1024, 1024);

    // Create scene.
    Scene scene(renderer.mDevice, renderer.mDeviceContext, maxNumParticles, maxNumParticleClouds);
    Camera& camera = scene.mCamera;
    camera.mPosition = glm::vec3(0.f, 0.f, -5.f);

    // Create particle system.
    ParticleSystem particleSystem(renderer.mDevice, renderer.mDeviceContext);

    // Create particle cloud system.
    ParticleCloudSystem partilceCloudSystem(renderer.mDevice, renderer.mDeviceContext);

    // Create particle cloud sorter.
    ParticleCloudSorter particleCloudSorter(renderer.mDevice, renderer.mDeviceContext);

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
        { PROFILE("FRAME: " + std::to_string(scene.mMaxNumParticles), true);
            long long newTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            duration += dt = static_cast<float>(newTime - lastTime)/1000.f;
            lastTime = newTime;

            // Camera.
            camera.Update(20.f, dt, &renderer);

            // Particle clouds sort.
            particleCloudSorter.Sort(scene);

            // Particle clouds update.
            partilceCloudSystem.Update(scene, dt);

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
