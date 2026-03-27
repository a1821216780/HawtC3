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

/// @file PcsLFEMMesh.h
/// @brief 翼型截面自动网格生成（SelfMesh=false 模式）
///        从翼型外形 + 铺层厚度自动构建 Q4/Q8 有限元网格

#include "../PcsL_Types.h"
#include "PcsLFEMTypes.h"
#include "../../IO/Log/LogHelper.h"

#include <Eigen/Dense>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <tuple>
#include <utility>
#include <numeric>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Qahse::PcsL::FEM
{
    using Qahse::IO::Log::LogHelper;

    /// 网格节点
    struct MeshNode
    {
        int    id = 0;
        double x  = 0;
        double y  = 0;
    };

    /// 网格单元
    struct MeshElement_
    {
        std::vector<int> nodenum;   ///< 节点编号列表
        int    materialId = 0;      ///< 材料 ID
        double theta      = 0;      ///< 纤维角 (rad)
    };

    /// 网格生成结果
    struct MeshResult
    {
        std::vector<MeshNode>     nodes;
        std::vector<MeshElement_> elements;
    };

    // ===================================================================
    //  辅助函数
    // ===================================================================

    /// 在升序数组中查找 x 所属区间的左端索引
    inline int FindSectorIndex(const Eigen::VectorXd& xvec, double x)
    {
        for (int i = 0; i < xvec.size() - 1; i++) {
            if (x >= xvec(i) - 1e-12 && x <= xvec(i + 1) + 1e-12)
                return i;
        }
        return (int)xvec.size() - 2; // fallback: last interval
    }

    /// 在升序数组中查找第一个 >= x 的索引
    inline int FindInsertIndex(const Eigen::VectorXd& xvec, double x)
    {
        for (int i = 0; i < xvec.size(); i++) {
            if (xvec(i) >= x - 1e-12) return i;
        }
        return (int)xvec.size();
    }

    /// 1D 线性插值
    inline double Interp1D(const Eigen::VectorXd& xv, const Eigen::VectorXd& yv, double x)
    {
        int n = (int)xv.size();
        if (x <= xv(0)) return yv(0);
        if (x >= xv(n - 1)) return yv(n - 1);
        for (int i = 0; i < n - 1; i++) {
            if (x >= xv(i) - 1e-12 && x <= xv(i + 1) + 1e-12) {
                double t = (x - xv(i)) / (xv(i + 1) - xv(i));
                return yv(i) + t * (yv(i + 1) - yv(i));
            }
        }
        return yv(n - 1);
    }

    /// 向已排序的 Eigen::VectorXd 中插入一个值（如果不存在），返回新向量
    inline Eigen::VectorXd InsertSorted(const Eigen::VectorXd& v, double val, double tol = 1e-10)
    {
        // 检查是否已存在
        for (int i = 0; i < v.size(); i++) {
            if (std::abs(v(i) - val) < tol) return v; // already exists
        }
        int idx = FindInsertIndex(v, val);
        Eigen::VectorXd result(v.size() + 1);
        result.head(idx) = v.head(idx);
        result(idx) = val;
        if (idx < v.size())
            result.tail(v.size() - idx) = v.tail(v.size() - idx);
        return result;
    }

    /// 向排序向量中插入值并同步插值 y 向量
    inline void InsertSortedXY(Eigen::VectorXd& x, Eigen::VectorXd& y, double xval, double tol = 1e-10)
    {
        for (int i = 0; i < x.size(); i++) {
            if (std::abs(x(i) - xval) < tol) return;
        }
        double yval = Interp1D(x, y, xval);
        int idx = FindInsertIndex(x, xval);
        Eigen::VectorXd newx(x.size() + 1), newy(y.size() + 1);
        newx.head(idx) = x.head(idx);
        newy.head(idx) = y.head(idx);
        newx(idx) = xval;
        newy(idx) = yval;
        if (idx < x.size()) {
            newx.tail(x.size() - idx) = x.tail(x.size() - idx);
            newy.tail(y.size() - idx) = y.tail(y.size() - idx);
        }
        x = newx;
        y = newy;
    }

    /// 同步插入 x, y, tx, ty
    inline void InsertSortedXYTT(Eigen::VectorXd& x, Eigen::VectorXd& y,
                                 Eigen::VectorXd& tx, Eigen::VectorXd& ty,
                                 double xval, double tol = 1e-10)
    {
        for (int i = 0; i < x.size(); i++) {
            if (std::abs(x(i) - xval) < tol) return;
        }
        double yval  = Interp1D(x, y, xval);
        double txval = Interp1D(x, tx, xval);
        double tyval = Interp1D(x, ty, xval);
        int idx = FindInsertIndex(x, xval);
        auto inserter = [&](Eigen::VectorXd& v, double val) {
            Eigen::VectorXd nv(v.size() + 1);
            nv.head(idx) = v.head(idx);
            nv(idx) = val;
            if (idx < v.size()) nv.tail(v.size() - idx) = v.tail(v.size() - idx);
            v = nv;
        };
        inserter(x, xval);
        inserter(y, yval);
        inserter(tx, txval);
        inserter(ty, tyval);
    }

    // ===================================================================
    //  计算切向量（法向量方向，用于铺层厚度生长方向）
    // ===================================================================

    /// 计算翼面法向量（用于层厚生长方向）
    /// TopSurface=true: 法线指向内侧（上翼面）
    /// TopSurface=false: 法线指向内侧（下翼面）
    inline std::pair<Eigen::VectorXd, Eigen::VectorXd>
    ComputeTangential(const Eigen::VectorXd& x, const Eigen::VectorXd& y, bool topSurface)
    {
        int nx = (int)x.size();
        Eigen::VectorXd tx(nx), ty(nx);

        for (int i = 1; i < nx - 1; i++) {
            double dx = x(i + 1) - x(i - 1);
            double dy = y(i + 1) - y(i - 1);
            double ds = std::sqrt(dx * dx + dy * dy);
            if (topSurface) {
                tx(i) =  dy / ds;
                ty(i) = -dx / ds;
            } else {
                tx(i) = -dy / ds;
                ty(i) =  dx / ds;
            }
        }
        // 前缘: 法线指向 +x 方向（翼面从前缘开始）
        tx(0) = 1.0;
        ty(0) = 0.0;
        // 后缘
        tx(nx - 1) = 0.0;
        ty(nx - 1) = topSurface ? -1.0 : 1.0;

        return {tx, ty};
    }

    // ===================================================================
    //  统一扇区铺层数
    // ===================================================================

    /// 将所有扇区的铺层数统一为最大值，通过细分较厚的层来实现
    inline SurfaceConfiguration RebuildToSameLayers(const SurfaceConfiguration& config)
    {
        SurfaceConfiguration result = config;
        int ns = config.N_scts;
        if (ns == 0) return result;

        // 找到最大层数
        int nlayers = 0;
        for (int i = 0; i < ns; i++) {
            if (config.Sector[i].N_laminas > nlayers)
                nlayers = config.Sector[i].N_laminas;
        }

        for (int i = 0; i < ns; i++) {
            int nl_i = config.Sector[i].N_laminas;
            if (nl_i == nlayers) {
                // 层数已一致，只合并 ply
                result.Sector[i].Laminae.resize(nlayers);
                for (int j = 0; j < nlayers; j++) {
                    result.Sector[i].Laminae[j].Thickness =
                        config.Sector[i].Laminae[j].Thickness * config.Sector[i].Laminae[j].PlyCount;
                    result.Sector[i].Laminae[j].PlyCount = 1;
                }
                continue;
            }

            // 按比例将 nl_i 层分割为 nlayers 层
            // 先计算每个原始层的厚度
            std::vector<double> origT(nl_i);
            double totalT = 0;
            for (int j = 0; j < nl_i; j++) {
                origT[j] = config.Sector[i].Laminae[j].Thickness
                         * config.Sector[i].Laminae[j].PlyCount;
                totalT += origT[j];
            }

            // 按厚度比例分配子层数（每个原始层至少1个子层）
            int extra = nlayers - nl_i; // 需要额外分配的子层数
            std::vector<int> subCount(nl_i, 1); // 每个原始层至少1个子层
            // 将额外子层按厚度比例分配给最厚的层
            for (int e = 0; e < extra; e++) {
                // 找到"当前每子层厚度"最大的原始层
                int bestIdx = 0;
                double bestRatio = 0;
                for (int j = 0; j < nl_i; j++) {
                    double ratio = origT[j] / subCount[j];
                    if (ratio > bestRatio) { bestRatio = ratio; bestIdx = j; }
                }
                subCount[bestIdx]++;
            }

            // 构建新的铺层列表
            result.Sector[i].N_laminas = nlayers;
            result.Sector[i].Laminae.resize(nlayers);
            int outIdx = 0;
            for (int j = 0; j < nl_i; j++) {
                double subT = origT[j] / subCount[j]; // 每个子层的厚度
                for (int s = 0; s < subCount[j]; s++) {
                    result.Sector[i].Laminae[outIdx].LayerId = outIdx;
                    result.Sector[i].Laminae[outIdx].PlyCount = 1;
                    result.Sector[i].Laminae[outIdx].Thickness = subT;
                    result.Sector[i].Laminae[outIdx].FiberOrientation =
                        config.Sector[i].Laminae[j].FiberOrientation;
                    result.Sector[i].Laminae[outIdx].MaterialId =
                        config.Sector[i].Laminae[j].MaterialId;
                    result.Sector[i].Laminae[outIdx].MatDisp =
                        config.Sector[i].Laminae[j].MatDisp;
                    outIdx++;
                }
            }
        }
        return result;
    }

    // ===================================================================
    //  半翼面网格构建
    // ===================================================================

    /// 为半翼面构建节点和 Q4 单元
    /// @param x_out, y_out 外表面坐标
    /// @param tx, ty 法向量
    /// @param config 铺层配置（已统一层数）
    /// @param materials 材料数组
    /// @param chord 弦长
    /// @param x_te, y_te 尾缘内交点
    inline std::pair<std::vector<MeshNode>, std::vector<MeshElement_>>
    BuildHalfMesh(const Eigen::VectorXd& x_out, const Eigen::VectorXd& y_out,
                  const Eigen::VectorXd& tx, const Eigen::VectorXd& ty,
                  const SurfaceConfiguration& config,
                  const std::vector<CompositeMaterial>& materials,
                  double chord, double x_te, double y_te,
                  bool topSurface = true)
    {
        int nx = (int)x_out.size();
        int nlayers = config.Sector[0].N_laminas;
        int nrows = nlayers + 1;

        // 扇区边界节点（归一化 → 实际弦坐标）
        Eigen::VectorXd xsec_scaled(config.xsec_node.size());
        for (int i = 0; i < (int)config.xsec_node.size(); i++)
            xsec_scaled(i) = config.xsec_node[i] * chord;

        // 节点坐标矩阵: (nlayers+1) x nx
        Eigen::MatrixXd nodeX(nrows, nx), nodeY(nrows, nx);
        nodeX.row(0) = x_out.transpose();
        nodeY.row(0) = y_out.transpose();

        // 逐层向内生长
        for (int layer = 0; layer < nlayers; layer++) {
            for (int j = 0; j < nx; j++) {
                int secIdx = FindSectorIndex(xsec_scaled, x_out(j));
                if (secIdx < 0) secIdx = 0;
                if (secIdx >= config.N_scts) secIdx = config.N_scts - 1;
                const auto& lam = config.Sector[secIdx].Laminae[layer];
                double t = lam.Thickness * lam.PlyCount;
                nodeX(layer + 1, j) = nodeX(layer, j) + t * tx(j);
                nodeY(layer + 1, j) = nodeY(layer, j) + t * ty(j);
            }
        }

        // 尾缘修正：如果有内交点，将最后一列内层节点拉到交点
        if (std::abs(x_te) > 1e-15 || std::abs(y_te) > 1e-15) {
            for (int i = 1; i < nrows; i++) {
                double frac = (double)i / nlayers;
                nodeX(i, nx - 1) = nodeX(0, nx - 1) + frac * (x_te - nodeX(0, nx - 1));
                nodeY(i, nx - 1) = nodeY(0, nx - 1) + frac * (y_te - nodeY(0, nx - 1));
            }
        }

        // 创建节点
        std::vector<MeshNode> nodes(nrows * nx);
        for (int i = 0; i < nrows; i++) {
            for (int j = 0; j < nx; j++) {
                int id = i * nx + j;
                nodes[id] = {id, nodeX(i, j), nodeY(i, j)};
            }
        }

        // 创建 Q4 单元: nlayers x (nx-1)
        std::vector<MeshElement_> elements(nlayers * (nx - 1));
        for (int i = 0; i < nlayers; i++) {
            for (int j = 0; j < nx - 1; j++) {
                int n0 = i * nx + j;
                int n1 = i * nx + j + 1;
                int n2 = (i + 1) * nx + j + 1;
                int n3 = (i + 1) * nx + j;

                int secIdx = FindSectorIndex(xsec_scaled, x_out(j));
                if (secIdx < 0) secIdx = 0;
                if (secIdx >= config.N_scts) secIdx = config.N_scts - 1;
                const auto& lam = config.Sector[secIdx].Laminae[i];

                int matId = lam.MaterialId;
                double fiberAngle = lam.FiberOrientation * M_PI / 180.0;

                // 上翼面法向向内（-y方向）使 {n0,n1,n2,n3} 为 CW，需翻转为 CCW
                // 下翼面法向向内（+y方向）使 {n0,n1,n2,n3} 已为 CCW
                if (topSurface)
                    elements[i * (nx - 1) + j] = {{n0, n3, n2, n1}, matId, fiberAngle};
                else
                    elements[i * (nx - 1) + j] = {{n0, n1, n2, n3}, matId, fiberAngle};
            }
        }
        return {nodes, elements};
    }

    // ===================================================================
    //  合并上下表面网格
    // ===================================================================

    /// 合并上下翼面网格，共享前缘节点
    inline MeshResult CombineHalves(
        const std::vector<MeshNode>& nodes_u, const std::vector<MeshElement_>& elems_u,
        const std::vector<MeshNode>& nodes_l, const std::vector<MeshElement_>& elems_l,
        int nlayers, double x_te)
    {
        int nt = nlayers + 1; // 沿厚度方向的节点行数
        int nnu = (int)nodes_u.size();
        int nnl = (int)nodes_l.size();
        int nxu = nnu / nt; // 上翼面每行节点数
        int nxl = nnl / nt; // 下翼面每行节点数

        // 构建下翼面节点→合并网格节点的映射
        // 下翼面前缘列(col 0)节点与上翼面前缘列(col 0)节点共享
        std::vector<int> lowerMap(nnl, -1);
        int nextId = nnu;

        for (int row = 0; row < nt; row++) {
            for (int col = 0; col < nxl; col++) {
                int lowId = row * nxl + col;
                if (col == 0) {
                    // 前缘列共享: lower(row,0) → upper(row,0) = row * nxu
                    lowerMap[lowId] = row * nxu;
                } else {
                    lowerMap[lowId] = nextId++;
                }
            }
        }

        int nn = nextId;
        MeshResult result;
        result.nodes.resize(nn);

        // 上翼面节点直接复制
        for (int i = 0; i < nnu; i++)
            result.nodes[i] = nodes_u[i];

        // 下翼面非共享节点
        for (int i = 0; i < nnl; i++) {
            int mapped = lowerMap[i];
            if (mapped >= nnu) {
                result.nodes[mapped] = {mapped, nodes_l[i].x, nodes_l[i].y};
            }
        }

        // 复制上翼面单元
        result.elements = elems_u;

        // 下翼面单元：重映射节点编号
        for (int ei = 0; ei < (int)elems_l.size(); ei++) {
            MeshElement_ elem = elems_l[ei];
            for (auto& n : elem.nodenum) {
                n = lowerMap[n];
            }
            result.elements.push_back(elem);
        }

        return result;
    }

    // ===================================================================
    //  腹板网格构建
    // ===================================================================

    /// 在上下翼面网格之间构建腹板网格
    /// @param result 合并后的上下翼面网格（将被修改，添加腹板节点和单元）
    /// @param nodes_u 上翼面节点
    /// @param nodes_l 下翼面节点
    /// @param section 截面定义
    /// @param materials 材料列表
    /// @param nlayers 铺层层数
    /// @param meshWeb 腹板沿高度方向的网格数
    /// @param nnu 上翼面节点总数
    /// @param xu, yu 上翼面外表面坐标
    /// @param xl, yl 下翼面外表面坐标
    inline void BuildWebMesh(MeshResult& result,
                             const BladeSection& section,
                             const std::vector<CompositeMaterial>& materials,
                             int nlayers, int meshWeb, int nnu,
                             const Eigen::VectorXd& xu, const Eigen::VectorXd& yu,
                             const Eigen::VectorXd& xl, const Eigen::VectorXd& yl)
    {
        int nweb = (int)section.WebConfiguration.Sector.size();
        if (nweb == 0) return;

        int nt = nlayers + 1;
        double chord = section.ChordLength;
        int nxu = nnu / nt;

        for (int w = 0; w < nweb; w++) {
            double webLoc = section.WebConfiguration.xsec_node[w]; // 归一化弦向位置
            double xu_web = webLoc * chord;

            // 在上翼面找到最近的列（外表面x坐标匹配）
            int col_u = 0;
            {
                double bestDist = 1e30;
                for (int j = 0; j < nxu; j++) {
                    double d = std::abs(result.nodes[j].x - xu_web);
                    if (d < bestDist) { bestDist = d; col_u = j; }
                }
            }

            // 上翼面内层节点索引: col_u + nlayers * nxu (最内层行, 对应列)
            int node_u_inner = nlayers * nxu + col_u;
            double x_top = result.nodes[node_u_inner].x;
            double y_top = result.nodes[node_u_inner].y;

            // 下翼面内层节点: 需要搜索合并后的节点
            // 下翼面节点ID从nnu开始(除LE共享), 内层是最后一行
            // 搜索所有下翼面内层节点(相比上翼面in x最近的)
            int node_l_inner = -1;
            {
                double bestDist = 1e30;
                for (int i = nnu; i < (int)result.nodes.size(); i++) {
                    double d = std::abs(result.nodes[i].x - xu_web);
                    if (d < bestDist) {
                        // 确认这是内层节点（y应该接近上翼面内层但在另一侧）
                        bestDist = d;
                        node_l_inner = i;
                    }
                }
                // 更精确：在候选中筛选y值最靠近内侧的
                double targetY = Interp1D(xl, yl, xu_web);
                double bestDist2 = 1e30;
                for (int i = nnu; i < (int)result.nodes.size(); i++) {
                    if (std::abs(result.nodes[i].x - xu_web) < 1e-8 * chord) {
                        // 找y最靠近下翼面内侧(y最大=最靠近chord line)
                        double dy = std::abs(result.nodes[i].y - targetY);
                        // 内层节点y应该大于外层(下翼面内层向上偏移)
                        if (result.nodes[i].y > targetY - 1e-10 && dy < bestDist2) {
                            // 找到内层的节点（比外表面更靠近中线的）
                        }
                        // 简化：找y值最大的（最靠近chord line）
                        if (result.nodes[i].y > result.nodes[node_l_inner].y || node_l_inner < nnu) {
                            node_l_inner = i;
                        }
                    }
                }
            }
            if (node_l_inner < 0) continue;

            double x_bot = result.nodes[node_l_inner].x;
            double y_bot = result.nodes[node_l_inner].y;

            // 腹板层数和总厚度
            int nWebLayers = section.WebConfiguration.Sector[w].N_laminas;
            double webThickness = 0;
            for (int k = 0; k < nWebLayers; k++) {
                webThickness += section.WebConfiguration.Sector[w].Laminae[k].Thickness
                              * section.WebConfiguration.Sector[w].Laminae[k].PlyCount;
            }

            // 创建腹板节点网格: (meshWeb+1) 行 × (nWebLayers+1) 列
            // 行: 沿高度方向（从上内层到下内层）
            // 列: 沿水平方向（腹板厚度方向）
            int nRows = meshWeb + 1;
            int nCols = nWebLayers + 1;
            double halfT = webThickness / 2.0;

            // 厚度方向的偏移量（水平方向）
            std::vector<double> colX(nCols);
            {
                double cumT = 0;
                colX[0] = -halfT;
                for (int c = 0; c < nWebLayers; c++) {
                    cumT += section.WebConfiguration.Sector[w].Laminae[c].Thickness
                          * section.WebConfiguration.Sector[w].Laminae[c].PlyCount;
                    colX[c + 1] = -halfT + cumT;
                }
            }

            double xc = (x_top + x_bot) / 2.0;

            // 创建节点并记录ID
            std::vector<std::vector<int>> webNodeIds(nRows, std::vector<int>(nCols));

            // 第一行(顶部)和最后一行(底部)的中心列连接到现有节点
            for (int r = 0; r < nRows; r++) {
                double frac = (double)r / meshWeb;
                double yBase = y_top + frac * (y_bot - y_top);
                double xBase = x_top + frac * (x_bot - x_top);

                for (int c = 0; c < nCols; c++) {
                    int newId = (int)result.nodes.size();
                    double nx_ = xBase + colX[c];
                    double ny_ = yBase;
                    result.nodes.push_back({newId, nx_, ny_});
                    webNodeIds[r][c] = newId;
                }
            }

            // 创建 Q4 单元: meshWeb × nWebLayers
            for (int r = 0; r < meshWeb; r++) {
                for (int c = 0; c < nWebLayers; c++) {
                    int n0 = webNodeIds[r][c];
                    int n1 = webNodeIds[r][c + 1];
                    int n2 = webNodeIds[r + 1][c + 1];
                    int n3 = webNodeIds[r + 1][c];

                    int matId = section.WebConfiguration.Sector[w].Laminae[c].MaterialId;
                    double theta = section.WebConfiguration.Sector[w].Laminae[c].FiberOrientation * M_PI / 180.0;

                    // CCW 绕序: 腹板向右看为 n0(左上)→n3(左下)→n2(右下)→n1(右上)
                    result.elements.push_back({{n0, n3, n2, n1}, matId, theta});
                }
            }
        }
    }

    // ===================================================================
    //  Q4 → Q8 转换
    // ===================================================================

    /// 将 Q4 网格转换为 Q8 网格（添加边中节点）
    inline MeshResult ConvertQ4ToQ8(const MeshResult& q4mesh)
    {
        MeshResult q8mesh;
        q8mesh.nodes = q4mesh.nodes;
        int nextId = (int)q8mesh.nodes.size();

        // 边→中点节点映射：(min_id, max_id) → mid_node_id
        std::map<std::pair<int, int>, int> edgeMidMap;

        for (const auto& elem : q4mesh.elements) {
            if (elem.nodenum.size() != 4) continue;

            int corners[4] = {elem.nodenum[0], elem.nodenum[1], elem.nodenum[2], elem.nodenum[3]};
            int edgeNodes[4];

            std::pair<int, int> edges[4] = {
                {corners[0], corners[1]},
                {corners[1], corners[2]},
                {corners[2], corners[3]},
                {corners[3], corners[0]}
            };

            for (int i = 0; i < 4; i++) {
                auto sorted = std::make_pair(
                    std::min(edges[i].first, edges[i].second),
                    std::max(edges[i].first, edges[i].second)
                );

                auto it = edgeMidMap.find(sorted);
                if (it != edgeMidMap.end()) {
                    edgeNodes[i] = it->second;
                } else {
                    const auto& n1 = q8mesh.nodes[edges[i].first];
                    const auto& n2 = q8mesh.nodes[edges[i].second];
                    MeshNode mid = {nextId, (n1.x + n2.x) * 0.5, (n1.y + n2.y) * 0.5};
                    q8mesh.nodes.push_back(mid);
                    edgeMidMap[sorted] = nextId;
                    edgeNodes[i] = nextId;
                    nextId++;
                }
            }

            MeshElement_ q8elem;
            q8elem.nodenum = {corners[0], corners[1], corners[2], corners[3],
                              edgeNodes[0], edgeNodes[1], edgeNodes[2], edgeNodes[3]};
            q8elem.materialId = elem.materialId;
            q8elem.theta = elem.theta;
            q8mesh.elements.push_back(q8elem);
        }

        return q8mesh;
    }

    // ===================================================================
    //  网格 → PcsLUtils 转换
    // ===================================================================

    /// 将自动生成的网格转换为 FEM 求解器使用的 PcsLUtils
    inline PcsLUtils ConvertMeshToFEMUtils(const MeshResult& mesh,
                                           const std::vector<CompositeMaterial>& materials,
                                           PcsLElementType etype)
    {
        PcsLUtils u;
        u.nn_2d = (int)mesh.nodes.size();
        u.ne_2d = (int)mesh.elements.size();
        u.nmat = (int)materials.size();

        // 节点表: [id, x, y]
        u.nl_2d.resize(u.nn_2d, 3);
        for (int i = 0; i < u.nn_2d; i++) {
            u.nl_2d(i, 0) = mesh.nodes[i].id;
            u.nl_2d(i, 1) = mesh.nodes[i].x;
            u.nl_2d(i, 2) = mesh.nodes[i].y;
        }

        // 材料属性表
        u.matprops.resize(u.nmat, 10);
        for (int i = 0; i < u.nmat; i++) {
            const auto& m = materials[i];
            u.matprops(i, 0) = m.E1;   u.matprops(i, 1) = m.E2;   u.matprops(i, 2) = m.E3;
            u.matprops(i, 3) = m.G12;  u.matprops(i, 4) = m.G13;  u.matprops(i, 5) = m.G23;
            u.matprops(i, 6) = m.Nu12; u.matprops(i, 7) = m.Nu13; u.matprops(i, 8) = m.Nu23;
            u.matprops(i, 9) = m.Density;
        }

        // 确定每个单元的节点数
        FEM::ElemType femEtype = static_cast<FEM::ElemType>(static_cast<int>(etype));
        int nnpe = (femEtype == FEM::Q4) ? 4 : (femEtype == FEM::T6 ? 6 : 8);
        u.max_nnpe = nnpe;

        u.el_2d.resize(u.ne_2d, 1 + nnpe);
        u.emat.resize(u.ne_2d, 4);
        u.etype.resize(u.ne_2d, femEtype);

        for (int e = 0; e < u.ne_2d; e++) {
            u.el_2d(e, 0) = e;
            for (int n = 0; n < nnpe && n < (int)mesh.elements[e].nodenum.size(); n++) {
                u.el_2d(e, 1 + n) = mesh.elements[e].nodenum[n] + 1; // 1-based
            }

            int matId = mesh.elements[e].materialId;
            double theta = mesh.elements[e].theta * 180.0 / M_PI; // back to degrees

            // 找到材料索引
            int matIdx = 1;
            for (int mi = 0; mi < u.nmat; mi++) {
                if (materials[mi].Id == matId) { matIdx = mi + 1; break; }
            }

            u.emat(e, 0) = e;
            u.emat(e, 1) = matIdx;
            u.emat(e, 2) = theta;
            u.emat(e, 3) = 0;
        }

        u.elemInfo.resize(4);
        u.elemInfo[0] = MakeQ4Info();
        u.elemInfo[1] = MakeQ8Info();
        u.elemInfo[2] = MakeQ8RInfo();
        u.elemInfo[3] = MakeT6Info();

        return u;
    }

    // ===================================================================
    //  主入口：翼型自动网格生成
    // ===================================================================

    /// 从翼型外形 + 铺层数据自动生成有限元网格
    inline MeshResult CalAirfoilMesh(BladeSection& section, int secIndex,
                                     const std::vector<WebStructure>& webs,
                                     int Nweb, int Ib_sp_stn, int Ob_sp_stn,
                                     const std::vector<CompositeMaterial>& materials,
                                     int meshWeb = 4,
                                     int surMeshExpT = -1,
                                     bool considerTwistAndPitch = true)
    {
        double chord = section.ChordLength;
        double twist = section.TwistAngle * M_PI / 180.0;

        // 1. 统一上下翼面铺层数
        auto topConfig = RebuildToSameLayers(section.TopConfiguration);
        auto lowConfig = RebuildToSameLayers(section.LowConfiguration);

        int nlayers = topConfig.Sector[0].N_laminas;

        // 2. 复制翼面坐标并缩放到实际弦长
        Eigen::VectorXd xu = section.Airfoil.Xu * chord;
        Eigen::VectorXd yu = section.Airfoil.Yu * chord;
        Eigen::VectorXd xl = section.Airfoil.Xl * chord;
        Eigen::VectorXd yl = section.Airfoil.Yl * chord;

        // 3. 向翼面坐标中插入扇区分界点和腹板位置
        for (int j = 0; j < topConfig.N_scts; j++) {
            InsertSortedXY(xu, yu, topConfig.xsec_node[j] * chord);
        }
        for (int j = 0; j < lowConfig.N_scts; j++) {
            InsertSortedXY(xl, yl, lowConfig.xsec_node[j] * chord);
        }

        // 4. 计算法向量
        auto [txu, tyu] = ComputeTangential(xu, yu, true);
        auto [txl, tyl] = ComputeTangential(xl, yl, false);

        // 5. 插入腹板位置节点
        bool hasWeb = (secIndex >= Ib_sp_stn - 1 && secIndex <= Ob_sp_stn - 1 && Nweb > 0);
        if (hasWeb) {
            for (int w = 0; w < Nweb; w++) {
                double webX = webs[w].InnerChordPosition * chord;
                InsertSortedXYTT(xu, yu, txu, tyu, webX);
                InsertSortedXYTT(xl, yl, txl, tyl, webX);
            }
        }

        // 6. 尾缘内交点（上下内表面交叉点）
        // 简化处理：设为 (0, 0) 表示开放尾缘
        double x_te = 0.0, y_te = 0.0;

        // 7. 构建上下半翼面网格
        auto [nodes_u, elems_u] = BuildHalfMesh(xu, yu, txu, tyu, topConfig, materials, chord, x_te, y_te, true);
        auto [nodes_l, elems_l] = BuildHalfMesh(xl, yl, txl, tyl, lowConfig, materials, chord, x_te, y_te, false);

        // 8. 合并上下翼面
        MeshResult result = CombineHalves(nodes_u, elems_u, nodes_l, elems_l, nlayers, x_te);

        // 9. 腹板网格（简化版本）
        if (hasWeb) {
            BuildWebMesh(result, section, materials, nlayers, meshWeb, (int)nodes_u.size(),
                         xu, yu, xl, yl);
        }

        // 10. 旋转扭角
        if (considerTwistAndPitch && std::abs(twist) > 1e-15) {
            double xc = chord * section.AerodynamicCenter;
            double s = std::sin(twist), c = std::cos(twist);
            for (auto& nd : result.nodes) {
                double x = nd.x;
                double y = nd.y;
                nd.x = (x - xc) * c + y * s + xc;
                nd.y = -(x - xc) * s + y * c;
            }
        }

        return result;
    }

} // namespace Qahse::PcsL::FEM
