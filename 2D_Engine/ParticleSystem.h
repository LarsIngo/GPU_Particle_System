#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11.h>
#include <d3dcompiler.inl>

#include "Particle.h"
#include "Scene.h"

class ParticleSystem
{
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        // maxNumParticles Max number of particles.
        ParticleSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int maxNumParticles);

        // Destructor.
        ~ParticleSystem();

        // Bind pipeline.
        // sourceBuffer Buffer to read from.
        // targetBuffer Buffer to write to.
        void Bind(ID3D11ShaderResourceView* sourceBuffer, ID3D11UnorderedAccessView* targetBuffer);

        // Ünbind pipeline.
        void Unbind();

        // Update particles.
        // scene Scene to update.
        // dt Delta time.
        void Update(Scene& scene, float dt);

        // MetaData.
        struct MetaData
        {
            float dt;
            unsigned int numParticles;
            glm::vec3 worldPos;
            bool active;
            float pad[2];
        } mMetaData;

    private:

        // Initialise buffers and compute shader.
        void Initialise();

        unsigned int mMaxNumParticles;
        bool mFirstTime;

        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;
        ID3D11ComputeShader* mParticleCS;
        ID3D11ShaderResourceView* mMetaDataBuffer;
};
