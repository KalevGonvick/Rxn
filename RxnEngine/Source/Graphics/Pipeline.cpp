#include "Rxn.h"
#include "Pipeline.h"

namespace Rxn::Graphics
{
    Pipeline::Pipeline(const String &effectName, const GraphicsShaderSet &graphicsShaderSet, bool baseEffect, bool diskCache)
        : effectName(effectName)
        , effectFileName("pipelineLibrary.cache")
        , graphicsShaderSet(graphicsShaderSet)
        , baseEffect(baseEffect)
        , useDiskCache(diskCache)
    {
        pipelineDescriptor.SetGraphicsShaderSet(graphicsShaderSet);
    }

    Pipeline::Pipeline(const String & effectName, const String & fileName, const GraphicsShaderSet & graphicsShaderSet, bool baseEffect, bool diskCache)
        : effectName(effectName)
        , effectFileName(fileName)
        , graphicsShaderSet(graphicsShaderSet)
        , baseEffect(baseEffect)
        , useDiskCache(diskCache)
    {
        pipelineDescriptor.SetGraphicsShaderSet(graphicsShaderSet);
    }

    Pipeline::~Pipeline() = default;
}

