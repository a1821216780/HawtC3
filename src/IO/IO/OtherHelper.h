//**********************************************************************************************************************************
// LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,赵子祯
//
//    This file is part of Qahse.IO.
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
// # 防止头文件被多重包含
#ifndef OTHERHELPER_H
#define OTHERHELPER_H

#include <string>
#include <vector>
#include <array>
#include <queue>
#include <optional>
#include <typeinfo>
#include <Eigen/Dense>

namespace Qahse::IO::IO
{

	/// <summary>
	/// 提供各种操作的实用方法和类的集合，包括文件处理、字符串操作、进程管理等功能。
	/// 该静态类包含广泛的辅助方法和嵌套实用类，旨在简化常见的编程任务。
	/// </summary>
	/// <remarks>
	/// <para>此静态类包含各种辅助方法和嵌套实用类，旨在简化常见的编程任务，包括文件操作、进程执行、字符串格式化和系统信息检索等。</para>
	/// <para>主要功能模块：</para>
	/// <list type="bullet">
	/// <item>文件路径处理和规范化</item>
	/// <item>字符串匹配和操作</item>
	/// <item>数据结构工具类（固定队列、进度条）</item>
	/// <item>数学库加速支持</item>
	/// <item>进程和命令执行</item>
	/// <item>系统信息获取</item>
	/// <item>类型转换和反射操作</item>
	/// <item>文件压缩和解压</item>
	/// <item>时间日期处理</item>
	/// </list>
	/// </remarks>
	class OtherHelper
	{
	public:
		/*/// <summary>
		/// Fixed Queue Template Class - 一个大小固定的队列数组，实现先进先出(FIFO)的数据结构。
		/// 当队列达到容量上限时，新添加的元素会自动移除最早的元素，保持固定大小。
		/// </summary>
		/// <typeparam name="T">队列中元素的类型</typeparam>
		template<typename T>
		class FixedQueue
		{
		private:
			std::queue<T> stack;
			int num;

		public:
			/// <summary>
			/// 初始化一个新的FixedQueue实例，指定容量和初始填充值。
			/// </summary>
			/// <param name="capacity">队列的固定容量，必须大于0</param>
			/// <param name="value">用于初始化队列所有位置的默认值</param>
			FixedQueue(int capacity, const T& value = T{});

			/// <summary>
			/// 向队列添加一个新元素。如果队列已满，会自动移除最早的元素以维持固定容量。
			/// </summary>
			/// <param name="element">要添加到队列的元素</param>
			void Push(const T& element);

			/// <summary>
			/// 返回队列的固定容量大小。
			/// </summary>
			/// <returns>队列的最大容量</returns>
			int Count() const;

			/// <summary>
			/// 返回队列中已经填充数据的实际元素数量。
			/// </summary>
			/// <returns>队列当前包含的元素数量</returns>
			int PushCount() const;

			/// <summary>
			/// 获取队列中指定索引位置的元素。
			/// </summary>
			/// <param name="index">要访问的元素索引，从0开始</param>
			/// <returns>指定索引位置的元素</returns>
			T operator[](int index) const;

			bool Empty() const { return stack.empty(); }

			bool Full() const { return static_cast<int>(stack.size()) >= num; }

			T Front() const { return stack.front(); }
		};*/

		/// <summary>
		/// Progress Bar Class - 控制台进度条显示类，用于在控制台中显示任务执行进度的可视化表示。
		/// 支持百分比显示、进度条图形和自定义缩进级别。
		/// </summary>
		class ProgressBar
		{
		private:
			static constexpr int TotalBars = 30;
			static constexpr char ProgressBarChar = '#';
			static constexpr char BackgroundChar = '-';

			const int _totalIterations;
			int _currentIteration;
			std::string levels;

		public:
			/// <summary>
			/// 初始化一个新的ProgressBar实例，指定总迭代次数和缩进级别。
			/// </summary>
			/// <param name="totalIterations">进度条将代表的总迭代次数，必须是正整数</param>
			/// <param name="level">进度条的缩进级别，默认为0</param>
			ProgressBar(int totalIterations, int level = 0);

			/// <summary>
			/// 更新进度显示，基于当前迭代次数和总迭代次数计算并显示进度。
			/// </summary>
			void UpdateProgress();
		};

		/// <summary>
		/// 表示结构体字段信息的元组类型
		/// </summary>
		/// <typeparam name="T">结构体类型</typeparam>
		template <typename T>
		struct FieldInfo
		{
			std::string name;
			std::type_info const *type;
			T value;
		};

	public:
		/// <summary>
		/// 将指定路径转换为规范化的绝对路径。
		/// 此方法确保结果路径采用一致的格式，适合在文件系统操作中使用。
		/// </summary>
		/// <param name="path">要规范化并转换为绝对路径的输入路径，可以是相对路径或绝对路径</param>
		/// <returns>表示规范化绝对路径的字符串</returns>
		static std::string FormortPath(const std::string &path);

		/// <summary>
		/// 在字符串列表中找到与目标字符串最匹配的项。
		/// 使用Levenshtein距离算法计算字符串之间的编辑距离，返回距离最小的匹配项。
		/// </summary>
		/// <param name="strings">要搜索的字符串列表</param>
		/// <param name="target">目标字符串，用于比较匹配</param>
		/// <returns>包含最佳匹配项索引和字符串内容的元组</returns>
		static std::tuple<int, std::string> FindBestMatch(const std::vector<std::string> &strings, const std::string &target);

		/// <summary>
		/// 检测并尝试使用数学库加速（MKL、OpenBLAS、CUDA），返回当前可用的数学加速库状态。
		/// </summary>
		/// <param name="acc">指定要使用的加速库类型，支持"mkl"、"openblas"、"cuda"，默认为"mkl"</param>
		/// <returns>返回当前系统中可用的数学加速库状态字符串</returns>
		static std::string GetMathAcc();

		/// <summary>
		/// 设置属性加速器，（MKL、OpenBLAS、CUDA），返回当前可用的数学加速库状态。
		/// </summary>
		/// <param name="acc"></param>
		// static void SetMathAcc(const std::string& acc);

		/// <summary>
		/// 运行指定的可执行文件，并传递文件路径作为参数。
		/// </summary>
		/// <param name="exePath">要执行的可执行文件的完整路径</param>
		/// <param name="filePath">要传递给可执行文件的文件路径参数</param>
		static void SysRun(const std::string &exePath, const std::string &filePath);

		/// <summary>
		/// 在命令提示符(CMD)中执行指定的命令。
		/// </summary>
		/// <param name="command">要在CMD中执行的命令字符串</param>
		static void RunCmd(const std::string &command);

		/// <summary>
		/// 在新进程中执行指定的PowerShell命令。
		/// </summary>
		/// <param name="cmd">要执行的PowerShell命令字符串</param>
		static void RunPowershell(const std::string &cmd);

		/// <summary>
		/// 执行指定路径的外部可执行文件。
		/// </summary>
		/// <param name="path">要运行的可执行文件的完整路径</param>
		static void RunExe(const std::string &path);

		/// <summary>
		/// 获取当前程序集的项目名称。
		/// </summary>
		/// <returns>返回当前程序集的名称字符串</returns>
		static std::string GetCurrentProjectName();

		/// <summary>
		/// 获取当前正在执行的可执行文件名称（包含扩展名）。
		/// </summary>
		/// <returns>包含扩展名的可执行文件名称字符串</returns>
		static std::string GetCurrentExeName();

		/// <summary>
		/// 获取当前程序的版本号信息。
		/// </summary>
		/// <param name="path">可选的文件路径</param>
		/// <returns>版本号字符串</returns>
		static std::string GetCurrentVersion(const std::string &path = "");

		/// <summary>
		/// 获取当前程序集的版本号信息。
		/// </summary>
		/// <returns>版本号字符串</returns>
		static std::string GetCurrentAssemblyVersion();

		/// <summary>
		/// 获取当前编译模式，debug或者Release
		/// </summary>
		/// <returns>"Debug" "Release"</returns>
		static std::string GetCurrentBuildMode();

		/// <summary>
		/// 比较两个对象是否相等，可选择忽略指定字段。
		/// </summary>
		template <typename T>
		static bool AreEqual(const T &left, const T &right, const std::vector<std::string> &ignoreFieldsName = {});

		/// <summary>
		/// 确定当前应用程序是否为Windows Forms应用程序。
		/// </summary>
		/// <returns>如果是Windows Forms应用程序返回true，否则返回false</returns>
		static bool isWinform();

		/// <summary>
		/// 获取编译位数X64或者其他
		/// </summary>
		/// <returns>"_x64" "_x32"</returns>
		static std::string GetBuildMode();

		/// <summary>
		/// 在给定的文件目录下面找到给定文件后缀名称的一个路径列表
		/// </summary>
		/// <param name="directoryPath">文件夹</param>
		/// <param name="fileExtension">文件拓展名称".cs"</param>
		/// <returns>文件路径列表</returns>
		static std::vector<std::string> FindFilesWithExtension(const std::string &directoryPath, const std::string &fileExtension);

		/// <summary>
		/// 更改当前的文件参考路径
		/// </summary>
		// 在 OtherHelper 类声明中添加静态 CopyFileW 方法声明（仅限 Windows 平台）
#ifdef _WIN32
	public:
		// 复制单个文件（宽字符版本，直接调用 WinAPI CopyFileW）
		static void CopyFileW(const std::string &sourceDirectory, const std::string &targetDirectory,
							  const std::string &fileType, bool overwrite = true);
#endif
		/// <param name="mainFilePath">带后缀名称的文件路径</param>
		static void SetCurrentDirectoryW(const std::string &mainFilePath);

		/// <summary>
		/// 将对象转换为字符串表示
		/// </summary>
		template <typename T>
		static std::string Tostring(const T &message, char fg = '\t', bool coloum = true);

		template <typename T>
		struct ParseLineArgs
		{
			const std::vector<std::string> &lines;
			const std::string &filename;
			std::tuple<int, std::string> pp;
			std::optional<T> moren = std::nullopt;
			int num = 0;
			char fg = ' ';
			char fg1 = '\t';
			int station = 0;
			const std::vector<std::string> *namelist = nullptr;
			bool row = false;
			const std::string *titleLine = nullptr;
			bool warning = true;
			const std::string &errorInf = "";
		};

		/// <summary>
		/// 从字符串数组中解析数据行，支持多种解析模式和错误处理
		/// </summary>
		/// <typeparam name="T">要解析的目标类型</typeparam>
		/// <param name="lines">包含数据的字符串数组</param>
		/// <param name="filename">文件名，用于错误报告</param>
		/// <param name="pp">包含索引和键的元组，用于定位数据</param>
		/// <param name="moren">默认值，当解析失败时使用</param>
		/// <param name="num">数据编号或索引</param>
		/// <param name="fg">主分隔符字符</param>
		/// <param name="fg1">次级分隔符字符</param>
		/// <param name="station">解析位置索引</param>
		/// <param name="namelist">名称列表，用于验证</param>
		/// <param name="row">是否按行解析</param>
		/// <param name="titleLine">标题行内容</param>
		/// <param name="warning">是否显示警告信息</param>
		/// <param name="errorInf">自定义错误信息</param>
		/// <returns>解析后的类型T的值</returns>
		// template <typename T>
		// static T ParseLine(const std::vector<std::string> &lines, const std::string &filename,
		// 				   std::tuple<int, std::string> pp, std::optional<T> moren = std::nullopt,
		// 				   int num = 0, char fg = ' ', char fg1 = '\t', int station = 0,
		// 				   const std::vector<std::string> *namelist = nullptr, bool row = false,
		// 				   const std::string *titleLine = nullptr, bool warning = true,
		// 				   const std::string &errorInf = "");
		template <typename T>
		static T ParseLine(ParseLineArgs<T> args);
		/// <summary>
		/// 从字符串数组中读取输出关键字
		/// </summary>
		/// <param name="data">包含文本数据的字符串数组</param>
		/// <param name="index">开始读取的数据行索引位置</param>
		/// <param name="deleteSame">是否删除重复的字符串</param>
		/// <returns>提取出的关键字字符串数组</returns>
		static std::vector<std::string> ReadOutputWord(const std::vector<std::string> &data, int index, bool deleteSame);

		/// <summary>
		/// 在字符串数组中查找包含指定搜索词的所有行的索引。
		/// </summary>
		/// <param name="input">要搜索的字符串数组</param>
		/// <param name="searchTerm">要查找的搜索词或短语</param>
		/// <param name="path">文件路径，用于错误日志记录</param>
		/// <param name="error">找不到匹配时是否记录错误日志</param>
		/// <param name="show">当error为false时是否显示警告信息</param>
		/// <returns>包含匹配行索引的列表</returns>
		static std::vector<int> GetMatchingLineIndexes(const std::vector<std::string> &input, const std::string &searchTerm,
													   const std::string &path, bool error = true, bool show = true);

		/// <summary>
		/// 检查指定字符串中是否包含搜索词。
		/// </summary>
		/// <param name="input">要搜索的目标字符串</param>
		/// <param name="searchTerm">要查找的搜索词</param>
		/// <returns>如果找到搜索词返回true，否则返回false</returns>
		static bool GetMatchingLineIndexes(const std::string &input, const std::string &searchTerm);

		/// <summary>
		/// 获取结构体的字段信息数组（泛型版本）
		/// </summary>
		/// <typeparam name="T">结构体类型</typeparam>
		/// <param name="str">结构体实例</param>
		/// <returns>包含字段信息的数组</returns>
		template <typename T>
		static std::vector<FieldInfo<typename T::value_type>> GetStructFields(const T &str);

		/// <summary>
		/// 获取结构体的字段信息数组（通用版本）
		/// </summary>
		/// <param name="structure">结构体对象</param>
		/// <returns>包含字段名称、类型和值的元组数组</returns>
		template <typename T>
		static std::vector<std::tuple<std::string, const std::type_info *, T>> GetStructFields(const T &structure);

		/// <summary>
		/// 获取结构体的名称和类型信息数组
		/// </summary>
		/// <param name="structure">结构体对象</param>
		/// <returns>包含字段名称和类型的元组数组</returns>
		template <typename T>
		static std::vector<std::tuple<std::string, const std::type_info *>> GetStructNameAndType(const T &structure);

		/// <summary>
		/// 确定指定的类型是否表示用户定义的结构体
		/// </summary>
		/// <typeparam name="T">要检查的类型</typeparam>
		/// <returns>如果是结构体返回true，否则返回false</returns>
		template <typename T>
		static bool IsStruct();

		/// <summary>
		/// 确定指定的类型是否为值类型结构体或值类型结构体数组
		/// </summary>
		/// <typeparam name="T">要检查的类型</typeparam>
		/// <returns>如果是结构体或结构体数组返回true，否则返回false</returns>
		template <typename T>
		static bool IsStructOrStructArray();

		/// <summary>
		/// 确定指定的对象是否为列表类型
		/// </summary>
		/// <param name="obj">要检查的对象</param>
		/// <returns>如果是列表类型返回true，否则返回false</returns>
		template <typename T>
		static bool IsList(const T &obj);

		/// <summary>
		/// 获取结构体类型的名称
		/// </summary>
		/// <typeparam name="T">结构体类型</typeparam>
		/// <param name="structure">结构体实例</param>
		/// <returns>结构体类型的名称</returns>
		template <typename T>
		static std::string GetStructName(const T &structure);

		/// <summary>
		/// 获取当前计算机的处理器核心数量（逻辑处理器数量）。
		/// </summary>
		/// <returns>当前系统的逻辑处理器数量</returns>
		static int GetThreadCount();

		/// <summary>
		/// 尝试将字符串转换为指定的枚举类型。
		/// </summary>
		template <typename T>
		static bool TryConvertToEnum(const std::string &value, T &enumValue);

		/// <summary>
		/// 将字符串转换为指定的枚举类型。
		/// </summary>
		template <typename T>
		static T ConvertToEnum(const std::string &value);

		/// <summary>
		/// 将矩阵数据转换为带有行列标题的输出文件格式
		/// </summary>
		/// <param name="title">矩阵标题</param>
		/// <param name="rowtitle">行标题数组</param>
		/// <param name="columtitle">列标题数组</param>
		/// <param name="matrix">矩阵数据</param>
		/// <returns>格式化后的字符串数组</returns>
		template <typename MatrixType>
		static std::vector<std::string> ConvertMatrixTitleToOutfile(const std::string &title,
																	const std::vector<std::string> &rowtitle,
																	const std::vector<std::string> &columtitle,
																	const MatrixType &matrix);

		/// <summary>
		/// 获取指定文件路径的扩展名。
		/// </summary>
		/// <param name="path">文件路径</param>
		/// <returns>文件扩展名的小写字符串</returns>
		static std::string GetFileExtension(const std::string &path);

		/// <summary>
		/// 更改文件的扩展名并在文件系统中重命名文件。
		/// </summary>
		/// <param name="path">文件的原始路径，执行后会更新为新的路径</param>
		/// <param name="newExtension">新的文件扩展名</param>
		static void SetFileExtension(std::string &path, const std::string &newExtension);

		/// <summary>
		/// 在输入字符串的每个字符之间插入指定的分隔符字符串。
		/// </summary>
		/// <param name="source">要处理的输入字符串</param>
		/// <param name="spilt">用作字符间分隔符的字符串</param>
		/// <param name="num">分隔符在字符间重复的次数</param>
		/// <returns>在每个字符之间插入了指定分隔符的新字符串</returns>
		static std::string FillString(const std::string &source, const std::string &spilt, int num = 1);

		/// <summary>
		/// 生成指定长度的随机字符串，字符串由大小写字母组成。
		/// </summary>
		/// <param name="len">要生成的随机字符串的长度</param>
		/// <returns>由大小写字母组成的指定长度的随机字符串</returns>
		static std::string RandomString(int len);

		/// <summary>
		/// 将输入字符串居中显示在指定宽度的字符串中，空余部分用指定符号填充。
		/// </summary>
		/// <param name="input">要居中显示的输入字符串</param>
		/// <param name="width">目标字符串的总宽度</param>
		/// <param name="symbol">用于填充空余位置的字符</param>
		/// <returns>居中对齐后的字符串</returns>
		static std::string CenterText(const std::string &input, int width, char symbol = ' ');

		/// <summary>
		/// 设置控制台光标相对于当前位置的偏移位置。
		/// </summary>
		/// <param name="left">水平偏移量</param>
		/// <param name="top">垂直偏移量</param>
		static void SetCursorPosition(int left, int top);

		static std::tm GetSafeLocalTime(const std::time_t &time);

		static std::tm *GetSafeLocalTime(const std::time_t &time, const bool ptr);
		/// <summary>
		/// 获取当前年份的字符串表示。
		/// </summary>
		/// <returns>当前年份的字符串表示</returns>
		static std::string GetCurrentYear();

		/// <summary>
		/// 获取当前年份的整数表示。
		/// </summary>
		/// <param name="temp111">未使用的参数</param>
		/// <returns>当前年份的整数值</returns>
		static int GetCurrentYear(bool temp111);

		/// <summary>
		/// 获取当前月份的字符串表示。
		/// </summary>
		/// <returns>当前月份的字符串表示</returns>
		static std::string GetCurrentMonth();

		/// <summary>
		/// 获取当前月份的整数表示。
		/// </summary>
		/// <param name="temp111">未使用的参数</param>
		/// <returns>当前月份的整数值</returns>
		static int GetCurrentMonth(bool temp111);

		/// <summary>
		/// 获取当前日期的字符串表示。
		/// </summary>
		/// <returns>当前日期的字符串表示</returns>
		static std::string GetCurrentDay();

		/// <summary>
		/// 获取当前日期的整数表示。
		/// </summary>
		/// <param name="temp111">未使用的参数</param>
		/// <returns>当前日期的整数值</returns>
		static int GetCurrentDay(bool temp111);

		/// <summary>
		/// 获取当前小时的字符串表示。
		/// </summary>
		/// <returns>当前小时的字符串表示</returns>
		static std::string GetCurrentHour();

		/// <summary>
		/// 获取当前小时的整数表示。
		/// </summary>
		/// <param name="temp111">未使用的参数</param>
		/// <returns>当前小时的整数值</returns>
		static int GetCurrentHour(bool temp111);

		/// <summary>
		/// 获取当前分钟的字符串表示。
		/// </summary>
		/// <returns>当前分钟的字符串表示</returns>
		static std::string GetCurrentMinute();

		/// <summary>
		/// 获取当前分钟的整数表示。
		/// </summary>
		/// <param name="temp111">未使用的参数</param>
		/// <returns>当前分钟的整数值</returns>
		static int GetCurrentMinute(bool temp111);

		/// <summary>
		/// 获取当前秒的字符串表示。
		/// </summary>
		/// <returns>当前秒的字符串表示</returns>
		static std::string GetCurrentSecond();

		/// <summary>
		/// 获取当前秒的整数表示。
		/// </summary>
		/// <param name="temp111">未使用的参数</param>
		/// <returns>当前秒的整数值</returns>
		static int GetCurrentSecond(bool temp111);

		/// <summary>
		/// 获取当前时间以字符串形式表示。
		/// </summary>
		/// <returns>当前本地时间的字符串表示</returns>
		static std::string GetCurrentTimeW();

		/// <summary>
		/// 获取编译时间字符串。
		/// </summary>
		/// <returns>编译时间的字符串表示</returns>
		static std::string GetBuildTime();

		/// <summary>
		/// 去除列表中的重复元素，返回只包含唯一元素的新列表。
		/// </summary>
		template <typename T>
		static std::vector<T> Distinct(const std::vector<T> &values);

		/// <summary>
		/// 获取列表中的重复元素，返回包含所有重复项的新列表。
		/// </summary>
		template <typename T>
		static std::vector<T> Duplicates(const std::vector<T> &values);

		/// <summary>
		/// 去除数组中的重复元素，返回只包含唯一元素的新数组。
		/// </summary>
		template <typename T, size_t N>
		static std::array<T, N> Distinct(const std::array<T, N> &values);

		/// <summary>
		/// 获取数组中的重复元素，返回包含所有重复项的新数组。
		/// </summary>
		template <typename T, size_t N>
		static std::vector<T> Duplicates(const std::array<T, N> &values);

		/// <summary>
		/// 计算Levenshtein距离的辅助函数
		/// </summary>
		static int LevenshteinDistance(const std::string &a, const std::string &b);

		/// <summary>
		/// 字符串分割辅助函数
		/// </summary>
		static std::vector<std::string> SplitString(const std::string &str, const std::string &delimiter);

		/// <summary>
		/// 字符串trim辅助函数
		/// </summary>
		static std::string TrimString(const std::string &str);

		/// <summary>
		/// 转换为小写的辅助函数
		/// </summary>
		static std::string ToLowerString(const std::string &str);

		/// <summary>
		/// 检查文件是否存在
		/// </summary>
		static bool FileExists(const std::string &path);

		/// <summary>
		/// 检查目录是否存在
		/// </summary>
		static bool DirectoryExists(const std::string &path);

		/// <summary>
		/// 创建目录
		/// </summary>
		static void CreateDirectories(const std::string &path);
	};

} // End of OtherHelper class

#endif // OTHERHELPER_H