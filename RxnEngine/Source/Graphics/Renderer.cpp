#include "Rxn.h"
#include "Renderer.h"

namespace Rxn::Graphics
{
    Renderer::Renderer(int32 width, int32 height)
        : m_UseWarpDevice(false)
        , m_HasTearingSupport(false)
        , m_Width(width)
        , m_Viewport(0.0f, 0.0f, static_cast<float32>(width), static_cast<float32>(height))
        , m_ScissorRect(0, 0, static_cast<uint64>(width), static_cast<uint64>(height))
        , m_Height(height)
        , m_RTVDescriptorSize(0)
        , m_SRVDescriptorSize(0)
        , m_Initialized(false)
        , m_DrawIndex(0)
        , m_DynamicConstantBuffer(sizeof(DrawConstantBuffer), MaxDrawsPerFrame, SwapChainBuffers::TOTAL_BUFFERS)
        , m_PipelineLibrary(SwapChainBuffers::TOTAL_BUFFERS, RootParameterCB)
        , m_CommandQueueManager(RenderContext::GetGraphicsDevice())
        , m_CommandListManager(RenderContext::GetGraphicsDevice())
        , m_SwapChain(width, height)
        , m_RenderFence()
    {
        WCHAR assetsPath[512];
        GetAssetsPath(assetsPath, _countof(assetsPath));
        m_AssetsPath = assetsPath;
        m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
        memset(m_EnabledEffects, true, sizeof(m_EnabledEffects));
    }

    Renderer::~Renderer() = default;

    HRESULT Renderer::CreateRootSignature()
    {



        // We don't modify the SRV in the command list after SetGraphicsRootDescriptorTable
        // is executed on the GPU so we can use the default range behavior:
        // D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE
        CD3DX12_DESCRIPTOR_RANGE1 ranges[RootParametersCount]{};
        ranges[RootParameterSRV].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

        CD3DX12_ROOT_PARAMETER1 rootParameters[3]{};
        rootParameters[RootParameterUberShaderCB].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[RootParameterCB].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[RootParameterSRV].InitAsDescriptorTable(1, &ranges[RootParameterSRV], D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPointer<ID3DBlob> signature;
        ComPointer<ID3DBlob> error;

        HRESULT result;
        result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, RenderContext::GetHighestRootSignatureVersion(), &signature, &error);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to serialize root signature descriptor.");
            return result;
        }
        
        result = RenderContext::GetGraphicsDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to declare root signature.");
            return result;
        }

        NAME_D3D12_OBJECT(m_RootSignature);

        return S_OK;
    }

    HRESULT Renderer::CreateVertexBufferResource()
    {

        // Define the geometry for a cube.
        std::vector<VertexPositionColour> cubeVertices;
        cubeVertices.push_back(VertexPositionColour{ { -1.0f, 1.0f, -1.0f, 1.0f }, { GetRandomColour(), GetRandomColour(), GetRandomColour() } });
        cubeVertices.push_back(VertexPositionColour{ {  1.0f, 1.0f, -1.0f, 1.0f }, { GetRandomColour(), GetRandomColour(), GetRandomColour() } });
        cubeVertices.push_back(VertexPositionColour{ {  1.0f, 1.0f, 1.0f, 1.0f }, { GetRandomColour(), GetRandomColour(), GetRandomColour() } });
        cubeVertices.push_back(VertexPositionColour{ { -1.0f, 1.0f, 1.0f, 1.0f }, { GetRandomColour(), GetRandomColour(), GetRandomColour() } });

        cubeVertices.push_back(VertexPositionColour{ { -1.0f, -1.0f, -1.0f, 1.0f }, { GetRandomColour(),GetRandomColour(), GetRandomColour() } });
        cubeVertices.push_back(VertexPositionColour{ {  1.0f, -1.0f, -1.0f, 1.0f }, { GetRandomColour(),GetRandomColour(), GetRandomColour() } });
        cubeVertices.push_back(VertexPositionColour{ {  1.0f, -1.0f, 1.0f, 1.0f }, { GetRandomColour(),GetRandomColour(), GetRandomColour() } });
        cubeVertices.push_back(VertexPositionColour{ { -1.0f, -1.0f, 1.0f, 1.0f }, { GetRandomColour(),GetRandomColour(), GetRandomColour() } });

        std::vector<UINT> cubeIndices;

        cubeIndices.push_back(0);
        cubeIndices.push_back(1);
        cubeIndices.push_back(3);

        cubeIndices.push_back(1);
        cubeIndices.push_back(2);
        cubeIndices.push_back(3);


        cubeIndices.push_back(3);
        cubeIndices.push_back(2);
        cubeIndices.push_back(7);

        cubeIndices.push_back(6);
        cubeIndices.push_back(7);
        cubeIndices.push_back(2);


        cubeIndices.push_back(2);
        cubeIndices.push_back(1);
        cubeIndices.push_back(6);

        cubeIndices.push_back(5);
        cubeIndices.push_back(6);
        cubeIndices.push_back(1);


        cubeIndices.push_back(1);
        cubeIndices.push_back(0);
        cubeIndices.push_back(5);

        cubeIndices.push_back(4);
        cubeIndices.push_back(5);
        cubeIndices.push_back(0);


        cubeIndices.push_back(0);
        cubeIndices.push_back(3);
        cubeIndices.push_back(4);

        cubeIndices.push_back(7);
        cubeIndices.push_back(4);
        cubeIndices.push_back(3);


        cubeIndices.push_back(7);
        cubeIndices.push_back(6);
        cubeIndices.push_back(4);

        cubeIndices.push_back(5);
        cubeIndices.push_back(4);
        cubeIndices.push_back(6);

        m_Shape.ReadDataFromRaw(cubeVertices, cubeIndices);

        HRESULT result;

        result = m_Shape.UploadGpuResources(RenderContext::GetGraphicsDevice().Get(), m_CommandQueueManager.GetCommandQueue(GOHKeys::CmdQueue::PRIMARY).Get(), m_CommandAllocators[m_FrameIndex].Get(), m_CommandListManager.GetCommandList(GOHKeys::CmdList::INIT).Get());
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to upload shape resources to gpu");
            return result;
        }

        std::vector<VertexPositionUV> quadVertices;
        quadVertices.push_back(VertexPositionUV{ { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } });
        quadVertices.push_back(VertexPositionUV{ { -1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } });
        quadVertices.push_back(VertexPositionUV{ { 1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } });
        quadVertices.push_back(VertexPositionUV{ { 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } });

        m_Quad.ReadDataFromRaw(quadVertices);

        result = m_Quad.UploadGpuResources(RenderContext::GetGraphicsDevice().Get(), m_CommandQueueManager.GetCommandQueue(GOHKeys::CmdQueue::PRIMARY).Get(), m_CommandAllocators[m_FrameIndex].Get(), m_CommandListManager.GetCommandList(GOHKeys::CmdList::INIT).Get());
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to upload quad resources to gpu");
            return result;
        }

        return S_OK;
    }

    HRESULT Renderer::CreateTextureUploadHeap(ComPointer<ID3D12Resource> &textureUploadHeap)
    {
        HRESULT result;

        // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
        // the command list that references it has finished executing on the GPU.
        // We will flush the GPU at the end of this method to ensure the resource is not
        // prematurely destroyed.


        // Describe and create a Texture2D.
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Width = TextureWidth;
        textureDesc.Height = TextureHeight;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        const auto heapTextureProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        result = RenderContext::GetGraphicsDevice()->CreateCommittedResource(&heapTextureProps, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_Texture));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create committed resource for texture.");
            return result;
        }

        // Create the GPU upload buffer.
        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_Texture.Get(), 0, 1);
        const auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const auto uploadBufferSizeDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
        ThrowIfFailed(RenderContext::GetGraphicsDevice()->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &uploadBufferSizeDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUploadHeap)));

        // Copy data to the intermediate upload heap and then schedule a copy 
        // from the upload heap to the Texture2D.
        std::vector<uint8> texture = GenerateTextureData();

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = &texture[0];
        textureData.RowPitch = static_cast<int64>(TextureWidth) * TextureBytesPerPixel;
        textureData.SlicePitch = textureData.RowPitch * TextureHeight;

        UpdateSubresources(m_CommandListManager.GetCommandList(GOHKeys::CmdList::INIT).Get(), m_Texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
        const auto transition = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_CommandListManager.GetCommandList(GOHKeys::CmdList::INIT)->ResourceBarrier(1, &transition);

        // Describe and create a SRV for the texture.
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        RenderContext::GetGraphicsDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_SRVHeap->GetCPUDescriptorHandleForHeapStart());

        return S_OK;
    }

    // Generate a simple black and white checkerboard texture.
    std::vector<uint8> Renderer::GenerateTextureData()
    {
        const uint32 rowPitch = TextureWidth * TextureBytesPerPixel;
        const uint32 cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
        const uint32 cellHeight = TextureWidth >> 3;    // The height of a cell in the checkerboard texture.
        const uint32 textureSize = rowPitch * TextureHeight;

        std::vector<uint8> data(textureSize);
        uint8 *pData = &data[0];

        for (uint32 n = 0; n < textureSize; n += TextureBytesPerPixel)
        {
            uint32 x = n % rowPitch;
            uint32 y = n / rowPitch;
            uint32 i = x / cellPitch;
            uint32 j = y / cellHeight;

            if (i % 2 == j % 2)
            {
                pData[n] = 0x00;        // R
                pData[n + 1] = 0x00;    // G
                pData[n + 2] = 0x00;    // B
                pData[n + 3] = 0xff;    // A
            }
            else
            {
                pData[n] = 0xff;        // R
                pData[n + 1] = 0xff;    // G
                pData[n + 2] = 0xff;    // B
                pData[n + 3] = 0xff;    // A
            }
        }

        return data;
    }

    void Renderer::ToggleEffect(Mapped::EffectPipelineType type)
    {
        if (m_EnabledEffects[type])
        {
            m_RenderFence.SignalFence(m_CommandQueueManager.GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), m_FrameIndex);
            m_RenderFence.WaitInfinite(m_FrameIndex);
            m_RenderFence.IncrementFenceValue(m_FrameIndex);
            m_PipelineLibrary.DestroyShader(type);
        }

        m_EnabledEffects[type] = !m_EnabledEffects[type];
    }








}
