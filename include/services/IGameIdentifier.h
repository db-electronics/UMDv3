#pragma once

#include <string>

class IGameIdentifier{
public:
    /// @brief Initialize the game identifier service
    /// @param settings 
    /// @return 
    virtual bool Init(const std::string& settings) = 0;

    /// @brief Check if a game exists
    /// @param gameId 
    /// @return 
    virtual bool GameExists(const std::string& gameId) = 0;

    /// @brief Get the name of the game
    /// @param gameId 
    /// @return 
    virtual std::string GetGameName(const std::string& gameId) = 0;
};
