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
#include "Display.h"
#include "PipelineLibrary.h"
#include "Renderable.h"
#include "DescriptorHeapDesc.h"

namespace Rxn::Graphics
{
    struct DrawConstantBuffer
    {
        DirectX::XMMATRIX worldViewProjection;
    };

    class RXN_ENGINE_API Scene
    {
    public:

        Scene();
        ~Scene();

    public:
        
        void Init(ID3D12Device8 *pDevice, GPU::SwapChain &swapChain, D3D_ROOT_SIGNATURE_VERSION version);
        void ReleaseResourceViews();
        void CreateSwapChainRenderTargetView(GPU::SwapChain &swapChain, uint32 swapChainBufferIndex, CD3DX12_CPU_DESCRIPTOR_HANDLE &rtvHandle);
        void CreateRenderTargetCopy(uint32 copyFromIndex, uint32 destinationIndex, CD3DX12_CPU_DESCRIPTOR_HANDLE &rtvHandle);
        void CreateShaderResourceViewForRenderTargetView(ID3D12Resource *pResource, uint32 mipLevel = 1);

        /* -------------< temp functions >------------- */
        void AddShapeFromRaw(ID3D12Device8 *pDevice, const std::vector<VertexPositionColour> &vertices, const std::vector<UINT> &indices, ID3D12GraphicsCommandList6 *cmdList);
        void AddQuadFromRaw(ID3D12Device8 *pDevice, const std::vector<VertexPositionUV> &quadVertices, ID3D12GraphicsCommandList6 *cmdList);
        /* -------------------------------------------- */
        

        /**
         * .
         * 
         * \param projMat
         * \param frameIndex
         * \param drawIndex
         */
        void UpdateConstantBufferByIndex(const DirectX::XMMATRIX &projMat, const uint32 frameIndex, const uint32 bufferOffset);


        /**
         * .
         * 
         * \param cmdList
         */
        void DrawSceneShapes(ID3D12GraphicsCommandList6 *cmdList) const;

    public:

        Buffer::DynamicConstantBuffer & GetDynamicConstantBuffer();
        Camera & GetCamera();
        ID3D12RootSignature *GetRootSignature();
        Mapped::PipelineLibrary &GetPipelineLibrary();
        Basic::Quad & GetQuad();
        ID3D12Resource * GetRenderTarget(const uint32 index);
        
        ID3D12DescriptorHeap * GetRtvHeap();
        ID3D12DescriptorHeap * GetSrvHeap();

        uint32 GetRtvDescriptorHeapSize() const;
        uint32 GetSrvDescriptorHeapSize() const;

        CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetCPUDescriptorHandle(uint32 rtvIndex);
       

    private:

        void CreateSrvForResource(const D3D12_RESOURCE_DESC &resourceDesc, ID3D12Resource *resource);
        D3D12_RESOURCE_DESC GetResourceForRenderTarget(uint32 rtvIndex);

    private:

        Camera m_Camera;

        uint32 m_RTVDescriptorSize = 0;
        uint32 m_SRVDescriptorSize = 0;

        Basic::Quad m_Quad;
        std::vector<std::shared_ptr<Basic::Shape>> m_SceneShapes;

        ComPointer<ID3D12DescriptorHeap> m_RTVHeap;
        ComPointer<ID3D12DescriptorHeap> m_SRVHeap;
        ComPointer<ID3D12RootSignature> m_RootSignature;

        Buffer::DynamicConstantBuffer m_DynamicConstantBuffer;
        std::vector<std::pair<uint32, ComPointer<ID3D12Resource>>> m_Rtvs;
        Mapped::PipelineLibrary m_PipelineLibrary;

    };
}
