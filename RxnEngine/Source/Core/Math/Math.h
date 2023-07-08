#pragma once
#include <vector>

namespace Rxn::Core::Math
{
    template<typename T>
    struct FVector2D
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a float or double.");
        union
        {
            struct
            {
                T x;
                T y;
            }
        }
    };

    template<typename T>
    struct FVector3D
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a float or double.");
        union
        {
            struct
            {
                T x;
                T y;
                T z;
            }
        }
    };

    template<typename T>
    struct FVector4D
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
            }
        }
    };

    template<typename T>
    struct IVector2D
    {
        static_assert(std::is_integral_v<T>, "T has to be an integral.");
        union
        {
            struct
            {
                T x;
                T y;
            }
        }
    };

    template<typename T>
    struct IVector3D
    {
        static_assert(std::is_integral_v<T>, "T has to be an integral.");
        union
        {
            struct
            {
                T x;
                T y;
                T z;
            }
        }
    };

    template<typename T>
    struct IVector4D
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
            }
        }
    };




}
