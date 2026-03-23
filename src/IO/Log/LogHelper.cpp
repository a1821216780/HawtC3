//**********************************************************************************************************************************
//LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,赵子祯
//
//    This file is part of HawtC3.IO.Log.
//
// Licensed under the Boost Software License - Version 1.0 - August 17th, 2003
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.HawtC.cn/licenses.txt
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//**********************************************************************************************************************************

#include "LogHelper.h";
#include "../IO/OtherHelper.h";
#include "../IO/TimesHelper.h";
#include "../IO/Extensions.h";
#include "../System/Console.h";
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <csignal>
#include <ctime>
#include <algorithm>



//
//#ifdef _WIN32
//#include <conio.h>
//#endif


using namespace HawtC3::IO::IO;
using namespace HawtC3::IO::System;

namespace HawtC3 {
	namespace IO {
		namespace Log {

			// Static member definitions
			std::shared_ptr<Email> Datastore::EmailSet = nullptr;

			//std::string LogData::LogPath = "";
			//std::vector<std::shared_ptr<IOutFile*>> LogData::OutFilelist;
			//std::vector<std::shared_ptr<BASE_VTK>> LogData::IO_VTK;
			//std::vector<std::string> LogData::log_inf;

			int LogHelper::mode = 0;
			bool LogHelper::firstshowinformation = true;


			// LogData implementation
			void LogData::Add(const std::string& message) {
				if (log_inf.empty()) {
					throw std::runtime_error("Please call DisplayInformation function first!");
				}
				else {
					log_inf.push_back(message);
				}
			}

			void LogData::Initialize() {
				log_inf.clear();
				log_inf.push_back("----------! HawtC3." + OtherHelper::GetCurrentProjectName() + " Log file. !----------");
				OutFilelist.clear();
				// Initialize LogPath
				std::string currentPath = Extensions::GetABSPath(std::filesystem::current_path().string());
				LogPath = Extensions::GetABSPath(currentPath + "./HawtC3."+ OtherHelper::GetCurrentProjectName()+  ".log");

			}

			void LogHelper::UnhandledExceptionHandler(int signal) {
				std::string signalName;
				switch (signal) {
				case SIGINT: signalName = "SIGINT"; break;
				case SIGTERM: signalName = "SIGTERM"; break;
				case SIGSEGV: signalName = "SIGSEGV"; break;
				case SIGFPE: signalName = "SIGFPE"; break;
				default: signalName = "Unknown Signal"; break;
				}

				ErrorLog("未知错误：Signal " + signalName, "", "", 20, "UnhandledExceptionHandler");
				EndProgram(true, "", true);
			}

			void LogHelper::DisinfV1(bool lics) {
				// Set console title (Windows only)
				std::filesystem::path exepath = std::filesystem::current_path() / OtherHelper::GetCurrentExeName();


				std::string title = OtherHelper::GetCurrentProjectName() + OtherHelper::GetBuildMode() + "_" +
					OtherHelper::GetCurrentVersion(exepath.string()) + "  " + OtherHelper::GetCurrentBuildMode();

				Console::SetTitle(title.c_str());


				int lp = 100;

				WriteLog("Debug 调试模式 develop by 赵子祯 Debug 调试模式 develop by 赵子祯 Debug 调试模式 develop by 赵子祯", "",
					false, "", ConsoleColor::Green);
				WriteLog(OtherHelper::CenterText("------------------------------------------------------------------------------------------", lp),
					"", false, "", ConsoleColor::Cyan);
				WriteLog(OtherHelper::CenterText("!       000                                00     00    00  0000000   00000  000000      !", lp),
					"", false, "", ConsoleColor::Cyan);
				WriteLog(OtherHelper::CenterText("!     0     0    000      000      00000   00    00    00  0        0       00     00    !", lp),
					"", false, "", ConsoleColor::Cyan);
				WriteLog(OtherHelper::CenterText("!     0     0  0     0  0     0   0     0  00   0000   00  000000  0       00      00    !", lp),
					"", false, "", ConsoleColor::Cyan);
				WriteLog(OtherHelper::CenterText("!      000    0      0 0       0 0     0    0  0   0  0   0        0       00     00     !", lp),
					"", false, "", ConsoleColor::Cyan);
				WriteLog(OtherHelper::CenterText("!             0 0000   0  000    0     0     00     00    0000000   00000  0000000       !", lp),
					"", false, "", ConsoleColor::Cyan);
				WriteLog(OtherHelper::CenterText("!             0        0     0                                                           !", lp),
					"", false, "", ConsoleColor::Cyan);
				WriteLog(OtherHelper::CenterText("!            0          0000                                                             !", lp),
					"", false, "", ConsoleColor::Cyan);
				WriteLog(OtherHelper::CenterText("!                                                                                        !", lp),
					"", false, "", ConsoleColor::Cyan);
				WriteLog(OtherHelper::CenterText("!                    Copyright (c) HawtC2.Team.ZZZ,赵子祯 licensed under GPL v3 .                   !", lp),
					"", false, "", ConsoleColor::Red);
				WriteLog(OtherHelper::CenterText("!                                                                                        !", lp),
					"", false, "", ConsoleColor::Cyan);
				WriteLog(OtherHelper::CenterText("!   Copyright (c)  Key Laboratory of Jiangsu province High-Tech design of wind turbine   !", lp),
					"", false, "", ConsoleColor::Green);
				WriteLog(OtherHelper::CenterText("!                                                                                        !", lp),
					"", false, "", ConsoleColor::Green);

				WriteLog(OtherHelper::CenterText("******                 Running " + OtherHelper::GetCurrentProjectName() +
					" (v" + OtherHelper::GetCurrentVersion(exepath.string()) + "  " + std::to_string(TimesHelper::GetCurrentYear()) + "-" +
					std::to_string(TimesHelper::GetCurrentMonth()) + ")                  ******", lp),
					"", false, "[Message]", ConsoleColor::Green);
				WriteLog(OtherHelper::CenterText("------------------------------------------------------------------------------------------", lp),
					"", false, "", ConsoleColor::Cyan);
				WriteLog("Debug 调试模式 develop by 赵子祯 Debug 调试模式 develop by 赵子祯 Debug 调试模式 develop by 赵子祯", "",
					false, "", ConsoleColor::Green);
				WriteLog("Debug 调试模式 develop by 赵子祯 Debug 调试模式 develop by 赵子祯 Debug 调试模式 develop by 赵子祯", "",
					false, "", ConsoleColor::Green);
				WriteLog(OtherHelper::GetMathAcc());

				mode = 0;
			}

			void LogHelper::DisinfV2(bool lics) {
				std::string url = "http://www.HawtC.cn";

				WriteLog("HawtC3." + OtherHelper::GetCurrentProjectName() + " - Tel:13935201274  E:1821216780@qq.com " + url,
					"", false);
				std::cout << "************************************************************************************" << '\n';

				std::filesystem::path exepath = std::filesystem::current_path() / OtherHelper::GetCurrentExeName();

				WriteLog(OtherHelper::GetCurrentProjectName() + " Vision:V" + OtherHelper::GetCurrentVersion(exepath.string()) + "_" +
					OtherHelper::GetCurrentBuildMode() + OtherHelper::GetBuildMode() + ",BuildAt: " +
					OtherHelper::GetBuildTime() + " Math:" + OtherHelper::GetMathAcc(),
					"", false, "", ConsoleColor::Green);
				WriteLog(" > Publish: Powered by Key Laboratory of Jiangsu province high tech design @Tg Team",
					"", false);


				firstshowinformation = false;
				mode = 1;
			}

			void LogHelper::DisplayInformation(bool UnhandledException, bool lic) {
				if (firstshowinformation) {
					// Initialize LogData
					LogData::Initialize();

					if (OtherHelper::GetCurrentBuildMode() == "Debug") {
						DisinfV1(lic);
					}
					else {
						DisinfV2(lic);
					}
					firstshowinformation = false;

					if (OtherHelper::GetCurrentBuildMode() == "Release") {
						if (UnhandledException) {
							std::signal(SIGINT, UnhandledExceptionHandler);
							std::signal(SIGTERM, UnhandledExceptionHandler);
							std::signal(SIGSEGV, UnhandledExceptionHandler);
							std::signal(SIGFPE, UnhandledExceptionHandler);
						}
					}
				}
			}

			void LogHelper::EndInformation(double tfinal, double dt, const std::chrono::steady_clock::time_point& start_time) {
				auto end_time = std::chrono::steady_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
				double elapsedTime = duration.count() / 1000.0;

				std::string timeString = (elapsedTime > 60.0) ?
					std::to_string(elapsedTime / 60.0) + " min" :
					std::to_string(elapsedTime) + " sec";

				std::cout << '\n';
				WriteLog("Simulation Run Finished! The tf= " + std::to_string(tfinal) + "s Step=" +
					std::to_string(dt) + "s Cost real time=" + timeString, "", true, "[Message]",
					ConsoleColor::White, true, 0);
			}

			void LogHelper::EndProgram(bool forceTerminate, const std::string& outstring, bool keepstay,
				bool extcall, int sleeptime) {
				std::string message;
				if (outstring.empty()) {
					message = forceTerminate ?
						"************* ! " + OtherHelper::FillString(OtherHelper::GetCurrentProjectName() + ".RUN.", " ", 1) + " E R R O R ! **************" :
						OtherHelper::GetCurrentProjectName() + " Run completed Normaly!";
				}
				else {
					message = outstring;
				}

				LogData::Add(message);

				// Write all logs to file
				std::ofstream logFile(LogData::LogPath);
				for (const auto& line : LogData::log_inf) {
					logFile << line << '\n';
				}
				logFile.close();

				// Close VTK files
				for (auto& vtkFile : LogData::IO_VTK) {
					// vtkFile->OutVTKClose(); // Would need proper implementation
				}

				// Write output files
				for (auto& outFile : LogData::OutFilelist) {
					WriteLogO(std::string("Write ") + "output file" + " Out File!");
					// outFile->Outfinish(false); // Would need proper implementation
				}

				if (!forceTerminate) {
					std::cout << '\n';
					WriteLog(OtherHelper::GetCurrentProjectName() + " terminated normally!",
						"", false, "", ConsoleColor::Green);
					std::this_thread::sleep_for(std::chrono::milliseconds(sleeptime));

					if (keepstay) {
						std::cin.get();
					}
					else {
						if (!extcall) {
							throw std::runtime_error("");
							//std::exit(0);
						}
					}
				}
				else {
					std::cout << '\n';
					WriteLog(" Aborting " + OtherHelper::GetCurrentProjectName(),
						"", false, "", ConsoleColor::Red);

					std::this_thread::sleep_for(std::chrono::milliseconds(sleeptime));

					if (!keepstay) {
						if (!extcall) {
							if (OtherHelper::GetCurrentProjectName() != "dotnet") {
								throw std::runtime_error("");
								//std::exit(0);
							}
						}
					}
					else {
						std::cin.get();
					}
				}
			}

			void LogHelper::WarnLog(const std::string& debugmessage, const std::string& relasemessage,
				ConsoleColor color1, int leval, const std::string& FunctionName) {
				std::string message = relasemessage.empty() ? debugmessage : relasemessage;

				if (mode == 0) {
					std::string formattedMessage = FunctionName + " : " + debugmessage;
					WriteLog(formattedMessage, "", true, "[WARNING]", color1, true, leval);
				}
				else {
					std::string formattedMessage = FunctionName + " : " + message;
					WriteLog(formattedMessage, "", true, "[WARNING]", color1, true, leval);
				}
			}

			void LogHelper::ErrorLog(const std::string& message, const std::string& relmessage,
				const std::string& title, int outtime, const std::string& FunctionName) {
				std::string functionName = FunctionName;
				if (functionName.empty()) {
					functionName = "UnknownFunction"; // In C++ we cannot easily get function name like in C#
				}

				std::string formattedMessage = functionName + " ERROR! " + message;

				std::string errorTitle = title.empty() ?
					"*********************! " + OtherHelper::FillString(OtherHelper::GetCurrentProjectName() + ".RUN.ERROR", " ", 1) + " !*********************\n" :
					title;

				std::cout << '\n';

				std::string displayMessage = relmessage.empty() ? formattedMessage : relmessage;

				if (mode == 0) {
					WriteLog(formattedMessage, "", true, errorTitle, ConsoleColor::Red);
				}
				else {
					WriteLog(displayMessage, "", true, errorTitle, ConsoleColor::Red);
				}
#ifdef _DEBUG
				throw std::runtime_error(formattedMessage);
				return;
#else
				///throw std::runtime_error(formattedMessage);
				EndProgram(true);
#endif



				if (OtherHelper::GetCurrentProjectName() != "dotnet") {
					while (true) {
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					}
				}
			}

			void LogHelper::WriteLog(const std::string& message, const std::string& relaesemes,
				bool show_title, const std::string& title, ConsoleColor color,
				bool newline, int leval) {
				std::string displayMessage = relaesemes.empty() ? message : relaesemes;

				std::string levelPadding(leval, ' ');

				if (mode == 0) {
					if (show_title) {
#ifdef _DEBUG
						if (newline) {
							std::cout << " " << message << '\n';
						}
						else {
							std::cout << " " << message;
						}
#else
						std::cout << '\n';
						if (color != ConsoleColor::White) {
						 Console::SetForegroundColor(color);
						}
						std::cout << levelPadding << title;
						if (newline) {
							std::cout << " " << message << '\n';
						}
						else {
							std::cout << " " << message;
						}
						Console::ResetColor();
#endif
					}
					else {
						if (color != ConsoleColor::White) {
							Console::SetForegroundColor(color);
						}
						if (newline) {
							std::cout << message << '\n';
						}
						else {
							std::cout << message;
						}
						Console::ResetColor();
					}
					LogData::Add(message);
				}
				else {
					if (show_title) {
						std::cout << '\n';
						if (color != ConsoleColor::White) {
							Console::SetForegroundColor(color);
						}
						std::cout << levelPadding << title;
						if (newline) {
							std::cout << " " << displayMessage << '\n';
						}
						else {
							std::cout << " " << displayMessage;
						}
						Console::ResetColor();
					}
					else {
						if (color != ConsoleColor::White) {
							Console::SetForegroundColor(color);
						}
						if (newline) {
							std::cout << displayMessage << '\n';
						}
						else {
							std::cout << displayMessage;
						}
						Console::ResetColor();
					}
					LogData::Add(displayMessage);
				}
			}

			void LogHelper::WriteLog(const Eigen::MatrixXf& message) {
				std::cout << "[Message]  " << message << '\n';
				std::stringstream ss;
				ss << message;
				LogData::Add(ss.str());
			}

			void LogHelper::WriteLog(const Eigen::MatrixXd& message) {
				std::cout << "[Message]  " << message << '\n';
				std::stringstream ss;
				ss << message;
				LogData::Add(ss.str());
			}

			void LogHelper::WriteLog(const std::vector<double>& message) {
				WriteLog("");
				std::stringstream ss;
				for (size_t i = 0; i < message.size(); ++i) {
					ss << " " << message[i];
				}
				std::string messageStr = ss.str();
				std::cout << messageStr << '\n';
				LogData::Add(messageStr);
			}

			void LogHelper::WriteLog(const std::vector<float>& message) {
				WriteLog("");
				std::stringstream ss;
				for (size_t i = 0; i < message.size(); ++i) {
					ss << " " << message[i];
				}
				std::string messageStr = ss.str();
				std::cout << messageStr << '\n';
				LogData::Add(messageStr);
			}

			void LogHelper::WriteLog(const Eigen::VectorXf& message) {
				WriteLog("");
				std::stringstream ss;
				for (int i = 0; i < message.size(); ++i) {
					ss << " " << message[i];
				}
				std::string messageStr = ss.str();
				std::cout << messageStr << '\n';
				LogData::Add(messageStr);
			}

			void LogHelper::WriteLog(const Eigen::VectorXd& message) {
				WriteLog("");
				std::stringstream ss;
				for (int i = 0; i < message.size(); ++i) {
					ss << " " << message[i];
				}
				std::string messageStr = ss.str();
				std::cout << messageStr << '\n';
				LogData::Add(messageStr);
			}

			void LogHelper::DebugLog(const std::string& message, const std::string& relaesemes,
				bool show_title, const std::string& title, ConsoleColor color,
				bool newline, int leval) {
				std::string displayMessage = relaesemes.empty() ? message : relaesemes;
				std::string levelPadding(leval, ' ');

				if (mode == 0) {
					if (show_title) {
#ifdef _DEBUG
						if (newline) {
							std::cout << " " << message << '\n';
						}
						else {
							std::cout << " " << message;
						}
#else
						if (color != ConsoleColor::White) {
							Console::SetForegroundColor(color);
						}
						std::cout << levelPadding << title;
						if (newline) {
							std::cout << " " << message << '\n';
						}
						else {
							std::cout << " " << message;
						}
						Console::ResetColor();
#endif
					}
					else {
						if (color != ConsoleColor::White) {
							Console::SetForegroundColor(color);
						}
						if (newline) {
							std::cout << message << '\n';
						}
						else {
							std::cout << message;
						}
						Console::ResetColor();
					}
					LogData::Add(message);
				}
				else {
					if (show_title) {
						std::cout << '\n';
						if (color != ConsoleColor::White) {
							Console::SetForegroundColor(color);
						}
						std::cout << levelPadding << title;
						if (newline) {
							std::cout << " " << displayMessage << '\n';
						}
						else {
							std::cout << " " << displayMessage;
						}
						Console::ResetColor();
					}
					else {
						if (color != ConsoleColor::White) {
							Console::SetForegroundColor(color);
						}
						if (newline) {
							std::cout << displayMessage << '\n';
						}
						else {
							std::cout << displayMessage;
						}
						Console::ResetColor();
					}
					LogData::Add(displayMessage);
				}
			}

			void LogHelper::DebugLog(const Eigen::MatrixXf& message) {
				std::cout << "[Message]  " << message << '\n';
				std::stringstream ss;
				ss << message;
				LogData::Add(ss.str());
			}

			void LogHelper::DebugLog(const Eigen::MatrixXd& message) {
				std::cout << "[Message]  " << message << '\n';
				std::stringstream ss;
				ss << message;
				LogData::Add(ss.str());
			}

			void LogHelper::DebugLog(const std::vector<double>& message) {
				DebugLog("");
				std::stringstream ss;
				for (size_t i = 0; i < message.size(); ++i) {
					ss << " " << message[i];
				}
				std::string messageStr = ss.str();
				std::cout << messageStr << '\n';
				LogData::Add(messageStr);
			}

			void LogHelper::DebugLog(const std::vector<float>& message) {
				DebugLog("");
				std::stringstream ss;
				for (size_t i = 0; i < message.size(); ++i) {
					ss << " " << message[i];
				}
				std::string messageStr = ss.str();
				std::cout << messageStr << '\n';
				LogData::Add(messageStr);
			}

			void LogHelper::DebugLog(const Eigen::VectorXf& message) {
				DebugLog("");
				std::stringstream ss;
				for (int i = 0; i < message.size(); ++i) {
					ss << " " << message[i];
				}
				std::string messageStr = ss.str();
				std::cout << messageStr << '\n';
				LogData::Add(messageStr);
			}

			void LogHelper::DebugLog(const Eigen::VectorXd& message) {
				DebugLog("");
				std::stringstream ss;
				for (int i = 0; i < message.size(); ++i) {
					ss << " " << message[i];
				}
				std::string messageStr = ss.str();
				std::cout << messageStr << '\n';
				LogData::Add(messageStr);
			}

			void LogHelper::WriteLogO(const Eigen::VectorXf& message) {
				std::stringstream ss;
				for (int i = 0; i < message.size(); ++i) {
					ss << '\t' << message[i];
				}
				LogData::Add(ss.str());
			}

			template<typename T>
			void LogHelper::WriteLogO(const std::vector<T>& message) {
				std::stringstream ss;
				for (const auto& item : message) {
					ss << '\t' << item;
				}
				LogData::Add(ss.str());
			}

			// Explicit template instantiations
			template void LogHelper::WriteLogO<std::string>(const std::vector<std::string>&);
			template void LogHelper::WriteLogO<int>(const std::vector<int>&);
			template void LogHelper::WriteLogO<float>(const std::vector<float>&);
			template void LogHelper::WriteLogO<double>(const std::vector<double>&);

			void LogHelper::WriteLogO(const Eigen::VectorXd& message) {
				std::stringstream ss;
				for (int i = 0; i < message.size(); ++i) {
					ss << " " << message[i];
				}
				LogData::Add(ss.str());
			}

			void LogHelper::WriteLogO(const std::string& message, bool show_log, const std::string& title) {
				std::string formattedMessage = message;
				if (show_log) {
					formattedMessage = title + ": " + message;
				}
				LogData::Add(formattedMessage);
			}

			void LogHelper::WriteLogO(const Eigen::MatrixXf& message) {
				std::stringstream ss;
				ss << "[Message]  " << message;
				LogData::Add(ss.str());
			}

			void LogHelper::WriteLogO(const Eigen::MatrixXd& message) {
				std::stringstream ss;
				ss << "[Message]  " << message;
				LogData::Add(ss.str());
			}

			void LogHelper::WriteLogO(double message) {
				LogData::Add("[Message]  " + std::to_string(message));
			}

			void LogHelper::WriteLogO(float message) {
				LogData::Add("[Message]  " + std::to_string(message));
			}

			void LogHelper::WriteLogO(const std::vector<double>& message) {
				std::stringstream ss;
				for (size_t i = 0; i < message.size(); ++i) {
					ss << " " << message[i];
				}
				LogData::Add(ss.str());
			}

			void LogHelper::WriteLogO(const std::vector<float>& message) {
				std::stringstream ss;
				for (size_t i = 0; i < message.size(); ++i) {
					ss << " " << message[i];
				}
				LogData::Add(ss.str());
			}

		} // namespace Log
	} // namespace IO
} // namespace HawtC3