#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>

class renderEngine {
 public:
  renderEngine();
  ~renderEngine();

  void Initialise(const char *title, int xpos, int ypos, int w, int h,
                  bool fullscreen);

  void Events();
  void Update();
  void Render();
  void Clean();
  bool Running() { return isRunning; };

  static SDL_Renderer *renderer;

 private:
  int tick = 0;
  bool isRunning;
  SDL_Window *window;
};