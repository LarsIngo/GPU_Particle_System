#include "ParticleRenderer.h"

#include "DxAssert.h"
#include "DxHelp.h"

ParticleRenderer::ParticleRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int maxNumParticles)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;

    // Create pipeline.
    Initialise();

    // Create meta data buffer.
    DxHelp::CreateCPUwriteGPUreadStructuredBuffer<MetaData>(mpDevice, 1, &mMetaDataBuffer);
}

ParticleRenderer::~ParticleRenderer()
{
    mInputLayout->Release();
    mVertexShader->Release();
    mGeometryShader->Release();
    mPixelShader->Release();
    mSamplerState->Release();
    mBlendState->Release();
    mDepthSencilState->Release();
    mRasterizerState->Release();

    mMetaDataBuffer->Release();
}

void ParticleRenderer::Bind(ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView)
{
    mpDeviceContext->VSSetShader(mVertexShader, NULL, NULL);
    mpDeviceContext->GSSetShader(mGeometryShader, NULL, NULL);
    mpDeviceContext->PSSetShader(mPixelShader, NULL, NULL);
    mpDeviceContext->PSSetSamplers(0, 1, &mSamplerState);
    float blendFactor[] = { 0.f, 0.f, 0.f, 0.f };
    UINT sampleMask = 0xffffffff;
    mpDeviceContext->OMSetBlendState(mBlendState, blendFactor, sampleMask);
    mpDeviceContext->OMSetDepthStencilState(mDepthSencilState, NULL);
    mpDeviceContext->RSSetState(mRasterizerState);
    mpDeviceContext->IASetInputLayout(mInputLayout);
    mpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    mpDeviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
}

void ParticleRenderer::Unbind()
{
    mpDeviceContext->VSSetShader(nullptr, NULL, NULL);
    mpDeviceContext->GSSetShader(nullptr, NULL, NULL);
    mpDeviceContext->PSSetShader(nullptr, NULL, NULL);
    void *const p[1] = { NULL };
    mpDeviceContext->PSSetSamplers(0, 1, (ID3D11SamplerState**)p);
    mpDeviceContext->OMSetBlendState(NULL, NULL, NULL);
    mpDeviceContext->OMSetDepthStencilState(NULL, NULL);
    mpDeviceContext->RSSetState(NULL);
    mpDeviceContext->IASetInputLayout(NULL);
    mpDeviceContext->GSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
    UINT vbStride = sizeof(Particle);
    UINT vbOffset = 0;
    mpDeviceContext->IASetVertexBuffers(0, 1, (ID3D11Buffer**)p, &vbStride, &vbOffset);
    mpDeviceContext->OMSetRenderTargets(1, (ID3D11RenderTargetView**)p, nullptr);
}

void ParticleRenderer::Render(const glm::mat4& vpMatix, const glm::vec3& lensPostion, Scene& scene)
{
    // Update buffers.
    mMetaData.vpMatrix = vpMatix;
    mMetaData.lensPosition = lensPostion;
    DxHelp::WriteStructuredBuffer<MetaData>(mpDeviceContext, &mMetaData, 1, mMetaDataBuffer);

    // Bind buffers.
    UINT vbStride = sizeof(Particle);
    UINT vbOffset = 0;

    scene.mParticlesGPUSwapBuffer->UpdateVertexBuffer();
    ID3D11Buffer* vBuffer = scene.mParticlesGPUSwapBuffer->GetVertexBuffer();
    mpDeviceContext->IASetVertexBuffers(0, 1, &vBuffer, &vbStride, &vbOffset);
    mpDeviceContext->GSSetShaderResources(0, 1, &mMetaDataBuffer);

    // Draw particles.
    mpDeviceContext->Draw(scene.mActiveNumParticles, 0);
}

void ParticleRenderer::Initialise()
{
    // Create pipeline.
    {
        // Create vertex shader & input layput.
        {
            D3D11_INPUT_ELEMENT_DESC inputDesc[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "SCALE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "LIFETIME", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };

            ID3DBlob* compiledShader = nullptr;
            ID3DBlob* errorBlob = nullptr;
            HRESULT hr = D3DCompileFromFile(
                L"Particles_Render_VS.hlsl",
                nullptr,
                nullptr,
                "main",
                "vs_5_0",
                0,
                0,
                &compiledShader,
                &errorBlob
            );
            if (FAILED(hr)) {
                char* errorMsg = (char*)errorBlob->GetBufferPointer();
                OutputDebugStringA(errorMsg);
                errorBlob->Release();
            }

            DxAssert(mpDevice->CreateVertexShader(
                compiledShader->GetBufferPointer(),
                compiledShader->GetBufferSize(),
                NULL,
                &mVertexShader
            ), S_OK);

            int inputLayoutSize = sizeof(inputDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);
            mpDevice->CreateInputLayout(
                inputDesc,
                inputLayoutSize,
                compiledShader->GetBufferPointer(),
                compiledShader->GetBufferSize(),
                &mInputLayout
            );
            compiledShader->Release();
        }

        // Create geometry shader.
        {
        ID3DBlob* compiledShader = nullptr;
        ID3DBlob* errorBlob = nullptr;
        HRESULT hr = D3DCompileFromFile(
            L"Particles_Render_GS.hlsl",
            nullptr,
            nullptr,
            "main",
            "gs_5_0",
            0,
            0,
            &compiledShader,
            &errorBlob
        );
        if (FAILED(hr)) {
            char* errorMsg = (char*)errorBlob->GetBufferPointer();
            OutputDebugStringA(errorMsg);
            errorBlob->Release();
        }

        DxAssert(mpDevice->CreateGeometryShader(
            compiledShader->GetBufferPointer(),
            compiledShader->GetBufferSize(),
            NULL,
            &mGeometryShader
        ), S_OK);
        compiledShader->Release();
        }

        // Create pixel shader.
        {
            ID3DBlob* compiledShader = nullptr;
            ID3DBlob* errorBlob = nullptr;
            HRESULT hr = D3DCompileFromFile(
                L"Particles_Render_PS.hlsl",
                nullptr,
                nullptr,
                "main",
                "ps_5_0",
                0,
                0,
                &compiledShader,
                &errorBlob
            );
            if (FAILED(hr)) {
                char* errorMsg = (char*)errorBlob->GetBufferPointer();
                OutputDebugStringA(errorMsg);
                errorBlob->Release();
            }

            DxAssert(mpDevice->CreatePixelShader(
                compiledShader->GetBufferPointer(),
                compiledShader->GetBufferSize(),
                NULL,
                &mPixelShader
            ), S_OK);
            compiledShader->Release();
        }
    }

    // Create sampler state.
    {
        D3D11_SAMPLER_DESC sampDesc;
        ZeroMemory(&sampDesc, sizeof(D3D11_SAMPLER_DESC));
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.MipLODBias = 0.f;
        sampDesc.MaxAnisotropy = 1;
        sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampDesc.BorderColor[0] = 1.f;
        sampDesc.BorderColor[1] = 1.f;
        sampDesc.BorderColor[2] = 1.f;
        sampDesc.BorderColor[3] = 1.f;
        sampDesc.MinLOD = -D3D11_FLOAT32_MAX;
        sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

        DxAssert(mpDevice->CreateSamplerState(&sampDesc, &mSamplerState), S_OK);
    }

    // Create blend state.
    {
        D3D11_BLEND_DESC blendDesc;
        ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
        blendDesc.AlphaToCoverageEnable = false;
        blendDesc.IndependentBlendEnable = true;

        blendDesc.RenderTarget[0].BlendEnable = true;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        DxAssert(mpDevice->CreateBlendState(&blendDesc, &mBlendState), S_OK);
    }

    // Create depth sencil state.
    {
        D3D11_DEPTH_STENCIL_DESC dsDesc;
        ZeroMemory(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
        dsDesc.DepthEnable = true;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
        dsDesc.StencilEnable = false;
        dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
        dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

        DxAssert(mpDevice->CreateDepthStencilState(&dsDesc, &mDepthSencilState), S_OK);
    }

    // Create rasterizer state.
    {
        D3D11_RASTERIZER_DESC rDesc;
        ZeroMemory(&rDesc, sizeof(D3D11_RASTERIZER_DESC));
        rDesc.FillMode = D3D11_FILL_SOLID;
        rDesc.CullMode = D3D11_CULL_BACK;
        rDesc.FrontCounterClockwise = false;
        rDesc.DepthBias = 0;
        rDesc.SlopeScaledDepthBias = 0.0f;
        rDesc.DepthBiasClamp = 0.0f;
        rDesc.DepthClipEnable = true;
        rDesc.ScissorEnable = false;
        rDesc.MultisampleEnable = false;
        rDesc.AntialiasedLineEnable = false;

        DxAssert(mpDevice->CreateRasterizerState(&rDesc, &mRasterizerState), S_OK);
    }
}
