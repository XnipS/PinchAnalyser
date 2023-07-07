#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <string.h>

#include <cstddef>
#include <cstring>
#include <iostream>
#include <string>

#include "../include/renderEngine.h"

SDL_Texture* playerTexture;
SDL_Rect srcR, destR;

SDL_Renderer* renderEngine::renderer = nullptr;

renderEngine::renderEngine() {}
renderEngine::~renderEngine() {}

void renderEngine::Initialise(const char* title, int xpos, int ypos, int w,
                              int h, bool fullscreen) {
  int flags = 0;
  if (fullscreen) {
    flags = SDL_WINDOW_FULLSCREEN;
  }
  if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
    std::cout << "SDL Initialised!" << std::endl;

    window = SDL_CreateWindow(title, xpos, ypos, w, h, flags);
    if (window) {
      std::cout << "Window instantiated!" << std::endl;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer) {
      SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
      std::cout << "Renderer instantiated!" << std::endl;
    }
    isRunning = true;
  } else {
    isRunning = false;
  }
}

void renderEngine::Events() {
  SDL_Event event;
  SDL_PollEvent(&event);
  switch (event.type) {
    case SDL_QUIT:
      isRunning = false;
      break;
    default:
      break;
  }
}

void renderEngine::Update() {
  tick++;
  // std::cout << tick << std::endl;
}

void renderEngine::Render() {
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);
}

void renderEngine::Clean() {
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  std::cout << "Engine Cleaned!" << std::endl;
}