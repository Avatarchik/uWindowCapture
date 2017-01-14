#include <vector>
#include <algorithm>

#include "Window.h"
#include "WindowManager.h"
#include "Unity.h"
#include "Uploader.h"
#include "Message.h"
#include "Util.h"
#include "Debug.h"

using namespace Microsoft::WRL;



namespace
{


}



Window::Window(HWND hWnd, int id)
    : window_(hWnd)
    , id_(id)
{
    if (hWnd == ::GetDesktopWindow())
    {
        title_ = L"Desktop";
        isDesktop_ = true;
        mode_ = CaptureMode::BitBlt;
    }
    else
    {
        owner_ = ::GetWindow(hWnd, GW_OWNER);
        isAltTabWindow_ = ::IsAltTabWindow(hWnd);
    }
}


Window::~Window()
{
    StopCapture();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        DeleteBitmap();
    }
}


HWND Window::GetHandle() const
{
    return window_;
}


HWND Window::GetOwner() const
{
    return ::GetWindow(window_, GW_OWNER);
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


void Window::UpdateTitle()
{
    if (isDesktop_) return;

    const auto titleLength = ::GetWindowTextLengthW(window_);
    std::vector<WCHAR> buf(titleLength + 1);
    if (::GetWindowTextW(window_, &buf[0], static_cast<int>(buf.size())))
    {
        title_ = &buf[0];
    }
}


UINT Window::GetTitleLength() const
{
    return static_cast<UINT>(title_.length());
}


const std::wstring& Window::GetTitle() const
{
    return title_;
}


void Window::CreateBitmapIfNeeded(HDC hDc, UINT width, UINT height)
{
    if (bufferWidth_ == width && bufferHeight_ == height) return;
    if (width == 0 || height == 0) return;

    std::lock_guard<std::mutex> lock(mutex_);

    bufferWidth_ = width;
    bufferHeight_ = height;

    {
        DeleteBitmap();
        bitmap_ = ::CreateCompatibleBitmap(hDc, width, height);
        buffer_.ExpandIfNeeded(width * height * 4);
    }

    MessageManager::Get().Add({ MessageType::WindowSizeChanged, id_, window_ });
}


void Window::DeleteBitmap()
{
    if (bitmap_ != nullptr) 
    {
        if (!::DeleteObject(bitmap_)) OutputApiError("DeleteObject");
        bitmap_ = nullptr;
    }
}


void Window::SetTexturePtr(ID3D11Texture2D* ptr)
{
    unityTexture_ = ptr;
}


ID3D11Texture2D* Window::GetTexturePtr() const
{
    return unityTexture_;
}


void Window::SetCaptureMode(CaptureMode mode)
{
    mode_ = mode;
}


CaptureMode Window::GetCaptureMode() const
{
    return mode_;
}


void Window::StartCapture()
{
    captureThreadLoop_.Start([&]
    {
        if (!IsWindow() || !IsVisible() || (mode_ == CaptureMode::None))
        {
            return;
        }

        if (isCaptureRequested_)
        {
            isCaptureRequested_ = false;
            CaptureInternal();
            RequestUpload();
        }
    });
}


void Window::StopCapture()
{
    captureThreadLoop_.Stop();
}


void Window::RequestCapture()
{
    isCaptureRequested_ = true;
}


void Window::CaptureInternal()
{
    if (!IsWindow() || !IsVisible()) return;

    auto hDc = ::GetDC(window_);

    RECT rect;
    if (FAILED(::GetWindowRect(window_, &rect)))
    {
        OutputApiError("GetWindowRect");
    }
    const auto width = rect.right - rect.left;
    const auto height = rect.bottom - rect.top;

    if (width == 0 || height == 0)
    {
        if (!::ReleaseDC(window_, hDc)) OutputApiError("ReleaseDC");
        return;
    }

    CreateBitmapIfNeeded(hDc, width, height);

    auto hDcMem = ::CreateCompatibleDC(hDc);
    HGDIOBJ preObject = ::SelectObject(hDcMem, bitmap_);

    BOOL result = false;
    switch (mode_)
    {
        case CaptureMode::PrintWindow:
        {
            result = ::PrintWindow(window_, hDcMem, PW_RENDERFULLCONTENT);
            if (!result) OutputApiError("PrintWindow");
            break;
        }
        case CaptureMode::BitBlt:
        {
            result = ::BitBlt(hDcMem, 0, 0, bufferWidth_, bufferHeight_, hDc, 0, 0, SRCCOPY);
            if (!result) OutputApiError("BitBlt");
            break;
        }
        case CaptureMode::BitBltAlpha:
        {
            result = ::BitBlt(hDcMem, 0, 0, bufferWidth_, bufferHeight_, hDc, 0, 0, SRCCOPY | CAPTUREBLT);
            if (!result) OutputApiError("BitBlt");
            break;
        }
        default:
        {
            break;
        }
    }

    if (result)
    {
        BITMAPINFOHEADER bmi {};
        bmi.biWidth       = static_cast<LONG>(width);
        bmi.biHeight      = -static_cast<LONG>(height);
        bmi.biPlanes      = 1;
        bmi.biSize        = sizeof(BITMAPINFOHEADER);
        bmi.biBitCount    = 32;
        bmi.biCompression = BI_RGB;
        bmi.biSizeImage   = 0;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!::GetDIBits(hDcMem, bitmap_, 0, height, buffer_.Get(), reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS))
            {
                OutputApiError("GetDIBits");
            }
        }
    }

    ::SelectObject(hDcMem, preObject);

    if (!::DeleteDC(hDcMem)) OutputApiError("DeleteDC");
    if (!::ReleaseDC(window_, hDc)) OutputApiError("ReleaseDC");
}


void Window::RequestUpload()
{
    if (Uploader::IsNull()) return;
    Uploader::Get().RequestUploadInBackgroundThread(id_);
}


void Window::UploadTextureToGpu()
{
    if (!unityTexture_.load() || Uploader::IsNull()) return;

    std::lock_guard<std::mutex> lock(mutex_);

    bool shouldUpdateTexture = true;

    if (sharedTexture_)
    {
        D3D11_TEXTURE2D_DESC desc;
        sharedTexture_->GetDesc(&desc);
        if (desc.Width == bufferWidth_ && desc.Height == bufferHeight_)
        {
            shouldUpdateTexture = false;
        }
    }

    if (shouldUpdateTexture)
    {
        sharedTexture_ = Uploader::Get().CreateCompatibleSharedTexture(unityTexture_.load());

        if (!sharedTexture_)
        {
            Debug::Error(__FUNCTION__, " => Shared texture is null.");
            return;
        }

        ComPtr<IDXGIResource> dxgiResource;
        sharedTexture_.As(&dxgiResource);
        if (FAILED(dxgiResource->GetSharedHandle(&sharedHandle_)))
        {
            Debug::Error(__FUNCTION__, " => GetSharedHandle() failed.");
            return;
        }
    }

    {
        ComPtr<ID3D11DeviceContext> context;
        Uploader::Get().GetDevice()->GetImmediateContext(&context);
        context->UpdateSubresource(sharedTexture_.Get(), 0, nullptr, buffer_.Get(), bufferWidth_ * 4, 0);
        context->Flush();
    }

    hasNewTextureUploaded_ = true;
}


void Window::Render()
{
    if (!hasNewTextureUploaded_) return;
    hasNewTextureUploaded_ = false;

    if (unityTexture_.load() && sharedTexture_ && sharedHandle_)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        ComPtr<ID3D11DeviceContext> context;
        GetUnityDevice()->GetImmediateContext(&context);

        ComPtr<ID3D11Texture2D> texture;
        if (FAILED(GetUnityDevice()->OpenSharedResource(sharedHandle_, __uuidof(ID3D11Texture2D), &texture)))
        {
            Debug::Error(__FUNCTION__, " => OpenSharedResource() failed.");
            return;
        }

        context->CopyResource(unityTexture_.load(), texture.Get());

        MessageManager::Get().Add({ MessageType::WindowCaptured, id_, window_ });
    }
}