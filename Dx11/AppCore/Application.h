// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

class ChildWindow;

class Application
{
public:
	enum MouseBtn {
		MBT_Left,
		MBT_Middle,
		MBT_Right
	};

	Application(HINSTANCE instance);
	virtual ~Application();

	bool Initiate(char* className, char* windowName, unsigned width, unsigned height, bool fullscreen, WNDPROC winProc, bool sRGBRT = true, int samplesCnt = 1);

	virtual void Deinit();

	void AddChild(char* className, char* windowName, unsigned width, unsigned height, WNDPROC winProc);

	virtual void DoMessageLoop();

	virtual void Run();

	virtual void PreRender();
	virtual void PostRender();

	virtual void Update(float delta);

	virtual void HandleWindowsMessage(const MSG& msg);

	virtual void KeyDown(unsigned int key);
		
	virtual void KeyUp(unsigned int key);

	virtual void MouseMove(int x, int y);

	virtual void MouseButtonDown(MouseBtn button, int x, int y);

	virtual void MouseButtonUp(MouseBtn button, int x, int y);

	HINSTANCE GetInstance() const;

	HWND GetHandle() const;

	void SetRunning(bool run);

	bool IsRunning() const;

	bool IsInitiated() const;

	unsigned GetWidth() const;

	unsigned GetHeight() const;

	float GetLastFrameTime() const;

private:
	struct FPSEstimator
	{
		FPSEstimator();
		void AddFrame(float frameTime);
		float EstimateFPS() const;

	private:
#ifndef MINIMAL_SIZE
		boost::circular_buffer<float> Buffer;
#else
		float m_LastFrameTime;
#endif
	};

	typedef std::vector<std::shared_ptr<ChildWindow>> ChildrenVec;
	ChildrenVec m_Children;

	HINSTANCE m_hInstance;
	HWND m_hWindow;

	unsigned m_Width;
	unsigned m_Height;

	std::string m_ClassName;
	std::string m_WindowName;

	bool m_Run;

	FPSEstimator m_FPSEstimator;

	long long int m_LastTick;
	long long int m_PerfFrequency;
	float m_Delta;
};

inline HINSTANCE Application::GetInstance() const
{
	return m_hInstance;
}

inline HWND Application::GetHandle() const
{
	return m_hWindow;
}

inline void Application::SetRunning(bool run)
{
	m_Run = run;
}

inline bool Application::IsRunning() const
{
	return m_Run;
}

inline bool Application::IsInitiated() const
{
	return !!m_hWindow;
}

inline unsigned Application::GetWidth() const
{
	return m_Width;
}

inline unsigned Application::GetHeight() const
{
	return m_Height;
}
