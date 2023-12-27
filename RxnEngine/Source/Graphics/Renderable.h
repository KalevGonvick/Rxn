#pragma once

namespace Rxn::Graphics::Basic
{
    class RXN_ENGINE_API Renderable
    {
    public:

        Renderable();
        ~Renderable();

    public:

        virtual HRESULT LoadFromFile(const wchar_t *filename) = 0;
        virtual HRESULT UploadGpuResources(ComPointer<ID3D12Device8> device, ComPointer<ID3D12CommandQueue> cmdQueue, ComPointer<ID3D12CommandAllocator> cmdAlloc, ComPointer<ID3D12GraphicsCommandList6> cmdList) = 0;
        virtual void DrawInstanced(ComPointer<ID3D12GraphicsCommandList6> frameCmdList, uint32 instanceCount) = 0;


        template<typename T>
        void CopyDataToHeap(uint8 **heap, const std::vector<T> &data)
        {
            for (auto &datum : data)
            {
                memcpy(*heap, &datum, sizeof(datum));
                *heap += sizeof(datum);
            }
        }

        template<typename T>
        static void CalculateBufferInfo(const std::vector<T> &data, uint32 &stride, uint32 &size)
        {
            stride = sizeof(T);
            for (auto &datum : data)
            {
                size += sizeof(datum);
            }
        }

    protected:

        HRESULT CreateCommittedBufferDestinationResource(ComPointer<ID3D12Device8> device, ComPointer<ID3D12Resource> &buffer, const uint32 &size) const;
        HRESULT CreateCommittedUploadBufferResource(ComPointer<ID3D12Device8> device, ComPointer<ID3D12Resource> &buffer, const uint32 &size) const;

        void AddCopyRegionTransition(ComPointer<ID3D12GraphicsCommandList6> cmdList, ComPointer<ID3D12Resource> &buffer, const uint32 &size, ComPointer<ID3D12Resource> &uploadBuffer) const;

        uint8 *MapAndGetHeapLocationFromBuffer(ComPointer<ID3D12Resource> &buffer);

    };
}
