#pragma once
#include <vector>

namespace Rxn::Core::Math
{
    
    template <typename T> 
    __forceinline T AlignUpWithMask(T value, size_t mask)
    {
        return (T)(((size_t)value + mask) & ~mask);
    }

    template <typename T> 
    __forceinline T AlignUp(T value, size_t alignment)
    {
        return AlignUpWithMask(value, alignment - 1);
    }

    template <typename T> 
    __forceinline T AlignDownWithMask(T value, size_t mask)
    {
        return (T)((size_t)value & ~mask);
    }

    template <typename T> 
    __forceinline T AlignDown(T value, size_t alignment)
    {
        return AlignDownWithMask(value, alignment - 1);
    }

    static inline uint32 Murmer32Scramble(uint32 k) {
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        return k;
    }

    static inline uint32 Murmer3(const uint8 *key, uint64 len, uint32 seed)
    {
        uint32 hashResult = seed;
        uint32 inputString;

        for (uint64 i = len >> 2; i; i--) 
        {
            memcpy(&inputString, key, sizeof(uint32));
            key += sizeof(uint32);
            hashResult ^= Murmer32Scramble(inputString);
            hashResult = (hashResult << 13) | (hashResult >> 19);
            hashResult = hashResult * 5 + 0xe6546b64;
        }

        inputString = 0;
        for (uint64 i = len & 3; i; i--) 
        {
            inputString <<= 8;
            inputString |= key[i - 1];
        }

        hashResult ^= Murmer32Scramble(inputString);
        hashResult ^= len;
        hashResult ^= hashResult >> 16;
        hashResult *= 0x85ebca6b;
        hashResult ^= hashResult >> 13;
        hashResult *= 0xc2b2ae35;
        hashResult ^= hashResult >> 16;
        return hashResult;
    }

    inline uint64 HashRange(const uint32 *const Begin, const uint32 *const End, uint64 Hash)
    {
        // An inexpensive hash for CPUs lacking SSE4.2
        for (const uint32 *Iter = Begin; Iter < End; ++Iter)
            Hash = 16777619U * Hash ^ *Iter;


        return Hash;
    }

    template <typename T> 
    inline uint64 HashState(const T *StateDesc, uint64 Count = 1, uint64 Hash = 2166136261U)
    {
        static_assert((sizeof(T) & 3) == 0 && alignof(T) >= 4, "State object is not word-aligned");
        return HashRange((uint32 *)StateDesc, (uint32 *)(StateDesc + Count), Hash);
    }




}
