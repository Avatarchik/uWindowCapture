#include "Window.h"
#include "WindowTexture.h"
#include "IconTexture.h"
#include "WindowManager.h"
#include "Debug.h"
#include "Util.h"



Window::Window(HWND hWnd, int id)
    : window_(hWnd)
    , id_(id)
    , windowTexture_(std::make_shared<WindowTexture>(this))
    , iconTexture_(std::make_shared<IconTexture>(this))
{
    if (hWnd == ::GetDesktopWindow())
    {
        title_ = L"Desktop";
        isDesktop_ = true;
        windowTexture_->SetCaptureMode(CaptureMode::BitBlt);
    }
    else
    {
        isAltTabWindow_ = ::IsAltTabWindow(hWnd);
        const auto mode = (owner_ == NULL) ? CaptureMode::PrintWindow : CaptureMode::BitBltAlpha;
        windowTexture_->SetCaptureMode(mode);
    }
}


Window::~Window()
{
}


int Window::GetId() const
{
    return id_;
}


HWND Window::GetHandle() const
{
    return window_;
}


HWND Window::GetOwner() const
{
    return owner_;
}


HWND Window::GetParent() const
{
    return parent_;
}


HINSTANCE Window::GetInstance() const
{
    return instance_;
}


DWORD Window::GetProcessId() const
{
    return processId_;
}


DWORD Window::GetThreadId() const
{
    return threadId_;
}


bool Window::IsAltTab() const
{
    return isAltTabWindow_;
}


bool Window::IsDesktop() const
{
    return isDesktop_;
}


BOOL Window::IsWindow() const
{
    return ::IsWindow(window_);
}


BOOL Window::IsVisible() const
{
    return ::IsWindowVisible(window_);
}


BOOL Window::IsEnabled() const
{
    return ::IsWindowEnabled(window_);
}


BOOL Window::IsUnicode() const
{
    return ::IsWindowUnicode(window_);
}


BOOL Window::IsZoomed() const
{
    return ::IsZoomed(window_);
}


BOOL Window::IsIconic() const
{
    return ::IsIconic(window_);
}


BOOL Window::IsHungUp() const
{
    return ::IsHungAppWindow(window_);
}


BOOL Window::IsTouchable() const
{
    return ::IsTouchWindow(window_, NULL);
}


BOOL Window::MoveWindow(int x, int y)
{
    return MoveAndScaleWindow(x, y, GetWidth(), GetHeight());
}


BOOL Window::ScaleWindow(int width, int height)
{
    return MoveAndScaleWindow(GetX(), GetY(), width, height);
}


BOOL Window::MoveAndScaleWindow(int x, int y, int width, int height)
{
    bool repaint = (width != GetWidth() || height != GetHeight());
    return ::MoveWindow(window_, x, y, width, height, repaint);
}


UINT Window::GetX() const
{
    return rect_.left;
}


UINT Window::GetY() const
{
    return rect_.top;
}


UINT Window::GetWidth() const
{
    return rect_.right - rect_.left;
}


UINT Window::GetHeight() const
{
    return rect_.bottom - rect_.top;
}


UINT Window::GetZOrder() const
{
    return zOrder_;
}


UINT Window::GetBufferWidth() const
{
    return windowTexture_->GetWidth();
}


UINT Window::GetBufferHeight() const
{
    return windowTexture_->GetHeight();
}


UINT Window::GetIconWidth() const
{
    return iconTexture_->GetWidth();
}


UINT Window::GetIconHeight() const
{
    return iconTexture_->GetHeight();
}


void Window::UpdateTitle()
{
    ::GetWindowTitle(window_, title_);
}


UINT Window::GetTitleLength() const
{
    return static_cast<UINT>(title_.length());
}


const std::wstring& Window::GetTitle() const
{
    return title_;
}


void Window::SetWindowTexture(ID3D11Texture2D* ptr)
{
    windowTexture_->SetUnityTexturePtr(ptr);
}


ID3D11Texture2D* Window::GetWindowTexture() const
{
    return windowTexture_->GetUnityTexturePtr();
}


void Window::SetIconTexture(ID3D11Texture2D* ptr)
{
    iconTexture_->SetUnityTexturePtr(ptr);
}


ID3D11Texture2D* Window::GetIconTexture() const
{
    return iconTexture_->GetUnityTexturePtr();
}


void Window::SetCaptureMode(CaptureMode mode)
{
    windowTexture_->SetCaptureMode(mode);
}


CaptureMode Window::GetCaptureMode() const
{
    return windowTexture_->GetCaptureMode();
}


void Window::Capture()
{
    // Run this scope in the thread loop managed by CaptureManager.

    if (!IsWindow() || !IsVisible())
    {
        return;
    }

    UWC_SCOPE_TIMER(WindowCapture)

    if (windowTexture_->Capture())
    {
        iconTexture_->CaptureOnce();

        if (auto& uploader = WindowManager::GetUploadManager())
        {
            uploader->RequestUpload(id_);
        }
    }
}


void Window::Upload()
{
    // Run this scope in the thread loop managed by UploadManager.

    if (windowTexture_->Upload())
    {
        iconTexture_->UploadOnce();
        hasNewTextureUploaded_ = true;
    }
}


void Window::Render()
{
    // Run this scope in the unity rendering thread.

    if (!hasNewTextureUploaded_) return;
    hasNewTextureUploaded_ = false;

    windowTexture_->Render();
    iconTexture_->RenderOnce();
}
