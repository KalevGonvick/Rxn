/*****************************************************************//**
 * \file   MemoryMappedPipelineStateObjectCache.h
 * \brief  
 * 
 * \author kalev
 * \date   December 2023
 *********************************************************************/
#pragma once
#include "MemoryMappedFile.h"

namespace Rxn::Graphics::Mapped
{
    class MemoryMappedPipelineStateObjectCache : public MemoryMappedFile
    {
    public:

        /**
         * .
         * 
         * \param pBlob
         */
        void Update(ID3DBlob *pBlob);
    };
}
