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

/// @file PcsLFEMVTK.h
/// @brief PcsL 截面 VTK (.vtu) 输出
///
/// 基于 vtu11 库输出截面有限元网格和应力/应变/翘曲场。

#include "Pcs_FEM_Types.h"
#include "Pcs_FEM_Stress.h"
#include "../../IO/VTK/vtu11-cpp17.hpp"
#include "../../IO/Log/LogHelper.h"

#include <string>
#include <vector>
#include <filesystem>

namespace Qahse::PcsL::FEM
{
    using Qahse::IO::Log::LogHelper;

    /// 将截面网格输出为 VTK .vtu 文件
    /// @param filename   输出文件名（含 .vtu 后缀）
    /// @param u          FEM 数据结构（含节点和单元信息）
    /// @param stressResult 应力/应变结果（可选，nullptr 表示只输出网格）
    /// @param loadCase   输出哪个载荷工况的应力应变 (0-5), 默认 0
    inline void WritePcsLVTK(const std::string& filename,
                              const PcsLUtils& u,
                              const StressResult* stressResult = nullptr,
                              int loadCase = 0)
    {
        namespace fs = std::filesystem;
        // 确保目录存在
        fs::path fpath(filename);
        if (fpath.has_parent_path())
            fs::create_directories(fpath.parent_path());

        // ---- 构建点坐标 ----
        std::vector<double> points(u.nn_2d * 3);
        for (int i = 0; i < u.nn_2d; i++) {
            points[3 * i]     = u.nl_2d(i, 1);  // x
            points[3 * i + 1] = u.nl_2d(i, 2);  // y
            points[3 * i + 2] = 0.0;             // z = 0 (2D截面)
        }

        // ---- 构建单元连接 ----
        std::vector<vtu11::VtkIndexType> connectivity;
        std::vector<vtu11::VtkIndexType> offsets;
        std::vector<vtu11::VtkCellType>  types;

        connectivity.reserve(u.ne_2d * 8);

        for (int e = 0; e < u.ne_2d; e++) {
            ElemType et = u.etype[e];
            int nnpe = 4;
            vtu11::VtkCellType vtkType = 9; // VTK_QUAD
            if (et == Q8 || et == Q8R) {
                nnpe = 8;
                vtkType = 23; // VTK_QUADRATIC_QUAD
            } else if (et == T6) {
                nnpe = 6;
                vtkType = 22; // VTK_QUADRATIC_TRIANGLE
            }

            for (int n = 0; n < nnpe; n++) {
                int nodeId = u.el_2d(e, 1 + n) - 1; // 1-based → 0-based
                connectivity.push_back(static_cast<vtu11::VtkIndexType>(nodeId));
            }
            offsets.push_back(static_cast<vtu11::VtkIndexType>(connectivity.size()));
            types.push_back(vtkType);
        }

        // ---- 构建数据集 ----
        std::vector<vtu11::DataSetInfo> dataSetInfo;
        std::vector<vtu11::DataSetData> dataSetData;

        // CellData: 材料 ID
        {
            vtu11::DataSetData matIds(u.ne_2d);
            for (int e = 0; e < u.ne_2d; e++)
                matIds[e] = static_cast<double>(u.emat(e, 1));
            dataSetInfo.emplace_back("MaterialId", vtu11::DataSetType::CellData, 1);
            dataSetData.push_back(std::move(matIds));
        }

        // CellData: 纤维角
        {
            vtu11::DataSetData fiberAngle(u.ne_2d);
            for (int e = 0; e < u.ne_2d; e++)
                fiberAngle[e] = static_cast<double>(u.emat(e, 2));
            dataSetInfo.emplace_back("FiberAngle", vtu11::DataSetType::CellData, 1);
            dataSetData.push_back(std::move(fiberAngle));
        }

        // 应力/应变数据（如有）
        if (stressResult && !stressResult->elemResults.empty()) {
            int lc = (loadCase >= 0 && loadCase < 6) ? loadCase : 0;
            const char* strainNames[6] = {"Strain_11", "Strain_22", "Strain_12",
                                           "Strain_13", "Strain_23", "Strain_33"};
            const char* stressNames[6] = {"Stress_11", "Stress_22", "Stress_12",
                                           "Stress_13", "Stress_23", "Stress_33"};

            // 应变分量 (CellData)
            for (int comp = 0; comp < 6; comp++) {
                vtu11::DataSetData strainData(u.ne_2d);
                for (int e = 0; e < u.ne_2d; e++)
                    strainData[e] = stressResult->elemResults[e].strain_beam(comp, lc);
                dataSetInfo.emplace_back(strainNames[comp], vtu11::DataSetType::CellData, 1);
                dataSetData.push_back(std::move(strainData));
            }

            // 应力分量 (CellData)
            for (int comp = 0; comp < 6; comp++) {
                vtu11::DataSetData stressData(u.ne_2d);
                for (int e = 0; e < u.ne_2d; e++)
                    stressData[e] = stressResult->elemResults[e].stress_beam(comp, lc);
                dataSetInfo.emplace_back(stressNames[comp], vtu11::DataSetType::CellData, 1);
                dataSetData.push_back(std::move(stressData));
            }

            // 翘曲位移 (PointData)
            const auto& sol = stressResult->solutions;
            if (sol.X.rows() == u.nn_2d) {
                // Warping_X (PointData)
                {
                    vtu11::DataSetData warpX(u.nn_2d);
                    for (int i = 0; i < u.nn_2d; i++)
                        warpX[i] = sol.X(i, lc);
                    dataSetInfo.emplace_back("Warping_X", vtu11::DataSetType::PointData, 1);
                    dataSetData.push_back(std::move(warpX));
                }
                // Warping_Y (PointData)
                {
                    vtu11::DataSetData warpY(u.nn_2d);
                    for (int i = 0; i < u.nn_2d; i++)
                        warpY[i] = sol.Y(i, lc);
                    dataSetInfo.emplace_back("Warping_Y", vtu11::DataSetType::PointData, 1);
                    dataSetData.push_back(std::move(warpY));
                }
                // Warping_Z (PointData, w 分量即轴向翘曲)
                {
                    vtu11::DataSetData warpZ(u.nn_2d);
                    for (int i = 0; i < u.nn_2d; i++) {
                        int globalDof = 3 * i + 2;
                        if (globalDof < stressResult->solutions.X.rows())
                            warpZ[i] = 0.0; // 需要从 w 中提取
                        else
                            warpZ[i] = 0.0;
                    }
                    dataSetInfo.emplace_back("Warping_Z", vtu11::DataSetType::PointData, 1);
                    dataSetData.push_back(std::move(warpZ));
                }
            }
        }

        // ---- 写出 VTU ----
        vtu11::Vtu11UnstructuredMesh mesh{points, connectivity, offsets, types};
        vtu11::writeVtu(filename, mesh, dataSetInfo, dataSetData, "Ascii");

        std::cout << "  VTK file written: " << filename << std::endl;
    }

    /// 为所有载荷工况输出 VTK 文件
    inline void WritePcsLVTK_AllLoadCases(const std::string& baseFilename,
                                           const PcsLUtils& u,
                                           const StressResult& stressResult)
    {
        namespace fs = std::filesystem;
        fs::path base(baseFilename);
        std::string stem = base.stem().string();
        std::string dir = base.has_parent_path() ? base.parent_path().string() : ".";

        const char* lcNames[6] = {"Fx", "Fy", "Fxy", "Mx", "My", "Mz"};
        for (int lc = 0; lc < 6; lc++) {
            std::string fname = dir + "/" + stem + "_LC" + std::to_string(lc + 1) + "_" + lcNames[lc] + ".vtu";
            WritePcsLVTK(fname, u, &stressResult, lc);
        }
    }

} // namespace Qahse::PcsL::FEM
