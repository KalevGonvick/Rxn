//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#include "Rxn.h"
#include "Model.h"
#include "Core/Span.h"

#include <fstream>
#include <unordered_set>

namespace Rxn::Graphics::Basic
{
    const D3D12_INPUT_ELEMENT_DESC c_elementDescs[Attribute::Count] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
    };

    const uint32_t c_sizeMap[] =
    {
        12, // Position
        12, // Normal
        8,  // TexCoord
        12, // Tangent
        12, // Bitangent
    };

    const uint32_t c_prolog = 'MSHL';

    enum FileVersion
    {
        FILE_VERSION_INITIAL = 0,
        CURRENT_FILE_VERSION = FILE_VERSION_INITIAL
    };

    struct FileHeader
    {
        unsigned int Prolog;
        unsigned int Version;

        unsigned int MeshCount;
        unsigned int AccessorCount;
        unsigned int BufferViewCount;
        unsigned int BufferSize;
    };

    struct MeshHeader
    {
        unsigned int Indices;
        unsigned int IndexSubsets;
        unsigned int Attributes[Attribute::Count];

        unsigned int Meshlets;
        unsigned int MeshletSubsets;
        unsigned int UniqueVertexIndices;
        unsigned int PrimitiveIndices;
        unsigned int CullData;
    };

    struct BufferView
    {
        unsigned int Offset;
        unsigned int Size;
    };

    struct Accessor
    {
        unsigned int BufferView;
        unsigned int Offset;
        unsigned int Size;
        unsigned int Stride;
        unsigned int Count;
    };

    unsigned int GetFormatSize(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
        case DXGI_FORMAT_R32G32B32_FLOAT: return 12;
        case DXGI_FORMAT_R32G32_FLOAT: return 8;
        case DXGI_FORMAT_R32_FLOAT: return 4;
        default: throw std::exception("Unimplemented type");
        }
    }

    template <typename T, typename U>
    constexpr T DivRoundUp(T num, U denom)
    {
        return (num + denom - 1) / denom;
    }

    template <typename T>
    size_t GetAlignedSize(T size)
    {
        const size_t alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
        const size_t alignedSize = (size + alignment - 1) & ~(alignment - 1);
        return alignedSize;
    }

    HRESULT Model::LoadFromFile(const wchar_t *filename)
    {
        std::ifstream stream(filename, std::ios::binary);
        if (!stream.is_open())
        {
            return E_INVALIDARG;
        }

        std::vector<MeshHeader> meshes;
        std::vector<BufferView> bufferViews;
        std::vector<Accessor> accessors;

        FileHeader header;
        stream.read(reinterpret_cast<char *>(&header), sizeof(header));

        if (header.Prolog != c_prolog)
        {
            return E_FAIL; // Incorrect file format.
        }

        if (header.Version != CURRENT_FILE_VERSION)
        {
            return E_FAIL; // Version mismatch between export and import serialization code.
        }

        // Read mesh metdata
        meshes.resize(header.MeshCount);
        stream.read(reinterpret_cast<char *>(meshes.data()), meshes.size() * sizeof(meshes[0]));

        accessors.resize(header.AccessorCount);
        stream.read(reinterpret_cast<char *>(accessors.data()), accessors.size() * sizeof(accessors[0]));

        bufferViews.resize(header.BufferViewCount);
        stream.read(reinterpret_cast<char *>(bufferViews.data()), bufferViews.size() * sizeof(bufferViews[0]));

        m_buffer.resize(header.BufferSize);
        stream.read(reinterpret_cast<char *>(m_buffer.data()), header.BufferSize);

        char eofbyte;
        stream.read(&eofbyte, 1); // Read last byte to hit the eof bit

        assert(stream.eof()); // There's a problem if we didn't completely consume the file contents.

        stream.close();

        // Populate mesh data from binary data and metadata.
        m_meshes.resize(meshes.size());
        for (uint32_t i = 0; i < static_cast<uint32_t>(meshes.size()); ++i)
        {
            auto const &meshView = meshes[i];
            auto &mesh = m_meshes[i];

            // Index data
            {
                const Accessor &accessor = accessors[meshView.Indices];
                const BufferView &bufferView = bufferViews[accessor.BufferView];

                mesh.IndexSize = accessor.Size;
                mesh.IndexCount = accessor.Count;

                mesh.Indices = Core::MakeSpan(m_buffer.data() + bufferView.Offset, bufferView.Size);
            }

            // Index Subset data
            {
                const Accessor &accessor = accessors[meshView.IndexSubsets];
                const BufferView &bufferView = bufferViews[accessor.BufferView];

                mesh.IndexSubsets = Core::MakeSpan(reinterpret_cast<Subset *>(m_buffer.data() + bufferView.Offset), accessor.Count);
            }

            // Vertex data & layout metadata

            // Determine the number of unique Buffer Views associated with the vertex attributes & copy vertex buffers.
            std::vector<uint32_t> vbMap;

            mesh.LayoutDesc.pInputElementDescs = mesh.LayoutElems;
            mesh.LayoutDesc.NumElements = 0;

            for (uint32_t j = 0; j < Attribute::Count; ++j)
            {
                if (meshView.Attributes[j] == -1)
                    continue;

                const Accessor &accessor = accessors[meshView.Attributes[j]];

                auto it = std::find(vbMap.begin(), vbMap.end(), accessor.BufferView);
                if (it != vbMap.end())
                {
                    continue; // Already added - continue.
                }

                // New buffer view encountered; add to list and copy vertex data
                vbMap.push_back(accessor.BufferView);
                const BufferView &bufferView = bufferViews[accessor.BufferView];

                Core::Span<uint8_t> verts = Core::MakeSpan(m_buffer.data() + bufferView.Offset, bufferView.Size);

                mesh.VertexStrides.push_back(accessor.Stride);
                mesh.Vertices.push_back(verts);
                mesh.VertexCount = static_cast<uint32_t>(verts.size()) / accessor.Stride;
            }

            // Populate the vertex buffer metadata from accessors.
            for (uint32_t j = 0; j < Attribute::Count; ++j)
            {
                if (meshView.Attributes[j] == -1)
                    continue;

                const Accessor &accessor = accessors[meshView.Attributes[j]];

                // Determine which vertex buffer index holds this attribute's data
                auto it = std::find(vbMap.begin(), vbMap.end(), accessor.BufferView);

                D3D12_INPUT_ELEMENT_DESC desc = c_elementDescs[j];
                desc.InputSlot = static_cast<uint32_t>(std::distance(vbMap.begin(), it));

                mesh.LayoutElems[mesh.LayoutDesc.NumElements++] = desc;
            }

            // Meshlet data
            {
                const Accessor &accessor = accessors[meshView.Meshlets];
                const BufferView &bufferView = bufferViews[accessor.BufferView];

                mesh.Meshlets = Core::MakeSpan(reinterpret_cast<Meshlet *>(m_buffer.data() + bufferView.Offset), accessor.Count);
            }

            // Meshlet Subset data
            {
                const Accessor &accessor = accessors[meshView.MeshletSubsets];
                const BufferView &bufferView = bufferViews[accessor.BufferView];

                mesh.MeshletSubsets = Core::MakeSpan(reinterpret_cast<Subset *>(m_buffer.data() + bufferView.Offset), accessor.Count);
            }

            // Unique Vertex Index data
            {
                const Accessor &accessor = accessors[meshView.UniqueVertexIndices];
                const BufferView &bufferView = bufferViews[accessor.BufferView];

                mesh.UniqueVertexIndices = Core::MakeSpan(m_buffer.data() + bufferView.Offset, bufferView.Size);
            }

            // Primitive Index data
            {
                const Accessor &accessor = accessors[meshView.PrimitiveIndices];
                const BufferView &bufferView = bufferViews[accessor.BufferView];

                mesh.PrimitiveIndices = Core::MakeSpan(reinterpret_cast<PackedTriangle *>(m_buffer.data() + bufferView.Offset), accessor.Count);
            }

            // Cull data
            {
                const Accessor &accessor = accessors[meshView.CullData];
                const BufferView &bufferView = bufferViews[accessor.BufferView];

                mesh.CullingData = Core::MakeSpan(reinterpret_cast<CullData *>(m_buffer.data() + bufferView.Offset), accessor.Count);
            }
        }

        // Build bounding spheres for each mesh
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_meshes.size()); ++i)
        {
            auto &m = m_meshes[i];

            uint32_t vbIndexPos = 0;

            // Find the index of the vertex buffer of the position attribute
            for (uint32_t j = 1; j < m.LayoutDesc.NumElements; ++j)
            {
                auto const &desc = m.LayoutElems[j];
                if (strcmp(desc.SemanticName, "POSITION") == 0)
                {
                    vbIndexPos = j;
                    break;
                }
            }

            // Find the byte offset of the position attribute with its vertex buffer
            uint32_t positionOffset = 0;

            for (uint32_t j = 0; j < m.LayoutDesc.NumElements; ++j)
            {
                auto const &desc = m.LayoutElems[j];
                if (strcmp(desc.SemanticName, "POSITION") == 0)
                {
                    break;
                }

                if (desc.InputSlot == vbIndexPos)
                {
                    positionOffset += GetFormatSize(m.LayoutElems[j].Format);
                }
            }

            auto *v0 = reinterpret_cast<DirectX::XMFLOAT3 *>(m.Vertices[vbIndexPos].data() + positionOffset);
            uint32_t stride = m.VertexStrides[vbIndexPos];

            DirectX::BoundingSphere::CreateFromPoints(m.BoundingSphere, m.VertexCount, v0, stride);

            if (i == 0)
            {
                m_boundingSphere = m.BoundingSphere;
            }
            else
            {
                DirectX::BoundingSphere::CreateMerged(m_boundingSphere, m_boundingSphere, m.BoundingSphere);
            }
        }

        return S_OK;
    }

    HRESULT Model::UploadGpuResources(ComPointer<ID3D12Device8> device, ComPointer<ID3D12CommandQueue> cmdQueue, ComPointer<ID3D12CommandAllocator> cmdAlloc, ComPointer<ID3D12GraphicsCommandList6> cmdList)
    {
        for (uint32_t i = 0; i < m_meshes.size(); ++i)
        {
            auto &m = m_meshes[i];

            // Create committed D3D resources of proper sizes
            auto indexDesc = CD3DX12_RESOURCE_DESC::Buffer(m.Indices.size());
            auto meshletDesc = CD3DX12_RESOURCE_DESC::Buffer(m.Meshlets.size() * sizeof(m.Meshlets[0]));
            auto cullDataDesc = CD3DX12_RESOURCE_DESC::Buffer(m.CullingData.size() * sizeof(m.CullingData[0]));
            auto vertexIndexDesc = CD3DX12_RESOURCE_DESC::Buffer(DivRoundUp(m.UniqueVertexIndices.size(), 4) * 4);
            auto primitiveDesc = CD3DX12_RESOURCE_DESC::Buffer(m.PrimitiveIndices.size() * sizeof(m.PrimitiveIndices[0]));
            auto meshInfoDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(MeshInfo));

            auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &indexDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m.IndexResource)));
            ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &meshletDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m.MeshletResource)));
            ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &cullDataDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m.CullDataResource)));
            ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &vertexIndexDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m.UniqueVertexIndexResource)));
            ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &primitiveDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m.PrimitiveIndexResource)));
            ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &meshInfoDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m.MeshInfoResource)));


            m.IBView.BufferLocation = m.IndexResource->GetGPUVirtualAddress();
            m.IBView.Format = m.IndexSize == 4 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
            m.IBView.SizeInBytes = m.IndexCount * m.IndexSize;

            m.VertexResources.resize(m.Vertices.size());
            m.VBViews.resize(m.Vertices.size());

            for (uint32_t j = 0; j < m.Vertices.size(); ++j)
            {
                auto vertexDesc = CD3DX12_RESOURCE_DESC::Buffer(m.Vertices[j].size());
                device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &vertexDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m.VertexResources[j]));

                m.VBViews[j].BufferLocation = m.VertexResources[j]->GetGPUVirtualAddress();
                m.VBViews[j].SizeInBytes = static_cast<uint32_t>(m.Vertices[j].size());
                m.VBViews[j].StrideInBytes = m.VertexStrides[j];
            }

            // Create upload resources
            std::vector<ComPointer<ID3D12Resource>> vertexUploads;
            ComPointer<ID3D12Resource>              indexUpload;
            ComPointer<ID3D12Resource>              meshletUpload;
            ComPointer<ID3D12Resource>              cullDataUpload;
            ComPointer<ID3D12Resource>              uniqueVertexIndexUpload;
            ComPointer<ID3D12Resource>              primitiveIndexUpload;
            ComPointer<ID3D12Resource>              meshInfoUpload;

            auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &indexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexUpload)));
            ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &meshletDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&meshletUpload)));
            ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &cullDataDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&cullDataUpload)));
            ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &vertexIndexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uniqueVertexIndexUpload)));
            ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &primitiveDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&primitiveIndexUpload)));
            ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &meshInfoDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&meshInfoUpload)));

            // Map & copy memory to upload heap
            vertexUploads.resize(m.Vertices.size());
            for (uint32_t j = 0; j < m.Vertices.size(); ++j)
            {
                auto vertexDesc = CD3DX12_RESOURCE_DESC::Buffer(m.Vertices[j].size());
                ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &vertexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexUploads[j])));

                uint8_t *memory = nullptr;
                vertexUploads[j]->Map(0, nullptr, reinterpret_cast<void **>(&memory));
                std::memcpy(memory, m.Vertices[j].data(), m.Vertices[j].size());
                vertexUploads[j]->Unmap(0, nullptr);
            }

            {
                uint8_t *memory = nullptr;
                indexUpload->Map(0, nullptr, reinterpret_cast<void **>(&memory));
                std::memcpy(memory, m.Indices.data(), m.Indices.size());
                indexUpload->Unmap(0, nullptr);
            }

            {
                uint8_t *memory = nullptr;
                meshletUpload->Map(0, nullptr, reinterpret_cast<void **>(&memory));
                std::memcpy(memory, m.Meshlets.data(), m.Meshlets.size() * sizeof(m.Meshlets[0]));
                meshletUpload->Unmap(0, nullptr);
            }

            {
                uint8_t *memory = nullptr;
                cullDataUpload->Map(0, nullptr, reinterpret_cast<void **>(&memory));
                std::memcpy(memory, m.CullingData.data(), m.CullingData.size() * sizeof(m.CullingData[0]));
                cullDataUpload->Unmap(0, nullptr);
            }

            {
                uint8_t *memory = nullptr;
                uniqueVertexIndexUpload->Map(0, nullptr, reinterpret_cast<void **>(&memory));
                std::memcpy(memory, m.UniqueVertexIndices.data(), m.UniqueVertexIndices.size());
                uniqueVertexIndexUpload->Unmap(0, nullptr);
            }

            {
                uint8_t *memory = nullptr;
                primitiveIndexUpload->Map(0, nullptr, reinterpret_cast<void **>(&memory));
                std::memcpy(memory, m.PrimitiveIndices.data(), m.PrimitiveIndices.size() * sizeof(m.PrimitiveIndices[0]));
                primitiveIndexUpload->Unmap(0, nullptr);
            }

            {
                MeshInfo info = {};
                info.IndexSize = m.IndexSize;
                info.MeshletCount = static_cast<uint32_t>(m.Meshlets.size());
                info.LastMeshletVertCount = m.Meshlets.back().VertCount;
                info.LastMeshletPrimCount = m.Meshlets.back().PrimCount;


                uint8_t *memory = nullptr;
                meshInfoUpload->Map(0, nullptr, reinterpret_cast<void **>(&memory));
                std::memcpy(memory, &info, sizeof(MeshInfo));
                meshInfoUpload->Unmap(0, nullptr);
            }

            // Populate our command list
            cmdList->Reset(cmdAlloc, nullptr);

            for (uint32_t j = 0; j < m.Vertices.size(); ++j)
            {
                cmdList->CopyResource(m.VertexResources[j].Get(), vertexUploads[j].Get());
                const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m.VertexResources[j].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                cmdList->ResourceBarrier(1, &barrier);
            }

            D3D12_RESOURCE_BARRIER postCopyBarriers[6];

            cmdList->CopyResource(m.IndexResource.Get(), indexUpload.Get());
            postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m.IndexResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            cmdList->CopyResource(m.MeshletResource.Get(), meshletUpload.Get());
            postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m.MeshletResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            cmdList->CopyResource(m.CullDataResource.Get(), cullDataUpload.Get());
            postCopyBarriers[2] = CD3DX12_RESOURCE_BARRIER::Transition(m.CullDataResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            cmdList->CopyResource(m.UniqueVertexIndexResource.Get(), uniqueVertexIndexUpload.Get());
            postCopyBarriers[3] = CD3DX12_RESOURCE_BARRIER::Transition(m.UniqueVertexIndexResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            cmdList->CopyResource(m.PrimitiveIndexResource.Get(), primitiveIndexUpload.Get());
            postCopyBarriers[4] = CD3DX12_RESOURCE_BARRIER::Transition(m.PrimitiveIndexResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            cmdList->CopyResource(m.MeshInfoResource.Get(), meshInfoUpload.Get());
            postCopyBarriers[5] = CD3DX12_RESOURCE_BARRIER::Transition(m.MeshInfoResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

            cmdList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);

            ThrowIfFailed(cmdList->Close());

            ID3D12CommandList *ppCommandLists[] = { cmdList };
            cmdQueue->ExecuteCommandLists(1, ppCommandLists);

            // Create our sync fence
            ComPointer<ID3D12Fence> fence;
            ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

            cmdQueue->Signal(fence.Get(), 1);

            // Wait for GPU
            if (fence->GetCompletedValue() != 1)
            {
                HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
                fence->SetEventOnCompletion(1, event);

                WaitForSingleObjectEx(event, INFINITE, false);
                CloseHandle(event);
            }
        }

        return S_OK;
    }
}


