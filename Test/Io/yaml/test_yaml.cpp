// test_yaml.cpp - YML 类的单元测试
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "../../../src/IO/Yaml/Yaml.h"

using namespace HawtC3::IO::Yaml;
namespace fs = std::filesystem;

// ============================================================================
// 辅助工具
// ============================================================================

/// 创建临时 YAML 文件并返回路径，测试结束后自动删除
class TempYamlFile {
public:
	explicit TempYamlFile(const std::string& content, const std::string& name = "test.yml") {
		path_ = (fs::temp_directory_path() / ("hawtc3_test_" + name)).string();
		std::ofstream f(path_);
		f << content;
		f.close();
	}
	~TempYamlFile() {
		if (fs::exists(path_)) fs::remove(path_);
	}
	const std::string& path() const { return path_; }
private:
	std::string path_;
};


// ============================================================================
// Node 测试
// ============================================================================

TEST(NodeTest, Clone) {
	Node parent;
	parent.name = "root";
	parent.value = "";
	parent.space = 0;
	parent.tier = 0;

	Node child;
	child.name = "child";
	child.value = "val";
	child.hasValue = true;
	child.space = 2;
	child.tier = 1;
	child.parent = &parent;

	Node* cloned = child.Clone();
	EXPECT_EQ(cloned->name, "child");
	EXPECT_EQ(cloned->value, "val");
	EXPECT_TRUE(cloned->hasValue);
	EXPECT_EQ(cloned->space, 2);
	EXPECT_EQ(cloned->tier, 1);
	ASSERT_NE(cloned->parent, nullptr);
	EXPECT_EQ(cloned->parent->name, "root");
	// Clone 生成的是独立副本
	EXPECT_NE(cloned->parent, &parent);

	delete cloned->parent;
	delete cloned;
}


// ============================================================================
// YML 默认构造
// ============================================================================

TEST(YMLTest, DefaultConstructor) {
	YML yml;
	EXPECT_TRUE(yml.nodeList.empty());
	EXPECT_TRUE(yml.getPath().empty());
}


// ============================================================================
// AddNode + read
// ============================================================================

TEST(YMLTest, AddNodeAndRead) {
	YML yml;
	yml.AddNode("server.host", "localhost");
	yml.AddNode("server.port", "8080");

	EXPECT_EQ(yml.read("server.host"), "localhost");
	EXPECT_EQ(yml.read("server.port"), "8080");
}

TEST(YMLTest, AddNodeDeep) {
	YML yml;
	yml.AddNode("a.b.c.d", "deep_value");
	EXPECT_EQ(yml.read("a.b.c.d"), "deep_value");
	// 中间节点应自动创建
	EXPECT_TRUE(yml.ChickfindNodeByKey("a"));
	EXPECT_TRUE(yml.ChickfindNodeByKey("a.b"));
	EXPECT_TRUE(yml.ChickfindNodeByKey("a.b.c"));
}

TEST(YMLTest, AddNodeUpdateExisting) {
	YML yml;
	yml.AddNode("key", "v1");
	EXPECT_EQ(yml.read("key"), "v1");
	yml.AddNode("key", "v2");
	EXPECT_EQ(yml.read("key"), "v2");
}

TEST(YMLTest, AddNodeWithParent) {
	YML yml;
	yml.AddNode("parent", "");
	yml.AddNode("parent", "child", "hello");
	EXPECT_EQ(yml.read("parent.child"), "hello");
}

TEST(YMLTest, AddNodeRootLevel) {
	YML yml;
	yml.AddNode("", "rootkey", "rootval");
	EXPECT_EQ(yml.read("rootkey"), "rootval");
}


// ============================================================================
// modify
// ============================================================================

TEST(YMLTest, Modify) {
	YML yml;
	yml.AddNode("config.name", "old");
	yml.modify("config.name", "new");
	EXPECT_EQ(yml.read("config.name"), "new");
}

TEST(YMLTest, ModifyNonExisting) {
	YML yml;
	// 修改不存在的键不会崩溃
	yml.modify("no.such.key", "value");
	EXPECT_EQ(yml.read("no.such.key"), "");
}


// ============================================================================
// findNodeByKey / ChickfindNodeByKey
// ============================================================================

TEST(YMLTest, FindNodeByKey) {
	YML yml;
	yml.AddNode("database.host", "127.0.0.1");
	Node* node = yml.findNodeByKey("database.host");
	ASSERT_NE(node, nullptr);
	EXPECT_EQ(node->value, "127.0.0.1");
	EXPECT_EQ(node->name, "host");
}

TEST(YMLTest, FindNodeByKeyNotFound) {
	YML yml;
	EXPECT_EQ(yml.findNodeByKey("nonexistent"), nullptr);
}

TEST(YMLTest, ChickfindNodeByKey) {
	YML yml;
	yml.AddNode("x.y", "z");
	EXPECT_TRUE(yml.ChickfindNodeByKey("x.y"));
	EXPECT_FALSE(yml.ChickfindNodeByKey("x.z"));
}


// ============================================================================
// GetNodeKey
// ============================================================================

TEST(YMLTest, GetNodeKey) {
	YML yml;
	yml.AddNode("level1.level2.level3", "v");
	Node* node = yml.findNodeByKey("level1.level2.level3");
	ASSERT_NE(node, nullptr);
	EXPECT_EQ(yml.GetNodeKey(node), "level1.level2.level3");
}


// ============================================================================
// DeleteNode
// ============================================================================

TEST(YMLTest, DeleteNode) {
	YML yml;
	yml.AddNode("a.b", "1");
	yml.AddNode("a.c", "2");
	EXPECT_TRUE(yml.ChickfindNodeByKey("a.b"));

	yml.DeleteNode("a.b");
	EXPECT_FALSE(yml.ChickfindNodeByKey("a.b"));
	// 兄弟节点不受影响
	EXPECT_TRUE(yml.ChickfindNodeByKey("a.c"));
}

TEST(YMLTest, DeleteNodeWithChildren) {
	YML yml;
	yml.AddNode("root.parent.child1", "v1");
	yml.AddNode("root.parent.child2", "v2");
	yml.DeleteNode("root.parent");
	EXPECT_FALSE(yml.ChickfindNodeByKey("root.parent"));
	EXPECT_FALSE(yml.ChickfindNodeByKey("root.parent.child1"));
	EXPECT_FALSE(yml.ChickfindNodeByKey("root.parent.child2"));
}


// ============================================================================
// FindChildren
// ============================================================================

TEST(YMLTest, FindChildren) {
	YML yml;
	yml.AddNode("menu.file", "");
	yml.AddNode("menu.edit", "");
	yml.AddNode("menu.view", "");
	auto children = yml.FindChildren("menu");
	EXPECT_EQ(children.size(), 3);
}


// ============================================================================
// Clone
// ============================================================================

TEST(YMLTest, Clone) {
	YML yml;
	yml.AddNode("a.b", "v1");
	yml.AddNode("a.c", "v2");

	YML cloned = yml.Clone();
	EXPECT_EQ(cloned.read("a.b"), "v1");
	EXPECT_EQ(cloned.read("a.c"), "v2");

	// 修改克隆不影响原对象
	cloned.modify("a.b", "changed");
	EXPECT_EQ(yml.read("a.b"), "v1");
	EXPECT_EQ(cloned.read("a.b"), "changed");
}


// ============================================================================
// Move 语义
// ============================================================================

TEST(YMLTest, MoveConstructor) {
	YML yml;
	yml.AddNode("key", "value");

	YML moved(std::move(yml));
	EXPECT_EQ(moved.read("key"), "value");
	// 原对象应为空
	EXPECT_TRUE(yml.nodeList.empty());
}

TEST(YMLTest, MoveAssignment) {
	YML yml;
	yml.AddNode("key", "value");

	YML other;
	other = std::move(yml);
	EXPECT_EQ(other.read("key"), "value");
	EXPECT_TRUE(yml.nodeList.empty());
}


// ============================================================================
// AddYAML 合并
// ============================================================================

TEST(YMLTest, AddYAML) {
	YML base;
	base.AddNode("config.host", "a.com");
	base.AddNode("config.port", "80");

	YML overlay;
	overlay.AddNode("config.port", "443");
	overlay.AddNode("config.ssl", "true");

	base.AddYAML(overlay);
	EXPECT_EQ(base.read("config.host"), "a.com");
	EXPECT_EQ(base.read("config.port"), "443");    // 被覆盖
	EXPECT_EQ(base.read("config.ssl"), "true");     // 新增
}


// ============================================================================
// 文件保存与加载
// ============================================================================

TEST(YMLTest, SaveAndLoad) {
	std::string tmpPath = (fs::temp_directory_path() / "hawtc3_test_save.yml").string();

	// 保存
	{
		YML yml;
		yml.AddNode("database.host", "localhost");
		yml.AddNode("database.port", "5432");
		yml.save(tmpPath);
	}
	EXPECT_TRUE(fs::exists(tmpPath));

	// 重新加载
	{
		YML yml(tmpPath);
		EXPECT_EQ(yml.read("database.host"), "localhost");
		EXPECT_EQ(yml.read("database.port"), "5432");
	}

	fs::remove(tmpPath);
}

TEST(YMLTest, LoadFromFile) {
	std::string content =
		"server:\n"
		"  host: example.com\n"
		"  port: 9090\n"
		"  debug: true\n";
	TempYamlFile tmp(content);

	YML yml(tmp.path());
	EXPECT_EQ(yml.read("server.host"), "example.com");
	EXPECT_EQ(yml.read("server.port"), "9090");
	EXPECT_EQ(yml.read("server.debug"), "true");
}

TEST(YMLTest, LoadSkipsComments) {
	std::string content =
		"# This is a comment\n"
		"key: value\n"
		"# Another comment\n"
		"key2: value2\n";
	TempYamlFile tmp(content);

	YML yml(tmp.path());
	EXPECT_EQ(yml.read("key"), "value");
	EXPECT_EQ(yml.read("key2"), "value2");
}

TEST(YMLTest, LoadSkipsBlankLines) {
	std::string content =
		"a: 1\n"
		"\n"
		"b: 2\n"
		"\n\n"
		"c: 3\n";
	TempYamlFile tmp(content);

	YML yml(tmp.path());
	EXPECT_EQ(yml.read("a"), "1");
	EXPECT_EQ(yml.read("b"), "2");
	EXPECT_EQ(yml.read("c"), "3");
}

TEST(YMLTest, LoadNonExistentFileDoesNotCrash) {
	// 打开不存在的文件不应崩溃
	YML yml("nonexistent_path_12345.yml");
	EXPECT_TRUE(yml.ChickfindNodeByKey("OpenWECD.Information.YMLVersion"));
}


// ============================================================================
// ToYmlValueString 系列
// ============================================================================

TEST(YMLTest, ToYmlValueString_String) {
	EXPECT_EQ(YML::ToYmlValueString(std::string("  hello  ")), "hello");
	EXPECT_EQ(YML::ToYmlValueString(std::string("")), "");
}

TEST(YMLTest, ToYmlValueString_StringArray) {
	std::vector<std::string> arr = { "a", "b", "c" };
	std::string result = YML::ToYmlValueString(arr);
	EXPECT_EQ(result, "[ a , b , c ]");
}

TEST(YMLTest, ToYmlValueString_Int) {
	EXPECT_EQ(YML::ToYmlValueString(42), "42");
	EXPECT_EQ(YML::ToYmlValueString(-1), "-1");
	EXPECT_EQ(YML::ToYmlValueString(0), "0");
}

TEST(YMLTest, ToYmlValueString_IntArray) {
	std::vector<int> arr = { 1, 2, 3 };
	EXPECT_EQ(YML::ToYmlValueString(arr), "[ 1 , 2 , 3 ]");
}

TEST(YMLTest, ToYmlValueString_Double) {
	std::string s = YML::ToYmlValueString(3.14);
	EXPECT_FALSE(s.empty());
	EXPECT_NEAR(std::stod(s), 3.14, 1e-10);
}

TEST(YMLTest, ToYmlValueString_DoubleArray) {
	std::vector<double> arr = { 1.1, 2.2 };
	std::string s = YML::ToYmlValueString(arr);
	EXPECT_NE(s.find("1.1"), std::string::npos);
	EXPECT_NE(s.find("2.2"), std::string::npos);
}

TEST(YMLTest, ToYmlValueString_Bool) {
	EXPECT_EQ(YML::ToYmlValueString(true), "True");
	EXPECT_EQ(YML::ToYmlValueString(false), "False");
}

TEST(YMLTest, ToYmlValueString_BoolArray) {
	std::vector<bool> arr = { true, false, true };
	std::string result = YML::ToYmlValueString(arr);
	EXPECT_NE(result.find("True"), std::string::npos);
	EXPECT_NE(result.find("False"), std::string::npos);
}

TEST(YMLTest, ToYmlValueString_EigenVector) {
	Eigen::VectorXd v(3);
	v << 1.0, 2.0, 3.0;
	std::string s = YML::ToYmlValueString(v);
	EXPECT_NE(s.find("1"), std::string::npos);
	EXPECT_NE(s.find("2"), std::string::npos);
	EXPECT_NE(s.find("3"), std::string::npos);
}

TEST(YMLTest, ToYmlValueString_EigenMatrix) {
	Eigen::MatrixXd m(2, 2);
	m << 1.0, 2.0,
	     3.0, 4.0;
	std::string s = YML::ToYmlValueString(m, 0);
	EXPECT_FALSE(s.empty());
}


// ============================================================================
// 版本号
// ============================================================================

TEST(YMLTest, VersionConstant) {
	EXPECT_NE(std::string(YML::ymlversion), "");
	// 版本号格式应为 x.y.z
	std::string ver(YML::ymlversion);
	int dotCount = 0;
	for (char c : ver) { if (c == '.') dotCount++; }
	EXPECT_GE(dotCount, 2);
}


// ============================================================================
// 读取不存在的 key
// ============================================================================

TEST(YMLTest, ReadNonExistentKey) {
	YML yml;
	EXPECT_EQ(yml.read("no.such.key"), "");
}
