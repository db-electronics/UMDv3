#pragma once

#include <STM32SD.h>
#include <string>

namespace umd{
    class GameIdentifier{
    public:
        GameIdentifier();
        ~GameIdentifier();

        bool Init(const std::string& basePath);
        bool GameExists(const std::string& gameId);
        std::string GetGameName(const std::string& gameId);

    private:
        std::string mBasePath = "/UMD/";

        bool FileExists(const std::string& filePath);
    };
}
