#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <algorithm> 
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "TextEditor.h"
#include "plas_syntax_check.h"
#pragma comment(lib, "d3d11.lib")
#define NOMINMAX
#undef min
#undef max
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CreateRenderTarget();
void CleanupRenderTarget();
void RenderCustomTitleBar();
void ToggleFullscreen(HWND hWnd);
void ResizeAndSyncImGui(HWND hWnd);

HWND                    g_hWnd = NULL;
ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
IDXGISwapChain* g_pSwapChain = NULL;
ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

bool g_IsFullscreen = false;
RECT g_WindowRect = {};

static TextEditor       plasEditor;
static char             cppOutput[16384] = "// translated C++ appears here\n";
static float            consoleHeight = 120.0f;
static std::vector<PlasError> syntaxErrors;

ImVec4 bg = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
ImVec4 panel = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
ImVec4 accent = ImVec4(0.20f, 0.45f, 0.70f, 1.00f);
ImVec4 accentHover = ImVec4(0.35f, 0.60f, 0.85f, 1.00f);
ImVec4 border = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);

void ToggleFullscreen(HWND hWnd)
{
    g_pd3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    CleanupRenderTarget();
    if (!g_IsFullscreen)
    {
        GetWindowRect(hWnd, &g_WindowRect);
        SetWindowLong(hWnd, GWL_STYLE, WS_POPUP);
        MONITORINFO mi = { sizeof(mi) };
        if (GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &mi))
        {
            SetWindowPos(hWnd, HWND_TOP,
                mi.rcMonitor.left, mi.rcMonitor.top,
                mi.rcMonitor.right - mi.rcMonitor.left,
                mi.rcMonitor.bottom - mi.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
        ShowWindow(hWnd, SW_SHOW);
        g_IsFullscreen = true;
    }
    else
    {
        SetWindowLong(hWnd, GWL_STYLE, WS_POPUP);
        SetWindowPos(hWnd, HWND_NOTOPMOST,
            g_WindowRect.left, g_WindowRect.top,
            g_WindowRect.right - g_WindowRect.left,
            g_WindowRect.bottom - g_WindowRect.top,
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        ShowWindow(hWnd, SW_SHOW);
        g_IsFullscreen = false;
        SendMessage(hWnd, WM_SIZE, SIZE_RESTORED,
            (LPARAM)((g_WindowRect.bottom - g_WindowRect.top) << 16 |
                (g_WindowRect.right - g_WindowRect.left)));
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(
            float(g_WindowRect.right - g_WindowRect.left),
            float(g_WindowRect.bottom - g_WindowRect.top));
        ImGui::GetMainViewport()->Size = io.DisplaySize;
    }
}

void ResizeAndSyncImGui(HWND hWnd)
{
    RECT client;
    GetClientRect(hWnd, &client);
    g_pSwapChain->ResizeBuffers(0,
        client.right - client.left,
        client.bottom - client.top,
        DXGI_FORMAT_UNKNOWN, 0);
    CreateRenderTarget();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(
        float(client.right),
        float(client.bottom));
    ImGui::GetMainViewport()->Size = io.DisplaySize;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEXA wc = { sizeof(wc), CS_CLASSDC, WndProc, 0, 0,
        GetModuleHandleA(NULL), NULL, NULL, NULL, NULL,
        "PLAS_IDE_Class", NULL };
    RegisterClassExA(&wc);
    g_hWnd = CreateWindowExA(0, wc.lpszClassName, "PLAS Studio",
        WS_POPUP, 100, 100, 1200, 700, NULL, NULL, wc.hInstance, NULL);
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 1200;
    sd.BufferDesc.Height = 700;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0,
        NULL, 0, D3D11_SDK_VERSION, &sd,
        &g_pSwapChain, &g_pd3dDevice,
        NULL, &g_pd3dDeviceContext)))
    {
        MessageBoxA(NULL, "Failed to init D3D11.", "Error", MB_OK | MB_ICONERROR);
        return -1;
    }
    CreateRenderTarget();
    ShowWindow(g_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hWnd);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\FiraCode-Regular.ttf", 16.0f);
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 16.0f);
    ImGui::StyleColorsDark();
    auto& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.FramePadding = ImVec2(10, 6);
    style.ItemSpacing = ImVec2(10, 8);
    style.WindowPadding = ImVec2(12, 12);
    style.WindowBorderSize = 0;
    ImVec4* cms = style.Colors;
    cms[ImGuiCol_WindowBg] = bg;
    cms[ImGuiCol_ChildBg] = panel;
    cms[ImGuiCol_FrameBg] = panel;
    cms[ImGuiCol_FrameBgHovered] = accent;
    cms[ImGuiCol_FrameBgActive] = accentHover;
    cms[ImGuiCol_Button] = panel;
    cms[ImGuiCol_ButtonHovered] = accent;
    cms[ImGuiCol_ButtonActive] = accentHover;
    cms[ImGuiCol_Header] = panel;
    cms[ImGuiCol_HeaderHovered] = accent;
    cms[ImGuiCol_HeaderActive] = accentHover;
    cms[ImGuiCol_Border] = border;
    cms[ImGuiCol_Tab] = panel;
    cms[ImGuiCol_TabHovered] = accent;
    cms[ImGuiCol_TabActive] = accentHover;
    cms[ImGuiCol_TabUnfocused] = panel;
    cms[ImGuiCol_TabUnfocusedActive] = accentHover;
    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    auto defs = TextEditor::LanguageDefinition::CPlusPlus();
    for (auto& kw : { "include","create","print","ask","if","else","repeat","loop","end","output" })
        defs.mKeywords.insert(kw);
    plasEditor.SetLanguageDefinition(defs);
    plasEditor.SetPalette(TextEditor::GetDarkPalette());
    plasEditor.SetText("// Welcome to PLAS Studio!\n"
        "// Write simple instructions below and hit Compile.\n"
        "// Example:\n"
        "// ask \"What's your name?\" into name\n"
        "// print \"Hello, \" + name\n");

    MSG msg{};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        RenderCustomTitleBar();
        ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(vp->Pos.x, vp->Pos.y + 30));
        ImGui::SetNextWindowSize(ImVec2(vp->Size.x, vp->Size.y - 30));
        ImGui::Begin("PLAS Studio", nullptr, ImGuiWindowFlags_NoDecoration);
        ImVec2 full = ImGui::GetContentRegionAvail();
        ImGui::BeginChild("Sidebar", ImVec2(150, full.y), true);
        ImGui::Text("Explorer"); ImGui::Separator(); ImGui::Text("Settings");
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("MainPanel", ImVec2(full.x - 150, full.y), false);
        if (ImGui::BeginTabBar("##tabs"))
        {
            if (ImGui::BeginTabItem("Editor.plas"))
            {
                ImGui::BeginChild("EditorArea", ImVec2(-1, full.y - consoleHeight - 30), true);
                plasEditor.Render("##plasEditor");
                ImGui::EndChild();
                ImGui::Spacing();
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                {
                    auto lines = plasEditor.GetTextLines();
                    syntaxErrors.clear();
                    CheckPLASSyntax(lines, syntaxErrors);
                    TextEditor::ErrorMarkers markers;
                    for (auto& e : syntaxErrors)
                        markers[e.line] = e.message;
                    plasEditor.SetErrorMarkers(markers);
                    bool hasError = !syntaxErrors.empty();

                    if (hasError) ImGui::BeginDisabled();

                    if (ImGui::Button("Compile"))
                    {
                        std::ofstream inputFile("plas_input.txt");
                        if (!inputFile) {
                            strncpy_s(cppOutput, sizeof(cppOutput), "[Error] Could not write plas_input.txt", _TRUNCATE);
                            goto render_panels;
                        }
                        inputFile << plasEditor.GetText(); inputFile.close();
                        int result = system("python plas_to_cpp.py > plas_output.txt");
                        if (result != 0) {
                            strncpy_s(cppOutput, sizeof(cppOutput), "[Error] Python script failed.\n", _TRUNCATE);
                            goto render_panels;
                        }
                        std::ifstream outputFile("plas_output.txt");
                        if (outputFile) {
                            std::ostringstream ss;
                            ss << outputFile.rdbuf();
                            strncpy_s(cppOutput, sizeof(cppOutput), ss.str().c_str(), _TRUNCATE);
                        }
                        else {
                            strncpy_s(cppOutput, sizeof(cppOutput), "[Error] Could not read plas_output.txt", _TRUNCATE);
                        }
                    }
                    if (hasError) ImGui::EndDisabled();
                    ImGui::PopStyleVar();
                }
                ImGui::SameLine(); if (ImGui::Button("Run")) {};
                ImGui::SameLine(); if (ImGui::Button("Clear")) plasEditor.SetText("");
                ImGui::SameLine(); if (ImGui::Button("Copy Output")) ImGui::SetClipboardText(cppOutput);
            render_panels:
                ImGui::InvisibleButton("##splitter", ImVec2(-1, 5));
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                    consoleHeight = std::max(
                        50.0f,
                        std::min(
                            full.y - 50.0f,
                            consoleHeight - ImGui::GetIO().MouseDelta.y));

                if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                if (ImGui::BeginTabBar("##bottomtabs"))
                {
                    if (ImGui::BeginTabItem("Output"))
                    {
                        ImGui::BeginChild("Console", ImVec2(-1, consoleHeight), true);
                        ImGui::TextColored(accent, "[PLAS] Tip: Compile to see C++.");
                        ImGui::InputTextMultiline("##output", cppOutput, IM_ARRAYSIZE(cppOutput), ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
                        ImGui::EndChild();
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Errors"))
                    {
                        ImGui::BeginChild("ErrorList", ImVec2(-1, consoleHeight), true);
                        if (syntaxErrors.empty())
                            ImGui::TextDisabled("No syntax errors!");
                        else
                            for (auto& e : syntaxErrors)
                                ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Line %d: %s", e.line, e.message.c_str());
                        ImGui::EndChild();
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Settings")) { ImGui::Text("Settings here..."); ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }
        ImGui::EndChild();
        ImGui::End();
        ImGui::Render();
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&bg);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);
    }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupRenderTarget();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pd3dDeviceContext) g_pd3dDeviceContext->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();
    DestroyWindow(g_hWnd);
    return 0;
}

void CreateRenderTarget()
{
    ID3D11Texture2D* back = nullptr;
    if (SUCCEEDED(g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&back))) && back)
    {
        g_pd3dDevice->CreateRenderTargetView(back, NULL, &g_mainRenderTargetView);
        back->Release();
    }
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

void RenderCustomTitleBar()
{
    ImGuiViewport* vp = ImGui::GetMainViewport();
    const float h = 30.0f;
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(ImVec2(vp->Size.x, h));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, bg);
    ImGui::Begin("##TitleBar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::SetCursorPosY((h - ImGui::GetFontSize()) * 0.5f);
    ImGui::Text("PLAS Studio");
    ImGui::SameLine(); ImGui::SetCursorPosX(vp->Size.x - 90);
    if (ImGui::Button("_", ImVec2(h - 6, h - 6))) ShowWindow(g_hWnd, SW_MINIMIZE);
    ImGui::SameLine(); if (ImGui::Button(g_IsFullscreen ? "[]" : "[]", ImVec2(h - 6, h - 6))) ToggleFullscreen(g_hWnd);
    ImGui::SameLine(); if (ImGui::Button("X", ImVec2(h - 6, h - 6))) PostQuitMessage(0);
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return TRUE;
    switch (msg)
    {
    case WM_NCHITTEST: {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hWnd, &pt);
        if (pt.y >= 0 && pt.y < 30 && !ImGui::IsAnyItemHovered()) return HTCAPTION;
    } break;
    case WM_SIZE:
        if (g_pd3dDevice && wParam != SIZE_MINIMIZED) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hWnd, msg, wParam, lParam);
}
