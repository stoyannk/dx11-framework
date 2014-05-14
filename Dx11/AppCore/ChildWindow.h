// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

class ChildWindow : public std::enable_shared_from_this<ChildWindow>
{
public:
	ChildWindow();
	
	void Start(char* className, char* windowName, unsigned width, unsigned height, HINSTANCE instance, WNDPROC winProc);

	virtual void DoMessageLoop();

private:
	void Run(HINSTANCE instance, WNDPROC winProc);

	bool Initialize(HINSTANCE instance, WNDPROC winProc);

private:
	HWND m_hWindow;

	unsigned m_Width;
	unsigned m_Height;

	std::string m_ClassName;
	std::string m_WindowName;

	std::unique_ptr<std::thread> m_Thread;
};