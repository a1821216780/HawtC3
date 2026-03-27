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
//     http://www.HawtC.cn/licenses/txt
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

#ifndef BINARYFILEHANDLER_H
#define BINARYFILEHANDLER_H

#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <type_traits>
#include <cstdint>
#include <Eigen/Dense>
#include "../Log/LogHelper.h"
#include "../Math/LinearAlgebraHelper.h"

namespace Qahse
{
    namespace IO
    {
        namespace IO
        {

            /// <summary>
            /// 二进制文件处理器，提供二进制文件的读取、写入和追加功能
            /// </summary>
            /// <remarks>
            /// 该类封装了二进制文件的操作，支持多种数据类型的读写，包括基本数据类型、数组和矩阵。
            /// 使用示例：
            /// <code>
            /// // 写入数据到二进制文件
            /// BinaryFile writer("data.bin", "write");
            /// writer.WriteData(123);
            /// writer.WriteData("Hello World");
            /// writer.WriteData(std::vector<double>{1.0, 2.0, 3.0});
            ///
            /// // 从二进制文件读取数据
            /// BinaryFile reader("data.bin", "read");
            /// int number = reader.ReadData<int>();
            /// std::string text = reader.ReadData<std::string>();
            /// std::vector<double> array = reader.ReadData<std::vector<double>>();
            /// </code>
            /// </remarks>
            class BinaryFile
            {
            private:
                /**
                 * @brief 文件路径，包含文件名和扩展名。
                 *        该成员变量用于记录当前操作的二进制文件路径。
                 *        例如："data.bin"。
                 */
                std::string filePath; ///< 当前二进制文件路径

                /**
                 * @brief 写入文件流指针。
                 *        仅在写入或追加模式下有效。
                 *        通过unique_ptr自动管理资源，避免内存泄漏。
                 */
                std::unique_ptr<std::ofstream> writer; ///< 写入流

                /**
                 * @brief 读取文件流指针。
                 *        仅在读取模式下有效。
                 *        通过unique_ptr自动管理资源，避免内存泄漏。
                 */
                std::unique_ptr<std::ifstream> reader; ///< 读取流

                /**
                 * @brief 当前文件是否处于读取模式。
                 *        true 表示正在读取，false 表示未读取。
                 */
                bool isReading; ///< 读取模式标志

                /**
                 * @brief 当前文件是否处于写入/追加模式。
                 *        true 表示正在写入，false 表示未写入。
                 */
                bool isWriting; ///< 写入模式标志

            public:
                /// <summary>
                /// 初始化 BinaryFile 类的新实例，根据指定模式打开文件
                /// </summary>
                /// <param name="filePath">文件路径，包含文件扩展名</param>
                /// <param name="mode">文件访问模式："read"/"r"(读取)、"write"/"w"(写入)、"append"/"a"(追加)</param>
                /// <remarks>
                /// 构造函数根据模式参数创建相应的文件流。支持大小写不敏感的模式参数。
                /// 使用示例：
                /// <code>
                /// // 创建用于写入的二进制文件
                /// BinaryFile writer("output.bin", "write");
                ///
                /// // 创建用于读取的二进制文件
                /// BinaryFile reader("input.bin", "read");
                ///
                /// // 创建用于追加的二进制文件
                /// BinaryFile appender("log.bin", "append");
                /// </code>
                /// </remarks>
                /**
                 * @brief 构造函数，根据指定模式打开二进制文件。
                 * @param filePath 文件路径，包含文件名和扩展名。
                 * @param mode 文件访问模式："read"/"r"(读取)、"write"/"w"(写入)、"append"/"a"(追加)。
                 * @note 支持大小写不敏感的模式参数。
                 * @code
                 * // 创建用于写入的二进制文件
                 * BinaryFile writer("output.bin", "write");
                 * // 创建用于读取的二进制文件
                 * BinaryFile reader("input.bin", "read");
                 * // 创建用于追加的二进制文件
                 * BinaryFile appender("log.bin", "append");
                 * @endcode
                 */
                BinaryFile(const std::string &filePath, const std::string &mode = "read");

                /// <summary>
                /// 析构函数，自动释放文件流资源
                /// </summary>
                /// <remarks>
                /// 当对象被垃圾回收时自动调用，确保文件流被正确关闭。
                /// 使用示例：
                /// <code>
                /// // 对象超出作用域时自动调用析构函数
                /// {
                ///     BinaryFile file("temp.bin", "write");
                ///     // 文件操作...
                /// } // 此处析构函数被调用，自动关闭文件流
                /// </code>
                /// </remarks>
                /**
                 * @brief 析构函数，自动关闭文件流并释放资源。
                 * @details 当对象生命周期结束时自动调用，确保文件流被正确关闭，防止资源泄漏。
                 * @code
                 * {
                 *     BinaryFile file("temp.bin", "write");
                 *     // 文件操作...
                 * } // 此处析构函数被调用，自动关闭文件流
                 * @endcode
                 */
                ~BinaryFile();

                /// <summary>
                /// 手动关闭当前文件流，释放系统资源
                /// </summary>
                /// <remarks>
                /// 建议在完成文件操作后主动调用此方法，而不是依赖析构函数。
                /// 使用示例：
                /// <code>
                /// BinaryFile file("data.bin", "write");
                /// file.WriteData(42);
                /// file.Close(); // 主动关闭文件流
                /// </code>
                /// </remarks>
                /**
                 * @brief 手动关闭当前文件流，释放系统资源。
                 * @details 建议在完成文件操作后主动调用此方法，而不是依赖析构函数。
                 * @code
                 * BinaryFile file("data.bin", "write");
                 * file.WriteData(42);
                 * file.Close(); // 主动关闭文件流
                 * @endcode
                 */
                void Close();

                /// <summary>
                /// 泛型方法，将指定类型的数据写入二进制文件
                /// </summary>
                /// <typeparam name="T">要写入的数据类型，支持基本类型、数组和矩阵</typeparam>
                /// <param name="data">要写入的数据</param>
                /// <remarks>
                /// 支持写入多种数据类型，包括：基本类型(int, float, double, string等)、一维数组、多维数组、矩阵类型。
                /// 对于数组类型，会先写入数组长度，再写入数组元素。
                /// 使用示例：
                /// <code>
                /// BinaryFile file("data.bin", "write");
                /// file.WriteData(123);                              // 写入整数
                /// file.WriteData(3.14);                             // 写入浮点数
                /// file.WriteData(std::string("Hello"));             // 写入字符串
                /// file.WriteData(std::vector<int>{1, 2, 3});        // 写入整数数组
                /// </code>
                /// </remarks>
                /**
                 * @brief 泛型方法，将指定类型的数据写入二进制文件。
                 * @tparam T 要写入的数据类型，支持基本类型、数组和矩阵。
                 * @param data 要写入的数据。
                 * @details 支持写入多种数据类型，包括：基本类型(int, float, double, string等)、一维数组、多维数组、矩阵类型。
                 *          对于数组类型，会先写入数组长度，再写入数组元素。
                 * @code
                 * BinaryFile file("data.bin", "write");
                 * file.WriteData(123);                              // 写入整数
                 * file.WriteData(3.14);                             // 写入浮点数
                 * file.WriteData(std::string("Hello"));             // 写入字符串
                 * file.WriteData(std::vector<int>{1, 2, 3});        // 写入整数数组
                 * @endcode
                 */
                template <typename T>
                void WriteData(const T &data);

                /// <summary>
                /// 泛型方法，将指定类型的数据以行的形式写入文件
                /// </summary>
                /// <typeparam name="T">要写入的数据类型</typeparam>
                /// <param name="data">要写入的数据，不能为 null</param>
                /// <remarks>
                /// 该方法内部调用 WriteData 方法，提供与 WriteLine 语义一致的接口。
                /// 使用示例：
                /// <code>
                /// BinaryFile file("output.bin", "write");
                /// file.WriteLine(42);        // 写入整数
                /// file.WriteLine("Text");    // 写入字符串
                /// file.WriteLine(true);      // 写入布尔值
                /// </code>
                /// </remarks>
                /**
                 * @brief 泛型方法，将指定类型的数据以行的形式写入文件。
                 * @tparam T 要写入的数据类型。
                 * @param data 要写入的数据，不能为 null。
                 * @details 该方法内部调用 WriteData 方法，提供与 WriteLine 语义一致的接口。
                 * @code
                 * BinaryFile file("output.bin", "write");
                 * file.WriteLine(42);        // 写入整数
                 * file.WriteLine("Text");    // 写入字符串
                 * file.WriteLine(true);      // 写入布尔值
                 * @endcode
                 */
                template <typename T>
                void WriteLine(const T &data);

                /// <summary>
                /// 将格式化的双精度浮点数以行的形式写入文件
                /// </summary>
                /// <param name="format">标准或自定义数值格式字符串，用于确定数值的格式</param>
                /// <param name="message">要格式化并写入的双精度浮点数值</param>
                /// <remarks>
                /// 使用指定的格式字符串对数值进行格式化后写入文件。
                /// 使用示例：
                /// <code>
                /// BinaryFile file("numbers.bin", "write");
                /// file.WriteLine("F2", 3.14159);     // 写入 "3.14"
                /// file.WriteLine("E", 1234.5);       // 写入 "1.234500E+003"
                /// file.WriteLine("P", 0.85);         // 写入 "85.00 %"
                /// </code>
                /// </remarks>
                /**
                 * @brief 将格式化的双精度浮点数以行的形式写入文件。
                 * @param format 标准或自定义数值格式字符串，用于确定数值的格式。
                 * @param message 要格式化并写入的双精度浮点数值。
                 * @details 使用指定的格式字符串对数值进行格式化后写入文件。
                 * @code
                 * BinaryFile file("numbers.bin", "write");
                 * file.WriteLine("F2", 3.14159);     // 写入 "3.14"
                 * file.WriteLine("E", 1234.5);       // 写入 "1.234500E+003"
                 * file.WriteLine("P", 0.85);         // 写入 "85.00 %"
                 * @endcode
                 */
                void WriteLine(const std::string &format, double message);

                /// <summary>
                /// 将格式化的单精度浮点数以行的形式写入文件
                /// </summary>
                /// <param name="format">标准或自定义数值格式字符串，用于确定浮点数值的格式</param>
                /// <param name="message">要格式化并写入的单精度浮点数值</param>
                /// <remarks>
                /// 使用指定的格式字符串对单精度浮点数进行格式化后写入文件。
                /// 使用示例：
                /// <code>
                /// BinaryFile file("data.bin", "write");
                /// file.WriteLine("F1", 2.718f);      // 写入 "2.7"
                /// file.WriteLine("N", 1000.5f);      // 写入 "1,000.5"
                /// file.WriteLine("C", 99.99f);       // 写入货币格式
                /// </code>
                /// </remarks>
                /**
                 * @brief 将格式化的单精度浮点数以行的形式写入文件。
                 * @param format 标准或自定义数值格式字符串，用于确定浮点数值的格式。
                 * @param message 要格式化并写入的单精度浮点数值。
                 * @details 使用指定的格式字符串对单精度浮点数进行格式化后写入文件。
                 * @code
                 * BinaryFile file("data.bin", "write");
                 * file.WriteLine("F1", 2.718f);      // 写入 "2.7"
                 * file.WriteLine("N", 1000.5f);      // 写入 "1,000.5"
                 * file.WriteLine("C", 99.99f);       // 写入货币格式
                 * @endcode
                 */
                void WriteLine(const std::string &format, float message);

                /// <summary>
                /// 向输出流写入一个行终止符
                /// </summary>
                /// <remarks>
                /// 此方法仅写入行终止符到输出流，用于在输出中创建空行。
                /// 使用示例：
                /// <code>
                /// BinaryFile file("text.bin", "write");
                /// file.WriteData("第一行");
                /// file.WriteLine();          // 写入空行
                /// file.WriteData("第三行");
                /// </code>
                /// </remarks>
                /**
                 * @brief 向输出流写入一个行终止符。
                 * @details 此方法仅写入行终止符到输出流，用于在输出中创建空行。
                 * @code
                 * BinaryFile file("text.bin", "write");
                 * file.WriteData("第一行");
                 * file.WriteLine();          // 写入空行
                 * file.WriteData("第三行");
                 * @endcode
                 */
                void WriteLine();

                /// <summary>
                /// 泛型方法，将指定类型的数据写入底层存储
                /// </summary>
                /// <typeparam name="T">要写入的数据类型</typeparam>
                /// <param name="data">要写入的数据，不能为 null</param>
                /// <remarks>
                /// 该方法内部调用 WriteData 方法，提供与 Write 语义一致的接口。
                /// 使用示例：
                /// <code>
                /// BinaryFile file("output.bin", "write");
                /// file.Write(100);           // 写入整数，不换行
                /// file.Write(" items");      // 继续写入字符串
                /// file.Write(3.14);          // 继续写入浮点数
                /// </code>
                /// </remarks>
                /**
                 * @brief 泛型方法，将指定类型的数据写入底层存储。
                 * @tparam T 要写入的数据类型。
                 * @param data 要写入的数据，不能为 null。
                 * @details 该方法内部调用 WriteData 方法，提供与 Write 语义一致的接口。
                 * @code
                 * BinaryFile file("output.bin", "write");
                 * file.Write(100);           // 写入整数，不换行
                 * file.Write(" items");      // 继续写入字符串
                 * file.Write(3.14);          // 继续写入浮点数
                 * @endcode
                 */
                template <typename T>
                void Write(const T &data);

                /// <summary>
                /// 写入格式化的双精度数值字符串表示
                /// </summary>
                /// <param name="format">标准或自定义数值格式字符串，用于确定数值的格式</param>
                /// <param name="message">要格式化并写入的数值</param>
                /// <remarks>
                /// 使用指定格式字符串对数值进行格式化，然后写入结果字符串。
                /// 使用示例：
                /// <code>
                /// BinaryFile file("report.bin", "write");
                /// file.Write("Currency: ");
                /// file.Write("C", 1234.56);          // 写入货币格式 "$1,234.56"
                /// file.Write(", Percentage: ");
                /// file.Write("P1", 0.1234);          // 写入百分比格式 "12.3%"
                /// </code>
                /// </remarks>
                /**
                 * @brief 写入格式化的双精度数值字符串表示。
                 * @param format 标准或自定义数值格式字符串，用于确定数值的格式。
                 * @param message 要格式化并写入的数值。
                 * @details 使用指定格式字符串对数值进行格式化，然后写入结果字符串。
                 * @code
                 * BinaryFile file("report.bin", "write");
                 * file.Write("Currency: ");
                 * file.Write("C", 1234.56);          // 写入货币格式 "$1,234.56"
                 * file.Write(", Percentage: ");
                 * file.Write("P1", 0.1234);          // 写入百分比格式 "12.3%"
                 * @endcode
                 */
                void Write(const std::string &format, double message);

                /// <summary>
                /// 写入格式化的单精度浮点数值字符串表示
                /// </summary>
                /// <param name="format">标准或自定义数值格式字符串，用于确定浮点数值的格式</param>
                /// <param name="message">要格式化并写入的单精度浮点数值</param>
                /// <remarks>
                /// 使用指定格式字符串对单精度浮点数进行格式化，然后写入结果字符串。
                /// 使用示例：
                /// <code>
                /// BinaryFile file("measurements.bin", "write");
                /// file.Write("Temperature: ");
                /// file.Write("F1", 23.456f);         // 写入 "23.5"
                /// file.Write("°C, Humidity: ");
                /// file.Write("P0", 0.678f);          // 写入 "68%"
                /// </code>
                /// </remarks>
                /**
                 * @brief 写入格式化的单精度浮点数值字符串表示。
                 * @param format 标准或自定义数值格式字符串，用于确定浮点数值的格式。
                 * @param message 要格式化并写入的单精度浮点数值。
                 * @details 使用指定格式字符串对单精度浮点数进行格式化，然后写入结果字符串。
                 * @code
                 * BinaryFile file("measurements.bin", "write");
                 * file.Write("Temperature: ");
                 * file.Write("F1", 23.456f);         // 写入 "23.5"
                 * file.Write("°C, Humidity: ");
                 * file.Write("P0", 0.678f);          // 写入 "68%"
                 * @endcode
                 */
                void Write(const std::string &format, float message);

                /// <summary>
                /// 获取一个值，指示最后一次读取操作是否成功到达文件末尾
                /// </summary>
                /// <remarks>
                /// 通过尝试读取一个测试值来判断是否已到达文件末尾。如果无法读取则返回 true，表示已到末尾。
                /// 使用示例：
                /// <code>
                /// BinaryFile file("data.bin", "read");
                /// while (!file.Read2last())
                /// {
                ///     int value = file.ReadData<int>();
                ///     std::cout << value << '\n';
                /// }
                /// // 已读取到文件末尾
                /// </code>
                /// </remarks>
                /**
                 * @brief 获取一个值，指示最后一次读取操作是否成功到达文件末尾。
                 * @details 通过尝试读取一个测试值来判断是否已到达文件末尾。如果无法读取则返回 true，表示已到末尾。
                 * @code
                 * BinaryFile file("data.bin", "read");
                 * while (!file.Read2last())
                 * {
                 *     int value = file.ReadData<int>();
                 *     std::cout << value << '\n';
                 * }
                 * // 已读取到文件末尾
                 * @endcode
                 */
                bool Read2last();

                /// <summary>
                /// 从底层数据源读取指定类型的数据
                /// </summary>
                /// <typeparam name="T">要读取的数据类型，支持基本类型、数组和多维数组等</typeparam>
                /// <returns>从数据源读取的指定类型数据。对于数组类型，会先读取数组长度，然后读取数组元素</returns>
                /// <remarks>
                /// 支持广泛的数据类型，包括基本类型、数组和多维数组。对于不支持的类型会记录错误并返回默认值。
                /// 确保读取的类型与底层数据源中的数据格式匹配，以避免运行时错误。
                /// 使用示例：
                /// <code>
                /// BinaryFile file("data.bin", "read");
                /// int number = file.ReadData<int>();              // 读取整数
                /// std::string text = file.ReadData<std::string>(); // 读取字符串
                /// std::vector<double> array = file.ReadData<std::vector<double>>(); // 读取双精度数组
                /// </code>
                /// </remarks>
                /**
                 * @brief 从底层数据源读取指定类型的数据。
                 * @tparam T 要读取的数据类型，支持基本类型、数组和多维数组等。
                 * @return 从数据源读取的指定类型数据。对于数组类型，会先读取数组长度，然后读取数组元素。
                 * @details 支持广泛的数据类型，包括基本类型、数组和多维数组。对于不支持的类型会记录错误并返回默认值。
                 *          确保读取的类型与底层数据源中的数据格式匹配，以避免运行时错误。
                 * @code
                 * BinaryFile file("data.bin", "read");
                 * int number = file.ReadData<int>();              // 读取整数
                 * std::string text = file.ReadData<std::string>(); // 读取字符串
                 * std::vector<double> array = file.ReadData<std::vector<double>>(); // 读取双精度数组
                 * @endcode
                 */
                template <typename T>
                T ReadData();

            private:
                /**
                 * @brief 内部辅助方法：写入基本数据类型。
                 * @tparam T 基本数据类型（如int、float、double等）。
                 * @param data 要写入的基本类型数据。
                 * @details 直接以二进制方式写入数据，无类型转换。
                 */
                template <typename T>
                void WriteBasicType(const T &data);

                /**
                 * @brief 内部辅助方法：读取基本数据类型。
                 * @tparam T 基本数据类型（如int、float、double等）。
                 * @return 读取到的基本类型数据。
                 * @details 直接以二进制方式读取数据，无类型转换。
                 */
                template <typename T>
                T ReadBasicType();

                /**
                 * @brief 内部辅助方法：写入字符串。
                 * @param str 要写入的字符串。
                 * @details 先写入字符串长度，再写入字符串内容。
                 */
                void WriteString(const std::string &str);

                /**
                 * @brief 内部辅助方法：读取字符串。
                 * @return 读取到的字符串。
                 * @details 先读取字符串长度，再读取字符串内容。
                 */
                std::string ReadString();

                /**
                 * @brief 内部辅助方法：格式化数值为字符串（双精度）。
                 * @param format 格式字符串。
                 * @param value 要格式化的双精度数值。
                 * @return 格式化后的字符串。
                 */
                std::string FormatNumber(const std::string &format, double value);

                /**
                 * @brief 内部辅助方法：格式化数值为字符串（单精度）。
                 * @param format 格式字符串。
                 * @param value 要格式化的单精度数值。
                 * @return 格式化后的字符串。
                 */
                std::string FormatNumber(const std::string &format, float value);
            };

        } // namespace IO
    } // namespace IO
} // namespace Qahse

#endif // BINARYFILEHANDLER_H