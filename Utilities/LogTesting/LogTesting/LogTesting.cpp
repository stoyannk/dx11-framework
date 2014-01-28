// LogTesting.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "../../Logger.h"

using namespace Logging;

struct LogTester
{
	static const int iter = 1024;
	LogTester(int id) : m_Id(id)
	{}

	void operator()()
	{
		std::ostringstream mydata;
		mydata << "Thread data ";
		mydata << m_Id;
		mydata << " iteration: ";

		for(int i=0; i<iter; ++i)
		{
			std::ostringstream stream;
			stream << mydata.str() << i;

			//STLOG(Sev_Error, Fac_Rendering, stream.str());
			STLOG(Sev_Error, Fac_Rendering, std::make_tuple(mydata.str(), " || ", i));

			Sleep(1);
		}
	}

	int m_Id;
};

int _tmain(int argc, _TCHAR* argv[])
{
	Logger::Initialize();

	Logger::Get().AddTarget(new std::ofstream("output.txt"));

	static const int threadsCount = 10;
	boost::thread threads[threadsCount]; 

	for(int i=0; i<threadsCount; ++i)
	{
		threads[i] = boost::thread(LogTester(i));
	}

	for(int i=0; i<threadsCount; ++i)
	{
		threads[i].join();
	}
	
	Logger::Deinitialize();

	return 0;
}

