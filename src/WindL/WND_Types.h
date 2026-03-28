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

/// @brief 机构动力学分析相关函数的命名空间
namespace Qahse::WindL
{



    /// @brief 风场运行模式
    enum class RunMode {
        GENERATE = 0,   ///< 生成模式
        IMPORT = 1      ///< 导入模式
    };

    /// @brief 风场模型
    enum class WindModelType {
        /// @brief Kaimal 正常湍流模型
        KAIMAL,
        
        /// @brief von Karman 湍流模型
        VONKARMAN,
        
        /// @brief 极端湍流模型 (Extreme Turbulence Model)
        ETM,
        
        /// @brief 极端风速模型 (Extreme Wind speed Model)
        EWM,
        
        /// @brief 极端运行阵风 (Extreme Operating Gust)
        EOG,
        
        /// @brief 极端风向变化 (Extreme Direction Change)
        EDC,
        
        /// @brief 极端相干阵风伴随风向变化 (Extreme Coherent Gust)
        ECD,
        
        /// @brief 极端风剪切 (Extreme Wind Shear)
        EWS,
        
        /// @brief 均匀风场
        UNIFORM
    };

    /// @brief IEC 标准版本
    enum class IecStandard {
        /// @brief IEC 61400-1 第二版
        ED2,
        
        /// @brief IEC 61400-1 第三版
        ED3,
        
        /// @brief IEC 61400-1 第四版
        ED4
    };

    /// @brief 风机等级
    enum class TurbineClass {
        /// @brief I 类风机 (高风速)
        Class_I,
        
        /// @brief II 类风机 (中风速)
        Class_II,
        
        /// @brief III 类风机 (低风速)
        Class_III
    };

    /// @brief 湍流等级
    enum class TurbulenceClass {
        /// @brief A 级 (高湍流)
        Class_A,
        
        /// @brief B 级 (中湍流)
        Class_B,
        
        /// @brief C 级 (低湍流)
        Class_C
    };

    /// @brief 剪切类型
    enum class ShearType {
        PL,         ///< 幂律 (Power Law)
        LOG,        ///< 对数法则 (Logarithmic)
        NONE,       ///< 无剪切
        TABULAR     ///< 表格化非线性廓线 (扩展项)
    };

    /// @brief 生成算法
    enum class GenMethod {
        SRM,        ///< 随机分量法 (快速)
        VEERS,      ///< Veers 模型 (经典)
        AUTO        ///< 自动选择
    };

    /// @brief 插值方法
    enum class InterpMethod {
        TRILINEAR,  ///< 三线性插值
        CUBIC       ///< 三次样条插值
    };

    /// @brief Qahse风场配置文件(.qwd)生成器参数结构体
    struct WindLInput
    {
        // ---------------------- 时间与IO定义 --------------------------------------
        /// @brief 模式: 0 GENERATE(生成) 或 1 IMPORT(导入)
        RunMode mode = RunMode::GENERATE;
        
        /// @brief 风场模型: KAIMAL/VONKARMAN/ETM/EWM等
        WindModelType windModel = WindModelType::KAIMAL;

        // ---------------------- IEC 标准参数 --------------------------------------
        /// @brief 标准版次: ED2/ED3/ED4
        IecStandard iecStandard = IecStandard::ED3;
        
        /// @brief 风力机等级: I/II/III
        TurbineClass turbineClass = TurbineClass::Class_I;
        
        /// @brief 湍流等级: A/B/C
        TurbulenceClass turbClass = TurbulenceClass::Class_B;
        
        /// @brief 参考风速 [m/s]
        double vRef = 50.0;
        
        /// @brief 转子直径 [m]
        double rotorDiameter = 126.0;

        // ---------------------- 基本风参数 ----------------------------------------
        /// @brief 轮毂高度平均风速 [m/s]
        double meanWindSpeed = 11.4;
        
        /// @brief 参考高度 [m]
        double refHeight = 87.6;
        
        /// @brief 剪切类型: PL(幂律) / LOG(对数) / NONE / TABULAR
        ShearType shearType = ShearType::PL;
        
        /// @brief 幂律指数
        double shearExp = 0.14;
        
        /// @brief 地表粗糙度 [m]
        double roughness = 0.01;
        
        /// @brief 水平入流角 [deg]
        double horAngle = 0.0;
        
        /// @brief 垂直入流角 [deg]
        double vertAngle = 0.0;

        // ---------------------- 湍流网格参数 --------------------------------------
        /// @brief 湍流强度 [%] (0=IEC NTM 自动)
        double turbIntensity = 0.0;
        
        /// @brief 随机种子
        int turbSeed = 1;
        
        /// @brief Y 方向网格点数
        int gridPtsY = 36;
        
        /// @brief Z 方向网格点数
        int gridPtsZ = 36;
        
        /// @brief 风场宽度 [m]
        double fieldDimY = 150.0;
        
        /// @brief 风场高度 [m]
        double fieldDimZ = 150.0;
        
        /// @brief 风场时长 [s]
        double simTime = 660.0;
        
        /// @brief 风场时间步长 [s]
        double timeStep = 0.05;
        
        /// @brief 时域边界镜像循环
        bool cycleWind = true;
        
        /// @brief 生成三分量 u,v,w
        bool threeComp = true;

        // ---------------------- 扩展 / 瞬态事件控制 (新增) -------------------------
        /// @brief 阵风(EOG/EDC等)起始时间 [s]
        double gustStartTime = 0.0;
        
        /// @brief 阵风(EOG/EDC等)持续时间 [s]
        double gustDuration = 10.5;
        
        /// @brief 风向随高度线性偏转斜率 [deg/m]
        double windVeer = 0.0;

        // ---------------------- 风文件生成算法 -------------------------------------
        /// @brief 算法: SRM(快速)/VEERS(经典)/AUTO(自动)
        GenMethod genMethod = GenMethod::AUTO;
        
        /// @brief 使用 MKL FFT 加速
        bool useFft = true;
        
        /// @brief 插值方法: TRILINEAR(三线性)/CUBIC(三次)
        InterpMethod interpMethod = InterpMethod::TRILINEAR;

        // ---------------------- 输出控制 ------------------------------------------
        /// @brief 输出路径 (相对当前 .qwd 目录)
        std::string savePath = "./result";
        
        /// @brief 打印摘要
        bool sumPrint = true;
    };

}