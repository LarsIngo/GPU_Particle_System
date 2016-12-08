#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11.h>
#include <d3dcompiler.inl>

#include <glm/glm.hpp>

#include "Particle.h"
#include "Camera.h"
#include "Scene.h"

class ParticleRenderer 
{
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        // maxNumParticles Max number of particles.
        ParticleRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int maxNumParticles);

        // Destructor.
        ~ParticleRenderer();

        // Bind pipeline.
        // renderTargetView Render taget.
        // depthStencilView Depth buffer.
        void Bind(ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView);

        // Unbind pipeline.
        void Unbind();

        // Render particles.
        // vpMatix View projection matrix.
        // lensPostion Camera world position.
        // scene Scene.
        void Render(const glm::mat4& vpMatix, const glm::vec3& lensPostion, Scene& scene);

    private:
        // Initialise shaders and states.
        void Initialise();

        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;
        ID3D11InputLayout* mInputLayout;
        ID3D11VertexShader* mVertexShader;
        ID3D11GeometryShader* mGeometryShader;
        ID3D11PixelShader* mPixelShader;
        ID3D11SamplerState* mSamplerState;
        ID3D11BlendState* mBlendState;
        ID3D11DepthStencilState* mDepthSencilState;
        ID3D11RasterizerState* mRasterizerState;

        struct MetaData
        {
            glm::mat4 vpMatrix;
            glm::vec3 lensPosition;
            glm::vec3 lensUpDirection;
            float pad[2];
            
        } mMetaData;
        ID3D11ShaderResourceView* mMetaDataBuffer;
};
