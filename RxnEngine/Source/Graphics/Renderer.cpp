#include "Rxn.h"
#include "Renderer.h"

namespace Rxn::Graphics
{
    Renderer::Renderer(int32 width, int32 height)
        : m_CommandQueueManager(RenderContext::GetGraphicsDevice())
        , m_CommandListManager(RenderContext::GetGraphicsDevice())
        , m_AllocatorPool(D3D12_COMMAND_LIST_TYPE_DIRECT)
        , m_Display(width, height)
        , m_PipelineLibrary(SwapChainBuffers::TOTAL_BUFFERS, RootParameterCB)
        , m_Fence()
        , m_Initialized(false)
    {
        WCHAR assetsPath[512];
        GetAssetsPath(assetsPath, _countof(assetsPath));
        m_AssetsPath = assetsPath;
        
        //memset(m_EnabledEffects, true, sizeof(m_EnabledEffects));
        for (int x = 0; x < Mapped::EffectPipelineTypeCount; x++)
        {
            m_EnabledEffects.push_back(true);
        }
    }

    Renderer::~Renderer() = default;

    void Renderer::CreateTextureUploadHeap(ComPointer<ID3D12Resource> &textureUploadHeap)
    {


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

        std::vector<uint8> texture = GenerateTextureData();
        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = &texture[0];
        textureData.RowPitch = static_cast<int64>(TextureWidth) * TextureBytesPerPixel;
        textureData.SlicePitch = textureData.RowPitch * TextureHeight;

        m_Scene.AddTexture(textureUploadHeap, textureDesc, m_CommandListManager.GetCommandList(GOHKeys::CmdList::INIT).Get(), textureData);

    }

    // Generate a simple black and white checkerboard texture.
    std::vector<uint8> Renderer::GenerateTextureData() const
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
            m_Fence.SignalFence(m_CommandQueueManager.GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), m_Display.GetFrameIndex());
            m_Fence.WaitInfinite(m_Display.GetFrameIndex());
            m_Fence.IncrementFenceValue(m_Display.GetFrameIndex());
            m_PipelineLibrary.DestroyShader(type);
        }

        m_EnabledEffects[type] = !m_EnabledEffects[type];
    }








}
