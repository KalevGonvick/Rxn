#pragma once
#include "Camera.h"
#include "DynamicConstantBuffer.h"
#include "Shape.h"
#include "Quad.h"
#include "SwapChain.h"
#include "Display.h"
#include "Renderable.h"

namespace Rxn::Graphics
{
    class RXN_ENGINE_API Scene
    {
    public:

        Scene();
        ~Scene();

    public:

        void SetSerializedRootSignature(CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc);
        
        /* -------------< temp functions >------------- */
        void AddShapeFromRaw(const std::vector<VertexPositionColour> &vertices, const std::vector<UINT> &indices, ID3D12CommandAllocator *cmdAl, ID3D12CommandQueue *cmdQueue, ID3D12GraphicsCommandList *cmdList);
        void AddQuadFromRaw(const std::vector<VertexPositionUV> &quadVertices, ID3D12CommandAllocator *cmdAl, ID3D12CommandQueue *cmdQueue, ID3D12GraphicsCommandList *cmdList);
        void AddTexture(ComPointer<ID3D12Resource> &textureUploadHeap, D3D12_RESOURCE_DESC textureDesc, ID3D12GraphicsCommandList *cmdList, D3D12_SUBRESOURCE_DATA textureData);
        void InitHeaps();


        void ReleaseResourceViews();
        void InitSceneRenderTargets(const uint32 frameIndex, GPU::SwapChain swapChain);
        void SetDynamicConstantBufferByIndex(ComPointer<ID3D12GraphicsCommandList> frameCmdList, const uint32 frameIndex, uint32 drawIndex);
        void ClearOutputMergerRenderTarget(ComPointer<ID3D12GraphicsCommandList> frameCmdList, const uint32 frameIndex);
        void SetShaderResourceViewDescriptorHeap(ComPointer<ID3D12GraphicsCommandList> frameCmdList);
        void UpdateConstantBufferByIndex(const DirectX::XMMATRIX & projMat, const uint32 frameIndex, const uint32 drawIndex);

        /* Getters */
        DrawConstantBuffer *GetConstantBuffer(const uint32 drawIndex, const uint32 frameIndex);
        Buffer::DynamicConstantBuffer & GetDynamicConstantBuffer();
        Camera & GetCamera();
        ComPointer<ID3D12RootSignature> & GetRootSignature();
        
        Basic::Quad & GetQuad();
        ComPointer<ID3D12Resource> & GetRenderTarget(const uint32 index);
        
        ComPointer<ID3D12DescriptorHeap> & GetRtvHeap();
        ComPointer<ID3D12DescriptorHeap> & GetSrvHeap();
        ComPointer<ID3D12Resource> & GetIntermediateRenderTarget();

        const uint32 GetRtvDescriptorHeapSize();
        const uint32 GetSrvDescriptorHeapSize();

        void DrawSceneShapes(ComPointer<ID3D12GraphicsCommandList> cmdList) const;

    private:

        void CreateRenderTargets(GPU::SwapChain &swapChain, CD3DX12_CPU_DESCRIPTOR_HANDLE &rtvHandle);
        void CreateIntermediateRenderTarget(const uint32 frameIndex, CD3DX12_CPU_DESCRIPTOR_HANDLE &rtvHandle);
        void CreateShaderResourceViewForResource(D3D12_RESOURCE_DESC &resDesc);

    private:

        ComPointer<ID3D12Resource> m_Texture;
        Basic::Quad m_Quad;
        std::vector<std::shared_ptr<Basic::Shape>> m_SceneShapes;

        Camera m_Camera;
        Buffer::DynamicConstantBuffer m_DynamicConstantBuffer;
        

        ComPointer<ID3D12Resource> m_RenderTargets[SwapChainBuffers::TOTAL_BUFFERS];
        ComPointer<ID3D12DescriptorHeap> m_RTVHeap;
        uint32 m_RTVDescriptorSize;

        ComPointer<ID3D12RootSignature> m_RootSignature;
        
        ComPointer<ID3D12Resource> m_IntermediateRenderTarget;
        
        ComPointer<ID3D12DescriptorHeap> m_SRVHeap;
        uint32 m_SRVDescriptorSize;

    };
}
