#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <mutex>
#include <atomic>

#include "Buffer.h"


enum class CaptureMode;


class Window
{
friend class WindowManager;
public:
    Window(HWND hwnd, int id);
    ~Window();

    int GetId() const;
    HWND GetHandle() const;
    HWND GetOwner() const;
    HWND GetParent() const;
    HINSTANCE GetInstance() const;
    DWORD GetProcessId() const;
    DWORD GetThreadId() const;

    UINT GetX() const;
    UINT GetY() const;
    UINT GetWidth() const;
    UINT GetHeight() const;
    UINT GetZOrder() const;
    UINT GetBufferWidth() const;
    UINT GetBufferHeight() const;

    void UpdateTitle();
    UINT GetTitleLength() const;
    const std::wstring& GetTitle() const;

    void SetTexturePtr(ID3D11Texture2D* ptr);
    ID3D11Texture2D* GetTexturePtr() const;

    void SetCaptureMode(CaptureMode mode);
    CaptureMode GetCaptureMode() const;

    void Capture();
    void Render();
    void UploadTextureToGpu();

    bool IsAltTab() const;
    bool IsDesktop() const;
    BOOL IsWindow() const;
    BOOL IsVisible() const;
    BOOL IsEnabled() const;
    BOOL IsUnicode() const;
    BOOL IsZoomed() const;
    BOOL IsIconic() const;
    BOOL IsHungUp() const;
    BOOL IsTouchable() const;

    BOOL MoveWindow(int x, int y);
    BOOL ScaleWindow(int width, int height);
    BOOL MoveAndScaleWindow(int x, int y, int width, int height);

private:
    BOOL CaptureWindow();
    void RequestUpload();

    std::shared_ptr<class WindowTexture> texture_;

    const int id_ = -1;
    const HWND window_ = nullptr;
    RECT rect_;
    UINT zOrder_ = 0;
    HWND owner_ = nullptr;
    HWND parent_ = nullptr;
    HINSTANCE instance_ = nullptr;
    DWORD processId_ = -1;
    DWORD threadId_ = -1;
    std::wstring title_ = L"";

    std::atomic<bool> hasNewTextureUploaded_ = false;
    std::atomic<bool> isAlive_ = true;
    std::atomic<bool> isDesktop_ = false;
    std::atomic<bool> isAltTabWindow_ = false;
};
