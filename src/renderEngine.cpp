#include "../include/renderEngine.h"

#include <GL/gl.h>
#include <GL/glext.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>

#include <cstdio>

#include "../include/core.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <cstddef>
#include <iostream>

#include "../depend/imgui/backends/imgui_impl_opengl3.h"
#include "../depend/imgui/backends/imgui_impl_sdl2.h"
#include "../depend/imgui/imgui.h"

renderEngine::renderEngine() {}
renderEngine::~renderEngine() {}
SDL_Window* window;
SDL_GLContext gl_context;
ImGuiIO io;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

GLuint my_image_texture = 0;
int my_image_width = FB_SIZE;
int my_image_height = FB_SIZE;

// Simple helper function to load an image into a OpenGL texture with common
// settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture,
                         int* out_width, int* out_height) {
  // Load from file
  int image_width = 0;
  int image_height = 0;
  unsigned char* image_data =
      stbi_load(filename, &image_width, &image_height, NULL, 4);
  if (image_data == NULL) return false;

  // Create a OpenGL texture identifier
  GLuint image_texture;
  glGenTextures(1, &image_texture);
  glBindTexture(GL_TEXTURE_2D, image_texture);

  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  GL_CLAMP_TO_EDGE);  // This is required on WebGL for non
                                      // power-of-two textures
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  // Same

  // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);

  *out_texture = image_texture;
  *out_width = image_width;
  *out_height = image_height;

  return true;
}

void renderEngine::FloodImage(Colour3 col) {
  float pixels[my_image_width * my_image_height * 3];
  for (int x = 0; x < (my_image_width); x++) {
    for (int y = 0; y < (my_image_height); y++) {
      pixels[(y * my_image_width * 3) + (x * 3)] = col.r;
      pixels[(y * my_image_width * 3) + (x * 3) + 1] = col.b;
      pixels[(y * my_image_width * 3) + (x * 3) + 2] = col.g;
    }
  }

  UpdateImage(pixels);
}

void renderEngine::UpdateImage(float colours[]) {
  // printf("Image updated!\n");
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, my_image_width, my_image_height, 0,
               GL_RGB, GL_FLOAT, colours);
  glBindTexture(GL_TEXTURE_2D, 0);

  my_image_texture = tex;
}

void renderEngine::Initialise(const char* title, int w, int h) {
  // SDL Attributes
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  // Create window with flags
  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL);
  window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, w, h, window_flags);
  gl_context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, gl_context);

  // Setup ImGui context
  IMGUI_CHECKVERSION();

  ImGui::CreateContext();
  io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init();
  isRunning = true;
  Colour3 col;
  col.r = .2;
  col.g = .2;
  col.b = .2;
  FloodImage(col);
}
void renderEngine::Update() {
  tick++;

  // Handle events
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT) isRunning = false;
    if (event.type == SDL_WINDOWEVENT &&
        event.window.event == SDL_WINDOWEVENT_CLOSE &&
        event.window.windowID == SDL_GetWindowID(window))
      isRunning = false;
  }

  //  Start the ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  // Imgui goes here
  // ImGui::ShowDemoWindow();

  // Menu Bar
  ImGui::BeginMainMenuBar();
  ImGui::Text("NIP-Engine");
  ImGui::Separator();
  ImGui::Text("v0.1");
  ImGui::EndMainMenuBar();

  // Main Simulation
  ImGui::Begin("Ideal Gas Simulator", NULL,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                   ImGuiWindowFlags_NoCollapse);
  ImGui::Text("Size = %d x %d. Tickrate = %d. Tick = %d.", my_image_width,
              my_image_height, FB_TARGET_TICKRATE, tick);
  ImGui::Image((void*)(intptr_t)my_image_texture,
               ImVec2(my_image_width * FB_IMAGE_SCALE,
                      my_image_height * FB_IMAGE_SCALE));
  ImGui::End();

  // Top left Overlay
  if (currentDebugInfo.size() > 0) {
    ImGui::SetNextWindowBgAlpha(0.35f);
    const float PAD = 10.0f;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = (work_pos.x + PAD);
    window_pos.y = (work_pos.y + PAD);
    window_pos_pivot.x = 0.0f;
    window_pos_pivot.y = 0.0f;
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::Begin("Debug", NULL,
                 ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav |
                     ImGuiWindowFlags_NoFocusOnAppearing |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking);
    ImGui::Text("Debug");
    for (int i = 0; i < currentDebugInfo.size(); i++) {
      ImGui::Text("%s", currentDebugInfo[i].c_str());
    }
    // ImGui::Text("Fluid Count: %i", fluidEngine::);
    ImGui::End();
  }
}

void renderEngine::Render() {
  // Render
  ImGui::Render();
  glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
  glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
               clear_color.z * clear_color.w, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  SDL_GL_SwapWindow(window);
}

void renderEngine::Clean() {
  // Clean Imgui
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  // Clean SDL
  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();
  std::cout << "Engine Cleaned!" << std::endl;
}