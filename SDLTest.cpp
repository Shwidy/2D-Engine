// ==========================================================
// File: SDLTest.cpp
// Author: Junjie He
// Date: 23/03/2026
// Description:
// This program demonstrates basic SDL3 window creation,
// destruction, window state control, and event monitoring.
// It also loads and renders an image using SDL3_image.
// ==========================================================

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>

int main(int argc, char* argv[]) {
    // Initialize the SDL video subsystem
    // This must be called before creating a window or renderer
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create a resizable SDL window
    // Parameters: title, width, height, flags
    SDL_Window* window = SDL_CreateWindow(
        "SDL3 Window Control Demo",
        800,
        600,
        SDL_WINDOW_RESIZABLE
    );

    // Check whether the window was created successfully
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create a renderer associated with the window
    // The renderer is used to draw textures and graphics
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    // Check whether the renderer was created successfully
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Load an image file into an SDL surface
    // Make sure "test.png" exists in the program's working directory
    SDL_Surface* surface = IMG_Load("test.png");

    // Check whether the image was loaded successfully
    if (!surface) {
        std::cerr << "IMG_Load failed: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Convert the loaded surface into a texture for rendering
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    // The surface is no longer needed after creating the texture
    SDL_DestroySurface(surface);

    // Check whether the texture was created successfully
    if (!texture) {
        std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Control variables for the main loop
    bool running = true;
    bool fullscreen = false;
    SDL_Event event;

    // Main application loop
    while (running) {
        // Process all pending events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                // Close the application when the window close button is pressed
                running = false;
                break;

            case SDL_EVENT_KEY_DOWN:
                // Handle keyboard input
                if (event.key.key == SDLK_ESCAPE) {
                    // Press ESC to quit the application
                    running = false;
                }
                else if (event.key.key == SDLK_F) {
                    // Press F to toggle fullscreen/windowed mode
                    fullscreen = !fullscreen;

                    if (fullscreen) {
                        SDL_SetWindowFullscreen(window, true);
                        std::cout << "Switched to fullscreen mode" << std::endl;
                    }
                    else {
                        SDL_SetWindowFullscreen(window, false);
                        std::cout << "Switched to windowed mode" << std::endl;
                    }
                }
                else if (event.key.key == SDLK_1) {
                    // Press 1 to resize the window to 800x600
                    SDL_SetWindowSize(window, 800, 600);
                    std::cout << "Window resized to 800x600" << std::endl;
                }
                else if (event.key.key == SDLK_2) {
                    // Press 2 to resize the window to 1024x768
                    SDL_SetWindowSize(window, 1024, 768);
                    std::cout << "Window resized to 1024x768" << std::endl;
                }
                else if (event.key.key == SDLK_3) {
                    // Press 3 to resize the window to 1280x720
                    SDL_SetWindowSize(window, 1280, 720);
                    std::cout << "Window resized to 1280x720" << std::endl;
                }
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                // Detect when the window size has changed
                std::cout << "Window resized event: "
                    << event.window.data1 << " x "
                    << event.window.data2 << std::endl;
                break;

            case SDL_EVENT_WINDOW_MINIMIZED:
                // Detect when the window is minimized
                std::cout << "Window minimized" << std::endl;
                break;

            case SDL_EVENT_WINDOW_MAXIMIZED:
                // Detect when the window is maximized
                std::cout << "Window maximized" << std::endl;
                break;

            case SDL_EVENT_WINDOW_RESTORED:
                // Detect when the window is restored from minimized/maximized state
                std::cout << "Window restored" << std::endl;
                break;

            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                // Detect when the window gains keyboard/mouse focus
                std::cout << "Window focus gained" << std::endl;
                break;

            case SDL_EVENT_WINDOW_FOCUS_LOST:
                // Detect when the window loses keyboard/mouse focus
                std::cout << "Window focus lost" << std::endl;
                break;
            }
        }

        // Clear the rendering target before drawing
        SDL_RenderClear(renderer);

        // Render the texture to the entire window
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);

        // Present the rendered frame to the screen
        SDL_RenderPresent(renderer);
    }

    // Release all allocated resources before exiting
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}