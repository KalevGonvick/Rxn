/*****************************************************************//**
 * \file   CommonTypes.h
 * \brief  Base namespace containing types used in all areas of the engine.
 * 
 * \author kalev
 * \date   December 2023
 *********************************************************************/
#pragma once


namespace Rxn
{
    /* Vectors */
    template<typename T>
    struct RXN_ENGINE_API FVector2D
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a float or double.");
        union
        {
            struct
            {
                T x;
                T y;
            };
        };
    };


    template<typename T>
    struct RXN_ENGINE_API FVector3D
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a float or double.");
        union
        {
            struct
            {
                T x;
                T y;
                T z;
            };
        };
    };

    template<typename T>
    struct RXN_ENGINE_API FVector4D
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a float or double.");
        union
        {
            struct
            {
                T x;
                T y;
                T z;
                T w;
            };
        };
    };

    template<typename T>
    struct RXN_ENGINE_API IVector2D
    {
        static_assert(std::is_integral_v<T>, "T has to be an integral.");
        union
        {
            struct
            {
                T x;
                T y;
            };
        };
    };

    template<typename T>
    struct RXN_ENGINE_API IVector3D
    {
        static_assert(std::is_integral_v<T>, "T has to be an integral.");
        union
        {
            struct
            {
                T x;
                T y;
                T z;
            };
        };
    };

    template<typename T>
    struct RXN_ENGINE_API IVector4D
    {
        static_assert(std::is_integral_v<T>, "T has to be an integral.");
        union
        {
            struct
            {
                T x;
                T y;
                T z;
                T w;
            };
        };
    };

    struct RXN_ENGINE_API VertexPositionColour
    {
        FVector4D<float32> position;
        FVector3D<float32> colour;
    };

    struct RXN_ENGINE_API VertexPositionUV
    {
        FVector4D<float32> position;
        FVector2D<float32> uv;
    };

    const int8 RXN_VERTEX_TYPE = 1;
    struct RXN_ENGINE_API RxnVertexEntry
    {
        VertexPositionColour vertex;
    };

    const int8 RXN_INDEX_TYPE = 2;
    struct RXN_ENGINE_API RxnIndexEntry
    {
        uint32 index;
    };

    const int8 RXN_QUAD_TYPE = 3;
    struct RXN_ENGINE_API RxnQuadEntry
    {
        VertexPositionUV vertex;
    };

    struct RXN_ENGINE_API NonCopyable
    {
        NonCopyable() = default;
        NonCopyable(const NonCopyable &) = delete;
        NonCopyable &operator=(const NonCopyable &) = delete;
    };
}

