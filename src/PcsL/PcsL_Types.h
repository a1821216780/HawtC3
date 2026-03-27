#pragma once
//**********************************************************************************************************************************
// LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,赵子祯
//
//    This file is part of Qahse.IO.Math
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

#include <string>
#include <vector>
#include <Eigen/Dense>

namespace Qahse::PcsL
{
    /**
     * @brief 复合材料属性结构体
     * @details 定义正交各向异性复合材料的弹性常数、密度及失效分析强度参数，
     *          适用于叶片截面有限元分析中的材料建模。
     *
     * @par 使用示例
     * @code
     * Qahse::PcsL::CompositeMaterial mat;
     * mat.Id      = 1;
     * mat.Name    = "GFRP_UD";
     * mat.E1      = 40e9;   // 纤维方向弹性模量
     * mat.E2      = 10e9;
     * mat.Nu12    = 0.3;
     * mat.Density = 1800.0;
     * @endcode
     */
    struct CompositeMaterial
    {
        int Id = 0;         ///< 材料编号，与铺层定义中的 MaterialId 对应
        double E1 = 0;      ///< 纤维方向（1方向）弹性模量 [Pa]
        double E2 = 0;      ///< 横向（2方向）弹性模量 [Pa]
        double E3 = 0;      ///< 厚度方向（3方向）弹性模量 [Pa]
        double G12 = 0;     ///< 1-2平面内剪切模量 [Pa]
        double G13 = 0;     ///< 1-3平面内剪切模量 [Pa]
        double G23 = 0;     ///< 2-3平面内剪切模量 [Pa]
        double Nu12 = 0;    ///< 1-2平面泊松比（纤维方向受拉，横向收缩）
        double Nu13 = 0;    ///< 1-3平面泊松比
        double Nu23 = 0;    ///< 2-3平面泊松比
        double Density = 0; ///< 材料密度 [kg/m³]
        std::string Name;   ///< 材料名称，便于识别和输出
        // ========== 失效分析强度参数 ==========
        double S1t = 0;  ///< 纤维方向拉伸强度（1方向，Tensile）[Pa]
        double S2t = 0;  ///< 横向拉伸强度（2方向）[Pa]
        double S3t = 0;  ///< 厚度方向拉伸强度（3方向）[Pa]
        double S1c = 0;  ///< 纤维方向压缩强度（1方向，Compressive）[Pa]
        double S2c = 0;  ///< 横向压缩强度（2方向）[Pa]
        double S3c = 0;  ///< 厚度方向压缩强度（3方向）[Pa]
        double S12s = 0; ///< 1-2平面剪切强度 [Pa]
        double S13s = 0; ///< 1-3平面剪切强度 [Pa]
        double S23s = 0; ///< 2-3平面剪切强度 [Pa]
    };

    /**
     * @brief 单层（Lamina）铺层材料定义
     * @details 描述复合材料层合板中单个铺层的几何与材料信息，包括铺层厚度、纤维方向和所用材料。
     *
     * @par 使用示例
     * @code
     * Qahse::PcsL::Lamina ply;
     * ply.LayerId          = 1;
     * ply.PlyCount         = 3;      // 3层叠加
     * ply.Thickness        = 0.002;  // 单层厚度 2mm
     * ply.FiberOrientation = 0.0;    // 纤维方向 0°
     * ply.MaterialId       = 2;      // 对应 CompositeMaterial.Id=2
     * @endcode
     */
    struct Lamina
    {
        int LayerId = 0;             ///< 铺层编号（层编号，从1起）
        int PlyCount = 0;            ///< 该铺层的重复层数（叠层数）
        double Thickness = 0;        ///< 单层厚度 [m]
        double FiberOrientation = 0; ///< 纤维方向角 [度]，相对于参考方向
        int MaterialId = 0;          ///< 所用材料编号，对应 CompositeMaterial.Id
        std::string MatDisp;         ///< 材料显示名称（可选，用于输出和调试）
    };

    /**
     * @brief 铺层扇区定义
     * @details 将截面表面划分为若干扇区，每个扇区包含一组铺层（Lamina）定义，
     *          用于描述截面不同弦向位置的铺层分布。
     *
     * @par 使用示例
     * @code
     * Qahse::PcsL::SectorLaminae sector;
     * sector.Sect_num  = 1;       // 第1扇区
     * sector.N_laminas = 2;       // 含2层铺层
     * sector.Laminae.push_back(ply1);
     * sector.Laminae.push_back(ply2);
     * @endcode
     */
    struct SectorLaminae
    {
        int Sect_num = 0;            ///< 扇区编号（从1起）
        int N_laminas = 0;           ///< 该扇区的铺层数量
        std::vector<Lamina> Laminae; ///< 该扇区所有铺层的列表
    };

    /**
     * @brief 表面铺层配置（上翼面、下翼面或腹板）
     * @details 描述某一翼面或腹板沿弦向的扇区划分及各扇区铺层配置，
     *          xsec_node 给出各扇区边界的归一化弦向坐标。
     *
     * @par 使用示例
     * @code
     * Qahse::PcsL::SurfaceConfiguration topSurf;
     * topSurf.N_scts = 3;                         // 3个扇区
     * topSurf.xsec_node = {0.0, 0.3, 0.7, 1.0};  // 扇区分界点（归一化弦向坐标）
     * topSurf.Sector.push_back(sector1);
     * @endcode
     */
    struct SurfaceConfiguration
    {
        int N_scts = 0;                    ///< 该表面的扇区数量
        std::vector<double> xsec_node;     ///< 各扇区分界点的归一化弦向坐标（长度为 N_scts+1）
        std::vector<SectorLaminae> Sector; ///< 各扇区的铺层配置列表（长度为 N_scts）
    };

    /**
     * @brief 翼型外形几何数据
     * @details 存储翼型轮廓的离散节点坐标，包括完整轮廓和分开存储的上/下翼面坐标，
     *          坐标均以弦长归一化（X ∈ [0,1]）。
     *
     * @par 使用示例
     * @code
     * Qahse::PcsL::AirfoilShape af;
     * af.N_af_nodes = 100;
     * af.X.resize(100);   // 全轮廓 x 坐标
     * af.Y.resize(100);   // 全轮廓 y 坐标
     * af.Xu.resize(51);   // 上翼面 x 坐标
     * af.Yu.resize(51);   // 上翼面 y 坐标
     * af.Xl.resize(51);   // 下翼面 x 坐标
     * af.Yl.resize(51);   // 下翼面 y 坐标
     * @endcode
     */
    struct AirfoilShape
    {
        int N_af_nodes = 0; ///< 翼型轮廓总节点数
        Eigen::VectorXd X;  ///< 全轮廓节点 x 坐标（归一化弦向，0=前缘，1=后缘）
        Eigen::VectorXd Y;  ///< 全轮廓节点 y 坐标（厚度方向，归一化）
        Eigen::VectorXd Xu; ///< 上翼面节点 x 坐标
        Eigen::VectorXd Yu; ///< 上翼面节点 y 坐标
        Eigen::VectorXd Xl; ///< 下翼面节点 x 坐标
        Eigen::VectorXd Yl; ///< 下翼面节点 y 坐标
    };

    /**
     * @brief 叶片截面定义
     * @details 描述叶片某一展向位置的气动外形与结构铺层信息，包括弦长、扭角、
     *          翼型形状以及上/下翼面和腹板的铺层配置。
     *
     * @par 使用示例
     * @code
     * Qahse::PcsL::BladeSection sec;
     * sec.SpanLocation      = 10.0;         // 距叶根 10m
     * sec.ChordLength       = 2.5;          // 弦长 2.5m
     * sec.TwistAngle        = 5.0;          // 扭角 5°
     * sec.AerodynamicCenter = 0.25;         // 气动中心在弦长25%处
     * sec.AirfoilFile       = "NACA64.dat";
     * sec.LayupFile         = "layup_r10.dat";
     * @endcode
     */
    struct BladeSection
    {
        double SpanLocation = 0;      ///< 截面展向位置（距叶根距离）[m]
        double AerodynamicCenter = 0; ///< 气动中心位置（归一化弦向坐标，0=前缘）
        double ChordLength = 0;       ///< 截面弦长 [m]
        double TwistAngle = 0;        ///< 截面扭角 [度]，相对于叶片参考轴
        std::string AirfoilFile;      ///< 翼型外形数据文件路径
        std::string LayupFile;        ///< 铺层配置文件路径

        AirfoilShape Airfoil;                  ///< 翼型几何形状数据
        SurfaceConfiguration TopConfiguration; ///< 上翼面铺层配置
        SurfaceConfiguration LowConfiguration; ///< 下翼面铺层配置
        SurfaceConfiguration WebConfiguration; ///< 腹板铺层配置
    };

    /**
     * @brief 腹板（Shear Web）结构定义
     * @details 描述叶片截面中腹板的位置信息，以归一化弦向坐标给出腹板内/外侧边界。
     *
     * @par 使用示例
     * @code
     * Qahse::PcsL::WebStructure web;
     * web.WebId              = 1;
     * web.InnerChordPosition = 0.2;  // 腹板靠前缘侧位于弦长20%处
     * web.OuterChordPosition = 0.6;  // 腹板靠后缘侧位于弦长60%处
     * @endcode
     */
    struct WebStructure
    {
        int WebId = 0;                 ///< 腹板编号（从1起）
        double InnerChordPosition = 0; ///< 腹板靠近前缘一侧的归一化弦向坐标（0=前缘）
        double OuterChordPosition = 0; ///< 腹板靠近后缘一侧的归一化弦向坐标（1=后缘）
    };

    /**
     * @brief FEM 单元类型枚举
     * @details 指定截面有限元网格所使用的单元类型，数值与 FEM::ElemType 一致。
     *          - Q4：4节点四边形单元（双线性）
     *          - Q8：8节点四边形单元（二次，完全积分）
     *          - Q8R：8节点四边形单元（减缩积分，减少剪切锁死）
     *          - T6：6节点三角形单元（二次）
     *
     * @par 使用示例
     * @code
     * Qahse::PcsL::PcsLElementType eType = Qahse::PcsL::PcsLElementType::Q8;
     * if (eType == Qahse::PcsL::PcsLElementType::Q8)
     *     std::cout << "使用Q8单元" << std::endl;
     * @endcode
     */
    enum class PcsLElementType
    {
        Q4 = 1,  ///< 4节点双线性四边形单元
        Q8 = 2,  ///< 8节点二次四边形单元（完全积分）
        Q8R = 3, ///< 8节点二次四边形单元（减缩积分）
        T6 = 4   ///< 6节点二次三角形单元
    };

    /**
     * @brief PcsL 截面分析主输入文件结构体
     * @details 汇总截面有限元分析所需的全部输入参数，包括叶片基本信息、FEM求解参数、
     *          截面/腹板定义、应力计算选项、自定义网格数据和材料属性。
     *          对应 PcsL 输入文件（.pcs/.dat）解析后的内存表示。
     *
     * @par 使用示例
     * @code
     * Qahse::PcsL::PcsLInput input;
     * input.path         = "blade.pcs";
     * input.BladeLength  = 63.0;          // 叶片长度 63m
     * input.SectionCount = 20;            // 20个截面
     * input.FEMSolve     = true;          // 启用FEM求解
     * input.ElememtType  = Qahse::PcsL::PcsLElementType::Q8; // 使用Q8单元
     * input.SurMeshExpT  = 2;            // 上翼面网格加密系数
     * // ... 填充 Sections, Webs, Material 后调用求解器
     * @endcode
     */
    struct PcsLInput
    {
        std::string path;              ///< 主输入文件路径（绝对或相对路径）
        std::vector<std::string> data; ///< 原始文件内容（按行存储，便于调试）

        // ========== 基本信息 ==========
        double BladeLength = 0;      ///< 叶片总长度 [m]
        int SectionCount = 0;        ///< 叶片截面数量
        int MaterialTypeCount = 0;   ///< 材料类型数量
        int OutputFormat = 3;        ///< 输出格式（1=文本, 2=二进制, 3=两者）
        bool UseTabDelimiter = true; ///< 输出文件是否使用Tab分隔符（false则用空格）
        bool FEMSolve = false;       ///< 是否执行FEM截面刚度求解
        bool CalStress = false;      ///< 是否计算截面应力/应变分布
        std::string MaterialsPath;   ///< 材料数据库文件路径

        // ========== FEM 方法参数 ==========
        bool SelfMesh = false;                             ///< 是否使用程序自动生成网格（false=导入外部网格）
        PcsLElementType ElememtType = PcsLElementType::Q4; ///< FEM单元类型（Q4/Q8/Q8R/T6）
        bool Shear_Center = true;                          ///< 是否计算剪切中心
        bool VTKShow = false;                              ///< 是否输出VTK可视化文件
        bool SaveCatch = false;                            ///< 是否保存中间计算缓存
        int MeshWeb = 4;                                   ///< 腹板网格沿厚度方向的单元层数
        int SurMeshExpT = -1;                              ///< 上翼面网格加密系数（-1=默认均匀）
        int SurMeshExpC = -1;                              ///< 下翼面网格加密系数（-1=默认均匀）
        std::vector<double> SurMeshExpCNorm;               ///< 加密位置的归一化弦向坐标列表
        bool OutMeshfFile = true;                          ///< 是否输出网格文件
        bool ConsidertwistAndpitch = true;                 ///< 计算刚度时是否考虑扭角和预倾角

        // ========== 截面与腹板 ==========
        std::vector<BladeSection> Sections; ///< 所有叶片截面定义（长度为 SectionCount）
        int Nweb = 0;                       ///< 腹板数量
        int Ib_sp_stn = 0;                  ///< 腹板内侧（靠叶根）截面编号
        int Ob_sp_stn = 0;                  ///< 腹板外侧（靠叶尖）截面编号
        std::vector<WebStructure> Webs;     ///< 腹板结构定义列表（长度为 Nweb）

        // ========== 输出 ==========
        std::string SumPath; ///< 汇总结果输出文件路径

        // ========== 应力计算 ==========
        std::string StrainMeshFile;        ///< 应力/应变计算所用的网格文件路径
        std::string StrainCache;           ///< 应变计算缓存文件路径（可加速重复计算）
        bool CalFailure = false;           ///< 是否进行失效准则分析
        std::vector<int> FailureCriterion; ///< 失效准则编号列表（如 1=最大应力，2=Tsai-Wu等）
        int ExtForceNum = 0;               ///< 外部截面力数量
        Eigen::MatrixXd ExtForce;          ///< 外部截面力矩阵，尺寸 (ExtForceNum, 6)，每行 [Fx,Fy,Fz,Mx,My,Mz]

        // ========== 自定义网格 ==========
        std::string FEMMeshPath;          ///< 外部自定义FEM网格文件路径
        int FEMNodeNum = 0;               ///< 自定义网格节点总数
        int FEMElementNum = 0;            ///< 自定义网格单元总数
        Eigen::MatrixXd FEMNodeMatrix;    ///< 节点坐标矩阵，尺寸 (N, 3)，列为 [id, x, y]
        Eigen::MatrixXd FEMElementMatrix; ///< 单元连接矩阵，尺寸 (N, cols)，列数取决于单元类型

        // ========== 材料 ==========
        std::vector<CompositeMaterial> Material; ///< 所有材料属性列表（长度为 MaterialTypeCount）
    };

} // namespace Qahse::PcsL