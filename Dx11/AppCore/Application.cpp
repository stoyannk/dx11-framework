// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#include <Utilities\Assertions.h>

#include "Application.h"
#include "ChildWindow.h"

#include <numeric>

Application::FPSEstimator::FPSEstimator()
	: Buffer(10)
{}

void Application::FPSEstimator::AddFrame(float frameTime)
{
	Buffer.push_back(frameTime);
}

float Application::FPSEstimator::EstimateFPS() const
{
	float sum = std::accumulate(Buffer.begin(), Buffer.end(), 0.f);

	return sum / Buffer.size();
}

Application::Application(HINSTANCE instance)
	: m_hInstance(instance)
	, m_hWindow(0)
	, m_Run(false)
	, m_Width(0)
	, m_Height(0)
	, m_Delta(0)
{
	LARGE_INTEGER freq;
	::QueryPerformanceFrequency(&freq);

	m_PerfFrequency = freq.QuadPart;
}

Application::~Application()
{}

bool Application::Initiate(char* className, char* windowName, unsigned width, unsigned height, bool fullscreen, WNDPROC winProc, bool sRGBRT, int samplesCnt)
{
	Assert(!m_hWindow, false);

	m_Width = width;
	m_Height = height;

	m_ClassName = className;
	m_WindowName = windowName;

	// Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = winProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = m_hInstance;
    wcex.hIcon = LoadIcon(m_hInstance, (LPCTSTR)IDI_APPLICATION);
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
	wcex.lpszClassName = m_ClassName.c_str();
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_APPLICATION);
    if(!RegisterClassEx(&wcex))
	{
        return false;
	}

    // Create window
    RECT rc = { 0, 0, m_Width, m_Height };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	m_hWindow = ::CreateWindowEx(NULL
								, m_ClassName.c_str()
								, m_WindowName.c_str()
								, fullscreen ? (WS_EX_TOPMOST | WS_POPUP) : WS_OVERLAPPEDWINDOW
								, fullscreen ? 0 : CW_USEDEFAULT
								, fullscreen ? 0 : CW_USEDEFAULT
								, fullscreen ? m_Width : (rc.right - rc.left)
								, fullscreen ? m_Height : (rc.bottom - rc.top)
								, NULL
								, NULL
								, m_hInstance
								, NULL);
    if(!m_hWindow)
	{
		return false;
	}

    ShowWindow(m_hWindow, SW_SHOW);

	SetRunning(true);

	// Set the thread affinity mask for better clock
	::SetThreadAffinityMask(::GetCurrentThread(), 1);

	LARGE_INTEGER ticks;
	::QueryPerformanceCounter(&ticks);
	m_LastTick = ticks.QuadPart;

    return true;
}

void Application::Deinit()
{
}

void Application::AddChild(char* className, char* windowName, unsigned width, unsigned height, WNDPROC winProc)
{
	std::shared_ptr<ChildWindow> child(new ChildWindow);
	child->Start(className, windowName, width, height, m_hInstance, winProc);
	m_Children.push_back(child);
}

void Application::DoMessageLoop()
{
	// Main message loop
    MSG msg = {0};
	while(WM_QUIT != msg.message && m_Run)
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            HandleWindowsMessage(msg);

			TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Run();

			static char title[256] = "";
			sprintf(title, "FPS: %f", 1.f / m_FPSEstimator.EstimateFPS());
			::SetWindowText(m_hWindow, title);
        }
	}

	Deinit();
}

void Application::Run()
{
	LARGE_INTEGER ticks;
	::QueryPerformanceCounter(&ticks);

	m_Delta	= (ticks.QuadPart - m_LastTick) / float(m_PerfFrequency);
	m_LastTick = ticks.QuadPart;

	m_FPSEstimator.AddFrame(m_Delta);

	Update(m_Delta);
}

void Application::Update(float delta)
{}

void Application::HandleWindowsMessage(const MSG& msg)
{
	switch(msg.message)
	{
		case WM_KEYDOWN:
			KeyDown(msg.wParam);
			break;

		case WM_KEYUP:
			KeyUp(msg.wParam);
			break;

		case WM_MOUSEMOVE:
			MouseMove(LOWORD(msg.lParam), HIWORD(msg.lParam));
			break;

		case WM_LBUTTONDOWN:
			MouseButtonDown(MBT_Left, LOWORD(msg.lParam), HIWORD(msg.lParam));
			break;
		case WM_MBUTTONDOWN:
			MouseButtonDown(MBT_Middle, LOWORD(msg.lParam), HIWORD(msg.lParam));
			break;
		case WM_RBUTTONDOWN:
			MouseButtonDown(MBT_Right, LOWORD(msg.lParam), HIWORD(msg.lParam));
			break;

		case WM_LBUTTONUP:
			MouseButtonUp(MBT_Left, LOWORD(msg.lParam), HIWORD(msg.lParam));
			break;
		case WM_MBUTTONUP:
			MouseButtonUp(MBT_Middle, LOWORD(msg.lParam), HIWORD(msg.lParam));
			break;
		case WM_RBUTTONUP:
			MouseButtonUp(MBT_Right, LOWORD(msg.lParam), HIWORD(msg.lParam));
			break;

		default:
			break;
	}
}

float Application::GetLastFrameTime() const
{
	return m_Delta;
}

void Application::KeyDown(unsigned int key)
{}
		
void Application::KeyUp(unsigned int key)
{}

void Application::MouseMove(int x, int y)
{}

void Application::MouseButtonDown(MouseBtn button, int x, int y)
{}

void Application::MouseButtonUp(MouseBtn button, int x, int y)
{}

void Application::PreRender()
{}

void Application::PostRender()
{}
