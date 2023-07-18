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
            };
        };
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
            };
        };
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
            };
        };
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
            };
        };
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
            };
        };
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
            };
        };
    };

    template <typename T> __forceinline T AlignUpWithMask(T value, size_t mask)
    {
        return (T)(((size_t)value + mask) & ~mask);
    }

    template <typename T> __forceinline T AlignUp(T value, size_t alignment)
    {
        return AlignUpWithMask(value, alignment - 1);
    }

    template <typename T> __forceinline T AlignDownWithMask(T value, size_t mask)
    {
        return (T)((size_t)value & ~mask);
    }

    template <typename T> __forceinline T AlignDown(T value, size_t alignment)
    {
        return AlignDownWithMask(value, alignment - 1);
    }

    // This requires SSE4.2 which is present on Intel Nehalem (Nov. 2008)
    // and AMD Bulldozer (Oct. 2011) processors.  I could put a runtime
    // check for this, but I'm just going to assume people playing with
    // DirectX 12 on Windows 10 have fairly recent machines.
#ifdef _M_X64
#define ENABLE_SSE_CRC32 1
#else
#define ENABLE_SSE_CRC32 0
#endif

#if ENABLE_SSE_CRC32
#pragma intrinsic(_mm_crc32_u32)
#pragma intrinsic(_mm_crc32_u64)
#endif

    inline size_t HashRange(const uint32 *const Begin, const uint32 *const End, uint64 Hash)
    {
#if ENABLE_SSE_CRC32
        const uint64 *Iter64 = (const uint64 *)Math::AlignUp(Begin, 8);
        const uint64 *const End64 = (const uint64 *const)Math::AlignDown(End, 8);

        // If not 64-bit aligned, start with a single u32
        if ((uint32 *)Iter64 > Begin)
            Hash = _mm_crc32_u32((uint32)Hash, *Begin);

        // Iterate over consecutive u64 values
        while (Iter64 < End64)
            Hash = _mm_crc32_u64((uint64)Hash, *Iter64++);

        // If there is a 32-bit remainder, accumulate that
        if ((uint32 *)Iter64 < End)
            Hash = _mm_crc32_u32((uint32)Hash, *(uint32 *)Iter64);
#else
        // An inexpensive hash for CPUs lacking SSE4.2
        for (const uint32 *Iter = Begin; Iter < End; ++Iter)
            Hash = 16777619U * Hash ^ *Iter;
#endif

        return Hash;
    }

    template <typename T> inline uint64 HashState(const T *StateDesc, uint64 Count = 1, uint64 Hash = 2166136261U)
    {
        static_assert((sizeof(T) & 3) == 0 && alignof(T) >= 4, "State object is not word-aligned");
        return HashRange((uint32 *)StateDesc, (uint32 *)(StateDesc + Count), Hash);
    }




}
