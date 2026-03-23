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

#pragma once
#ifndef LOGHELPER_H

#define LOGHELPER_H

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <fstream>
#include <thread>
#include <functional>
#include <exception>
#include <Eigen/Dense>
#include "LogInterfaces.h"
#include "../Interface/IOutFile.h"
#include "../System/Console.h";

#ifdef _WIN32
//#include <windows.h>
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h>
#endif

using namespace HawtC3::IO::System;

using HawtC3::IO::Interface::IOutFile;

namespace HawtC3 {
	namespace IO {
		namespace Log {
			/// <summary>
			/// 数据存储静态类
			/// </summary>
			class Datastore
			{
			public:
				static std::shared_ptr<Email> EmailSet;
			};

			/// <remarks>
			/// 存储日志文件的结构体。
			/// </remarks>
			class LogData {
			public:
				static inline std::string LogPath;


				/// <remarks>
				/// 存储了数控流的OutFile List 非常重要！！！
				/// </remarks>
				static  inline std::vector< IOutFile*> OutFilelist;

				/// <summary>
				/// 存储了VTK的风机结构体数据流，用来保存VTK的实现。
				/// </summary>
				static  inline std::vector<std::shared_ptr<BASE_VTK>> IO_VTK;

				/// <remarks>
				/// 当前保存的日志字符串数组
				/// </remarks>
				static  inline std::vector<std::string> log_inf;

				/// <summary>
				/// 添加日志消息
				/// </summary>
				/// <param name="message">要添加的消息</param>
				static void Add(const std::string& message);

				/// <remarks>
				/// 初始化结构体
				/// </remarks>
				static void Initialize();
			};

			/// <summary>
			/// 日志帮助类，提供统一的日志记录、错误处理和程序信息显示功能
			/// 支持控制台输出、文件记录、调试输出等多种日志模式
			/// </summary>
			/// <remarks>
			/// 该静态类是整个应用程序的日志管理中心，主要功能包括：
			/// - 应用程序启动信息显示和许可证管理
			/// - 多级别日志记录（普通日志、警告、错误、调试）
			/// - 控制台彩色输出和格式化显示
			/// - 矩阵、向量、数组等数据结构的日志记录
			/// - 程序异常处理和优雅退出
			/// - 文件日志存储和管理
			/// 
			/// 使用示例：
			/// <code>
			/// // 显示程序启动信息
			/// LogHelper::DisplayInformation(true, true);
			/// 
			/// // 记录普通日志
			/// LogHelper::WriteLog("程序开始执行", ConsoleColor::Green);
			/// 
			/// // 记录警告信息
			/// LogHelper::WarnLog("配置文件可能存在问题", "", ConsoleColor::DarkYellow, 0, "InitConfig");
			/// 
			/// // 记录错误并终止程序
			/// LogHelper::ErrorLog("致命错误：无法加载必要组件", "", "", 20, "LoadComponents");
			/// 
			/// // 记录矩阵数据
			/// Eigen::MatrixXd matrix = Eigen::MatrixXd::Random(3, 3);
			/// LogHelper::WriteLog(matrix);
			/// 
			/// // 程序正常结束
			/// LogHelper::EndProgram(false);
			/// </code>
			/// 
			/// 成员变量说明：
			/// - mode: 程序运行模式标识（0=Debug模式，1=Release模式）
			/// - firstshowinformation: 控制启动信息只显示一次的标志
			/// </remarks>
			class LogHelper {

			private:
				/// <remarks>
				/// 局部变量，用于表明当前程序是debug=0,release=1;用来表明程序log输出不同的信息
				/// </remarks>


				/// <remarks>
				/// 是否是第一次显示信息,避免多次调用信息展示输出。
				/// </remarks>
				static bool firstshowinformation;

				/// <summary>
				/// 处理应用程序未处理的异常事件
				/// </summary>
				/// <param name="signal">信号编号</param>
				/// <remarks>
				/// 该方法作为全局异常处理器，在应用程序发生未捕获异常时自动调用。
				/// 主要功能：
				/// - 记录异常详细信息到日志
				/// - 执行程序清理和优雅退出
				/// - 防止程序崩溃造成数据丢失
				/// 
				/// 使用示例：
				/// <code>
				/// // 在程序初始化时注册全局异常处理器
				/// std::signal(SIGINT, LogHelper::UnhandledExceptionHandler);
				/// 
				/// // 当发生未处理异常时，该方法会自动被调用
				/// throw std::runtime_error("测试异常");
				/// // 输出: "未知错误：测试异常"
				/// </code>
				/// </remarks>
				static void UnhandledExceptionHandler(int signal);

				/// <summary>
				/// 显示调试模式的程序启动信息和许可证详情（版本1）
				/// </summary>
				/// <param name="lics">是否启用许可证功能，true表示显示许可证信息</param>
				/// <remarks>
				/// 该方法专门用于Debug模式下的程序启动信息显示，特点：
				/// - 显示详细的ASCII艺术字Logo
				/// - 输出版权信息和开发者信息
				/// - 显示许可证状态和授权信息
				/// - 设置控制台标题和颜色主题
				/// - 初始化调试模式标识
				/// 
				/// 使用示例：
				/// <code>
				/// // 在Debug模式下显示启动信息
				/// LogHelper::DisinfV1(true);
				/// // 输出彩色的Logo和详细的程序信息
				/// // 设置mode = 0（调试模式）
				/// </code>
				/// </remarks>
				static void DisinfV1(bool lics);

				/// <summary>
				/// 显示发布版本的程序信息和许可证详情（版本2）
				/// </summary>
				/// <param name="lics">是否启用许可证功能，true表示显示许可证信息，false表示显示免费许可证</param>
				/// <remarks>
				/// 该方法专门用于发布模式下的程序信息展示，提供简洁的项目信息输出。
				/// 主要功能：
				/// 1. 显示项目名称、联系信息和官方网址
				/// 2. 输出版本信息、构建模式和构建时间
				/// 3. 显示发布机构和许可证状态
				/// 4. 设置程序运行模式为发布模式（mode=1）
				/// 5. 标记信息已显示，避免重复调用
				/// 
				/// 使用示例：
				/// <code>
				/// // 在程序启动时调用（通常仅在发布模式下）
				/// #ifndef _DEBUG
				/// LogHelper::DisinfV2(true);  // 启用许可证显示
				/// #endif
				/// 
				/// // 控制台输出示例：
				/// // HawtC3.ProjectName - Tel:13935201274  E:1821216780@qq.com http://www.HawtC.cn
				/// // ************************************************************************************
				/// // ProjectName Vision:VX.X.X_Release,BuildAt: 2025-08-09 Math:MKL
				/// //  > Publish: Powered by Key Laboratory of Jiangsu province high tech design @Tg Team
				/// //  > Licensd: UserName Lic at 2025-08-09 10:30:00
				/// </code>
				/// </remarks>
				static void DisinfV2(bool lics);

			public:




				static int mode;
				/// <summary>
				/// 根据构建模式显示应用程序信息并配置全局错误处理
				/// </summary>
				/// <param name="UnhandledException">是否注册全局未处理异常处理程序，true表示启用，false表示禁用（仅在发布模式下有效）</param>
				/// <param name="lic">是否显示许可证相关信息，true表示显示许可证信息，false表示不显示</param>
				/// <remarks>
				/// 该方法是程序启动时的主要信息显示入口，根据不同构建模式采用不同的显示策略。
				/// 主要功能：
				/// 1. 调试模式：调用DisinfV1显示详细的调试信息
				/// 2. 发布模式：调用DisinfV2显示简洁的发布信息，并可选注册全局异常处理
				/// 3. 确保信息只显示一次，避免重复调用
				/// 4. 在发布模式下可选择性启用全局异常处理
				/// 
				/// 使用示例：
				/// <code>
				/// // 程序启动时调用（推荐在main函数开始处）
				/// int main(int argc, char* argv[])
				/// {
				///     // 显示程序信息并启用异常处理
				///     LogHelper::DisplayInformation(true, true);
				///     
				///     // 仅显示信息，不启用异常处理
				///     LogHelper::DisplayInformation(false, false);
				///     
				///     // 后续程序逻辑...
				/// }
				/// 
				/// // 在类库中使用
				/// class MyLibrary
				/// {
				///     static void Initialize()
				///     {
				///         LogHelper::DisplayInformation();  // 使用默认参数
				///     }
				/// };
				/// </code>
				/// </remarks>
				static void DisplayInformation(bool UnhandledException = true, bool lic = true);

				/// <summary>
				/// 记录ODE求解器运行完成信息并提供计时统计
				/// </summary>
				/// <param name="tfinal">仿真的最终时间值，单位为秒</param>
				/// <param name="dt">仿真使用的时间步长，单位为秒</param>
				/// <param name="start_time">仿真开始时间点</param>
				/// <remarks>
				/// 该方法用于数值仿真结束时的信息记录，提供仿真参数和性能统计。
				/// 主要功能：
				/// 1. 计算并格式化仿真的实际运行时间
				/// 2. 根据时间长度自动选择合适的时间单位（秒或分钟）
				/// 3. 输出仿真完成信息，包括最终时间、时间步长和耗时
				/// 
				/// 使用示例：
				/// <code>
				/// // 在数值仿真程序中使用
				/// void RunSimulation()
				/// {
				///     auto start_time = std::chrono::steady_clock::now();
				///     double finalTime = 100.0;  // 仿真100秒
				///     double timeStep = 0.01;    // 时间步长0.01秒
				///     
				///     // 执行仿真循环
				///     for (double t = 0; t <= finalTime; t += timeStep)
				///     {
				///         // 仿真计算逻辑...
				///     }
				///     
				///     // 记录仿真完成信息
				///     LogHelper::EndInformation(finalTime, timeStep, start_time);
				///     // 输出: Simulation Run Finished! The tf= 100s Step=0.01s Cost real time=2.45 sec
				/// }
				/// 
				/// // 在长时间仿真中使用
				/// LogHelper::EndInformation(3600.0, 0.1, start_time);
				/// // 输出: Simulation Run Finished! The tf= 3600s Step=0.1s Cost real time=5.23 min
				/// </code>
				/// </remarks>
				static void EndInformation(double tfinal, double dt, const std::chrono::steady_clock::time_point& start_time);

				/// <summary>
				/// 终止程序执行，可选择强制终止并执行清理操作
				/// </summary>
				/// <param name="forceTerminate">是否强制终止程序，true表示立即退出，false表示正常终止</param>
				/// <param name="outstring">终止时记录的可选消息，如果未提供则使用默认消息</param>
				/// <param name="keepstay">是否在终止步骤完成后保持程序运行，true表示等待用户输入后退出</param>
				/// <param name="extcall">是否由外部调用触发的终止，true表示程序不退出</param>
				/// <param name="sleeptime">完成终止过程前等待的时间，单位为毫秒，默认2000毫秒</param>
				/// <remarks>
				/// 该方法执行程序的完整终止流程，包括日志写入、文件关闭和资源清理。
				/// 主要功能：
				/// 1. 记录程序终止消息到日志
				/// 2. 将所有日志信息写入文件
				/// 3. 关闭VTK文件输出流
				/// 4. 完成所有输出文件的写入
				/// 5. 根据参数决定程序退出方式
				/// 
				/// 使用示例：
				/// <code>
				/// // 正常程序结束
				/// LogHelper::EndProgram(false, "计算任务完成");
				/// 
				/// // 错误强制终止，保持控制台窗口
				/// LogHelper::EndProgram(true, "", true);
				/// 
				/// // 外部调用终止，不退出程序
				/// LogHelper::EndProgram(false, "外部调用结束", false, true);
				/// 
				/// // 快速退出，等待500毫秒
				/// LogHelper::EndProgram(false, "", false, false, 500);
				/// 
				/// // 在异常处理中使用
				/// try
				/// {
				///     // 程序主逻辑
				/// }
				/// catch (const std::exception& ex)
				/// {
				///     LogHelper::ErrorLog("程序异常: " + std::string(ex.what()));
				///     LogHelper::EndProgram(true);
				/// }
				/// </code>
				/// </remarks>
				static void EndProgram(bool forceTerminate = false, const std::string& outstring = "",
					bool keepstay = false, bool extcall = false, int sleeptime = 2000);

#pragma region 警告和报错信息

				/// <summary>
				/// 记录警告消息到控制台或日志文件，支持可选的格式化和元数据
				/// </summary>
				/// <param name="debugmessage">调试模式下记录的消息，通常包含更详细的信息用于开发目的</param>
				/// <param name="relasemessage">发布模式下记录的消息，如果未指定则使用debugmessage</param>
				/// <param name="color1">警告消息的控制台颜色，默认为深黄色</param>
				/// <param name="leval">警告的严重级别，可用于分类或过滤日志条目</param>
				/// <param name="FunctionName">发生警告的函数或上下文名称，用于日志的可追溯性</param>
				/// <remarks>
				/// 该方法根据当前程序模式（调试或发布）选择合适的消息进行记录。
				/// 主要功能：
				/// 1. 根据程序运行模式选择不同的消息内容
				/// 2. 自动添加函数名称前缀到警告消息
				/// 3. 使用指定颜色在控制台显示警告
				/// 4. 支持严重级别分类
				/// 
				/// 使用示例：
				/// <code>
				/// // 基本警告记录
				/// LogHelper::WarnLog("检测到潜在的性能问题", "", ConsoleColor::DarkYellow, 0, "DataProcessor");
				/// 
				/// // 不同模式的消息
				/// LogHelper::WarnLog(
				///     "变量x的值超出预期范围: " + std::to_string(x), 
				///     "数据范围警告",
				///     ConsoleColor::DarkYellow, 0,
				///     "ValidateInput"
				/// );
				/// 
				/// // 自定义颜色和级别
				/// LogHelper::WarnLog(
				///     "内存使用率较高", 
				///     "",
				///     ConsoleColor::Yellow, 
				///     1,
				///     "MemoryMonitor"
				/// );
				/// </code>
				/// </remarks>
				static void WarnLog(const std::string& debugmessage, const std::string& relasemessage = "",
					ConsoleColor color1 = ConsoleColor::DarkYellow, int leval = 0,
					const std::string& FunctionName = "");

				/// <summary>
				/// 记录错误消息并可选择性终止程序
				/// </summary>
				/// <param name="message">要记录的主要错误消息，不能为空字符串</param>
				/// <param name="relmessage">可选的相关消息，如果未提供则默认使用message的值</param>
				/// <param name="title">错误消息的可选标题，如果未提供则生成默认标题</param>
				/// <param name="outtime">程序退出前的超时时间，单位为秒，默认20秒</param>
				/// <param name="FunctionName">发生错误的函数名称，如果未提供则尝试自动确定</param>
				/// <remarks>
				/// 该方法格式化并记录错误消息到控制台，包括相关详细信息如标题和调用函数名称。
				/// 如果未提供FunctionName参数，方法会尝试通过堆栈跟踪自动确定调用函数的名称。
				/// 记录错误后程序将根据应用程序配置决定是否终止。
				/// 
				/// 使用示例：
				/// <code>
				/// // 基本错误记录
				/// LogHelper::ErrorLog("文件读取失败", "", "", 20, "LoadConfiguration");
				/// 
				/// // 带详细信息的错误
				/// LogHelper::ErrorLog(
				///     "数据库连接失败: " + ex.what(),
				///     "无法连接到数据库",
				///     "",
				///     20,
				///     "InitializeDatabase"
				/// );
				/// 
				/// // 自动获取函数名称
				/// void CriticalOperation()
				/// {
				///     try
				///     {
				///         // 关键操作
				///     }
				///     catch (const std::exception& ex)
				///     {
				///         LogHelper::ErrorLog("关键操作失败: " + std::string(ex.what()));
				///         // 会自动获取函数名"CriticalOperation"
				///     }
				/// }
				/// </code>
				/// </remarks>
				static void ErrorLog(const std::string& message, const std::string& relmessage = "",
					const std::string& title = "", int outtime = 20,
					const std::string& FunctionName = "");

#pragma endregion 警告和报错信息

#pragma region 常规 WRITE 输出

				/// <summary>
				/// 将日志消息写入控制台，支持自定义格式化选项
				/// </summary>
				/// <param name="message">要记录的主要消息</param>
				/// <param name="relaesemes">可选的相关消息，如果未提供则默认使用message的值</param>
				/// <param name="show_title">是否在消息前显示标题，默认为true</param>
				/// <param name="title">消息前显示的标题，如果show_title为true，默认为"[Message]"</param>
				/// <param name="color">控制台中消息文本的颜色，默认为白色</param>
				/// <param name="newline">是否在消息后添加换行符，默认为true</param>
				/// <param name="leval">消息的缩进级别，每个级别在标题和消息前添加一个空格，默认为0</param>
				/// <remarks>
				/// 该方法将指定的消息写入控制台，可选择性包含标题并应用格式化（如颜色和缩进）。
				/// 消息也会被添加到内部日志数据存储中。方法根据程序运行模式（调试或发布）选择合适的消息内容。
				/// 
				/// 使用示例：
				/// <code>
				/// // 基本消息记录
				/// LogHelper::WriteLog("程序启动成功");
				/// 
				/// // 带颜色的消息
				/// LogHelper::WriteLog("重要提示", "", true, "[Message]", ConsoleColor::Green);
				/// 
				/// // 带缩进和自定义标题的消息
				/// LogHelper::WriteLog("子任务完成", "", true, "[SubTask]", ConsoleColor::White, true, 2);
				/// 
				/// // 不换行的消息
				/// LogHelper::WriteLog("处理中...", "", true, "[Message]", ConsoleColor::White, false);
				/// </code>
				/// </remarks>
				static void WriteLog(const std::string& message, const std::string& relaesemes = "",
					bool show_title = true, const std::string& title = "[Message]",
					ConsoleColor color = ConsoleColor::White, bool newline = true, int leval = 0);

				/// <summary>
				/// 记录单精度浮点矩阵到日志并输出到控制台
				/// </summary>
				/// <param name="message">要记录的矩阵，不能为空</param>
				/// <remarks>
				/// 该方法将矩阵输出到控制台并将其字符串表示形式存储到应用程序的日志数据中。
				/// 矩阵在输出前会被转换为字符串表示形式。
				/// 
				/// 使用示例：
				/// <code>
				/// // 记录计算结果矩阵
				/// Eigen::MatrixXf resultMatrix(2, 2);
				/// resultMatrix << 1.0f, 2.0f, 3.0f, 4.0f;
				/// LogHelper::WriteLog(resultMatrix);
				/// // 输出: [Message]  MatrixXf 2x2
				/// //       1  2
				/// //       3  4
				/// </code>
				/// </remarks>
				static void WriteLog(const Eigen::MatrixXf& message);

				/// <summary>
				/// 记录双精度浮点矩阵到日志并输出到控制台
				/// </summary>
				/// <param name="message">要记录的矩阵，矩阵的每个元素都会被转换为字符串并包含在日志中</param>
				/// <remarks>
				/// 该方法将矩阵消息输出到控制台，并将其字符串表示形式添加到应用程序的日志数据中。
				/// 
				/// 使用示例：
				/// <code>
				/// // 记录计算结果矩阵
				/// Eigen::MatrixXd coeffMatrix(2, 2);
				/// coeffMatrix << 1.5, 2.3, 3.7, 4.1;
				/// LogHelper::WriteLog(coeffMatrix);
				/// // 输出矩阵的完整内容到控制台和日志文件
				/// </code>
				/// </remarks>
				static void WriteLog(const Eigen::MatrixXd& message);

				/// <summary>
				/// 记录双精度浮点数组到日志
				/// </summary>
				/// <param name="message">要包含在日志条目中的双精度浮点数数组，每个数字都会被转换为字符串并连接成单个日志消息</param>
				/// <remarks>
				/// 该方法将提供的数值格式化为单个字符串并写入控制台。
				/// 此外，格式化的消息会存储在应用程序的日志数据中。
				/// 
				/// 使用示例：
				/// <code>
				/// // 记录计算结果数组
				/// std::vector<double> results = {1.23, 4.56, 7.89, 10.11};
				/// LogHelper::WriteLog(results);
				/// // 输出: 1.23 4.56 7.89 10.11
				/// 
				/// // 记录传感器数据
				/// std::vector<double> sensorData = GetSensorReadings();
				/// LogHelper::WriteLog(sensorData);
				/// </code>
				/// </remarks>
				static void WriteLog(const std::vector<double>& message);

				/// <summary>
				/// 记录单精度浮点数组到日志
				/// </summary>
				/// <param name="message">要记录的浮点数数组，数组中的每个数字会被转换为字符串并包含在日志条目中，数字间用空格分隔</param>
				/// <remarks>
				/// 该方法将提供的数组格式化为单个字符串并写入控制台。
				/// 此外，格式化的字符串会存储在应用程序的日志数据中。
				/// 
				/// 使用示例：
				/// <code>
				/// // 记录测量数据
				/// std::vector<float> measurements = {1.2f, 3.4f, 5.6f, 7.8f};
				/// LogHelper::WriteLog(measurements);
				/// // 输出: 1.2 3.4 5.6 7.8
				/// 
				/// // 记录坐标数据
				/// std::vector<float> coordinates = {x, y, z};
				/// LogHelper::WriteLog(coordinates);
				/// </code>
				/// </remarks>
				static void WriteLog(const std::vector<float>& message);

				/// <summary>
				/// 记录单精度浮点向量的元素到日志
				/// </summary>
				/// <param name="message">要记录的浮点值向量，向量的每个元素都会转换为字符串并包含在日志条目中</param>
				/// <remarks>
				/// 该方法将向量的元素格式化为单个字符串，元素间用空格分隔，并将结果字符串写入控制台。
				/// 此外，日志条目会使用LogData::Add方法进行存储。
				/// 
				/// 使用示例：
				/// <code>
				/// // 记录计算向量
				/// Eigen::VectorXf velocity(3);
				/// velocity << 1.5f, 2.3f, 3.7f;
				/// LogHelper::WriteLog(velocity);
				/// // 输出: 1.5 2.3 3.7
				/// 
				/// // 记录状态向量
				/// Eigen::VectorXf state = GetCurrentState();
				/// LogHelper::WriteLog(state);
				/// </code>
				/// </remarks>
				static void WriteLog(const Eigen::VectorXf& message);

				/// <summary>
				/// 记录双精度浮点向量的元素到日志
				/// </summary>
				/// <param name="message">包含要记录值的双精度向量，向量的每个元素都会转换为字符串并包含在日志输出中</param>
				/// <remarks>
				/// 该方法将向量的元素格式化为单个字符串，元素间用空格分隔，并将结果字符串写入控制台。
				/// 此外，格式化的字符串会通过LogData::Add方法添加到应用程序的日志存储中。
				/// 
				/// 使用示例：
				/// <code>
				/// // 记录计算结果向量
				/// Eigen::VectorXd solution(3);
				/// solution << 1.234, 5.678, 9.012;
				/// LogHelper::WriteLog(solution);
				/// // 输出: 1.234 5.678 9.012
				/// 
				/// // 记录优化参数
				/// Eigen::VectorXd parameters = optimizer.GetParameters();
				/// LogHelper::WriteLog(parameters);
				/// </code>
				/// </remarks>
				static void WriteLog(const Eigen::VectorXd& message);

#pragma endregion WRITE 常规输出

#pragma region debug输出

				/// <summary>
				/// 记录调试消息到输出，可选择性包含标题、格式化选项和缩进级别
				/// </summary>
				/// <param name="message">要记录的主要消息</param>
				/// <param name="relaesemes">可选的相关消息，如果未提供则默认使用message的值</param>
				/// <param name="show_title">是否在输出中包含标题，默认为true</param>
				/// <param name="title">消息前显示的标题，默认为"[Message]"</param>
				/// <param name="color">消息使用的控制台颜色，默认为白色</param>
				/// <param name="newline">是否在消息后添加换行符，默认为true</param>
				/// <param name="leval">消息的缩进级别，默认为0</param>
				/// <remarks>
				/// 该方法使用调试输出写入调试信息，并可选择性地使用标题、缩进和颜色格式化消息。
				/// 输出行为可能会根据当前调试模式而有所不同。
				/// 
				/// 使用示例：
				/// <code>
				/// // 基本调试日志
				/// LogHelper::DebugLog("进入函数ProcessData");
				/// 
				/// // 带自定义标题的调试日志
				/// LogHelper::DebugLog("变量值检查", "", true, "[DEBUG]", ConsoleColor::Cyan);
				/// 
				/// // 缩进的调试消息
				/// LogHelper::DebugLog("子过程开始", "", true, "[Message]", ConsoleColor::White, true, 2);
				/// 
				/// // 调试模式和发布模式不同消息
				/// LogHelper::DebugLog("详细调试信息: " + details, "简化消息");
				/// </code>
				/// </remarks>
				static void DebugLog(const std::string& message, const std::string& relaesemes = "",
					bool show_title = true, const std::string& title = "[Message]",
					ConsoleColor color = ConsoleColor::White, bool newline = true, int leval = 0);

				/// <summary>
				/// 记录单精度浮点矩阵消息到调试输出并存储到应用程序的日志数据中
				/// </summary>
				/// <param name="message">要记录的矩阵，不能为空</param>
				/// <remarks>
				/// 该方法将矩阵写入调试输出，并将其字符串表示形式添加到应用程序的日志数据中。
				/// 
				/// 使用示例：
				/// <code>
				/// // 调试记录变换矩阵
				/// Eigen::MatrixXf transform = Eigen::MatrixXf::Identity(3, 3);
				/// LogHelper::DebugLog(transform);
				/// // 在调试输出窗口显示矩阵内容
				/// </code>
				/// </remarks>
				static void DebugLog(const Eigen::MatrixXf& message);

				/// <summary>
				/// 记录双精度浮点矩阵消息到调试输出并存储到应用程序的日志数据中
				/// </summary>
				/// <param name="message">要记录的矩阵，不能为空</param>
				/// <remarks>
				/// 该方法将矩阵写入调试输出，并将其字符串表示形式添加到应用程序的日志数据中用于进一步分析或跟踪。
				/// 
				/// 使用示例：
				/// <code>
				/// // 调试记录系数矩阵
				/// Eigen::MatrixXd coefficients(3, 3);
				/// coefficients << 2.5, 1.3, 0.7, 1.8, 3.2, 1.1, 0.9, 2.1, 2.8;
				/// LogHelper::DebugLog(coefficients);
				/// // 输出到调试窗口
				/// </code>
				/// </remarks>
				static void DebugLog(const Eigen::MatrixXd& message);

				/// <summary>
				/// 记录由数值组成的调试消息到应用程序的调试输出和内部日志存储
				/// </summary>
				/// <param name="message">要记录的数值数组，数组中的每个值都会转换为字符串并包含在日志消息中</param>
				/// <remarks>
				/// 该方法将提供的数值格式化为单个字符串，并写入调试输出。
				/// 此外，格式化的消息会存储在应用程序的内部日志存储中。
				/// 
				/// 使用示例：
				/// <code>
				/// // 调试记录计算中间结果
				/// std::vector<double> intermediateResults = {0.1234, 0.5678, 0.9012};
				/// LogHelper::DebugLog(intermediateResults);
				/// // 在调试输出中显示: 0.1234 0.5678 0.9012
				/// 
				/// // 调试记录迭代过程数据
				/// std::vector<double> iterationData = GetIterationData();
				/// LogHelper::DebugLog(iterationData);
				/// </code>
				/// </remarks>
				static void DebugLog(const std::vector<double>& message);

				/// <summary>
				/// 记录浮点数数组作为格式化字符串到调试输出
				/// </summary>
				/// <param name="message">要记录的float值数组，不能为空</param>
				/// <remarks>
				/// message数组中的每个元素都会转换为字符串并连接成单个空格分隔的字符串。
				/// 结果字符串会写入调试输出并存储在应用程序的日志数据中。
				/// 
				/// 使用示例：
				/// <code>
				/// // 调试记录传感器读数
				/// std::vector<float> sensorReadings = {23.5f, 24.1f, 23.8f, 24.3f};
				/// LogHelper::DebugLog(sensorReadings);
				/// // 调试输出: 23.5 24.1 23.8 24.3
				/// 
				/// // 调试记录坐标数据
				/// std::vector<float> coordinates = {x, y, z};
				/// LogHelper::DebugLog(coordinates);
				/// </code>
				/// </remarks>
				static void DebugLog(const std::vector<float>& message);

				/// <summary>
				/// 记录单精度浮点向量的内容到调试输出
				/// </summary>
				/// <param name="message">包含要记录的浮点数的向量，向量中的每个元素都会转换为字符串并包含在调试输出中</param>
				/// <remarks>
				/// 该方法将message向量的元素格式化为单个字符串，元素间用空格分隔，并写入调试输出。
				/// 此外，格式化的字符串会使用LogData::Add方法存储以便进一步处理或检索。
				/// 
				/// 使用示例：
				/// <code>
				/// // 调试记录速度向量
				/// Eigen::VectorXf velocity(3);
				/// velocity << 10.5f, 0.0f, -5.2f;
				/// LogHelper::DebugLog(velocity);
				/// // 调试输出: 10.5 0 -5.2
				/// 
				/// // 调试记录梯度向量
				/// Eigen::VectorXf gradient = ComputeGradient();
				/// LogHelper::DebugLog(gradient);
				/// </code>
				/// </remarks>
				static void DebugLog(const Eigen::VectorXf& message);

				/// <summary>
				/// 记录双精度向量的内容到调试输出和应用程序日志
				/// </summary>
				/// <param name="message">包含要记录值的向量，向量的每个元素都会转换为字符串并包含在日志输出中</param>
				/// <remarks>
				/// 该方法将向量的元素格式化为单个字符串，元素间用空格分隔，并写入调试输出。
				/// 此外，格式化的字符串会添加到应用程序的日志存储中。
				/// 
				/// 使用示例：
				/// <code>
				/// // 调试记录优化变量
				/// Eigen::VectorXd variables(3);
				/// variables << 1.234, 5.678, 9.012;
				/// LogHelper::DebugLog(variables);
				/// // 调试输出: 1.234 5.678 9.012
				/// 
				/// // 调试记录求解结果
				/// Eigen::VectorXd solution = solver.Solve();
				/// LogHelper::DebugLog(solution);
				/// </code>
				/// </remarks>
				static void DebugLog(const Eigen::VectorXd& message);

#pragma endregion debug输出

#pragma region 输出但是不在控制台显示

				/// <summary>
				/// 将指定的单精度浮点向量写入日志而不在控制台显示
				/// </summary>
				/// <param name="message">要记录的浮点值向量，向量中的每个值都会转换为字符串并以制表符分隔的字符串追加到日志中</param>
				/// <remarks>
				/// 该方法通过将每个元素转换为字符串并以制表符分隔连接来记录所提供的值向量。
				/// 日志条目使用LogData::Add方法存储。注意，此方法不会将日志条目输出到控制台。
				/// 
				/// 使用示例：
				/// <code>
				/// // 静默记录实验数据
				/// Eigen::VectorXf experimentData(3);
				/// experimentData << 1.1f, 2.2f, 3.3f;
				/// LogHelper::WriteLogO(experimentData);
				/// // 仅写入日志文件，格式：1.1	2.2	3.3
				/// 
				/// // 记录内部状态向量
				/// Eigen::VectorXf internalState = GetInternalState();
				/// LogHelper::WriteLogO(internalState);
				/// </code>
				/// </remarks>
				static void WriteLogO(const Eigen::VectorXf& message);

				/// <summary>
				/// 将消息集合写入日志，每个项目转换为字符串并用制表符分隔
				/// </summary>
				/// <typeparam name="T">message集合中元素的类型</typeparam>
				/// <param name="message">要记录的项目集合，每个项目都会转换为字符串并追加到日志条目中</param>
				/// <remarks>
				/// 该方法连接message集合中项目的字符串表示形式，用制表符字符分隔，并将结果字符串添加到日志中。
				/// 
				/// 使用示例：
				/// <code>
				/// // 记录字符串列表
				/// std::vector<std::string> statusList = {"Ready", "Processing", "Complete"};
				/// LogHelper::WriteLogO(statusList);
				/// // 日志输出：	Ready	Processing	Complete
				/// 
				/// // 记录数值列表
				/// std::vector<int> iterationCounts = {10, 25, 50, 100};
				/// LogHelper::WriteLogO(iterationCounts);
				/// // 日志输出：	10	25	50	100
				/// </code>
				/// </remarks>
				template<typename T>
				static void WriteLogO(const std::vector<T>& message);

				/// <summary>
				/// 将双精度向量的元素写入日志条目
				/// </summary>
				/// <param name="message">要记录的双精度浮点数向量，向量的每个元素都会转换为字符串并包含在日志条目中</param>
				/// <remarks>
				/// 该方法将向量的元素连接成单个字符串，元素间用空格分隔，并将结果字符串添加到日志中。
				/// 此操作不会修改输入向量。
				/// 
				/// 使用示例：
				/// <code>
				/// // 静默记录计算结果
				/// Eigen::VectorXd results(3);
				/// results << 1.234, 5.678, 9.012;
				/// LogHelper::WriteLogO(results);
				/// // 仅写入日志，不显示在控制台：1.234 5.678 9.012
				/// 
				/// // 记录统计数据
				/// Eigen::VectorXd statistics = CalculateStatistics();
				/// LogHelper::WriteLogO(statistics);
				/// </code>
				/// </remarks>
				static void WriteLogO(const Eigen::VectorXd& message);

				/// <summary>
				/// 将日志消息写入应用程序的日志存储
				/// </summary>
				/// <param name="message">要写入的日志消息，不能为空</param>
				/// <param name="show_log">指示日志消息是否应该以标题为前缀的值，true表示包含标题，否则为false</param>
				/// <param name="title">如果show_log为true时用于为日志消息添加前缀的标题，如果未指定则默认为"[Message]"</param>
				/// <remarks>
				/// 如果show_log为true，日志消息将以指定的title为前缀，后跟冒号和空格。
				/// 
				/// 使用示例：
				/// <code>
				/// // 带标题的日志消息
				/// LogHelper::WriteLogO("系统初始化完成", true, "[INIT]");
				/// // 日志输出：[INIT]: 系统初始化完成
				/// 
				/// // 不带标题的简单消息
				/// LogHelper::WriteLogO("数据处理中...", false);
				/// // 日志输出：数据处理中...
				/// 
				/// // 使用默认标题
				/// LogHelper::WriteLogO("操作完成");
				/// // 日志输出：[Message]: 操作完成
				/// </code>
				/// </remarks>
				static void WriteLogO(const std::string& message, bool show_log = true,
					const std::string& title = "[Message]");

				/// <summary>
				/// 将指定矩阵的内容记录到应用程序的日志系统
				/// </summary>
				/// <param name="message">包含要记录数据的矩阵，矩阵的每个元素都会转换为字符串表示并添加到日志中</param>
				/// <remarks>
				/// 该方法处理所提供的矩阵并记录其字符串表示形式。
				/// 日志系统可能会根据其内部配置格式化或存储数据。
				/// 
				/// 使用示例：
				/// <code>
				/// // 静默记录计算矩阵
				/// Eigen::MatrixXf transformMatrix = Eigen::MatrixXf::Identity(3, 3);
				/// transformMatrix(0, 2) = 2.5f;
				/// transformMatrix(1, 2) = 1.8f;
				/// LogHelper::WriteLogO(transformMatrix);
				/// // 仅写入日志文件，不在控制台显示
				/// 
				/// // 记录中间计算结果
				/// Eigen::MatrixXf intermediate = PerformCalculation();
				/// LogHelper::WriteLogO(intermediate);
				/// </code>
				/// </remarks>
				static void WriteLogO(const Eigen::MatrixXf& message);

				/// <summary>
				/// 将矩阵消息记录到应用程序的日志系统
				/// </summary>
				/// <param name="message">要记录的矩阵，矩阵的每个元素都会格式化为字符串</param>
				/// <remarks>
				/// 该方法将提供的矩阵格式化为字符串表示形式并追加到应用程序的日志中。
				/// 日志系统可能对日志条目的大小或数量施加限制。
				/// 
				/// 使用示例：
				/// <code>
				/// // 静默记录协方差矩阵
				/// Eigen::MatrixXd covarianceMatrix = CalculateCovariance(data);
				/// LogHelper::WriteLogO(covarianceMatrix);
				/// // 仅写入日志文件，格式化为字符串表示
				/// 
				/// // 记录求解器结果矩阵
				/// Eigen::MatrixXd solution = solver.GetSolutionMatrix();
				/// LogHelper::WriteLogO(solution);
				/// </code>
				/// </remarks>
				static void WriteLogO(const Eigen::MatrixXd& message);

				/// <summary>
				/// 将数值消息记录到应用程序的日志系统
				/// </summary>
				/// <param name="message">要记录的数值消息，通常表示需要记录的值或测量值，用于诊断或信息目的</param>
				/// <remarks>
				/// 该方法将提供的数值消息追加到应用程序的日志中。
				/// 消息以"[Message]"为前缀以指示其类型。
				/// 
				/// 使用示例：
				/// <code>
				/// // 记录关键性能指标
				/// double performanceMetric = CalculatePerformance();
				/// LogHelper::WriteLogO(performanceMetric);
				/// // 日志输出：[Message]  1.23456
				/// 
				/// // 记录计算结果
				/// double calculationResult = std::sqrt(value);
				/// LogHelper::WriteLogO(calculationResult);
				/// 
				/// // 记录误差值
				/// double error = std::abs(expected - actual);
				/// LogHelper::WriteLogO(error);
				/// </code>
				/// </remarks>
				static void WriteLogO(double message);

				/// <summary>
				/// 将带有数值的消息记录到应用程序的日志系统
				/// </summary>
				/// <param name="message">要记录的数值，该值会转换为字符串并包含在日志条目中</param>
				/// <remarks>
				/// 该方法将提供的消息追加到日志系统中。
				/// 日志条目以"[Message]"为前缀以指示记录数据的类型。
				/// 
				/// 使用示例：
				/// <code>
				/// // 记录单精度计算结果
				/// float temperature = GetTemperature();
				/// LogHelper::WriteLogO(temperature);
				/// // 日志输出：[Message]  23.5
				/// 
				/// // 记录进度百分比
				/// float progress = static_cast<float>(completedTasks) / totalTasks * 100;
				/// LogHelper::WriteLogO(progress);
				/// 
				/// // 记录传感器读数
				/// float sensorReading = ReadSensor();
				/// LogHelper::WriteLogO(sensorReading);
				/// </code>
				/// </remarks>
				static void WriteLogO(float message);

				/// <summary>
				/// 将包含指定数值数组的日志条目写入日志
				/// </summary>
				/// <param name="message">要记录的double值数组，数组中的每个值都会转换为字符串并包含在日志条目中</param>
				/// <remarks>
				/// 该方法将message数组中的所有数值连接成单个字符串，数值间用空格分隔，并将结果字符串添加到日志中。
				/// 
				/// 使用示例：
				/// <code>
				/// // 静默记录时间序列数据
				/// std::vector<double> timeSeries = {0.1, 0.2, 0.3, 0.4, 0.5};
				/// LogHelper::WriteLogO(timeSeries);
				/// // 日志输出：0.1 0.2 0.3 0.4 0.5
				/// 
				/// // 记录统计汇总数据
				/// std::vector<double> statistics = {mean, median, standardDeviation};
				/// LogHelper::WriteLogO(statistics);
				/// 
				/// // 记录迭代收敛历史
				/// std::vector<double> convergenceHistory = GetConvergenceHistory();
				/// LogHelper::WriteLogO(convergenceHistory);
				/// </code>
				/// </remarks>
				static void WriteLogO(const std::vector<double>& message);

				/// <summary>
				/// 将由指定浮点数数组中的元素组成的日志条目写入日志
				/// </summary>
				/// <param name="message">要记录的浮点数数组，数组中的每个元素都会转换为字符串并连接成单个日志条目</param>
				/// <remarks>
				/// 该方法通过将message数组中的每个元素转换为其字符串表示形式，并将它们连接成单个空格分隔的字符串来处理数组。
				/// 然后将此字符串添加到日志存储中。
				/// 
				/// 使用示例：
				/// <code>
				/// // 静默记录传感器校准数据
				/// std::vector<float> calibrationData = {0.98f, 1.02f, 0.99f, 1.01f};
				/// LogHelper::WriteLogO(calibrationData);
				/// // 日志输出：0.98 1.02 0.99 1.01
				/// 
				/// // 记录权重参数
				/// std::vector<float> weights = neuralNetwork.GetWeights();
				/// LogHelper::WriteLogO(weights);
				/// 
				/// // 记录频率响应数据
				/// std::vector<float> frequencyResponse = AnalyzeFrequency();
				/// LogHelper::WriteLogO(frequencyResponse);
				/// </code>
				/// </remarks>
				static void WriteLogO(const std::vector<float>& message);

#pragma endregion 输出但是不在控制台显示
			};

		} // namespace Log
	} // namespace IO
} // namespace HawtC3

#endif // !HawtC3_IO_LOG_LOGHELPER_H