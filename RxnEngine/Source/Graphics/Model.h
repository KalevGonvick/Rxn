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
#pragma once

#include "Renderable.h"
#include "Core/Span.h"
#include <DirectXCollision.h>
#include <algorithm>

namespace Rxn::Graphics::Basic
{
    struct Attribute
    {
        enum EType : uint32
        {
            Position,
            Normal,
            TexCoord,
            Tangent,
            Bitangent,
            Count
        };

        EType    Type;
        unsigned int Offset;
    };

    struct Subset
    {
        unsigned int Offset;
        unsigned int Count;
    };

    struct MeshInfo
    {
        unsigned int IndexSize;
        unsigned int MeshletCount;

        unsigned int LastMeshletVertCount;
        unsigned int LastMeshletPrimCount;
    };

    struct Meshlet
    {
        unsigned int VertCount;
        unsigned int VertOffset;
        unsigned int PrimCount;
        unsigned int PrimOffset;
    };

    struct PackedTriangle
    {
        unsigned int i0 : 10;
        unsigned int i1 : 10;
        unsigned int i2 : 10;
    };

    struct CullData
    {
        DirectX::XMFLOAT4 BoundingSphere; // xyz = center, w = radius
        unsigned char     NormalCone[4];  // xyz = axis, w = -cos(a + 90)
        float             ApexOffset;     // apex = center - axis * offset
    };

    struct Mesh
    {
        D3D12_INPUT_ELEMENT_DESC   LayoutElems[Attribute::Count];
        D3D12_INPUT_LAYOUT_DESC    LayoutDesc;

        std::vector<Core::Span<unsigned char>> Vertices;
        std::vector<unsigned int>      VertexStrides;
        unsigned int                   VertexCount;
        DirectX::BoundingSphere    BoundingSphere;

        Core::Span<Subset>               IndexSubsets;
        Core::Span<unsigned char>              Indices;
        unsigned int                   IndexSize;
        unsigned int                   IndexCount;

        Core::Span<Subset>               MeshletSubsets;
        Core::Span<Meshlet>              Meshlets;
        Core::Span<unsigned char>              UniqueVertexIndices;
        Core::Span<PackedTriangle>       PrimitiveIndices;
        Core::Span<CullData>             CullingData;

        // D3D resource references
        std::vector<D3D12_VERTEX_BUFFER_VIEW>  VBViews;
        D3D12_INDEX_BUFFER_VIEW                IBView;

        std::vector<ComPointer<ID3D12Resource>> VertexResources;
        ComPointer<ID3D12Resource>              IndexResource;
        ComPointer<ID3D12Resource>              MeshletResource;
        ComPointer<ID3D12Resource>              UniqueVertexIndexResource;
        ComPointer<ID3D12Resource>              PrimitiveIndexResource;
        ComPointer<ID3D12Resource>              CullDataResource;
        ComPointer<ID3D12Resource>              MeshInfoResource;

        // Calculates the number of instances of the last meshlet which can be packed into a single threadgroup.
        unsigned int GetLastMeshletPackCount(unsigned int subsetIndex, unsigned int maxGroupVerts, unsigned int maxGroupPrims)
        {
            if (Meshlets.size() == 0)
                return 0;

            const auto &subset = MeshletSubsets[subsetIndex];
            const auto &meshlet = Meshlets[subset.Offset + subset.Count - 1];

            return std::min(maxGroupVerts / meshlet.VertCount, maxGroupPrims / meshlet.PrimCount);
        }

        void GetPrimitive(unsigned int index, unsigned int &i0, unsigned int &i1, unsigned int &i2) const
        {
            auto &prim = PrimitiveIndices[index];
            i0 = prim.i0;
            i1 = prim.i1;
            i2 = prim.i2;
        }

        unsigned int GetVertexIndex(unsigned int index) const
        {
            const unsigned char *addr = UniqueVertexIndices.data() + index * IndexSize;
            if (IndexSize == 4)
            {
                return *std::bit_cast<const unsigned int *>(addr);
            }
            else
            {
                return *std::bit_cast<const unsigned short *>(addr);
            }
        }
    };

    class RXN_ENGINE_API Model : public Renderable
    {
    public:
        virtual HRESULT LoadFromFile(const wchar_t *filename) override;
        virtual HRESULT UploadGpuResources(ID3D12Device8 *pDevice, ID3D12GraphicsCommandList6 *pCmdList) override;

        unsigned int GetMeshCount() const { return static_cast<unsigned int>(m_meshes.size()); }
        const Mesh &GetMesh(unsigned int i) const { return m_meshes[i]; }

        const DirectX::BoundingSphere &GetBoundingSphere() const { return m_boundingSphere; }

        // Iterator interface
        auto begin() { return m_meshes.begin(); }
        auto end() { return m_meshes.end(); }

    private:
        std::vector<Mesh>                      m_meshes;
        DirectX::BoundingSphere                m_boundingSphere;

        std::vector<unsigned char>                   m_buffer;
    };
}

