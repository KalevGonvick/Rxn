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

        void SetSerializedRootSignature(const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC &rootSigDesc);
        
        /* -------------< temp functions >------------- */
        void AddShapeFromRaw(const std::vector<VertexPositionColour> &vertices, const std::vector<UINT> &indices, ID3D12CommandAllocator *cmdAl, ID3D12CommandQueue *cmdQueue, ID3D12GraphicsCommandList *cmdList);
        void AddQuadFromRaw(const std::vector<VertexPositionUV> &quadVertices, ID3D12CommandAllocator *cmdAl, ID3D12CommandQueue *cmdQueue, ID3D12GraphicsCommandList *cmdList);
        void AddTexture(ComPointer<ID3D12Resource> &textureUploadHeap, const D3D12_RESOURCE_DESC &textureDesc, ID3D12GraphicsCommandList *cmdList, D3D12_SUBRESOURCE_DATA textureData);
        void InitHeaps();

        static void InitHeap(uint32 descriptorCount, ComPointer<ID3D12DescriptorHeap> heap, D3D12_DESCRIPTOR_HEAP_TYPE type);

        /**
         * .
         * 
         */
        void ReleaseResourceViews();

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
         * \param frameCmdList
         * \param frameIndex
         * \param drawIndex
         */
        void SetDynamicConstantBufferByIndex(ComPointer<ID3D12GraphicsCommandList> frameCmdList, const uint32 frameIndex, uint32 drawIndex);

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
        void DrawSceneShapes(const ComPointer<ID3D12GraphicsCommandList> &cmdList) const;

        /* Getters */
        Buffer::DynamicConstantBuffer & GetDynamicConstantBuffer();
        Camera & GetCamera();
        ComPointer<ID3D12RootSignature> & GetRootSignature();
        
        Basic::Quad & GetQuad();
        ComPointer<ID3D12Resource> & GetRenderTarget(const uint32 index);
        
        ComPointer<ID3D12DescriptorHeap> & GetRtvHeap();
        ComPointer<ID3D12DescriptorHeap> & GetSrvHeap();
        ComPointer<ID3D12Resource> & GetIntermediateRenderTarget();

        uint32 GetRtvDescriptorHeapSize() const;
        uint32 GetSrvDescriptorHeapSize() const;

       

    private:

        void CreateSrvForResource(const D3D12_RESOURCE_DESC &resourceDesc, ComPointer<ID3D12Resource> &resource);

    private:

        ComPointer<ID3D12Resource> m_Texture;
        Basic::Quad m_Quad;
        std::vector<std::shared_ptr<Basic::Shape>> m_SceneShapes;

        Camera m_Camera;
        Buffer::DynamicConstantBuffer m_DynamicConstantBuffer{sizeof(DrawConstantBuffer), 256, static_cast<uint32>(SwapChainBuffers::TOTAL_BUFFERS)};
        

        ComPointer<ID3D12Resource> m_RenderTargets[static_cast<uint32>(SwapChainBuffers::TOTAL_BUFFERS)];
        ComPointer<ID3D12DescriptorHeap> m_RTVHeap;
        uint32 m_RTVDescriptorSize = 0;

        ComPointer<ID3D12RootSignature> m_RootSignature;
        
        ComPointer<ID3D12Resource> m_IntermediateRenderTarget;
        
        ComPointer<ID3D12DescriptorHeap> m_SRVHeap;
        uint32 m_SRVDescriptorSize = 0;

    };
}
