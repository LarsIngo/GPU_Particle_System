#include "ParticleSorter.h"

#include "DxAssert.h"
#include "DxHelp.h"

ParticleSorter::ParticleSorter(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;

    // Create buffers.
    Initialise();
}

ParticleSorter::~ParticleSorter()
{
    mMetaDataBuffer->Release();
    mParticleSortCS->Release();
}

void ParticleSorter::Bind(ID3D11ShaderResourceView* sourceBuffer, ID3D11UnorderedAccessView* targetBuffer)
{
    mpDeviceContext->CSSetShader(mParticleSortCS, NULL, NULL);
    mpDeviceContext->CSSetShaderResources(0, 1, &sourceBuffer);
    mpDeviceContext->CSSetShaderResources(1, 1, &mMetaDataBuffer);
    mpDeviceContext->CSSetUnorderedAccessViews(0, 1, &targetBuffer, NULL);
}

void ParticleSorter::Unbind()
{
    mpDeviceContext->CSSetShader(NULL, NULL, NULL);
    void *const p[1] = { NULL };
    mpDeviceContext->CSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->CSSetShaderResources(1, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->CSSetUnorderedAccessViews(0, 1, (ID3D11UnorderedAccessView**)p, NULL);
}

void ParticleSorter::Sort(Scene& scene)
{
    // Swap buffers.
    scene.mParticlesGPUSwapBuffer->Swap();

    unsigned int numPartices = scene.mMaxNumParticles;

    Bind(scene.mParticlesGPUSwapBuffer->GetSourceBuffer(), scene.mParticlesGPUSwapBuffer->GetTargetBuffer());

    mpDeviceContext->Dispatch(numPartices / 256, 1, 1);

    Unbind();
}

void ParticleSorter::Initialise()
{
    // Create meta buffer.
    DxHelp::CreateCPUwriteGPUreadStructuredBuffer<MetaData>(mpDevice, 1, &mMetaDataBuffer);

    // Create compute shader.
    ID3DBlob* errorBlob = nullptr;
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(L"Particles_Sort_CS.hlsl", nullptr, nullptr, "main", "cs_5_0", 0, NULL, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        char* errorMsg = (char*)errorBlob->GetBufferPointer();
        OutputDebugStringA(errorMsg);
        shaderBlob->Release();
    }
    DxAssert(mpDevice->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &mParticleSortCS), S_OK);
    shaderBlob->Release();
}
