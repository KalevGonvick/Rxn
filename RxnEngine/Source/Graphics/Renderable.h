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
        virtual HRESULT UploadGpuResources(ID3D12Device *device, ID3D12CommandQueue *cmdQueue, ID3D12CommandAllocator *cmdAlloc, ID3D12GraphicsCommandList *cmdList) = 0;


        template<typename T>
        void CopyDataToHeap(uint8 **heap, std::vector<T> data)
        {
            for (auto &datum : data)
            {
                memcpy(*heap, &datum, sizeof(datum));
                *heap += sizeof(datum);
            }
        }

        template<typename T>
        static void CalculateBufferInfo(std::vector<T> data, uint32 &stride, uint32 &size)
        {
            stride = sizeof(T);
            for (auto &datum : data)
            {
                size += sizeof(datum);
            }
        }

    protected:

        HRESULT CreateCommittedBufferDestinationResource(ID3D12Device *device, ComPointer<ID3D12Resource> &buffer, const uint32 &size);
        HRESULT CreateCommittedUploadBufferResource(ID3D12Device *device, ComPointer<ID3D12Resource> &buffer, const uint32 &size);

        void AddCopyRegionTransition(ID3D12GraphicsCommandList *cmdList, ComPointer<ID3D12Resource> &buffer, const uint32 &size, ComPointer<ID3D12Resource> &uploadBuffer);

        uint8 *MapAndGetHeapLocationFromBuffer(ComPointer<ID3D12Resource> &buffer);

    };
}
