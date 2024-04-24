#pragma once

#include <cstdint>

namespace umd{
    class BatchSizeCalculator{
    public:
        BatchSizeCalculator(){};

        void Init(uint32_t totalBytes, uint16_t batchSize){
            mBytesLeft = totalBytes;
            mMBatchSize = batchSize;
        }

        uint16_t Next(){
            if(mBytesLeft >= mMBatchSize){
                mBytesLeft -= mMBatchSize;
                return mMBatchSize;
            }
            else{
                return mBytesLeft;
            }
        };

    private:
        uint32_t mBytesLeft = 0;
        uint32_t mMBatchSize = 0;
};
}