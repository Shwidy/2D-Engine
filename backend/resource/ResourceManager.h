#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <unordered_map>

class ResourceManager {
private:
    std::unordered_map<std::string, SDL_Texture*> textures;


public:
    ResourceManager();
    SDL_Texture* getTexture(const std::string& path, SDL_Renderer* renderer);
    void destroy();
};