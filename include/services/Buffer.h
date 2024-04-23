#include <array>

namespace umd{

    template <size_t Size>
    class UmdBuffer{
        public:

            constexpr size_t Capacity() const { return Size; }
            size_t Size() const { return mSize; }
            void Resize(size_t newSize) { mSize = std::min(Size, newSize); }
            void Fill(uint8_t value) { mBuffer.fill(value); }

            // byte accesses
            uint8_t& operator[](size_t index) const { return mBuffer[index]; }
            const uint8_t& operator[](size_t index) const { return mBuffer[index]; }

            uint8_t* Data(){ return mBuffer.data(); }
            const uint8_t* Data() const { return mBuffer.data(); }

            // word accesses
            uint16_t& Word(size_t index) const { return *reinterpret_cast<uint16_t*>(&mBuffer[index]); }
            const uint16_t& Word(size_t index) const { return *reinterpret_cast<const uint16_t*>(&mBuffer[index]); }

            // big endian word accesses
            uint16_t& BigEndianWord(size_t index) const {
                uint16_t word = *reinterpret_cast<uint16_t*>(&mBuffer[index]);
                return ((word >> 8) & 0xff) | ((word & 0xff) << 8);
            }
            const uint16_t& BigEndianWord(size_t index) const {
                uint16_t word = *reinterpret_cast<uint16_t*>(&mBuffer[index]);
                return ((word >> 8) & 0xff) | ((word & 0xff) << 8);
            }

            // dword accesses
            uint32_t& Dword(size_t index) const { return *reinterpret_cast<uint32_t*>(&mBuffer[index]); }
            const uint32_t& Dword(size_t index) const { return *reinterpret_cast<const uint32_t*>(&mBuffer[index]); }

        private:
            std::array<uint8_t, Size> mBuffer;
            size_t mSize = Size;
    };
}