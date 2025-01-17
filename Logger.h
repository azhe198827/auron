/*!
* \brief
* A simple logging class
*
* \file
*
* This logger can log either into a file, or the standard output.
*
* \copyright
* Copyright 1993-2014 NVIDIA Corporation.  All rights reserved.
*
* NOTICE TO LICENSEE:
*
* This source code and/or documentation ("Licensed Deliverables") are
* subject to NVIDIA intellectual property rights under U.S. and
* international Copyright laws.
*
* These Licensed Deliverables contained herein is PROPRIETARY and
* CONFIDENTIAL to NVIDIA and is being provided under the terms and
* conditions of a form of NVIDIA software license agreement by and
* between NVIDIA and Licensee ("License Agreement") or electronically
* accepted by Licensee.  Notwithstanding any terms or conditions to
* the contrary in the License Agreement, reproduction or disclosure
* of the Licensed Deliverables to any third party without the express
* written consent of NVIDIA is prohibited.
*
* NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
* LICENSE AGREEMENT, NVIDIA MAKES NO REPRESENTATION ABOUT THE
* SUITABILITY OF THESE LICENSED DELIVERABLES FOR ANY PURPOSE.  IT IS
* PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.
* NVIDIA DISCLAIMS ALL WARRANTIES WITH REGARD TO THESE LICENSED
* DELIVERABLES, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY,
* NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
* NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
* LICENSE AGREEMENT, IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY
* SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
* DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
* WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
* ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
* OF THESE LICENSED DELIVERABLES.
*
* U.S. Government End Users.  These Licensed Deliverables are a
* "commercial item" as that term is defined at 48 C.F.R. 2.101 (OCT
* 1995), consisting of "commercial computer software" and "commercial
* computer software documentation" as such terms are used in 48
* C.F.R. 12.212 (SEPT 1995) and is provided to the U.S. Government
* only as a commercial end item.  Consistent with 48 C.F.R.12.212 and
* 48 C.F.R. 227.7202-1 through 227.7202-4 (JUNE 1995), all
* U.S. Government End Users acquire the Licensed Deliverables with
* only those rights set forth herein.
*
* Any use of the Licensed Deliverables in individual and commercial
* software must include, in the user documentation and internal
* comments to the code, the above Disclaimer and U.S. Government End
* Users Notice.
*/

#pragma once

#include "boost/date_time/posix_time/posix_time.hpp"
namespace pt = boost::posix_time;

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <mutex>
#include <time.h>

#ifdef _WIN32
#include <winsock.h>


#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SOCKET int
#define INVALID_SOCKET -1
#endif

namespace simplelogger{

	enum LogLevel {
		TRACE,
		DEBUG,
		INFO,
		WARN,
		ERR
	};

	class Logger {
	public:
		Logger(LogLevel level, bool bPrintTimeStamp) : level(level), bPrintTimeStamp(bPrintTimeStamp) {}
		virtual ~Logger() {}
		virtual std::ostream& GetStream() = 0;
		virtual void FlushStream() {}
		bool ShouldLogFor(LogLevel l) {
			return l >= level;
		}
		char* GetLead(LogLevel l, const char *szFile, int nLine, const char *szFunc) {
			if (l < TRACE || l > ERR) {
				sprintf(szLead, "[?????] ");
				return szLead;
			}
			const char *szLevels[] = { "TRACE", "DEBUG", "INFO", "WARN", "ERROR" };
			if (bPrintTimeStamp) {
				time_t t = time(NULL);
				//struct tm *ptm = localtime(&t);
				//sprintf(szLead, "[%-5s][%02d:%02d:%02d] ",
				//	szLevels[l], ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

				pt::ptime current_date_microseconds = pt::microsec_clock::local_time();
				std::string p = pt::to_simple_string(current_date_microseconds);

				sprintf(szLead, "[%-5s][%s] ", szLevels[l], p.c_str());
			}
			else {
				sprintf(szLead, "[%-5s] ", szLevels[l]);
			}
			return szLead;
		}
		void EnterCriticalSection() {
			mtx.lock();
		}
		void LeaveCriticalSection() {
			mtx.unlock();
		}
	private:
		LogLevel level;
		char szLead[80];
		bool bPrintTimeStamp;
		std::mutex mtx;
	};

	class LoggerFactory {
	public:
		static Logger* CreateFileLogger(std::string strFilePath,
			LogLevel level = DEBUG, bool bPrintTimeStamp = true) {
			return new FileLogger(strFilePath, level, bPrintTimeStamp);
		}
		static Logger* CreateConsoleLogger(LogLevel level = DEBUG,
			bool bPrintTimeStamp = true) {
			return new ConsoleLogger(level, bPrintTimeStamp);
		}
		static Logger* CreateUdpLogger(char *szHost, unsigned uPort, LogLevel level = DEBUG,
			bool bPrintTimeStamp = true) {
			return new UdpLogger(szHost, uPort, level, bPrintTimeStamp);
		}
	private:
		LoggerFactory() {}

		class FileLogger : public Logger {
		public:
			FileLogger(std::string strFilePath, LogLevel level, bool bPrintTimeStamp)
				: Logger(level, bPrintTimeStamp) {
				pFileOut = new std::ofstream();
				pFileOut->open(strFilePath.c_str());
			}
			~FileLogger() {
				pFileOut->close();
			}
			std::ostream& GetStream() {
				return *pFileOut;
			}
		private:
			std::ofstream *pFileOut;
		};

		class ConsoleLogger : public Logger {
		public:
			ConsoleLogger(LogLevel level, bool bPrintTimeStamp)
				: Logger(level, bPrintTimeStamp) {}
			std::ostream& GetStream() {
				return std::cout;
			}
		};

		class UdpLogger : public Logger {
		private:
			class UdpOstream : public std::ostream {
			public:
				UdpOstream(char *szHost, unsigned short uPort) : std::ostream(&sb), socket(INVALID_SOCKET){
#ifdef _WIN32
					WSADATA w;
					if (WSAStartup(0x0101, &w) != 0) {
						fprintf(stderr, "WSAStartup() failed.\n");
						return;
					}
#endif
					socket = ::socket(AF_INET, SOCK_DGRAM, 0);
					if (socket == INVALID_SOCKET) {
#ifdef _WIN32
						WSACleanup();
#endif
						fprintf(stderr, "socket() failed.\n");
						return;
					}
#ifdef _WIN32
					unsigned int b1, b2, b3, b4;
					sscanf(szHost, "%u.%u.%u.%u", &b1, &b2, &b3, &b4);
					struct in_addr addr = { (unsigned char)b1, (unsigned char)b2, (unsigned char)b3, (unsigned char)b4 };
#else
					struct in_addr addr = { inet_addr(szHost) };
#endif
					struct sockaddr_in s = { AF_INET, htons(uPort), addr };
					server = s;
				}
				~UdpOstream() {
					if (socket == INVALID_SOCKET) {
						return;
					}
#ifdef _WIN32
					closesocket(socket);
					WSACleanup();
#else
					close(socket);
#endif
				}
				void Flush() {
					if (sendto(socket, sb.str().c_str(), (int)sb.str().length() + 1,
						0, (struct sockaddr *)&server, (int)sizeof(sockaddr_in)) == -1) {
						fprintf(stderr, "sendto() failed.\n");
					}
					sb.str("");
				}

			private:
				std::stringbuf sb;
				SOCKET socket;
				struct sockaddr_in server;
			};
		public:
			UdpLogger(char *szHost, unsigned uPort, LogLevel level, bool bPrintTimeStamp)
				: Logger(level, bPrintTimeStamp), udpOut(szHost, (unsigned short)uPort) {}
			UdpOstream& GetStream() {
				return udpOut;
			}
			virtual void FlushStream() {
				udpOut.Flush();
			}
		private:
			UdpOstream udpOut;
		};
	};

}

#define LOG(pLogger, event, level) \
	do {													\
		if (!pLogger || !pLogger->ShouldLogFor(level)) {	\
			break;											\
				}													\
		pLogger->EnterCriticalSection();					\
		pLogger->GetStream()								\
			<< pLogger->GetLead(level, __FILE__, __LINE__,	\
				__FUNCTION__)								\
			<< event << std::endl;							\
		pLogger->FlushStream();								\
		pLogger->LeaveCriticalSection();					\
		} while (0);

#define LOG_TRACE(pLogger, event)	LOG(pLogger, event, simplelogger::TRACE)
#define LOG_DEBUG(pLogger, event)	LOG(pLogger, event, simplelogger::DEBUG)
#define LOG_INFO(pLogger, event)	LOG(pLogger, event, simplelogger::INFO)
#define LOG_WARN(pLogger, event)	LOG(pLogger, event, simplelogger::WARN)
#define LOG_ERROR(pLogger, event)	LOG(pLogger, event, simplelogger::ERR)

namespace sl = simplelogger;

template<sl::LogLevel level>
class GlobalLogger
{
public:
	static sl::Logger * Get(std::string root_path = "")
	{
		//static sl::Logger *log = sl::LoggerFactory::CreateConsoleLogger(level);
		static sl::Logger *log 
			= sl::LoggerFactory::CreateFileLogger(
			(boost::format("%s\\VSK_LOG\\%s_%d.txt") 
				% root_path.c_str()
				% "GPUlog" 
				//% GetCurrentThreadId()).str().c_str(),
				% GetCurrentProcessId()).str().c_str(), 
				level);
	
		return log;
	}
};