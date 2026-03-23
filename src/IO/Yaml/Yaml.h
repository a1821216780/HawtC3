//**********************************************************************************************************************************
//LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,赵子祯
//
//    This file is part of HawtC3.IO.Yaml
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
#ifndef YAML_H
#define YAML_H

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <Eigen/Dense>

namespace HawtC3 {
namespace IO {
namespace Yaml {

	/// <summary>
	/// YAML节点类，表示YAML文档中的一个节点
	/// </summary>
	/// <remarks>
	/// 该类包含节点的基本信息，包括名称、值、父节点引用和层级信息。
	/// 每个节点通过parent指针形成树状层级结构。
	///
	/// 使用示例：
	/// <code>
	/// Node* node = new Node();
	/// node->name = "database";
	/// node->value = "";      // 父节点通常没有值
	/// node->space = 0;       // 根节点缩进为0
	/// node->tier = 0;        // 根节点层级为0
	/// </code>
	/// </remarks>
	struct Node {
		/// <summary>
		/// 节点的名称（键名）
		/// </summary>
		std::string name;

		/// <summary>
		/// 节点的值，空字符串表示无值（父节点）
		/// </summary>
		std::string value;

		/// <summary>
		/// 指向父节点的指针，根节点为nullptr
		/// </summary>
		Node* parent = nullptr;

		/// <summary>
		/// 节点在YAML文件中的缩进空格数
		/// </summary>
		int space = 0;

		/// <summary>
		/// 节点的层级深度（0为根节点）
		/// </summary>
		int tier = 0;

		/// <summary>
		/// 标记该节点是否具有值（区分空字符串和无值）
		/// </summary>
		bool hasValue = false;

		/// <summary>
		/// 创建当前节点的深拷贝
		/// </summary>
		/// <returns>节点的完整副本，包括父节点的递归拷贝</returns>
		Node* Clone() const;
	};


	/// <summary>
	/// YAML配置文件解析和操作类，提供读取、写入、修改YAML文件的功能
	/// </summary>
	/// <remarks>
	/// 该类支持YAML格式的配置文件操作，包括节点的增删改查、文件保存等功能。
	/// 将C#版本的YML类转换为C++实现，使用Eigen库替代MathNet.Numerics。
	///
	/// 使用示例：
	/// <code>
	/// // 创建或加载YAML文件
	/// YML yaml("config.yml");
	///
	/// // 读取配置值
	/// std::string host = yaml.read("database.host");
	///
	/// // 修改配置值
	/// yaml.modify("database.host", "localhost");
	/// yaml.AddNode("server.port", "8080");
	///
	/// // 保存文件
	/// yaml.save();
	/// </code>
	/// </remarks>
	class YML {
	public:
		/// <summary>
		/// YAML格式版本号
		/// </summary>
		static constexpr const char* ymlversion = "2.0.016";

		/// <summary>
		/// 节点列表，存储所有解析或添加的YAML节点
		/// </summary>
		std::vector<Node*> nodeList;

		/// <summary>
		/// 默认构造函数，创建空的YAML对象
		/// </summary>
		YML();

		/// <summary>
		/// 构造函数，加载并解析指定路径的YAML文件
		/// </summary>
		/// <param name="path">YAML文件路径</param>
		explicit YML(const std::string& path);

		/// <summary>
		/// 析构函数，释放所有节点内存
		/// </summary>
		~YML();

		/// <summary>
		/// 移动构造函数
		/// </summary>
		YML(YML&& other) noexcept;

		/// <summary>
		/// 移动赋值运算符
		/// </summary>
		YML& operator=(YML&& other) noexcept;

		// 禁用拷贝，使用Clone()代替
		YML(const YML&) = delete;
		YML& operator=(const YML&) = delete;

		/// <summary>
		/// 创建当前YAML对象的深拷贝
		/// </summary>
		/// <returns>返回一个完全独立的YAML对象副本</returns>
		YML Clone() const;

		/// <summary>
		/// 获取文件路径
		/// </summary>
		/// <returns>YAML文件的路径</returns>
		std::string getPath() const { return path_; }

		/// <summary>
		/// 读取指定键的值，支持点分隔的多级键名
		/// </summary>
		/// <param name="key">要读取的键名，如"database.connection.host"</param>
		/// <returns>键对应的值字符串，如果不存在则返回空字符串</returns>
		std::string read(const std::string& key);

		/// <summary>
		/// 修改指定键的值
		/// </summary>
		/// <param name="key">要修改的键名，支持点分隔的层级结构</param>
		/// <param name="value">要设置的新值</param>
		void modify(const std::string& key, const std::string& value);

		/// <summary>
		/// 根据键名查找对应的节点
		/// </summary>
		/// <param name="key">键名，支持点分隔的层级结构</param>
		/// <returns>找到的节点指针，不存在则返回nullptr</returns>
		Node* findNodeByKey(const std::string& key);

		/// <summary>
		/// 检查指定键名的节点是否存在
		/// </summary>
		/// <param name="key">要检查的键名</param>
		/// <returns>存在返回true，否则返回false</returns>
		bool ChickfindNodeByKey(const std::string& key);

		/// <summary>
		/// 生成指定节点的完整键路径
		/// </summary>
		/// <param name="node">目标节点指针</param>
		/// <returns>从根节点到该节点的点分隔路径</returns>
		std::string GetNodeKey(Node* node);

		/// <summary>
		/// 将另一个YAML对象的节点合并到当前对象中
		/// </summary>
		/// <param name="yaml">要合并的YAML对象</param>
		void AddYAML(const YML& yaml);

		/// <summary>
		/// 添加或更新指定键的节点，自动创建中间层级
		/// </summary>
		/// <param name="key">节点的层级键名，用点号分隔</param>
		/// <param name="value">节点的值</param>
		void AddNode(const std::string& key, const std::string& value);

		/// <summary>
		/// 在指定父节点下添加子节点
		/// </summary>
		/// <param name="parentKey">父节点的键名，为空则添加为根节点</param>
		/// <param name="nodeName">新节点的名称</param>
		/// <param name="value">新节点的值</param>
		void AddNode(const std::string& parentKey, const std::string& nodeName, const std::string& value);

		/// <summary>
		/// 删除指定键的节点及其所有子节点
		/// </summary>
		/// <param name="key">要删除的节点键名</param>
		void DeleteNode(const std::string& key);

		/// <summary>
		/// 查找指定键节点的所有直接子节点
		/// </summary>
		/// <param name="key">父节点的键名</param>
		/// <returns>直接子节点的指针数组</returns>
		std::vector<Node*> FindChildren(const std::string& key);

		/// <summary>
		/// 保存YAML数据到文件
		/// </summary>
		/// <param name="savepath">保存路径，为空则使用默认路径</param>
		/// <param name="format">是否在保存前格式化数据</param>
		void save(const std::string& savepath = "", bool format = true);

#pragma region ToYmlValueString 类型转换为YAML字符串

		/// <summary>
		/// 将字符串转换为YAML兼容的值字符串（去除首尾空白）
		/// </summary>
		/// <param name="str">输入字符串</param>
		/// <returns>修剪后的字符串</returns>
		static std::string ToYmlValueString(const std::string& str);

		/// <summary>
		/// 将字符串数组转换为YAML列表格式字符串
		/// </summary>
		/// <param name="str">字符串数组</param>
		/// <returns>格式如"[ item1 , item2 , item3 ]"</returns>
		static std::string ToYmlValueString(const std::vector<std::string>& str);

		/// <summary>
		/// 将整数转换为YAML字符串
		/// </summary>
		static std::string ToYmlValueString(int value);

		/// <summary>
		/// 将整数数组转换为YAML列表格式字符串
		/// </summary>
		static std::string ToYmlValueString(const std::vector<int>& values);

		/// <summary>
		/// 将双精度浮点数转换为YAML字符串
		/// </summary>
		static std::string ToYmlValueString(double value);

		/// <summary>
		/// 将双精度浮点数数组转换为YAML列表格式字符串
		/// </summary>
		static std::string ToYmlValueString(const std::vector<double>& values);

		/// <summary>
		/// 将布尔值转换为YAML字符串
		/// </summary>
		static std::string ToYmlValueString(bool value);

		/// <summary>
		/// 将布尔值数组转换为YAML列表格式字符串
		/// </summary>
		static std::string ToYmlValueString(const std::vector<bool>& values);

		/// <summary>
		/// 将Eigen矩阵转换为YAML多行格式字符串
		/// </summary>
		/// <param name="data">Eigen矩阵</param>
		/// <param name="level">缩进级别</param>
		/// <returns>YAML格式的多行矩阵字符串</returns>
		static std::string ToYmlValueString(const Eigen::MatrixXd& data, int level);

		/// <summary>
		/// 将Eigen向量转换为YAML列表格式字符串
		/// </summary>
		/// <param name="data">Eigen向量</param>
		/// <param name="level">缩进级别（默认0）</param>
		/// <returns>YAML格式的向量字符串</returns>
		static std::string ToYmlValueString(const Eigen::VectorXd& data, int level = 0);

#pragma endregion ToYmlValueString

	private:
		/// <summary>
		/// YAML文件路径
		/// </summary>
		std::string path_;

		/// <summary>
		/// 文件原始行内容
		/// </summary>
		std::vector<std::string> lines_;

		/// <summary>
		/// 格式化时的层级计数器
		/// </summary>
		int tier_ = 0;

		/// <summary>
		/// 内部方法：检查键是否存在
		/// </summary>
		bool IffindNodeByKey(const std::string& key);

		/// <summary>
		/// 根据缩进级别查找最近的父节点
		/// </summary>
		Node* FindParentByIndentation(int indent);

		/// <summary>
		/// 递归收集指定节点的所有子节点
		/// </summary>
		void FindAllChildren(Node* parent, std::vector<Node*>& nodesToRemove);

		/// <summary>
		/// 格式化节点列表，按层级关系重新排列
		/// </summary>
		void formatting();

		/// <summary>
		/// 递归查找并添加子节点到列表
		/// </summary>
		void findChildren(Node* node, std::vector<Node*>& fNodeList);

		/// <summary>
		/// 计算字符串开头的空格数量
		/// </summary>
		int findPreSpace(const std::string& str);

		/// <summary>
		/// 根据缩进级别查找父节点
		/// </summary>
		Node* findParent(int space);

		/// <summary>
		/// 释放所有节点内存
		/// </summary>
		void clearNodes();

		/// <summary>
		/// 按字符分隔符拆分字符串
		/// </summary>
		static std::vector<std::string> splitByChars(const std::string& str, const std::string& delimiters);

		/// <summary>
		/// 按子字符串分隔符拆分字符串
		/// </summary>
		static std::vector<std::string> splitByString(const std::string& str, const std::string& delimiter);

		/// <summary>
		/// 移除数组中的空字符串元素
		/// </summary>
		static std::vector<std::string> removeEmpty(const std::vector<std::string>& vec);

		/// <summary>
		/// 去除数组中每个字符串的首尾空白
		/// </summary>
		static std::vector<std::string> trimAll(const std::vector<std::string>& vec);

		/// <summary>
		/// 去除字符串首尾空白
		/// </summary>
		static std::string trim(const std::string& str);

		/// <summary>
		/// 将字符串数组的前count个元素用点号连接
		/// </summary>
		static std::string joinDot(const std::vector<std::string>& v, int start, int end);
	};

} // namespace Yaml
} // namespace IO
} // namespace HawtC3

#endif // YAML_H
