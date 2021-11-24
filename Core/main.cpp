#include <iostream>
#include <Windows.h>
#include "Memory/Memory.hpp"
#include "..\SDK\Game Structure.hpp"
#include <vector>
#include "../imgui.h"
#include "../imgui_impl_win32.h"
#include "../imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include "../implot/implot.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"
#include "../Header.h"


// DirectX Initialization  ------------------------------------------------------------------------------------

    // Data
    static ID3D11Device* g_pd3dDevice = NULL;
    static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
    static IDXGISwapChain* g_pSwapChain = NULL;
    static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

    // Forward declarations of helper functions
    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



// Image Loading -----------------------------------------------------------------------------------------------

    // Simple helper function to load an image into a DX11 texture with common settings
    bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
    {
        // Load from disk into a raw RGBA buffer
        int image_width = 0;
        int image_height = 0;
        unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
        if (image_data == NULL)
            return false;

        // Create texture
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = image_width;
        desc.Height = image_height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;

        ID3D11Texture2D* pTexture = NULL;
        D3D11_SUBRESOURCE_DATA subResource;
        subResource.pSysMem = image_data;
        subResource.SysMemPitch = desc.Width * 4;
        subResource.SysMemSlicePitch = 0;
        g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

        // Create texture view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
        pTexture->Release();

        *out_width = image_width;
        *out_height = image_height;
        stbi_image_free(image_data);

        return true;
    }


// Global Parameters ---------------------------------------------------------------------------------------------


    bool quickLoad = true; // Load a placeholder image instead of the large map file to greatly accelerate compiling time


// MAIN PROGAM ---------------------------------------------------------------------------------------------------
int main()
{

// Handles, Hooks and Offsets
    // Search for process ID
    DWORD procID = Memory::GetProcID(L"DayZ_x64.exe");
    std::string checkPID = (procID == 0) ? "[ERROR] Process not found." : "Process Detected:";
    printf("%s %i \n", checkPID.c_str(), procID);

    // Get a handle to access process
    Memory::hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procID);
    std::string checkHandle = (Memory::hProcess == 0) ? "[ERROR] Failed to get handle!" : "Sucessfully opened handle:";
    printf("%s %p\n", checkHandle.c_str(), Memory::hProcess);

    // Find base address
    uintptr_t baseAddress = Memory::FindBaseAddress();
    std::string checkBaseAddress = (baseAddress == 0) ? "[ERROR] Failed to find base address: " : "Sucessfully found base address:";
    printf("%s %I64X\n", checkBaseAddress.c_str(), baseAddress);

    // Define addresses of interest
    World GameWorld = (World)Memory::ReadMemory<uintptr_t>(baseAddress + 0x403E7A0);
    printf("World Pointer Address: %p \n", GameWorld);


    uintptr_t WorldAddress = (uintptr_t)Memory::ReadMemory<uintptr_t>(baseAddress + 0x403E7A0); // I needed a variable storing the world address in uintptr_t format.. because converting just don't work.
    printf("uintptr_t world address = %p", WorldAddress);

    // Create application window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Sam's DayZ Cheat"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);



// DirectX Stuff
    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load map image
    int my_image_width = 0;
    int my_image_height = 0;
    ID3D11ShaderResourceView* my_texture = NULL;

    if (quickLoad) // Load a dummy image to speed up loading
    {
        bool ret = LoadTextureFromFile("map_topo_CROW.png", &my_texture, &my_image_width, &my_image_height);
        IM_ASSERT(ret);
    }
    else
    {
        bool ret = LoadTextureFromFile("map_topo.png", &my_texture, &my_image_width, &my_image_height);
        IM_ASSERT(ret);
    }
        

// ImGui Setup
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Menu / Global Options
    bool show_sam_window = false;
    bool show_entity_scanner = true;
    bool show_radar = true;
    bool show_radarOptions = true;
    bool show_direction_lines = true;


    // Static Params (These reset every loop!)
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


// Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {

        // Poll and handle messages (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Sam's DayZ Cheat");       // Create a window called "Hello, world!" and append into it.

            ImGui::Text("Main Menu");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Game Cheats", &show_sam_window);
            ImGui::Checkbox("Entity Scanner", &show_entity_scanner);
            ImGui::Checkbox("Radar", &show_radar);
            ImGui::Checkbox("Radar Options", &show_radarOptions); ImGui::SameLine;

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        if (show_sam_window)
        {
            ImGui::Begin("Game Cheat Menu", &show_sam_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Experimental settings");


            if (ImGui::Button("Toggle Grass"))
            {
                uintptr_t noGrassAddress = (WorldAddress + 0xB80);
                float noGrassValue = (float)Memory::ReadMemory<float>(noGrassAddress);
                float disableGrass = 0;
                float enableGrass = 10;

                if (noGrassValue == disableGrass) // If grass is currently disabled
                {
                    Memory::WriteMemory(noGrassAddress, enableGrass); // enable grass
                }
                else
                    Memory::WriteMemory(noGrassAddress, disableGrass); // disable grass 
            }
            ImGui::End();
        }

        if (show_entity_scanner)
        {

            ImGui::Begin("Entity Scanner", &show_entity_scanner);                          // Create a window called "Hello, world!" and append into it.
            ImGui::Text("Entity Count: %i", GameWorld.GetEntities().size());

            for (Entity CurrentEntity : GameWorld.GetEntities())
            {
                EntityType Type = CurrentEntity.GetEntityType();

                if (Type.GetSimName().GetContents() == "Particle")
                    continue;

                VisualState State = CurrentEntity.GetVisualState();
                Vector3 Coordinates = State.GetCoordinates();
                Vector3 Direction = State.GetDirection();

                ImGui::Text("Entity: %s - %s\n", Type.GetKlassName().GetContents().c_str(), Type.GetSimName().GetContents().c_str());
                ImGui::Text("X: %.3f   \nY: %.3f   \nZ: %.3f\nView: < %.3f, %.3f >\n\n", Coordinates.x, Coordinates.y, Coordinates.z, Direction.x, Direction.z);
            }
            ImGui::End();
        }

        static float slider_i = 1.0;
        static float dirLine_len = 0.02;

        if (show_radarOptions)
        {
            ImGui::Begin("Radar Options", &show_radarOptions);                          // Create a window called "Hello, world!" and append into it.
            ImGui::Separator();

            ImGui::SliderFloat("Map Opacity", &slider_i, 0.0f, 1.0f, "%.2f");

            ImGui::Checkbox("Show Direction Lines", &show_direction_lines);

            ImGui::SliderFloat("Line Length", &dirLine_len, 0.0f, 1.0f, "%.2f");

            ImGui::End();
        }

        if (show_radar)
        {
            ImGui::Begin("[EXPERIMENTAL] Radar", &show_radar);

            ImPlot::SetNextPlotLimits(-150, 150, -150, 150);
            static ImVec2 uv0(0, 0);
            static ImVec2 uv1(1, 1);

            ImVec4 tint(slider_i, slider_i, slider_i, slider_i); // This setting is changed from the 'Radar options' menu

            std::string fancyName;

            if (ImPlot::BeginPlot("ESP", NULL, NULL, ImVec2(-1, -1), ImPlotFlags_Equal, ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_NoTickLabels))
            {
                std::vector<Entity> Entities = GameWorld.GetEntities();
                Entity Player = Entities[0];
                Vector3 PlayerPosition = Entities[0].GetVisualState().GetCoordinates();
                ImVec2 rmin;
                ImVec2 rmax;

                ImVec2 bmin(0 - PlayerPosition.x, 0 - PlayerPosition.z);             // X min, Y min (Map image coordinates on plot)
                ImVec2 bmax(15360 - PlayerPosition.x, 15360 - PlayerPosition.z);     // X max, Y max (Map image coordinates on plot)
                ImPlot::PlotImage("", (void*)my_texture, bmin, bmax, uv0, uv1, tint);            // Plot image of map

                auto dl = ImGui::GetWindowDrawList();                                  // (For direction lines) Creating two dots above player at 0.45 and 0.55 Y

                for (Entity CurrentEntity : Entities)
                {
                    EntityType Type = CurrentEntity.GetEntityType();
                    
                    if (Type.GetSimName().GetContents() == "Particle")    // Block Particles From Radar
                        continue;

                    bool needsLine = false;

                    VisualState State = CurrentEntity.GetVisualState();
                    Vector3 EntityCoordinates = State.GetCoordinates();

                    if (show_direction_lines)
                    {
                        Vector3 Direction = State.GetDirection();
                        int PlotLimits = (int)ImPlot::GetPlotLimits().Y.Size();
                        rmin = ImPlot::PlotToPixels(EntityCoordinates.x - PlayerPosition.x, EntityCoordinates.z - PlayerPosition.z); // View direction vector BEGIN X, Y
                        rmax = ImPlot::PlotToPixels(EntityCoordinates.x - PlayerPosition.x + (Direction.z * (PlotLimits * -dirLine_len)), EntityCoordinates.z - PlayerPosition.z - (Direction.x * (PlotLimits * -dirLine_len)));     // View direction vector  END X, Y
                    }

                    std::string className = Type.GetKlassName().GetContents();

                    if (className == "dayzplayer")
                    {
                        ImPlot::SetNextFillStyle(ImVec4(0, 1, 0, 1.0f));
                        ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 9);
                        fancyName = "Player";
                        needsLine = true;
                    }
                    else if (className == "car")
                    {
                        ImPlot::SetNextFillStyle(ImVec4(1, 1, 1, 1.0f));
                        ImPlot::SetNextMarkerStyle(ImPlotMarker_Square, 8);
                        fancyName = "Vehicle";
                    }
                    else if (className == "dayzinfected")
                    {
                        ImPlot::SetNextFillStyle(ImVec4(0, 1, 0, 1.0f));
                        ImPlot::SetNextMarkerStyle(ImPlotMarker_Up, 8);
                        fancyName = "Zombie";
                        needsLine = true;
                    }
                    else if (className == "dayzanimal")
                    {
                        ImPlot::SetNextFillStyle(ImVec4(0, 1, 0, 1.0f));
                        ImPlot::SetNextMarkerStyle(ImPlotMarker_Up, 8);
                        fancyName = "Animal";
                        needsLine = true;
                    }
                    else
                    {
                        ImPlot::SetNextFillStyle(ImVec4(1, 0.2, 0, 1.0f));
                        ImPlot::SetNextMarkerStyle(ImPlotMarker_Asterisk, 8);
                        fancyName = "(Unknown)" + CurrentEntity.GetEntityType().GetSimName().GetContents();
                    }

                    float xpos[1] = { EntityCoordinates.x - PlayerPosition.x };     // Calculate entity position relative to player
                    float zpos[1] = { EntityCoordinates.z - PlayerPosition.z };     // Calculate entity position relative to player

                    ImPlot::PlotScatter(fancyName.c_str(), xpos, zpos, 100);

                    if (show_direction_lines && needsLine)
                            dl->AddLine(rmin, rmax, 0xFF0000FF, 5.0f);
                }

                ImPlot::EndPlot();
            }

            ImGui::End();
        }


        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;

    }

// -------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------
// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
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
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

/*
#include <iostream>
#include <Windows.h>
#include "Memory/Memory.hpp"
#include "..\SDK\Game Structure.hpp"
#include <vector>
#include "../imgui.h"
#include "../imgui_impl_win32.h"
#include "../imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include "../implot/implot.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"
#include "../Header.h"


// DirectX Initialization  ------------------------------------------------------------------------------------

    // Data
    static ID3D11Device* g_pd3dDevice = NULL;
    static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
    static IDXGISwapChain* g_pSwapChain = NULL;
    static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

    // Forward declarations of helper functions
    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



// Image Loading -----------------------------------------------------------------------------------------------

    // Simple helper function to load an image into a DX11 texture with common settings
    bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
    {
        // Load from disk into a raw RGBA buffer
        int image_width = 0;
        int image_height = 0;
        unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
        if (image_data == NULL)
            return false;

        // Create texture
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = image_width;
        desc.Height = image_height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;

        ID3D11Texture2D* pTexture = NULL;
        D3D11_SUBRESOURCE_DATA subResource;
        subResource.pSysMem = image_data;
        subResource.SysMemPitch = desc.Width * 4;
        subResource.SysMemSlicePitch = 0;
        g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

        // Create texture view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
        pTexture->Release();

        *out_width = image_width;
        *out_height = image_height;
        stbi_image_free(image_data);

        return true;
    }


// Global Parameters ---------------------------------------------------------------------------------------------


    bool quickLoad = false; // Load a placeholder image instead of the large map file to greatly accelerate compiling time


// MAIN PROGAM ---------------------------------------------------------------------------------------------------
int main()
{

// Handles, Hooks and Offsets
    // Search for process ID
    DWORD procID = Memory::GetProcID(L"DayZ_x64.exe");
    std::string checkPID = (procID == 0) ? "[ERROR] Process not found." : "Process Detected:";
    printf("%s %i \n", checkPID.c_str(), procID);

    // Get a handle to access process
    Memory::hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procID);
    std::string checkHandle = (Memory::hProcess == 0) ? "[ERROR] Failed to get handle!" : "Sucessfully opened handle:";
    printf("%s %p\n", checkHandle.c_str(), Memory::hProcess);

    // Find base address
    uintptr_t baseAddress = Memory::FindBaseAddress();
    std::string checkBaseAddress = (baseAddress == 0) ? "[ERROR] Failed to find base address: " : "Sucessfully found base address:";
    printf("%s %I64X\n", checkBaseAddress.c_str(), baseAddress);

    // Define addresses of interest
    World GameWorld = (World)Memory::ReadMemory<uintptr_t>(baseAddress + 0x403E7A0);
    printf("World Pointer Address: %p \n", GameWorld);


    uintptr_t WorldAddress = (uintptr_t)Memory::ReadMemory<uintptr_t>(baseAddress + 0x403E7A0); // I needed a variable storing the world address in uintptr_t format.. because converting just don't work.
    printf("uintptr_t world address = %p", WorldAddress);

    // Create application window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Sam's DayZ Cheat"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);



// DirectX Stuff
    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load map image
    int my_image_width = 0;
    int my_image_height = 0;
    ID3D11ShaderResourceView* my_texture = NULL;

    if (quickLoad) // Load a dummy image to speed up loading
    {
        bool ret = LoadTextureFromFile("map_topo_CROW.png", &my_texture, &my_image_width, &my_image_height);
        IM_ASSERT(ret);
    }
    else
    {
        bool ret = LoadTextureFromFile("map_topo.png", &my_texture, &my_image_width, &my_image_height);
        IM_ASSERT(ret);
    }


// ImGui Setup
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Menu / Global Options
    bool show_sam_window = false;
    bool show_entity_scanner = true;
    bool show_radar = true;
    bool show_radarOptions = true;
    bool show_direction_lines = true;


    // Static Params (These reset every loop!)
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


// Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {

        // Poll and handle messages (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Sam's DayZ Cheat");       // Create a window called "Hello, world!" and append into it.

            ImGui::Text("Main Menu");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Game Cheats", &show_sam_window);
            ImGui::Checkbox("Entity Scanner", &show_entity_scanner);
            ImGui::Checkbox("Radar", &show_radar);
            ImGui::Checkbox("Radar Options", &show_radarOptions); ImGui::SameLine;

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        if (show_sam_window)
        {
            ImGui::Begin("Game Cheat Menu", &show_sam_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Experimental settings");


            if (ImGui::Button("Toggle Grass"))
            {
                uintptr_t noGrassAddress = (WorldAddress + 0xB80);
                float noGrassValue = (float)Memory::ReadMemory<float>(noGrassAddress);
                float disableGrass = 0;
                float enableGrass = 10;

                if (noGrassValue == disableGrass) // If grass is currently disabled
                {
                    Memory::WriteMemory(noGrassAddress, enableGrass); // enable grass
                }
                else
                    Memory::WriteMemory(noGrassAddress, disableGrass); // disable grass
            }
            ImGui::End();
        }

        if (show_entity_scanner)
        {

            ImGui::Begin("Entity Scanner", &show_entity_scanner);                          // Create a window called "Hello, world!" and append into it.

            for (Entity CurrentEntity : GameWorld.GetEntities())
            {
                EntityType Type = CurrentEntity.GetEntityType();
                VisualState State = CurrentEntity.GetVisualState();

                if (Type.GetSimName().GetContents() == "Particle")
                    continue;
                ImGui::Text("Entity: %s - %s\n", Type.GetKlassName().GetContents().c_str(), Type.GetSimName().GetContents().c_str());
                ImGui::Text("X: %.3f   \nY: %.3f   \nZ: %.3f\nView: < %.3f, %.3f >\n\n", State.GetCoordinates()[0], State.GetCoordinates()[1], State.GetCoordinates()[2], State.GetCoordinates()[3], State.GetCoordinates()[4]);
            }
            ImGui::End();
        }

        static float slider_i = 1.0;
        static float dirLine_len = 0.02;

        if (show_radarOptions)
        {
            ImGui::Begin("Radar Options", &show_radarOptions);                          // Create a window called "Hello, world!" and append into it.
            ImGui::Separator();

            ImGui::SliderFloat("Map Opacity", &slider_i, 0.0f, 1.0f, "%.2f");

            ImGui::Checkbox("Show Direction Lines", &show_direction_lines);

            ImGui::SliderFloat("Line Length", &dirLine_len, 0.0f, 1.0f, "%.2f");

            ImGui::End();
        }

        if (show_radar)
        {
            ImGui::Begin("[EXPERIMENTAL] Radar", &show_radar);

            ImPlot::SetNextPlotLimits(-150, 150, -150, 150);
            static ImVec2 uv0(0, 0);
            static ImVec2 uv1(1, 1);

            ImVec4 tint(slider_i, slider_i, slider_i, slider_i); // This setting is changed from the 'Radar options' menu

            std::string fancyName;

            if (ImPlot::BeginPlot("ESP", NULL, NULL, ImVec2(-1, -1), ImPlotFlags_Equal, ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_NoTickLabels))
            {
                std::vector<Entity> Entities = GameWorld.GetEntities();
                Entity Player = Entities[0];
                auto PlayerPosition = Entities[0].GetVisualState().GetCoordinates();
                ImVec2 rmin;
                ImVec2 rmax;

                ImVec2 bmin(0 - PlayerPosition[0], 0 - PlayerPosition[2]);             // X min, Y min (Map image coordinates on plot)
                ImVec2 bmax(15360 - PlayerPosition[0], 15360 - PlayerPosition[2]);     // X max, Y max (Map image coordinates on plot)
                ImPlot::PlotImage("", (void*)my_texture, bmin, bmax, uv0, uv1, tint);            // Plot image of map

                auto dl = ImGui::GetWindowDrawList();                                  // (For direction lines) Creating two dots above player at 0.45 and 0.55 Y

                for (Entity CurrentEntity : Entities)
                {
                    EntityType Type = CurrentEntity.GetEntityType();

                    if (Type.GetSimName().GetContents() == "Particle")    // Block Particles From Radar
                        continue;

                    bool needsLine = false;

                    VisualState State = CurrentEntity.GetVisualState();

                    if (show_direction_lines)
                    {
                        int PlotLimits = (int)ImPlot::GetPlotLimits().Y.Size();
                        rmin = ImPlot::PlotToPixels(State.GetCoordinates()[0] - PlayerPosition[0], State.GetCoordinates()[2] - PlayerPosition[2]); // View direction vector BEGIN X, Y
                        rmax = ImPlot::PlotToPixels(State.GetCoordinates()[0] - PlayerPosition[0] + (State.GetCoordinates()[4] * (PlotLimits * -dirLine_len)), State.GetCoordinates()[2] - PlayerPosition[2] - (State.GetCoordinates()[3] * (PlotLimits * -dirLine_len)));     // View direction vector  END X, Y
                    }

                    std::string className = Type.GetKlassName().GetContents();

                    if (className == "dayzplayer")
                    {
                        ImPlot::SetNextFillStyle(ImVec4(0, 1, 0, 1.0f));
                        ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 9);
                        fancyName = "Player";
                        needsLine = true;
                    }

                    else if (className == "car")
                    {
                        ImPlot::SetNextFillStyle(ImVec4(1, 1, 1, 1.0f));
                        ImPlot::SetNextMarkerStyle(ImPlotMarker_Square, 8);
                        fancyName = "Vehicle";
                    }

                    else if (className == "dayzinfected")
                    {
                        ImPlot::SetNextFillStyle(ImVec4(0, 1, 0, 1.0f));
                        ImPlot::SetNextMarkerStyle(ImPlotMarker_Up, 8);
                        fancyName = "Zombie";
                        needsLine = true;
                    }

                    else if (className == "dayzanimal")
                    {
                        ImPlot::SetNextFillStyle(ImVec4(0, 1, 0, 1.0f));
                        ImPlot::SetNextMarkerStyle(ImPlotMarker_Up, 8);
                        fancyName = "Animal";
                        needsLine = true;
                    }

                    else
                    {
                        ImPlot::SetNextFillStyle(ImVec4(1, 0.2, 0, 1.0f));
                        ImPlot::SetNextMarkerStyle(ImPlotMarker_Asterisk, 8);
                        fancyName = "(Unknown)" + CurrentEntity.GetEntityType().GetSimName().GetContents();
                    }


                    float xpos[1] = { State.GetCoordinates()[0] - PlayerPosition[0] };     // Calculate entity position relative to player
                    float zpos[1] = { State.GetCoordinates()[2] - PlayerPosition[2] };     // Calculate entity position relative to player

                    ImPlot::PlotScatter(fancyName.c_str(), xpos, zpos, 100);

                    if (show_direction_lines)
                    {
                        if (needsLine)
                        {
                            dl->AddLine(rmin, rmax, 0xFF0000FF, 5.0f);
                        }
                    }
                }

                ImPlot::EndPlot();
            }

            ImGui::End();
        }


        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;

    }



















































// -------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------
// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
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
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
*/
