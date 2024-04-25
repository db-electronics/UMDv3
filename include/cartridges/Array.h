#pragma once

#include <array>
#include <cstdint>
#include <cstddef>

namespace cartridges{

    /// @brief Array class to facilitate aligned byte, word and dword accesses for cartridge data.
    /// It provides basic means to access the data in the array, and to keep track of the number 
    /// of bytes transferred being transferred per batch.

    class Array{
    public:

        /// @brief Set the number of bytes to transfer in total (i.e. the ROM size)
        /// @param bytesToTransfer 
        void SetTransferSize(size_t bytesToTransfer) { mBytesToTransfer = bytesToTransfer; }
        
        /// @brief Determine the batch size of the next transfer
        size_t Next(){
            if(mBytesToTransfer >= Sz){
                mAvailableSize = Sz;
                mBytesToTransfer -= Sz;
            }else{
                mAvailableSize = mBytesToTransfer;
            }
            return mAvailableSize;
        }

        /// @brief Get the total size of the array
        /// @return 
        constexpr size_t Size() const { return Sz; }

        /// @brief Get the number of valid bytes in the array
        size_t AvailableSize() const { return mAvailableSize; }

        /// @brief Set the number of valid bytes in the array, capped at less than or equal to the array size
        /// @param newSize 
        void SetAvailableSize(size_t newSize) { mAvailableSize = std::min(Sz, newSize); }

        
        uint8_t* begin() { return mArray; }
        uint8_t* end() { return mArray + mAvailableSize; }
        
        // byte accesses
        uint8_t& operator[](size_t index) { return mArray[index]; }

        uint8_t* Data(){ return mArray; }
        const uint8_t* Data() const { return mArray; }

        /// @brief uint16_t access aligned to 2 bytes
        /// @param index 
        /// @return 
        uint16_t& Word(size_t index) { return *reinterpret_cast<uint16_t*>(&mArray[index & 0xFFFFFFFE]); }

        /// @brief uint32_t access aligned to 4 bytes
        /// @param index 
        /// @return 
        uint32_t& Long(size_t index) { return *reinterpret_cast<uint32_t*>(&mArray[index & 0xFFFFFFFC]); }

    private:
    
        // // Helper function to check if a number is a power of two
        // static constexpr bool IsPowerOfTwo(size_t n) {
        //     return n && ((n & (n - 1)) == 0);
        // }

        // // Compile-time check to ensure Sz is a power of two
        // static_assert(IsPowerOfTwo(Sz), "Size must be a power of two");
        constexpr static size_t Sz = 512;
        uint8_t mArray[Sz];
        size_t mAvailableSize = Sz;
        size_t mBytesToTransfer = 0;
    };
}