#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <string>
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
    int GetParentId() const;
    HWND GetHandle() const;
    HWND GetOwnerHandle() const;
    HWND GetParentHandle() const;
    HINSTANCE GetInstance() const;
    DWORD GetProcessId() const;
    DWORD GetThreadId() const;

    UINT GetX() const;
    UINT GetY() const;
    UINT GetWidth() const;
    UINT GetHeight() const;
    UINT GetClientWidth() const;
    UINT GetClientHeight() const;
    UINT GetZOrder() const;
    UINT GetBufferWidth() const;
    UINT GetBufferHeight() const;
    UINT GetIconWidth() const;
    UINT GetIconHeight() const;
    const RECT & GetCaptureArea() const;

    UINT GetTitleLength() const;
    const std::wstring& GetTitle() const;

    void SetWindowTexture(ID3D11Texture2D* ptr);
    ID3D11Texture2D* GetWindowTexture() const;

    void SetIconTexture(ID3D11Texture2D* ptr);
    ID3D11Texture2D* GetIconTexture() const;

    void SetCaptureMode(CaptureMode mode);
    CaptureMode GetCaptureMode() const;

    void Capture();
    void Upload();
    void Render();

    void CaptureIcon();
    void UploadIcon();
    void RenderIcon();

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
    std::shared_ptr<class WindowTexture> windowTexture_ = std::make_shared<WindowTexture>(this);
    std::shared_ptr<class IconTexture> iconTexture_ = std::make_shared<IconTexture>(this);

    const int id_ = -1;
    int parentId_ = -1;
    int frameCount_ = 0;

    const HWND hWnd_ = nullptr;
    RECT windowRect_;
    RECT clientRect_;
    UINT zOrder_ = 0;
    HWND hWndOwner_ = nullptr;
    HWND hWndParent_ = nullptr;
    HINSTANCE instance_ = nullptr;
    DWORD processId_ = -1;
    DWORD threadId_ = -1;
    std::wstring title_ = L"";
    UINT displayId_ = -1;
    RECT captureArea_;

    std::atomic<bool> hasNewWindowTextureUploaded_ = false;
    std::atomic<bool> hasNewIconTextureUploaded_ = false;
    std::atomic<bool> isAlive_ = true;
    std::atomic<bool> isDesktop_ = false;
    std::atomic<bool> isAltTabWindow_ = false;
};
