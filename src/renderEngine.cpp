#include "../include/renderEngine.h"

#if defined(_WIN64)
#include <Windows.h>
#endif
#include <GL/gl.h>
#include <SDL.h>
#include <SDL_events.h>
#include <SDL_opengl.h>
#include <SDL_video.h>

#include <cstddef>
#include <cstdio>
#include <iostream>

#include "../depend/imgui/backends/imgui_impl_opengl3.h"
#include "../depend/imgui/backends/imgui_impl_sdl2.h"
#include "../depend/imgui/imgui.h"
#include "../include/core.h"

renderEngine::renderEngine() {}
renderEngine::~renderEngine() {}
SDL_Window* window;
SDL_GLContext gl_context;
ImGuiIO io;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

// Fluid image tank variables
GLuint my_image_texture = 0;

// Flood image with single colour
void renderEngine::FloodImage(Colour3 col) {
  float pixels[FB_CONTAINER_OUTPUT * FB_CONTAINER_OUTPUT * 3];
  for (int x = 0; x < (FB_CONTAINER_OUTPUT); x++) {
    for (int y = 0; y < (FB_CONTAINER_OUTPUT); y++) {
      pixels[(y * FB_CONTAINER_OUTPUT * 3) + (x * 3)] = col.r;
      pixels[(y * FB_CONTAINER_OUTPUT * 3) + (x * 3) + 1] = col.b;
      pixels[(y * FB_CONTAINER_OUTPUT * 3) + (x * 3) + 2] = col.g;
    }
  }
  UpdateImage(&pixels[0]);
}

// Set image pixels
void renderEngine::UpdateImage(float* colours) {
  glBindTexture(GL_TEXTURE_2D, my_image_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FB_CONTAINER_OUTPUT,
               FB_CONTAINER_OUTPUT, 0, GL_RGB, GL_FLOAT, colours);
  glBindTexture(GL_TEXTURE_2D, 0);
}

// Start engine
void renderEngine::Initialise(const char* title, int w, int h) {
  // SDL Attributes
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  // Create window with flags
  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL);
  window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, w, h, window_flags);

  // Initialise renderer
  gl_context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, gl_context);

  // Setup ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  io = ImGui::GetIO();
  (void)io;
  // ImGui::GetIO().IniFilename = NULL;
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  // Setup Platform/Renderer backends
  ImGui_ImplOpenGL3_Init();
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);

  // Setup Done
  isRunning = true;

  // Initialise texture on gpu
  glGenTextures(1, &my_image_texture);

  // Setup default tank
  const Colour3 col(.2, .2, .2);
  FloodImage(col);
}

// Tick renderengine
void renderEngine::Update() {
  // Tick
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

  // Menu Bar
  ImGui::BeginMainMenuBar();
  ImGui::Text("NIP-Engine");
  ImGui::Separator();
  ImGui::Text(FB_VERSION);
  ImGui::EndMainMenuBar();

  // Main Simulation
  ImGui::Begin("Fluidised Bed Simulator", NULL,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                   ImGuiWindowFlags_NoCollapse);
  ImGui::Text("Size: %d x %d. Scale: 1 pixel = %f m. Tickrate: %d. Tick: %d.",
              FB_CONTAINER_OUTPUT, FB_CONTAINER_OUTPUT,
              (FB_CONTAINER_SIZE / FB_CONTAINER_OUTPUT), FB_TARGET_TICKRATE,
              tick);
  ImGui::Image((void*)(intptr_t)my_image_texture,
               ImVec2(FB_CONTAINER_OUTPUT * FB_IMAGE_SCALE,
                      FB_CONTAINER_OUTPUT * FB_IMAGE_SCALE));
  ImGui::End();

  // Toolbox
  ImGui::Begin("Toolbox", NULL,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                   ImGuiWindowFlags_NoCollapse);
  ImGui::SliderFloat("Gravity (m/s/s)", &settings->gravity, 0, 0.5);
  ImGui::SliderFloat("Dampen (%)", &settings->dampen, 0, 1);
  ImGui::BeginDisabled(true);
  ImGui::SliderFloat("Heat", &settings->heat, 0, 1);
  ImGui::SliderInt("Fluid Holes", &settings->fluid_holes, 1, 51);
  ImGui::SliderFloat("Fluid Power", &settings->fluid_power, 0, 200);
  ImGui::EndDisabled();
  ImGui::End();

  // Sand Summoner
  ImGui::Begin("Sand Summoner", NULL,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                   ImGuiWindowFlags_NoCollapse);
  ImGui::InputDouble("Drag Coeff. (-)", &settings->dragCoefficient);
  ImGui::InputDouble("Mass (kg)", &settings->mass);
  ImGui::InputDouble("Radius (m)", &settings->radius);
  ImGui::Text("Total: %i", val_totalSand);
  addSand = 0;
  ImGui::SameLine();
  if (ImGui::Button("Add 1x")) {
    addSand = 1;
  }
  ImGui::SameLine();
  if (ImGui::Button("Add 10x")) {
    addSand = 10;
  }
  ImGui::SameLine();
  if (ImGui::Button("Add 100x")) {
    addSand = 100;
  }
  ImGui::SameLine();
  clearAllSand = ImGui::Button("Clear");
  ImGui::End();

  // Top left Overlay
  if (currentDebugInfo.size() > 0) {
    ImGui::SetNextWindowBgAlpha(0.35f);
    const float PAD = 10.0f;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = (work_pos.x + 10);
    window_pos.y = (work_pos.y + 10);
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
    ImGui::End();
  }
}

// Render
void renderEngine::Render() {
  // Imgui Render
  ImGui::Render();

  // Clear and render
  glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
  glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
               clear_color.z * clear_color.w, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  SDL_GL_SwapWindow(window);
}

// Clean
void renderEngine::Clean() {
  // Shutdown imgui graphic implementation
  ImGui_ImplOpenGL3_Shutdown();
  // Clean Imgui
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  // Clean SDL
  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();
  std::cout << "Engine Cleaned!" << std::endl;
}
