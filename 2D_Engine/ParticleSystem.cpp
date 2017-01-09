#include "ParticleSystem.h"

#include "DxHelp.h"

ParticleSystem::ParticleSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;

    Initialise();
}

ParticleSystem::~ParticleSystem()
{
    mParticleUpdateCS->Release();
    mMetaDataBuffer->Release();
}

void ParticleSystem::Bind(ID3D11ShaderResourceView* sourceBuffer, ID3D11UnorderedAccessView* targetBuffer)
{
    mpDeviceContext->CSSetShader(mParticleUpdateCS, NULL, NULL);
    mpDeviceContext->CSSetShaderResources(0, 1, &sourceBuffer);
    mpDeviceContext->CSSetShaderResources(1, 1, &mMetaDataBuffer);
    mpDeviceContext->CSSetUnorderedAccessViews(0, 1, &targetBuffer, NULL);
}

void ParticleSystem::Unbind()
{
    mpDeviceContext->CSSetShader(NULL, NULL, NULL);
    void *const p[1] = { NULL };
    mpDeviceContext->CSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->CSSetShaderResources(1, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->CSSetUnorderedAccessViews(0, 1, (ID3D11UnorderedAccessView**)p, NULL);
}

void ParticleSystem::Update(Scene& scene, float dt)
{
    // Swap buffers.
    scene.mParticlesGPUSwapBuffer->Swap();

    unsigned int numPartices = scene.mActiveNumParticles;

    // Update meta buffer.
    mMetaData.dt = dt;
    mMetaData.numParticles = numPartices;
    mMetaData.worldPos = glm::vec3(0,0,0);
    mMetaData.active = true;
    DxHelp::WriteStructuredBuffer<MetaData>(mpDeviceContext, &mMetaData, 1, mMetaDataBuffer);

    Bind(scene.mParticlesGPUSwapBuffer->GetSourceBuffer(), scene.mParticlesGPUSwapBuffer->GetTargetBuffer());

    mpDeviceContext->Dispatch(numPartices / 128 + 1, 1, 1);

    Unbind();
}

void ParticleSystem::Initialise()
{
    // Create meta buffer.
    DxHelp::CreateCPUwriteGPUreadStructuredBuffer<MetaData>(mpDevice, 1, &mMetaDataBuffer);

    // Create compute shader.
    ID3DBlob* errorBlob = nullptr;
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(L"Particles_Update_CS.hlsl", nullptr, nullptr, "main", "cs_5_0", 0, NULL, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        char* errorMsg = (char*)errorBlob->GetBufferPointer();
        OutputDebugStringA(errorMsg);
        shaderBlob->Release();
    }
    DxAssert(mpDevice->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &mParticleUpdateCS), S_OK);
    shaderBlob->Release();
}
