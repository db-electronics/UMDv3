#pragma once

#include "services/IGameIdentifier.h"
#include <STM32SD.h>
#include <string>

namespace umd{
    class SdFileGameIdentifier : public IGameIdentifier{
    public:
        bool Init(const std::string& basePath) override;
        bool GameExists(const std::string& gameId) override;
        std::string GetGameName(const std::string& gameId) override;

    private:
        std::string mBasePath = "/UMD/";

        bool FileExists(const std::string& filePath);
    };
}
