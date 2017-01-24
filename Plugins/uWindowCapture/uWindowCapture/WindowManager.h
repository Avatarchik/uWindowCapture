#pragma once

#include <Windows.h>
#include <map>
#include <vector>
#include <deque>
#include <memory>
#include <mutex>

#include "Singleton.h"
#include "Thread.h"
#include "CaptureManager.h"
#include "UploadManager.h"


class Window;


class WindowManager
{
    UWC_SINGLETON(WindowManager)

public:
    void Initialize();
    void Finalize();
    void Update();
    void Render();
    std::shared_ptr<Window> GetWindow(int id) const;
    std::shared_ptr<Window> GetWindowFromPoint(POINT point) const;
    std::shared_ptr<Window> GetCursorWindow() const;

    static const std::unique_ptr<CaptureManager>& GetCaptureManager();
    static const std::unique_ptr<UploadManager>& GetUploadManager();

private:
    std::shared_ptr<Window> FindParentWindow(const std::shared_ptr<Window>& window) const;
    std::shared_ptr<Window> FindOrAddWindow(HWND hwnd);

    void StartWindowHandleListThread();
    void StopWindowHandleListThread();
    void UpdateWindowHandleList();

    void UpdateWindows();
    void RenderWindows();

    std::unique_ptr<CaptureManager> captureManager_;
    std::unique_ptr<UploadManager> uploadManager_;

    std::map<int, std::shared_ptr<Window>> windows_;
    int lastWindowId_ = 0;
    std::weak_ptr<Window> cursorWindow_;

    ThreadLoop windowHandleListThreadLoop_;

    struct WindowInfo
    {
        HWND hWnd;
        HWND hOwner;
        HWND hParent;
        HINSTANCE hInstance;
        DWORD processId;
        DWORD threadId;
        RECT rect;
        UINT zOrder;
        std::wstring title;
    };
    std::vector<WindowInfo> windowInfoList_[2];
    mutable std::mutex windowsHandleListMutex_;
};

