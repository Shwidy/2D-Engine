#include "ResourceManager.h"
#include <SDL3_image/SDL_image.h>
#include <iostream>

ResourceManager::ResourceManager(){}


SDL_Texture* ResourceManager::getTexture(const std::string& path, SDL_Renderer* renderer) {
    auto it = textures.find(path);
    if (it != textures.end()) {
        return it->second;
    }

    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "IMG_Load failed: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);

    if (!texture) {
        SDL_Log("Loading texture: %s", path.c_str());
        std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    textures[path] = texture;
    return texture;
}

void ResourceManager::destroy() {
    for (auto& pair : textures) {
        if (pair.second) {
            SDL_DestroyTexture(pair.second);
        }
    }
    textures.clear();
}