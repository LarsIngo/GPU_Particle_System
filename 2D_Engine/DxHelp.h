#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11.h>
#include <d3dcompiler.h>

#include "DxAssert.h"

namespace DxHelp 
{
    // Creates vertex buffer.
    // device D3D11 device.
    // numOfElements Maximum number of elements (size of buffer equals sizeOf(T) * numOfElements).
    // vBuffer Vertex buffer.
    template <typename T>
    void CreateVertexBuffer(ID3D11Device* device, unsigned int numOfElements, ID3D11Buffer** vBuffer);

    // Creates constant buffer.
    // device D3D11 device.
    // data Pointer to data.
    // cBuffer Constant buffer.
    template <typename T>
    void CreateConstantBuffer(ID3D11Device* device, T* data, ID3D11Buffer** cBuffer);

    // Creates source stuctured buffer (CPU write, GPU read).
    // device D3D11 device.
    // numOfElements Maximum number of elements (size of buffer equals sizeOf(T) * numOfElements).
    // buffer Structured buffer.
    template <typename T>
    void CreateCPUwriteGPUreadStructuredBuffer(ID3D11Device* device, unsigned int numOfElements, ID3D11ShaderResourceView** buffer);

    // Creates target stuctured buffer (CPU read, GPU write).
    // device D3D11 device.
    // numOfElements Maximum number of elements (size of buffer equals sizeOf(T) * numOfElements).
    // buffer Structured buffer.
    template <typename T>
    void CreateCPUreadGPUwriteStructuredBuffer(ID3D11Device* device, unsigned int numOfElements, ID3D11UnorderedAccessView** buffer);

    // Write stuctured data to the GPU.
    // deviceContext D3D11 device context.
    // data Pointer to array of data.
    // numOfElements Number of elements in array.
    // buffer Structured buffer.
    template <typename T>
    void WriteStructuredBuffer(ID3D11DeviceContext* deviceContext, T* data, unsigned int numOfElements, ID3D11ShaderResourceView* buffer);

    // Reads structured data from the GPU.
    // deviceContext D3D11 device context.
    // buffer Structured buffer.
    // Returns pointer to data.
    template <typename T>
    T* ReadStructuredBuffer(ID3D11DeviceContext* deviceContext, ID3D11UnorderedAccessView* buffer);

    // Write buffer data to the GPU.
    // deviceContext D3D11 device context.
    // data Pointer to array of data.
    // numOfElements Number of elements in array.
    // buffer Buffer to write.
    template <typename T>
    void WriteBuffer(ID3D11DeviceContext* deviceContext, T* data, unsigned int numOfElements, ID3D11Buffer* buffer);
}

template <typename T>
inline void DxHelp::CreateVertexBuffer(ID3D11Device* device, unsigned int numOfElements, ID3D11Buffer** vBuffer)
{
    D3D11_BUFFER_DESC buffDesc;
    ZeroMemory(&buffDesc, sizeof(D3D11_BUFFER_DESC));
    buffDesc.ByteWidth = sizeof(T) * numOfElements;
    buffDesc.Usage = D3D11_USAGE_DYNAMIC;
    buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    buffDesc.MiscFlags = 0;
    buffDesc.StructureByteStride = 0;
    DxAssert(device->CreateBuffer(&buffDesc, NULL, vBuffer), S_OK);
}

template <typename T>
inline void DxHelp::CreateConstantBuffer(ID3D11Device* device, T* data, ID3D11Buffer** cBuffer)
{
    D3D11_BUFFER_DESC cbDesc;
    ZeroMemory(&cbDesc, sizeof(D3D11_BUFFER_DESC));
    cbDesc.ByteWidth = sizeof(T);
    cbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = 0;
    cbDesc.MiscFlags = 0;
    cbDesc.StructureByteStride = sizeof(T);

    D3D11_SUBRESOURCE_DATA initData;
    ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
    initData.pSysMem = data;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;

    DxAssert((*mpDevice)->CreateBuffer(&cbDesc, &initData, cBuffer), S_OK);
}

template <typename T>
inline void DxHelp::CreateCPUwriteGPUreadStructuredBuffer(ID3D11Device* device, unsigned int numOfElements, ID3D11ShaderResourceView** buffer)
{
    ID3D11Buffer* localBuffer;
    D3D11_BUFFER_DESC bDesc;
    ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
    bDesc.ByteWidth = sizeof(T) * numOfElements;
    bDesc.Usage = D3D11_USAGE_DYNAMIC;
    bDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bDesc.StructureByteStride = sizeof(T);
    DxAssert(device->CreateBuffer(&bDesc, NULL, &localBuffer), S_OK);

    D3D11_SHADER_RESOURCE_VIEW_DESC srcDesc;
    ZeroMemory(&srcDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    srcDesc.Format = DXGI_FORMAT_UNKNOWN;
    srcDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srcDesc.Buffer.FirstElement = 0;
    srcDesc.Buffer.ElementOffset = 0;
    srcDesc.Buffer.NumElements = numOfElements;
    DxAssert(device->CreateShaderResourceView(localBuffer, &srcDesc, buffer), S_OK);

    localBuffer->Release();
}

template <typename T>
inline void DxHelp::CreateCPUreadGPUwriteStructuredBuffer(ID3D11Device* device, unsigned int numOfElements, ID3D11UnorderedAccessView** buffer)
{
    ID3D11Buffer* uavBuffer;
    D3D11_BUFFER_DESC bDesc;
    ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
    bDesc.ByteWidth = sizeof(T) * numOfElements;
    bDesc.Usage = D3D11_USAGE_DEFAULT;
    bDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    bDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bDesc.StructureByteStride = sizeof(T);
    DxAssert(device->CreateBuffer(&bDesc, NULL, &uavBuffer), S_OK);

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = numOfElements;
    uavDesc.Buffer.Flags = 0;
    DxAssert(device->CreateUnorderedAccessView(uavBuffer, &uavDesc, buffer), S_OK);

    uavBuffer->Release();
}

template <typename T>
inline void DxHelp::WriteStructuredBuffer(ID3D11DeviceContext* deviceContext, T* data, unsigned int numOfElements, ID3D11ShaderResourceView* buffer)
{
    ID3D11Resource* res;
    buffer->GetResource(&res);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

    DxAssert(deviceContext->Map(res, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedResource), S_OK);
    memcpy(mappedResource.pData, data, sizeof(T) * numOfElements);
    deviceContext->Unmap(res, 0);
    res->Release();
}

template <typename T>
inline T* DxHelp::ReadStructuredBuffer(ID3D11DeviceContext* deviceContext, ID3D11UnorderedAccessView* buffer)
{
    ID3D11Resource* res;
    buffer->GetResource(&res);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

    DxAssert(deviceContext->Map(res, 0, D3D11_MAP_READ, 0, &mappedResource), S_OK);
    deviceContext->Unmap(res, 0);
    res->Release();

    return (T*)(mappedResource.pData);
}

template <typename T>
inline void DxHelp::WriteBuffer(ID3D11DeviceContext* deviceContext, T* data, unsigned int numOfElements, ID3D11Buffer* buffer)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

    DxAssert(deviceContext->Map(buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedResource), S_OK);
    memcpy(mappedResource.pData, data, sizeof(T) * numOfElements);
    deviceContext->Unmap(buffer, 0);
}
