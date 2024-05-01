#include "services/SdFileGameIdentifier.h"

/// @brief Init checks if a file named _db.txt exists in the base path i.e /UMD/MD/_db.txt
/// @param basePath 
/// @return 
bool umd::SdFileGameIdentifier::Init(const std::string& basePath)
{
    mBasePath = basePath;
    std::string filePath = mBasePath + "_db.txt";
    return FileExists(filePath);
}

/// @brief This implementation assumes the game ID is a file path on the SD card.
/// @param gameId 
/// @return 
bool umd::SdFileGameIdentifier::GameExists(const std::string& gameId)
{
    std::string filePath = mBasePath + gameId + ".txt";
    return FileExists(filePath);
}

/// @brief This implementation assumes the game name is the first and only line of text in the file.
/// Call GameExists before calling this function.
/// @param gameId 
/// @return 
std::string umd::SdFileGameIdentifier::GetGameName(const std::string& gameId)
{
    // open the file and return the first line of text as the game name
    std::array <char, 256> buffer;
    std::fill(buffer.begin(), buffer.end(), 0);

    File file = SD.open(gameId.c_str());

    // fill the buffer with text from the file
    int size = std::min(file.available(), 255);
    file.read(buffer.data(), size);

    // close the file
    file.close();

    // return the buffer as a string
    return std::string(buffer.data());
}

inline bool umd::SdFileGameIdentifier::FileExists(const std::string& filePath)
{
    if(SD.exists(filePath.c_str()))
    {
        return true;
    }
    return false;
}