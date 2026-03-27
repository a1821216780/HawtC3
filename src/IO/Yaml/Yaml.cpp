//**********************************************************************************************************************************
//LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,赵子祯
//
//    This file is part of Qahse.IO.Yaml
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

#include "Yaml.h"
#include "../Log/LogHelper.h"
#include "../IO/CheckError.h"
#include <iostream>
#include <regex>
#include <ctime>
#include <iomanip>
#include <chrono>

using namespace Qahse::IO::Log;
using namespace std;

namespace Qahse {
namespace IO {
namespace Yaml {

// ============================================================================
// Node 实现
// ============================================================================

/// <summary>
/// 创建当前节点的深拷贝，递归复制父节点链
/// </summary>
Node* Node::Clone() const {
	Node* node = new Node();
	node->name = name;
	node->value = value;
	node->hasValue = hasValue;
	if (parent != nullptr) {
		node->parent = parent->Clone();
	}
	node->space = space;
	node->tier = tier;
	return node;
}

// ============================================================================
// YML 辅助工具函数
// ============================================================================

/// <summary>
/// 按字符分隔符拆分字符串（等价于C# string.Split(char...)）
/// </summary>
std::vector<std::string> YML::splitByChars(const std::string& str, const std::string& delimiters) {
	std::vector<std::string> tokens;
	size_t start = 0;
	for (size_t i = 0; i <= str.size(); ++i) {
		if (i == str.size() || delimiters.find(str[i]) != std::string::npos) {
			tokens.push_back(str.substr(start, i - start));
			start = i + 1;
		}
	}
	return tokens;
}

/// <summary>
/// 按子字符串分隔符拆分字符串（等价于C# string.Split(string)）
/// </summary>
std::vector<std::string> YML::splitByString(const std::string& str, const std::string& delimiter) {
	std::vector<std::string> tokens;
	size_t start = 0;
	size_t pos = 0;
	while ((pos = str.find(delimiter, start)) != std::string::npos) {
		tokens.push_back(str.substr(start, pos - start));
		start = pos + delimiter.size();
	}
	tokens.push_back(str.substr(start));
	return tokens;
}

/// <summary>
/// 移除数组中的空字符串元素（等价于C# RemoveNull()）
/// </summary>
std::vector<std::string> YML::removeEmpty(const std::vector<std::string>& vec) {
	std::vector<std::string> result;
	for (const auto& s : vec) {
		if (!s.empty()) {
			result.push_back(s);
		}
	}
	return result;
}

/// <summary>
/// 去除字符串首尾空白
/// </summary>
std::string YML::trim(const std::string& str) {
	size_t start = str.find_first_not_of(" \t\n\r\f\v");
	if (start == std::string::npos) return "";
	size_t end = str.find_last_not_of(" \t\n\r\f\v");
	return str.substr(start, end - start + 1);
}

/// <summary>
/// 去除数组中每个字符串的首尾空白（等价于C# TrimNull()）
/// </summary>
std::vector<std::string> YML::trimAll(const std::vector<std::string>& vec) {
	std::vector<std::string> result;
	result.reserve(vec.size());
	for (const auto& s : vec) {
		result.push_back(trim(s));
	}
	return result;
}

/// <summary>
/// 将字符串数组中[start, end)范围的元素用点号连接
/// </summary>
std::string YML::joinDot(const std::vector<std::string>& v, int start, int end) {
	std::string result;
	for (int i = start; i < end; i++) {
		if (i > start) result += ".";
		result += v[i];
	}
	return result;
}


// ============================================================================
// YML 构造函数 / 析构函数
// ============================================================================

/// <summary>
/// 默认构造函数，创建空的YAML对象
/// </summary>
YML::YML() {}

/// <summary>
/// 构造函数，加载并解析指定路径的YAML文件
/// </summary>
/// <param name="path">YAML文件路径，自动解析节点层级</param>
YML::YML(const std::string& path) : path_(path) {
	// 读取文件所有行
	if (std::filesystem::exists(path)) {
		std::ifstream file(path);
		std::string line;
		while (std::getline(file, line)) {
			lines_.push_back(line);
		}
		file.close();
	}
	else {
		lines_.push_back("# Qahse.IO.Yaml by 赵子祯@zzz，will create file!");
	}

	// 逐行解析，建立节点层级关系
	for (size_t i = 0; i < lines_.size(); i++) {
		std::string line = lines_[i];
		std::string trimmedLine = trim(line);

		// 跳过空行
		if (trimmedLine.empty()) {
			continue;
		}
		// 跳过注释行
		if (trimmedLine[0] == '#') {
			continue;
		}
		// 必须包含冒号才是有效节点
		if (line.find(':') == std::string::npos) {
			continue;
		}

		// 按第一个冒号分割键值
		int spaceCount = findPreSpace(line);

		Node* node = new Node();
		node->space = spaceCount;

		// 提取键名（冒号前的部分，去除空白）
		size_t firstColon = trimmedLine.find(':');
		node->name = trim(trimmedLine.substr(0, firstColon));

		// 提取值
		if (firstColon == trimmedLine.size() - 1) {
			// 行以冒号结尾，可能是父节点或多行值
			node->hasValue = false;
			node->value = "";

			// 检查后续行是否有"-  "开头的多行值
			size_t j = i;
			while (j < lines_.size()) {
				j++;
				if (j == lines_.size()) {
					break;
				}
				std::string nextLine = lines_[j];
				size_t idc = nextLine.find("-  ");
				if (idc == std::string::npos) {
					break;
				}
				int num1 = static_cast<int>(nextLine.substr(0, idc).size());
				if (num1 == node->space + 1 || num1 == node->space + 2 || num1 == node->space + 3) {
					node->value = node->value + '\n' + lines_[j];
					node->hasValue = true;
				}
				else {
					break;
				}
			}
		}
		else {
			// 冒号后有值
			node->value = trimmedLine.substr(firstColon + 2);
			node->hasValue = true;
		}

		// 查找父节点
		node->parent = findParent(node->space);
		nodeList.push_back(node);
	}

	// 版本检查
	if (ChickfindNodeByKey("OpenWECD.Information.YMLVersion")) {
		std::string versionStr = read("OpenWECD.Information.YMLVersion");
		// 简化版本检查：仅记录日志
		auto splitVer = splitByChars(versionStr, ".");
		auto splitCur = splitByChars(std::string(ymlversion), ".");
		splitVer = removeEmpty(splitVer);
		splitCur = removeEmpty(splitCur);
		if (splitVer.size() >= 3 && splitCur.size() >= 3) {
			if (splitVer[0] != splitCur[0] || splitVer[1] != splitCur[1] || splitVer[2] != splitCur[2]) {
				LogHelper::ErrorLog("Cant support current YMLVersion " + versionStr +
					" file,Current program support " + std::string(ymlversion));
			}
		}
	}
	else {
		AddNode("OpenWECD.Information.YMLVersion", std::string(ymlversion));
	}

	// 添加作者和时间信息
	AddNode("OpenWECD.Information.Auther", "YML 模块由赵子祯独立开发 @copyright");

	// 获取当前时间字符串
	auto now = std::chrono::system_clock::now();
	auto time_t_now = std::chrono::system_clock::to_time_t(now);
	std::tm tm_now;
#ifdef _WIN32
	localtime_s(&tm_now, &time_t_now);
#else
	localtime_r(&time_t_now, &tm_now);
#endif
	std::ostringstream oss;
	oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
	AddNode("OpenWECD.Information.最后修改时间", oss.str());

	formatting();
}

/// <summary>
/// 析构函数，释放所有节点的堆内存
/// </summary>
YML::~YML() {
	clearNodes();
}

/// <summary>
/// 移动构造函数
/// </summary>
YML::YML(YML&& other) noexcept
	: path_(std::move(other.path_)),
	  lines_(std::move(other.lines_)),
	  nodeList(std::move(other.nodeList)),
	  tier_(other.tier_) {
	other.tier_ = 0;
}

/// <summary>
/// 移动赋值运算符
/// </summary>
YML& YML::operator=(YML&& other) noexcept {
	if (this != &other) {
		clearNodes();
		path_ = std::move(other.path_);
		lines_ = std::move(other.lines_);
		nodeList = std::move(other.nodeList);
		tier_ = other.tier_;
		other.tier_ = 0;
	}
	return *this;
}

/// <summary>
/// 释放所有节点内存
/// </summary>
void YML::clearNodes() {
	for (auto* node : nodeList) {
		delete node;
	}
	nodeList.clear();
}


// ============================================================================
// YML 核心操作方法
// ============================================================================

/// <summary>
/// 创建当前YAML对象的深拷贝
/// </summary>
YML YML::Clone() const {
	YML pp;
	pp.tier_ = tier_;
	pp.lines_ = lines_;
	pp.path_ = path_;
	for (size_t i = 0; i < nodeList.size(); i++) {
		pp.nodeList.push_back(nodeList[i]->Clone());
	}
	return pp;
}

/// <summary>
/// 读取指定键的值
/// </summary>
/// <param name="key">点分隔的层级键名</param>
/// <returns>键对应的值，不存在则返回空字符串</returns>
std::string YML::read(const std::string& key) {
	Node* node = findNodeByKey(key);
	if (node != nullptr) {
		return node->value;
	}
	return "";
}

/// <summary>
/// 修改指定键的值
/// </summary>
/// <param name="key">点分隔的层级键名</param>
/// <param name="value">要设置的新值</param>
void YML::modify(const std::string& key, const std::string& value) {
	Node* node = findNodeByKey(key);
	if (node != nullptr) {
		node->value = value;
		node->hasValue = true;
	}
}


// ============================================================================
// 节点查找方法
// ============================================================================

/// <summary>
/// 根据点分隔键名查找节点
/// </summary>
/// <param name="key">点分隔的层级键名，如"database.connection.host"</param>
/// <returns>匹配的节点指针，不存在则返回nullptr</returns>
Node* YML::findNodeByKey(const std::string& key) {
	auto ks = splitByChars(key, ".");
	ks = removeEmpty(ks);
	if (ks.empty()) return nullptr;

	for (size_t i = 0; i < nodeList.size(); i++) {
		Node* node = nodeList[i];
		if (node->name == ks[ks.size() - 1]) {
			// 验证父级链
			Node* tem = node;
			int count = 1;
			for (int j = static_cast<int>(ks.size()) - 2; j >= 0; j--) {
				if (tem->parent != nullptr && tem->parent->name == ks[j]) {
					count++;
					tem = tem->parent;
				}
			}
			if (count == static_cast<int>(ks.size())) {
				return node;
			}
		}
	}
	return nullptr;
}

/// <summary>
/// 检查指定键名的节点是否存在
/// </summary>
/// <param name="key">点分隔的层级键名</param>
/// <returns>存在返回true，否则返回false</returns>
bool YML::ChickfindNodeByKey(const std::string& key) {
	return findNodeByKey(key) != nullptr;
}

/// <summary>
/// 内部方法：检查键是否存在
/// </summary>
bool YML::IffindNodeByKey(const std::string& key) {
	return findNodeByKey(key) != nullptr;
}

/// <summary>
/// 生成节点的完整键路径（递归向上遍历父节点）
/// </summary>
std::string YML::GetNodeKey(Node* node) {
	if (node == nullptr) return "";
	if (node->parent == nullptr) {
		return node->name;
	}
	else {
		return GetNodeKey(node->parent) + "." + node->name;
	}
}


// ============================================================================
// 节点增删操作
// ============================================================================

/// <summary>
/// 将另一个YAML对象的所有节点合并到当前对象
/// </summary>
void YML::AddYAML(const YML& yaml) {
	for (auto* node : yaml.nodeList) {
		std::string key = const_cast<YML&>(yaml).GetNodeKey(node);
		if (IffindNodeByKey(key)) {
			modify(key, node->value);
		}
		else {
			std::string parentKey;
			if (node->parent != nullptr) {
				parentKey = const_cast<YML&>(yaml).GetNodeKey(node->parent);
			}
			AddNode(parentKey, node->name, node->value);
		}
	}
}

/// <summary>
/// 添加或更新节点，自动创建中间层级节点
/// </summary>
/// <param name="key">点分隔的完整键名</param>
/// <param name="value">节点的值</param>
void YML::AddNode(const std::string& key, const std::string& value) {
	// 如果键已存在，更新值
	if (ChickfindNodeByKey(key)) {
		modify(key, value);
		return;
	}

	// 按点号拆分键名
	auto keys = removeEmpty(trimAll(splitByChars(key, ".")));
	if (keys.size() == 1) {
		// 只有一个节点，添加为根节点
		AddNode("", keys[0], value);
	}
	else {
		// 确保中间层级节点存在
		for (size_t i = 1; i < keys.size(); i++) {
			std::string parkey = joinDot(keys, 0, static_cast<int>(i));
			Node* node1 = findNodeByKey(parkey);
			if (node1 == nullptr) {
				int ii = static_cast<int>(i) - 1;
				std::string parentPath = (ii > 0) ? joinDot(keys, 0, ii) : "";
				AddNode(parentPath, keys[i - 1], "");
			}
		}
		// 添加最终节点
		int lastIdx = static_cast<int>(keys.size()) - 1;
		std::string parentPath = joinDot(keys, 0, lastIdx);
		AddNode(parentPath, keys[lastIdx], value);
	}
}

/// <summary>
/// 在指定父节点下添加子节点
/// </summary>
/// <param name="parentKey">父节点键名，空字符串表示添加为根节点</param>
/// <param name="nodeName">新节点名称</param>
/// <param name="value">新节点的值</param>
void YML::AddNode(const std::string& parentKey, const std::string& nodeName, const std::string& value) {
	if (parentKey.empty()) {
		// 添加为根节点
		Node* newNode = new Node();
		newNode->name = nodeName;
		newNode->value = value;
		newNode->hasValue = !value.empty();
		newNode->space = 0;
		newNode->parent = nullptr;
		newNode->tier = 0;
		nodeList.push_back(newNode);
	}
	else {
		Node* parentNode = findNodeByKey(parentKey);
		if (parentNode != nullptr) {
			Node* newNode = new Node();
			newNode->name = nodeName;
			newNode->value = value;
			newNode->hasValue = !value.empty();
			newNode->space = parentNode->space + 2;
			newNode->parent = parentNode;
			newNode->tier = parentNode->tier + 1;
			nodeList.push_back(newNode);
		}
		else {
			LogHelper::ErrorLog("未找到父节点: " + parentKey);
		}
	}
}

/// <summary>
/// 删除指定键的节点及其所有子节点
/// </summary>
void YML::DeleteNode(const std::string& key) {
	Node* nodeToDelete = findNodeByKey(key);
	if (nodeToDelete != nullptr) {
		std::vector<Node*> nodesToRemove;
		nodesToRemove.push_back(nodeToDelete);
		FindAllChildren(nodeToDelete, nodesToRemove);

		// 从节点列表中移除并释放内存
		bool removed = false;
		for (auto* nodeToRm : nodesToRemove) {
			auto it = std::find(nodeList.begin(), nodeList.end(), nodeToRm);
			if (it != nodeList.end()) {
				nodeList.erase(it);
				delete nodeToRm;
				removed = true;
			}
		}
		if (removed) {
			formatting();
		}
	}
}

/// <summary>
/// 递归收集指定节点的所有子节点
/// </summary>
void YML::FindAllChildren(Node* parent, std::vector<Node*>& nodesToRemove) {
	for (auto* node : nodeList) {
		if (node->parent == parent) {
			nodesToRemove.push_back(node);
			FindAllChildren(node, nodesToRemove);
		}
	}
}

/// <summary>
/// 查找指定键节点的所有直接子节点
/// </summary>
std::vector<Node*> YML::FindChildren(const std::string& key) {
	auto parentNode = findNodeByKey(key);
	if (parentNode == nullptr) {
		LogHelper::ErrorLog("错误使用 YAML.FindChildren 因为没有匹配的节点");
		return {};
	}

	auto keyParts = removeEmpty(splitByChars(key, "."));
	std::string lastName = keyParts.back();

	std::vector<Node*> nodes;
	for (auto* node : nodeList) {
		if (node->parent != nullptr && node->parent->name == lastName) {
			std::string newkey = key + "." + node->name;
			if (findNodeByKey(newkey) != nullptr) {
				nodes.push_back(findNodeByKey(newkey));
			}
		}
	}
	return nodes;
}


// ============================================================================
// 文件保存与格式化
// ============================================================================

/// <summary>
/// 保存YAML数据到文件
/// </summary>
/// <param name="savepath">保存路径，为空则使用构造函数的路径</param>
/// <param name="format">是否在保存前格式化节点顺序</param>
void YML::save(const std::string& savepath, bool format) {
	if (path_.empty() && savepath.empty()) {
		LogHelper::ErrorLog("Error! when use save, because savepath and path is null");
		return;
	}

	std::string save = path_;
	if (!savepath.empty()) {
		save = savepath;
	}

	// 检查并创建目录
	std::filesystem::path dirPath = std::filesystem::path(save).parent_path();
	if (!dirPath.empty() && !std::filesystem::exists(dirPath)) {
		std::filesystem::create_directories(dirPath);
	}

	if (format) {
		formatting();
	}

	std::ofstream stream(save);
	for (size_t i = 0; i < nodeList.size(); i++) {
		Node* node = nodeList[i];
		std::ostringstream sb;
		// 添加缩进空格
		for (int j = 0; j < node->tier; j++) {
			sb << "  ";
		}
		sb << node->name << ": ";
		if (node->hasValue) {
			sb << node->value;
		}
		stream << sb.str() << "\n";
	}
	stream.flush();
	stream.close();
}

/// <summary>
/// 格式化节点列表，按层级关系重新排列节点顺序
/// </summary>
void YML::formatting() {
	// 找出根节点
	std::vector<Node*> parentNodes;
	for (auto* node : nodeList) {
		if (node->parent == nullptr) {
			parentNodes.push_back(node);
		}
	}

	std::vector<Node*> fNodeList;
	// 遍历根节点并递归添加子节点
	for (auto* node : parentNodes) {
		fNodeList.push_back(node);
		findChildren(node, fNodeList);
	}

	// 替换为格式化后的列表
	nodeList = fNodeList;
}

/// <summary>
/// 递归查找并添加子节点到格式化列表
/// </summary>
void YML::findChildren(Node* node, std::vector<Node*>& fNodeList) {
	tier_++;
	for (size_t i = 0; i < nodeList.size(); i++) {
		Node* item = nodeList[i];
		if (item->parent == node) {
			item->tier = tier_;
			fNodeList.push_back(item);
			findChildren(item, fNodeList);
		}
	}
	tier_--;
}

/// <summary>
/// 计算字符串开头的连续空格数量
/// </summary>
int YML::findPreSpace(const std::string& str) {
	int count = 0;
	for (char c : str) {
		if (c == ' ') {
			count++;
		}
		else {
			break;
		}
	}
	return count;
}

/// <summary>
/// 根据缩进级别从后往前查找父节点
/// </summary>
Node* YML::findParent(int space) {
	if (nodeList.empty()) {
		return nullptr;
	}
	// 从后往前查找缩进小于当前节点的最近节点作为父节点
	for (int i = static_cast<int>(nodeList.size()) - 1; i >= 0; i--) {
		Node* node = nodeList[i];
		if (node->space < space) {
			return node;
		}
	}
	return nullptr;
}

/// <summary>
/// 根据缩进级别查找最近的父节点
/// </summary>
Node* YML::FindParentByIndentation(int indent) {
	for (int i = static_cast<int>(nodeList.size()) - 1; i >= 0; i--) {
		if (nodeList[i]->space < indent) {
			return nodeList[i];
		}
	}
	return nullptr;
}


// ============================================================================
// ToYmlValueString 系列方法 - 类型转换为YAML字符串
// ============================================================================

/// <summary>
/// 将字符串转换为YAML值字符串（去除首尾空白）
/// </summary>
std::string YML::ToYmlValueString(const std::string& str) {
	return trim(str);
}

/// <summary>
/// 将字符串数组转换为YAML列表格式: "[ item1 , item2 , item3 ]"
/// </summary>
std::string YML::ToYmlValueString(const std::vector<std::string>& str) {
	std::ostringstream oss;
	oss << "[ ";
	for (size_t i = 0; i < str.size(); i++) {
		oss << str[i];
		if (i != str.size() - 1) {
			oss << " , ";
		}
	}
	oss << " ]";
	return oss.str();
}

/// <summary>
/// 将整数转换为YAML字符串
/// </summary>
std::string YML::ToYmlValueString(int value) {
	return std::to_string(value);
}

/// <summary>
/// 将整数数组转换为YAML列表格式: "[ 1 , 2 , 3 ]"
/// </summary>
std::string YML::ToYmlValueString(const std::vector<int>& values) {
	std::ostringstream oss;
	oss << "[ ";
	for (size_t i = 0; i < values.size(); i++) {
		oss << values[i];
		if (i != values.size() - 1) {
			oss << " , ";
		}
	}
	oss << " ]";
	return oss.str();
}

/// <summary>
/// 将双精度浮点数转换为YAML字符串
/// </summary>
std::string YML::ToYmlValueString(double value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

/// <summary>
/// 将双精度浮点数数组转换为YAML列表格式: "[ 1.5 , 2.7 , 3.14 ]"
/// </summary>
std::string YML::ToYmlValueString(const std::vector<double>& values) {
	std::ostringstream oss;
	oss << "[ ";
	for (size_t i = 0; i < values.size(); i++) {
		oss << values[i];
		if (i != values.size() - 1) {
			oss << " , ";
		}
	}
	oss << " ]";
	return oss.str();
}

/// <summary>
/// 将布尔值转换为YAML字符串 ("True" 或 "False")
/// </summary>
std::string YML::ToYmlValueString(bool value) {
	return value ? "True" : "False";
}

/// <summary>
/// 将布尔值数组转换为YAML列表格式: "[ True , False , True ]"
/// </summary>
std::string YML::ToYmlValueString(const std::vector<bool>& values) {
	std::ostringstream oss;
	oss << "[ ";
	for (size_t i = 0; i < values.size(); i++) {
		oss << (values[i] ? "True" : "False");
		if (i != values.size() - 1) {
			oss << " , ";
		}
	}
	oss << " ]";
	return oss.str();
}

/// <summary>
/// 将Eigen矩阵转换为YAML多行格式字符串
/// 每行格式为: "-  [ v1 , v2 , v3 ]"
/// </summary>
/// <param name="data">要转换的Eigen矩阵</param>
/// <param name="level">缩进级别，每级增加两个空格</param>
std::string YML::ToYmlValueString(const Eigen::MatrixXd& data, int level) {
	std::ostringstream sb;
	sb << "\n";
	std::string levr = " ";
	for (int i = 0; i < level; i++) {
		levr += "  ";
	}
	for (int i = 0; i < data.rows(); i++) {
		sb << levr << "-  [ ";
		for (int j = 0; j < data.cols(); j++) {
			sb << data(i, j);
			if (j != data.cols() - 1) {
				sb << " , ";
			}
		}
		sb << " ]";
		if (i != data.rows() - 1) {
			sb << "\n";
		}
	}
	return sb.str();
}

/// <summary>
/// 将Eigen向量转换为YAML列表格式字符串: "[ v1 , v2 , v3 ]"
/// </summary>
/// <param name="data">要转换的Eigen向量</param>
/// <param name="level">缩进级别（默认0）</param>
std::string YML::ToYmlValueString(const Eigen::VectorXd& data, int level) {
	std::vector<double> vec(data.data(), data.data() + data.size());
	return ToYmlValueString(vec);
}

} // namespace Yaml
} // namespace IO
} // namespace Qahse
