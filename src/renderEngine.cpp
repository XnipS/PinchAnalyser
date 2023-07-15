#include "../include/renderEngine.h"
#if defined(_WIN64)
#include <SDL_syswm.h>
#include <d3d11.h>

#include "../depend/imgui/backends/imgui_impl_dx11.h"
#else
#include <GL/gl.h>
#include <GL/glext.h>

#include "../depend/imgui/backends/imgui_impl_opengl3.h"
#endif
#include <SDL.h>
#include <SDL_events.h>
#include <SDL_video.h>

#include <cstddef>
#include <cstdio>
#include <iostream>

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
#if defined(_WIN64)
ID3D11ShaderResourceView* my_image_texture = NULL;
D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
D3D11_SUBRESOURCE_DATA subResource;
D3D11_TEXTURE2D_DESC desc;
ID3D11Texture2D* pTexture = NULL;
#else
GLuint my_image_texture = 0;
#endif

// FluidEngine variables
float* var_gravity;
float* var_dampen;
float* var_size;
float* var_heat;
int* var_holes;
float* var_holePower;

// DirectX11 variables
#if defined(_WIN64)
// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
#endif

// Flood image with single colour
void renderEngine::FloodImage(Colour3 col) {
  float pixels[FB_SIZE * FB_SIZE * 3];
  for (int x = 0; x < (FB_SIZE); x++) {
    for (int y = 0; y < (FB_SIZE); y++) {
      pixels[(y * FB_SIZE * 3) + (x * 3)] = col.r;
      pixels[(y * FB_SIZE * 3) + (x * 3) + 1] = col.b;
      pixels[(y * FB_SIZE * 3) + (x * 3) + 2] = col.g;
    }
  }
  UpdateImage(&pixels[0]);
}

// Set image pixels
void renderEngine::UpdateImage(float* colours) {
#if defined(_WIN64)
  D3D11_MAPPED_SUBRESOURCE mappedSubRes;
  // ZeroMemory(&mappedSubRes, sizeof(D3D11_MAPPED_SUBRESOURCE));
  ID3D11Resource* res;
  my_image_texture->GetResource(&res);
  g_pd3dDeviceContext->Map(res, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubRes);
  //  Update the vertex buffer here.
  memcpy(mappedSubRes.pData, colours, FB_SIZE * FB_SIZE * 3 * 4);
  //  Reenable GPU access to the vertex buffer data.
  g_pd3dDeviceContext->Unmap(res, 0);
#else

  glGenTextures(1, &my_image_texture);
  glBindTexture(GL_TEXTURE_2D, my_image_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FB_SIZE, FB_SIZE, 0, GL_RGB, GL_FLOAT,
               colours);
  glBindTexture(GL_TEXTURE_2D, 0);
#endif
}

// Transfer variables
void renderEngine::UpdateConfig(float* gravity, float* damp, float* size,
                                float* heat, int* holeCount, float* holePow) {
  var_gravity = gravity;
  var_dampen = damp;
  var_size = size;
  var_heat = heat;
  var_holes = holeCount;
  var_holePower = holePow;
}

// Start engine
void renderEngine::Initialise(const char* title, int w, int h) {
  // Window flags
#if defined(_WIN64)
  SDL_WindowFlags window_flags =
      (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
#else
  // SDL Attributes
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  // Create window with flags
  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL);
#endif
  window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, w, h, window_flags);

  // Initialise renderer
#if defined(_WIN64)
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInfo(window, &wmInfo);
  HWND hwnd = (HWND)wmInfo.info.win.window;

  // Initialize Direct3D
  if (!CreateDeviceD3D(hwnd)) {
    CleanupDeviceD3D();
    exit(1);
  }
#else
  gl_context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, gl_context);
#endif

  // Setup ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  // Setup Platform/Renderer backends
#if defined(_WIN64)
  ImGui_ImplSDL2_InitForD3D(window);
  ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
#else
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init();
#endif

  // Setup Done
  isRunning = true;

  // Initialise texture on gpu
#if defined(_WIN64)
  float pixels[FB_SIZE * FB_SIZE * 3];
  for (int x = 0; x < (FB_SIZE); x++) {
    for (int y = 0; y < (FB_SIZE); y++) {
      pixels[(y * FB_SIZE * 3) + (x * 3)] = 1;
      pixels[(y * FB_SIZE * 3) + (x * 3) + 1] = 1;
      pixels[(y * FB_SIZE * 3) + (x * 3) + 2] = 1;
    }
  }

  ZeroMemory(&desc, sizeof(desc));
  desc.Width = FB_SIZE;
  desc.Height = FB_SIZE;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  subResource.pSysMem = pixels;
  subResource.SysMemPitch =
      FB_SIZE * 3 * 4;  // I HAVE NO IDEA WHY BUT THIS FIXES THE TILING ISSUE
  subResource.SysMemSlicePitch = 0;

  ZeroMemory(&srvDesc, sizeof(srvDesc));
  srvDesc.Format = desc.Format;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = desc.MipLevels;

  g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

  g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &my_image_texture);
#else
  glGenTextures(1, &my_image_texture);
#endif

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
#if defined(_WIN64)
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplSDL2_NewFrame();
#else
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
#endif
  ImGui::NewFrame();

  // Imgui goes here
  // ImGui::ShowDemoWindow();

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
  ImGui::Text("Size = %d x %d. Tickrate = %d. Tick = %d.", FB_SIZE, FB_SIZE,
              FB_TARGET_TICKRATE, tick);
  ImGui::Image((void*)(intptr_t)my_image_texture,
               ImVec2(FB_SIZE * FB_IMAGE_SCALE, FB_SIZE * FB_IMAGE_SCALE));
  ImGui::End();

  // Toolbox
  ImGui::Begin("Toolbox", NULL,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                   ImGuiWindowFlags_NoCollapse);
  ImGui::SliderFloat("Gravity", var_gravity, 0, 1);
  ImGui::SliderFloat("Dampen", var_dampen, 0, 1);
  ImGui::SliderFloat("Size", var_size, 0, 10);
  ImGui::SliderFloat("Heat", var_heat, 0, 1);
  ImGui::SliderInt("Fluid Holes", var_holes, 1, 51);
  ImGui::SliderFloat("Fluid Power", var_holePower, 0, 1);
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
#if defined(_WIN64)
  const float clear_color_with_alpha[4] = {
      clear_color.x * clear_color.w, clear_color.y * clear_color.w,
      clear_color.z * clear_color.w, clear_color.w};
  g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
  g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView,
                                             clear_color_with_alpha);
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

  // Update and Render additional Platform Windows
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }
  g_pSwapChain->Present(0, 0);
#else
  glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
  glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
               clear_color.z * clear_color.w, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  SDL_GL_SwapWindow(window);
#endif
}

// Clean
void renderEngine::Clean() {
  // Shutdown imgui graphic implementation
#if defined(_WIN64)
  ImGui_ImplDX11_Shutdown();
#else
  ImGui_ImplOpenGL3_Shutdown();
#endif
  // Clean Imgui
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  // Clean SDL
  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();
  std::cout << "Engine Cleaned!" << std::endl;
}

// Helper functions to use DirectX11
#if defined(_WIN64)
bool CreateDeviceD3D(HWND hWnd) {
  // Setup swap chain
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 2;
  sd.BufferDesc.Width = 0;
  sd.BufferDesc.Height = 0;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hWnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  UINT createDeviceFlags = 0;
  // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevelArray[2] = {
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_0,
  };
  if (D3D11CreateDeviceAndSwapChain(
          nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
          featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
          &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
    return false;

  CreateRenderTarget();
  return true;
}

void CleanupDeviceD3D() {
  CleanupRenderTarget();
  if (g_pSwapChain) {
    g_pSwapChain->Release();
    g_pSwapChain = nullptr;
  }
  if (g_pd3dDeviceContext) {
    g_pd3dDeviceContext->Release();
    g_pd3dDeviceContext = nullptr;
  }
  if (g_pd3dDevice) {
    g_pd3dDevice->Release();
    g_pd3dDevice = nullptr;
  }
}

void CreateRenderTarget() {
  ID3D11Texture2D* pBackBuffer;
  g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
  g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr,
                                       &g_mainRenderTargetView);
  pBackBuffer->Release();
}

void CleanupRenderTarget() {
  if (g_mainRenderTargetView) {
    g_mainRenderTargetView->Release();
    g_mainRenderTargetView = nullptr;
  }
}
#endif