#include "Rxn.h"
#include "Renderable.h"

namespace Rxn::Graphics::Basic
{
    Renderable::Renderable()
    {
    }

    Renderable::~Renderable() = default;

    HRESULT Renderable::CreateCommittedBufferDestinationResource(ID3D12Device *device, ComPointer<ID3D12Resource> &buffer, const uint32 &size)
    {
        HRESULT result;

        const CD3DX12_RESOURCE_DESC resourceDescriptorBufferSize = CD3DX12_RESOURCE_DESC::Buffer(size);
        const CD3DX12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        result = device->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescriptorBufferSize, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&buffer));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create committed resource for default heap.");
            return result;
        }

        return S_OK;
    }

    HRESULT Renderable::CreateCommittedUploadBufferResource(ID3D12Device *device, ComPointer<ID3D12Resource> &buffer, const uint32 &size)
    {
        HRESULT result;

        const CD3DX12_RESOURCE_DESC resourceDescriptorBufferSize = CD3DX12_RESOURCE_DESC::Buffer(size);
        const CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        result = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescriptorBufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create committed resource for upload heap.");
            return result;
        }

        return S_OK;
    }

    uint8 *Renderable::MapAndGetHeapLocationFromBuffer(ComPointer<ID3D12Resource> &buffer)
    {
        uint8 *mem = nullptr;
        CD3DX12_RANGE readRange(0, 0);
        buffer->Map(0, &readRange, reinterpret_cast<void **>(&mem));
        return static_cast<uint8 *>(mem);
    }

    void Renderable::AddCopyRegionTransition(ID3D12GraphicsCommandList *cmdList, ComPointer<ID3D12Resource> &buffer, const uint32 &size, ComPointer<ID3D12Resource> &uploadBuffer)
    {
        cmdList->CopyBufferRegion(buffer.Get(), 0, uploadBuffer.Get(), 0, size);
        const auto transition = CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        cmdList->ResourceBarrier(1, &transition);
    }
}

