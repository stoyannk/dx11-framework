// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"
#include "ChildWindow.h"

ChildWindow::ChildWindow()
	: m_hWindow(0)
	, m_Width(0)
	, m_Height(0)
{}

void ChildWindow::Start(char* className, char* windowName, unsigned width, unsigned height, HINSTANCE instance, WNDPROC winProc)
{
	m_Width = width;
	m_Height = height;
	m_ClassName = className;
	m_WindowName = windowName;

	m_Thread.reset(new boost::thread(std::bind(&ChildWindow::Run, shared_from_this(), instance, winProc)));
}
	
void ChildWindow::Run(HINSTANCE instance, WNDPROC winProc)
{
	if(!Initialize(instance, winProc))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to start child window ", m_WindowName);
		return;
	}

	DoMessageLoop();
}
 
bool ChildWindow::Initialize(HINSTANCE instance, WNDPROC winProc)
{
	// Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = winProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = instance;
    wcex.hIcon = LoadIcon(instance, (LPCTSTR)IDI_APPLICATION);
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
	m_hWindow = ::CreateWindow(m_ClassName.c_str()
								, m_WindowName.c_str()
								, WS_OVERLAPPEDWINDOW
								, CW_USEDEFAULT
								, CW_USEDEFAULT
								, rc.right - rc.left, rc.bottom - rc.top
								, NULL
								, NULL
								, instance
								, NULL);
    if(!m_hWindow)
	{
		return false;
	}

    ShowWindow(m_hWindow, SW_SHOW);

    return true;
}

void ChildWindow::DoMessageLoop()
{
	// Main message loop
    MSG msg = {0};
	while(WM_QUIT != msg.message)
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
			TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
	}
}
 