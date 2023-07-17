#include "Rxn.h"
#include "Shape.h"

namespace Rxn::Graphics::Basic
{
    Shape::Shape()
        : m_VertexIndexBufferSize(0)
        , m_IndexDataSize(0)
        , m_VertexDataSize(0)
        //, m_QuadsSize(0)
    {

    }

    Shape::~Shape() = default;

    void Shape::ReadDataFromRaw(std::vector<VertexPositionColour> &vertices, std::vector<UINT> &indices)
    {

        m_Vertices = vertices;
        m_Indices = indices;

        CalculateBufferInfo(vertices, m_VertexStrideSize, m_VertexDataSize);
        CalculateBufferInfo(indices, m_IndexStrideSize, m_IndexDataSize);

        //UINT total = 0;
        //for (auto &vert : m_Vertices)
        //{
        //    UINT size = sizeof(vert);
        //    total += size;
        //    m_DataSize += size;
        //}

        //for (auto &index : m_Indices)
        //{
        //    UINT size = sizeof(index);
        //    total += size;
        //    m_IndexSize += size;
        //}


        m_VertexIndexBufferSize = m_VertexDataSize + m_IndexDataSize;
    }

    HRESULT Shape::LoadFromFile(const wchar_t *filename)
    {
        return E_NOTIMPL;
    }

    HRESULT Shape::UploadGpuResources(ID3D12Device *device, ID3D12CommandQueue *cmdQueue, ID3D12CommandAllocator *cmdAlloc, ID3D12GraphicsCommandList *cmdList)
    {

        /* const auto defaultBuf = CD3DX12_RESOURCE_DESC::Buffer(m_VertexIndexBufferSize);
         const auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
         ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &defaultBuf, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_VertexIndexBuffer)));
         */

        Renderable::CreateCommittedBufferDestinationResource(device, m_VertexIndexBuffer, m_VertexIndexBufferSize);

        /* const auto uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
         const auto uploadBuf = CD3DX12_RESOURCE_DESC::Buffer(m_VertexIndexBufferSize);

         ThrowIfFailed(device->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadBuf, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_VertexIndexUpload)));*/

        Renderable::CreateCommittedUploadBufferResource(device, m_VertexIndexUpload, m_VertexIndexBufferSize);

#ifdef _DEBUG
        NAME_D3D12_OBJECT(m_VertexIndexBuffer);
#endif

        //uint8_t *mem = nullptr;
        //CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        //ThrowIfFailed(m_VertexIndexUpload->Map(0, &readRange, reinterpret_cast<void **>(&mem)));

        //uint8_t *heap = static_cast<uint8_t *>(mem);

        uint8 *heap = Renderable::MapAndGetHeapLocationFromBuffer(m_VertexIndexUpload);
        /*for (auto &vert : m_Vertices)
        {
            std::memcpy(heap, &vert, sizeof(vert));
            heap += sizeof(vert);
        }*/

        Renderable::CopyDataToHeap(&heap, m_Vertices);

        /*for (auto index : m_Indices)
        {
            memcpy(heap, &index, sizeof(index));
            heap += sizeof(index);
        }*/

        Renderable::CopyDataToHeap(&heap, m_Indices);

        /*cmdList->CopyBufferRegion(m_VertexIndexBuffer.Get(), 0, m_VertexIndexUpload.Get(), 0, m_VertexIndexBufferSize);
        const auto transition = CD3DX12_RESOURCE_BARRIER::Transition(m_VertexIndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_INDEX_BUFFER);
        cmdList->ResourceBarrier(1, &transition);*/

        Renderable::AddCopyRegionTransition(cmdList, m_VertexIndexBuffer, m_VertexIndexBufferSize, m_VertexIndexUpload);

        m_VertexBufferView.BufferLocation = m_VertexIndexBuffer.Get()->GetGPUVirtualAddress();
        m_VertexBufferView.SizeInBytes = m_VertexDataSize;
        m_VertexBufferView.StrideInBytes = sizeof(VertexPositionColour);

        m_IndexBufferView.BufferLocation = m_VertexBufferView.BufferLocation + m_VertexDataSize;
        m_IndexBufferView.SizeInBytes = m_IndexDataSize;
        m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;

        return S_OK;
    }
}
