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

#include <cmath>
#include "Vec3f.h"
#include "Quaternion.h"
#include "../../Params.h"

namespace Qahse::IO::Math
{

    /**
     * @brief 绕空间中任意点 O 和任意轴 R 旋转单精度向量（角度以度为单位）
     *
     * 【功能】以点 O 为旋转中心，对当前单精度 float 向量绕轴 R 旋转 Angle 度。
     *         先平移到局部坐标系，执行四元数旋转，再平移回世界坐标系。
     * 【物理意义】Vec3f 用于渲染和后处理阶段（如涡面可视化、尾迹点云绘制），
     *             旋转操作将世界系坐标点变换到屏幕/局部坐标系。
     * 【参数】
     *   @param O      旋转中心点（float 精度）
     *   @param R      旋转轴方向单位向量（float 精度）
     *   @param Angle  旋转角度，单位：度（°）
     * 【使用案例】
     *   Vec3f vortex_pt(10.0f, 0.0f, 5.0f);
     *   Vec3f hub(0.0f, 0.0f, 80.0f);
     *   Vec3f shaft(1.0f, 0.0f, 0.0f);
     *   vortex_pt.Rotate(hub, shaft, 30.0);  // 绕主轴旋转30°用于可视化
     */
    void Vec3f::Rotate(Vec3f &O, Vec3f const &R, double Angle)
    {
        Quaternion qt;
        qt.set(Angle, R);
        Vec3f OP;
        OP.x = x - O.x;
        OP.y = y - O.y;
        OP.z = z - O.z;
        qt.conj(OP);
        x = O.x + OP.x;
        y = O.y + OP.y;
        z = O.z + OP.z;
    }

    /**
     * @brief 使用 Rodrigues 公式绕任意轴旋转单精度向量（角度以度为单位）
     *
     * 【功能】将轴向量 n 归一化后，利用 Rodrigues 旋转公式对当前 float 向量
     *         旋转 NTilt 度。公式：r'=(n·r)n + (n×r)×n·cosθ + (n×r)·sinθ
     * 【物理意义】与 Vec3::RotateN 相同，但使用单精度计算，适合后处理和
     *             可视化阶段的大量点变换，性能更高内存占用更少。
     * 【参数】
     *   @param n      旋转轴方向（内部自动归一化）
     *   @param NTilt  旋转角度，单位：度（°）
     * 【使用案例】
     *   Vec3f pt(5.0f, 0.0f, 2.0f);
     *   Vec3f tilt_axis(0.0f, 1.0f, 0.0f);
     *   pt.RotateN(tilt_axis, 5.0f);  // 5°仰角旋转用于网格生成
     */
    void Vec3f::RotateN(Vec3f n, float NTilt)
    {
        n.Normalize();
        Vec3f r(x, y, z);
        r = n * (n.dot(r)) + (n * r) * n * std::cos(NTilt / 180.0f * static_cast<float>(PI_)) + (n * r) * std::sin(NTilt / 180.0f * static_cast<float>(PI_));
        x = r.x;
        y = r.y;
        z = r.z;
    }

    /**
     * @brief 绕过点 O 的 X 轴旋转单精度向量（角度以度为单位）
     *
     * 【功能】以点 O 为旋转中心，在 YZ 平面内旋转当前 float 向量 XTilt 度。
     * 【物理意义】用于渲染和后处理中的叶片剖面点变换，
     *             处理横向挥舞变形后的气动面坐标更新。
     * 【参数】
     *   @param O      旋转中心点
     *   @param XTilt  绕 X 轴旋转角度，单位：度（°）
     * 【使用案例】
     *   Vec3f surface_pt(0.0f, 5.0f, 3.0f);
     *   Vec3f blade_root(0.0f, 0.0f, 0.0f);
     *   surface_pt.RotateX(blade_root, 2.0f);  // 翼面挥舞2°变形
     */
    void Vec3f::RotateX(Vec3f const &O, float XTilt)
    {
        Vec3f OP;
        OP.x = x - O.x;
        OP.y = y - O.y;
        OP.z = z - O.z;
        XTilt *= static_cast<float>(PI_) / 180.0f;
        y = O.y + OP.y * std::cos(XTilt) - OP.z * std::sin(XTilt);
        z = O.z + OP.y * std::sin(XTilt) + OP.z * std::cos(XTilt);
    }

    /**
     * @brief 绕过点 O 的 Y 轴旋转单精度向量（角度以度为单位）
     *
     * 【功能】以点 O 为旋转中心，在 XZ 平面内旋转当前 float 向量 YTilt 度。
     * 【物理意义】用于单精度渲染坐标中的仰角变换，处理叶片和塔筒
     *             剖面网格的初始位置生成。
     * 【参数】
     *   @param O      旋转中心点
     *   @param YTilt  绕 Y 轴旋转角度，单位：度（°）
     * 【使用案例】
     *   Vec3f mesh_pt(2.0f, 0.0f, 80.0f);
     *   Vec3f hub_f(0.0f, 0.0f, 80.0f);
     *   mesh_pt.RotateY(hub_f, 5.0f);  // 5°仰角下的网格点变换
     */
    void Vec3f::RotateY(Vec3f const &O, float YTilt)
    {
        Vec3f OP;
        OP.x = x - O.x;
        OP.y = y - O.y;
        OP.z = z - O.z;
        YTilt *= static_cast<float>(PI_) / 180.0f;
        x = O.x + OP.x * std::cos(YTilt) + OP.z * std::sin(YTilt);
        z = O.z - OP.x * std::sin(YTilt) + OP.z * std::cos(YTilt);
    }

    /**
     * @brief 绕过点 O 的 Z 轴旋转单精度向量（角度以度为单位）
     *
     * 【功能】以点 O 为旋转中心，在 XY 平面内旋转当前 float 向量 ZTilt 度。
     * 【物理意义】叶片三等分对称布置的旋转变换（120°/240°），
     *             用于从第一片叶片网格复制生成其他两片的初始坐标。
     * 【参数】
     *   @param O      旋转中心点（通常为轮毂中心的单精度坐标）
     *   @param ZTilt  绕 Z 轴旋转角度，单位：度（°）
     * 【使用案例】
     *   Vec3f blade1_tip(63.0f, 0.0f, 90.0f);
     *   Vec3f hub_f(0.0f, 0.0f, 90.0f);
     *   blade1_tip.RotateZ(hub_f, 120.0f);  // 旋转120°生成第二片叶片尖端
     */
    void Vec3f::RotateZ(Vec3f const &O, float ZTilt)
    {
        Vec3f OP;
        OP.x = x - O.x;
        OP.y = y - O.y;
        OP.z = z - O.z;
        ZTilt *= static_cast<float>(PI_) / 180.0f;
        x = O.x + OP.x * std::cos(ZTilt) + OP.y * std::sin(ZTilt);
        y = O.y - OP.x * std::sin(ZTilt) + OP.y * std::cos(ZTilt);
    }

    /**
     * @brief 绕原点的 Z 轴原地旋转（单精度，角度以弧度为单位）
     *
     * 【功能】在 XY 平面内对单精度向量进行原地旋转，为高性能渲染
     *         内循环的最小开销版本（无点 O 平移，无度/弧度转换）。
     * 【物理意义】用于涡粒子尾迹场的批量坐标旋转，每个时间步需要
     *             对数千个尾迹点执行相同角度的旋转更新。
     * 【参数】
     *   @param ZTilt  绕 Z 轴旋转角度，单位：弧度（rad）
     * 【使用案例】
     *   for (auto& p : wake_particles) p.pos.RotZ(omega * dt);  // 旋转一时间步
     */
    void Vec3f::RotZ(float ZTilt)
    {
        Vec3f OP;
        OP.x = x;
        OP.y = y;
        x = OP.x * std::cos(ZTilt) - OP.y * std::sin(ZTilt);
        y = OP.x * std::sin(ZTilt) + OP.y * std::cos(ZTilt);
    }

    /**
     * @brief 绕原点的 Y 轴原地旋转（单精度，角度以弧度为单位）
     *
     * 【功能】在 XZ 平面内对单精度向量进行原地旋转，弧度输入直接参与
     *         三角函数运算，避免多余的角度换算。
     * 【物理意义】在自由涡流尾迹（FVW）方法的涡粒子传输步骤中，
     *             批量更新粒子位置需要高效的原地旋转操作。
     * 【参数】
     *   @param YTilt  绕 Y 轴旋转角度，单位：弧度（rad）
     * 【使用案例】
     *   Vec3f vp(10.0f, 0.0f, 2.0f);
     *   vp.RotY(0.0873f);  // 旋转5°（弧度）修正仰角
     */
    void Vec3f::RotY(float YTilt)
    {
        Vec3f OP;
        OP.x = x;
        OP.z = z;
        x = OP.x * std::cos(YTilt) + OP.z * std::sin(YTilt);
        z = -OP.x * std::sin(YTilt) + OP.z * std::cos(YTilt);
    }

    /**
     * @brief 绕原点的 Y 轴旋转（单精度，角度以度为单位）
     *
     * 【功能】与 RotY(float) 功能相同，但输入角度单位为度（°），
     *         函数内部完成 float 精度的度到弧度转换后执行旋转。
     * 【物理意义】在文件解析和结构初始化阶段，仰角等参数以度为单位
     *             读入后，需调用此函数对单精度网格点做初始位置设置。
     * 【参数】
     *   @param YTilt  绕 Y 轴旋转角度，单位：度（°）
     * 【使用案例】
     *   Vec3f node_pt(5.0f, 0.0f, 30.0f);
     *   node_pt.RotateY(5.0f);  // 按5°仰角初始化节点位置（单精度版）
     */
    void Vec3f::RotateY(float YTilt)
    {
        YTilt *= static_cast<float>(PI_) / 180.0f;
        float xo = x;
        float zo = z;
        x = xo * std::cos(YTilt) + zo * std::sin(YTilt);
        z = -xo * std::sin(YTilt) + zo * std::cos(YTilt);
    }
}