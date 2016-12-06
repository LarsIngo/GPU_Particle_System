#include "ParticleSystem.h"

#include "DxHelp.h"

ParticleSystem::ParticleSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int maxNumParticles)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
    mMaxNumParticles = maxNumParticles;
    mFirstTime = true;

    Initialise();
}

ParticleSystem::~ParticleSystem()
{
    mParticleCS->Release();
    mParticleSourceBuffer->Release();
    mParticleTargetBuffer->Release();
    mMetaDataBuffer->Release();
}

void ParticleSystem::Bind()
{
    mpDeviceContext->CSSetShader(mParticleCS, NULL, NULL);
    mpDeviceContext->CSSetShaderResources(0, 1, &mParticleSourceBuffer);
    mpDeviceContext->CSSetShaderResources(1, 1, &mMetaDataBuffer);
    mpDeviceContext->CSSetUnorderedAccessViews(0, 1, &mParticleTargetBuffer, NULL);
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
    unsigned int numPartices = scene.mParticles->Size();
    assert(numPartices <= mMaxNumParticles);

    // Sort.
    //scene.mParticles->Sort();

    // Update meta buffer.

    mMetaData.dt = dt;
    mMetaData.numParticles = numPartices;
    mMetaData.worldPos = glm::vec3(0,0,0);
    mMetaData.active = true;
    DxHelp::WriteStructuredBuffer<MetaData>(mpDeviceContext, &mMetaData, 1, mMetaDataBuffer);

    // Write particle buffer.
    //DxHelp::WriteStructuredBuffer<Particle>(mpDeviceContext, scene.mParticles->GetArrPointer(), numPartices, mParticleSourceBuffer);

    Bind();

    mpDeviceContext->Dispatch(numPartices / 128, 1, 1);

    Unbind();

    //if (mFirstTime) 
    //{
    //    scene.mParticles->Delete();
    //    mFirstTime = false;
    //}

    //// Read particle buffer.
    //scene.mParticles->SetArrPointer(DxHelp::ReadStructuredBuffer<Particle>(mpDeviceContext, mParticleTargetBuffer), numPartices);
}

void ParticleSystem::Initialise()
{
    DxHelp::CreateCPUwriteGPUreadStructuredBuffer<Particle>(mpDevice, mMaxNumParticles, &mParticleSourceBuffer);
    DxHelp::CreateCPUreadGPUwriteStructuredBuffer<Particle>(mpDevice, mMaxNumParticles, &mParticleTargetBuffer);
    DxHelp::CreateCPUwriteGPUreadStructuredBuffer<MetaData>(mpDevice, 1, &mMetaDataBuffer);

    ID3DBlob* errorBlob = nullptr;
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(L"Particles_Update_CS.hlsl", nullptr, nullptr, "main", "cs_5_0", 0, NULL, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        char* errorMsg = (char*)errorBlob->GetBufferPointer();
        OutputDebugStringA(errorMsg);
        shaderBlob->Release();
    }
    DxAssert(mpDevice->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &mParticleCS), S_OK);
    shaderBlob->Release();
}
