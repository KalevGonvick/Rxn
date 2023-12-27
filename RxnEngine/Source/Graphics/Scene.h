/*****************************************************************//**
 * \file   Scene.h
 * \brief  
 * 
 * \author kalev
 * \date   December 2023
 *********************************************************************/
#pragma once
#include "Camera.h"
#include "DynamicConstantBuffer.h"
#include "Shape.h"
#include "Quad.h"
#include "SwapChain.h"
#include "Display.h"
#include "Renderable.h"
#include "DescriptorHeapDesc.h"

namespace Rxn::Graphics
{
    class RXN_ENGINE_API Scene
    {
    public:

        Scene();
        ~Scene();

    public:
        
        /* -------------< temp functions >------------- */
        void AddShapeFromRaw(const std::vector<VertexPositionColour> &vertices, const std::vector<UINT> &indices, ComPointer<ID3D12CommandAllocator> cmdAl, ComPointer<ID3D12CommandQueue> cmdQueue, ComPointer<ID3D12GraphicsCommandList6> cmdList);
        void AddQuadFromRaw(const std::vector<VertexPositionUV> &quadVertices, ComPointer<ID3D12CommandAllocator> cmdAl, ComPointer<ID3D12CommandQueue> cmdQueue, ComPointer<ID3D12GraphicsCommandList6> cmdList);
        void AddTexture(ComPointer<ID3D12Resource> &textureUploadHeap, const D3D12_RESOURCE_DESC &textureDesc, ID3D12GraphicsCommandList *cmdList, D3D12_SUBRESOURCE_DATA textureData);
        void InitHeaps();

        static void InitHeap(uint32 descriptorCount, ComPointer<ID3D12DescriptorHeap> &heap, D3D12_DESCRIPTOR_HEAP_TYPE type);

        /**
         * .
         * 
         */
        void ReleaseResourceViews();

        void CreateSwapChainRenderTargetView(GPU::SwapChain &swapChain, uint32 swapChainBufferIndex, CD3DX12_CPU_DESCRIPTOR_HANDLE &rtvHandle);

        void CreateRenderTargetCopy(uint32 copyFromIndex, uint32 destinationIndex, CD3DX12_CPU_DESCRIPTOR_HANDLE &rtvHandle);
        void CreateShaderResourceViewForRenderTargetView(ID3D12Resource *pResource, uint32 mipLevel = 1);

        /**
         * .
         * 
         * \param frameIndex
         * \param swapChain
         */
        void InitSceneRenderTargets(const uint32 frameIndex, GPU::SwapChain &swapChain);

        /**
         * .
         * 
         * \param projMat
         * \param frameIndex
         * \param drawIndex
         */
        void UpdateConstantBufferByIndex(const DirectX::XMMATRIX & projMat, const uint32 frameIndex, const uint32 drawIndex);


        /**
         * .
         * 
         * \param cmdList
         */
        void DrawSceneShapes(const ComPointer<ID3D12GraphicsCommandList6> &cmdList) const;

        /* Getters */
        Buffer::DynamicConstantBuffer & GetDynamicConstantBuffer();
        Camera & GetCamera();
        
        Basic::Quad & GetQuad();
        ComPointer<ID3D12Resource> & GetRenderTarget(const uint32 index);
        
        ComPointer<ID3D12DescriptorHeap> & GetRtvHeap();
        ComPointer<ID3D12DescriptorHeap> & GetSrvHeap();

        uint32 GetRtvDescriptorHeapSize() const;
        uint32 GetSrvDescriptorHeapSize() const;

       

    private:

        void CreateSrvForResource(const D3D12_RESOURCE_DESC &resourceDesc, ComPointer<ID3D12Resource> &resource);
        D3D12_RESOURCE_DESC GetResourceForRenderTarget(uint32 rtvIndex);

    private:

        ComPointer<ID3D12Resource> m_Texture;
        Basic::Quad m_Quad;
        std::vector<std::shared_ptr<Basic::Shape>> m_SceneShapes;

        Camera m_Camera;
        Buffer::DynamicConstantBuffer m_DynamicConstantBuffer{sizeof(DrawConstantBuffer), 256, 2};
        ComPointer<ID3D12DescriptorHeap> m_RTVHeap;
        uint32 m_RTVDescriptorSize = 0;

        std::vector<std::pair<uint32, ComPointer<ID3D12Resource>>> m_Rtvs;
        
        ComPointer<ID3D12DescriptorHeap> m_SRVHeap;
        uint32 m_SRVDescriptorSize = 0;

    };
}
