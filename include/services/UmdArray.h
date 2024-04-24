#pragma once

#include <array>
#include <cstdint>
#include <cstddef>

namespace umd{

    /// @brief Array class to facilitate aligned byte, word and dword accesses
    /// @tparam Sz 
    template <size_t Sz>
    class UmdArray{
    public:

        constexpr size_t Size() const { return Sz; }
        size_t AvailableSize() const { return mAvailableSize; }
        void SetAvailableSize(size_t newSize) { mAvailableSize = std::min(Sz, newSize); }

        // byte accesses
        uint8_t& operator[](size_t index) { return mArray[index]; }

        uint8_t* Data(){ return mArray.data(); }
        const uint8_t* Data() const { return mArray.data(); }

        /// @brief uint16_t access aligned to 2 bytes
        /// @param index 
        /// @return 
        uint16_t& Word(size_t index) { return *reinterpret_cast<uint16_t*>(&mArray[index & 0xFFFFFFFE]); }

        /// @brief uint32_t access aligned to 4 bytes
        /// @param index 
        /// @return 
        uint32_t& Long(size_t index) { return *reinterpret_cast<uint32_t*>(&mArray[index & 0xFFFFFFFC]); }

    private:
    
        // Helper function to check if a number is a power of two
        static constexpr bool IsPowerOfTwo(size_t n) {
            return n && ((n & (n - 1)) == 0);
        }

        // Compile-time check to ensure Sz is a power of two
        static_assert(IsPowerOfTwo(Sz), "Size must be a power of two");

        uint8_t mArray[Sz];
        size_t mAvailableSize = Sz;
    };
}