#pragma once

#include <tuple>

#include "LogBuffer.h"

#ifndef MINIMAL_SIZE
#define STLOG(SEVERITY, FACILITY, DATA) \
	Logging::Logger::Get().Log(SEVERITY, FACILITY, __LINE__, __FILE__, DATA)

#define SLOG(SEVERITY, FACILITY, ...) \
	Logging::Logger::Get().Log(Logging::SEVERITY, Logging::FACILITY, __LINE__, __FILE__, std::tie(__VA_ARGS__))

#define SLLOG(SEVERITY, FACILITY, ...) \
	Logging::Logger::Get().Log(Logging::SEVERITY, Logging::FACILITY, __LINE__, __FILE__, std::make_tuple(__VA_ARGS__))
#else
#define STLOG(SEVERITY, FACILITY, DATA)

#define SLOG(SEVERITY, FACILITY, ...)

#define SLLOG(SEVERITY, FACILITY, ...)
#endif

namespace Logging
{
	class Logger
	{
	public:
		static void Initialize(size_t size = 1024*2);
		static void Deinitialize();
		static Logger& Get();

		// Gains ownership
		void AddTarget(std::ostream* stream, bool takeOwnership = true);

		void SetMinimalLogSeverity(Severity severity)
		{
			m_MinimalLogSeverity = int(severity);
		}

		~Logger();

		void Log(Severity severity
			, Facility facility
			, unsigned line
			, const std::string& file
			, const std::string& data);

		template<typename Tuple>
		void Log(Severity severity
			, Facility facility
			, unsigned line
			, const std::string& file
			, const Tuple& data);

	private:

		class RealLogger
		{
		public:
			RealLogger(Logger* parent)
				: m_Parent(parent)
			{}
			void operator()();

		private:
			void Log() const;
			Logger* m_Parent;
		};
		friend class RealLogger;

		static Logger* Instance;

		Logger(size_t size);
		Logger(const Logger&);
		Logger& operator=(const Logger&);

		boost::scoped_ptr<LogBuffer> m_Buffer;

		typedef std::vector<std::pair<std::ostream*, bool>> Targets;
		Targets m_Targets;

		std::mutex m_TargetsMutex;

		std::thread m_LoggingThread;
		bool m_Run;

		HANDLE m_BufferLoggedEvent;
		HANDLE m_EndEvent;

		int m_MinimalLogSeverity;

	private:
		template<typename Tuple, unsigned N>
		struct UnpackLogData
		{
			static void Unpack(const Tuple& data, std::ostringstream& stream)
			{
				UnpackLogData<Tuple, N-1>::Unpack(data, stream);
				stream << std::get<N>(data);
			}
		};

		template<typename Tuple>
		struct UnpackLogData<Tuple, 0>
		{
			static void Unpack(const Tuple& data, std::ostringstream& stream)
			{
				stream << std::get<0>(data);
			}
		};
	};

	template<typename Tuple>
	void Logger::Log(Severity severity
			, Facility facility
			, unsigned line
			, const std::string& file
			, const Tuple& data)
	{
		if (m_MinimalLogSeverity > int(severity))
			return;

		std::ostringstream unpackedData;

		UnpackLogData<Tuple, std::tuple_size<Tuple>::value - 1>::Unpack(data, unpackedData);

		Log(severity, facility, line, file, unpackedData.str());
	}
}