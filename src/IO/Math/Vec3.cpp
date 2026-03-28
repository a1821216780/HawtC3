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
#include "Vec3.h"
#include "Vec3f.h"
#include "Quaternion.h"
#include "../../Params.h"

namespace Qahse::IO::Math
{

    /**
     * @brief 从单精度浮点向量 Vec3f 构造双精度向量 Vec3
     *
     * 【功能】将 float 精度的三维向量转换为 double 精度版本，便于高精度计算。
     * 【物理意义】在仿真中，渲染/前处理阶段常用 float 向量节省内存，
     *             计算阶段需要升精度以避免数值误差累积。
     * 【参数】
     *   @param vecf  输入的单精度三维向量 (Vec3f)，包含 x、y、z 分量
     * 【使用案例】
     *   Vec3f vf(1.0f, 2.0f, 3.0f);
     *   Vec3  vd(vf);  // 转换为双精度
     */
    Vec3::Vec3(Vec3f vecf)
        : x(vecf.x), y(vecf.y), z(vecf.z) {}

    /**
     * @brief 绕过原点的任意轴旋转向量（使用四元数，角度以度为单位）
     *
     * 【功能】以原点为旋转中心，将当前向量绕轴 R 旋转 Angle 度。
     *         内部使用四元数共轭变换，避免万向锁问题。
     * 【物理意义】用于旋转叶片坐标、气动力方向等需要任意轴旋转的场合。
     *             在叶片俯仰（pitch）、倾斜（tilt）变换中频繁使用。
     * 【参数】
     *   @param R      旋转轴方向向量（单位向量）
     *   @param Angle  旋转角度，单位：度（°）
     * 【使用案例】
     *   Vec3 blade_tip(63.0, 0.0, 0.0);
     *   Vec3 pitch_axis(1.0, 0.0, 0.0);
     *   blade_tip.Rotate(pitch_axis, 5.0);  // 绕桨距轴旋转5度
     */
    void Vec3::Rotate(Vec3 const &R, double Angle)
    {
        Quaternion qt;
        qt.set(Angle, R);
        qt.conj(x, y, z);
    }

    /**
     * @brief 绕空间中任意点 O 和任意轴 R 旋转向量（角度以度为单位）
     *
     * 【功能】先将当前点平移到以 O 为原点的局部坐标系，执行四元数旋转，
     *         再平移回世界坐标系。
     * 【物理意义】用于旋转结构节点坐标，例如绕轮毂中心旋转叶片节点，
     *             或绕塔顶中心计算机舱偏航（yaw）后的点位。
     * 【参数】
     *   @param O      旋转中心点（参考点坐标）
     *   @param R      旋转轴方向单位向量
     *   @param Angle  旋转角度，单位：度（°）
     * 【使用案例】
     *   Vec3 node(10.0, 0.0, 80.0);
     *   Vec3 hub_center(0.0, 0.0, 80.0);
     *   Vec3 shaft_axis(1.0, 0.0, 0.0);
     *   node.Rotate(hub_center, shaft_axis, 120.0);  // 绕主轴旋转120°（3叶片间距）
     */
    void Vec3::Rotate(Vec3 &O, Vec3 const &R, double Angle)
    {
        Quaternion qt;
        qt.set(Angle, R);
        Vec3 OP;
        OP.x = x - O.x;
        OP.y = y - O.y;
        OP.z = z - O.z;
        qt.conj(OP);
        x = O.x + OP.x;
        y = O.y + OP.y;
        z = O.z + OP.z;
    }

    /**
     * @brief 使用 Rodrigues 公式绕任意单位法向量旋转（角度以度为单位）
     *
     * 【功能】将轴向量 n 归一化后，利用 Rodrigues 旋转公式对当前向量旋转
     *         NTilt 度。公式：r' = (n·r)n + (n×r)×n·cos(θ) + (n×r)·sin(θ)
     * 【物理意义】主要用于处理结构倾斜（tilt）和锥角（cone）安装偏差，
     *             如风力机主轴仰角（tilt ~5°）导致叶片平面的空间旋转。
     * 【参数】
     *   @param n      旋转轴方向（函数内部会被归一化，无需提前归一化）
     *   @param NTilt  旋转角度，单位：度（°）
     * 【使用案例】
     *   Vec3 force(0.0, 0.0, 1000.0);
     *   Vec3 tilt_axis(0.0, 1.0, 0.0);
     *   force.RotateN(tilt_axis, 5.0);  // 模拟5°主轴仰角
     */
    void Vec3::RotateN(Vec3 n, double NTilt)
    {
        n.Normalize();
        Vec3 r(x, y, z);
        r = n * (n.dot(r)) + (n * r) * n * std::cos(NTilt / 180.0 * PI_) + (n * r) * std::sin(NTilt / 180.0 * PI_);
        x = r.x;
        y = r.y;
        z = r.z;
    }

    /**
     * @brief 绕过点 O 的 X 轴旋转（角度以度为单位）
     *
     * 【功能】以点 O 为旋转中心，绕与全局 X 轴平行的轴旋转当前向量。
     *         旋转矩阵：Ry=y·cosθ-z·sinθ, Rz=y·sinθ+z·cosθ
     * 【物理意义】用于处理结构横向倾斜，如叶片在 YZ 平面内的挥舞/摆振
     *             耦合变形分析，或海洋平台侧倾（roll）运动。
     * 【参数】
     *   @param O      旋转中心点
     *   @param XTilt  绕 X 轴旋转角度，单位：度（°）
     * 【使用案例】
     *   Vec3 platform_node(5.0, 3.0, -20.0);
     *   Vec3 waterline_center(0.0, 0.0, 0.0);
     *   platform_node.RotateX(waterline_center, 2.0);  // 模拟2°平台横摇
     */
    void Vec3::RotateX(Vec3 const &O, double XTilt)
    {
        Vec3 OP;
        OP.x = x - O.x;
        OP.y = y - O.y;
        OP.z = z - O.z;
        XTilt *= PI_ / 180.0;
        y = O.y + OP.y * std::cos(XTilt) - OP.z * std::sin(XTilt);
        z = O.z + OP.y * std::sin(XTilt) + OP.z * std::cos(XTilt);
    }

    /**
     * @brief 绕过点 O 的 Y 轴旋转（角度以度为单位）
     *
     * 【功能】以点 O 为旋转中心，绕与全局 Y 轴平行的轴旋转当前向量。
     *         旋转矩阵：Rx=x·cosθ+z·sinθ, Rz=-x·sinθ+z·cosθ
     * 【物理意义】主要用于模拟风力机主轴仰角（shaft tilt），使旋转平面相对
     *             水平面倾斜约 5°，影响叶片尖端轨迹和气动入流计算。
     * 【参数】
     *   @param O      旋转中心点
     *   @param YTilt  绕 Y 轴旋转角度，单位：度（°）
     * 【使用案例】
     *   Vec3 blade_root(0.0, 0.0, 80.0);
     *   Vec3 hub_center(0.0, 0.0, 80.0);
     *   blade_root.RotateY(hub_center, 5.0);  // 模拟5°主轴仰角
     */
    void Vec3::RotateY(Vec3 const &O, double YTilt)
    {
        Vec3 OP;
        OP.x = x - O.x;
        OP.y = y - O.y;
        OP.z = z - O.z;
        YTilt *= PI_ / 180.0;
        x = O.x + OP.x * std::cos(YTilt) + OP.z * std::sin(YTilt);
        z = O.z - OP.x * std::sin(YTilt) + OP.z * std::cos(YTilt);
    }

    /**
     * @brief 绕过点 O 的 Z 轴旋转（角度以度为单位）
     *
     * 【功能】以点 O 为旋转中心，绕与全局 Z 轴平行的轴旋转当前向量。
     *         用于水平面内的对称性旋转（如3叶片120°间距布置）。
     * 【物理意义】风力机偏航（yaw）旋转的主要算子，将机舱/叶片坐标系
     *             绕塔顶中心在水平面内转动以对准风向。
     * 【参数】
     *   @param O      旋转中心点（通常为塔顶或整机重心在水平面的投影）
     *   @param ZTilt  绕 Z 轴旋转角度，单位：度（°）
     * 【使用案例】
     *   Vec3 blade_tip(63.0, 0.0, 90.0);
     *   Vec3 tower_top(0.0, 0.0, 90.0);
     *   blade_tip.RotateZ(tower_top, 30.0);  // 偏航30°
     */
    void Vec3::RotateZ(Vec3 const &O, double ZTilt)
    {
        Vec3 OP;
        OP.x = x - O.x;
        OP.y = y - O.y;
        OP.z = z - O.z;
        ZTilt *= PI_ / 180.0;
        x = O.x + OP.x * std::cos(ZTilt) + OP.y * std::sin(ZTilt);
        y = O.y - OP.x * std::sin(ZTilt) + OP.y * std::cos(ZTilt);
    }

    /**
     * @brief 绕原点的 Z 轴原地旋转（角度以弧度为单位）
     *
     * 【功能】在 XY 平面内对当前向量进行旋转，不涉及 Z 分量变化。
     *         这是 RotateZ(O, ZTilt) 的弧度版本，性能更高（无度到弧度转换）。
     * 【物理意义】在尾流计算中用于快速旋转涡粒子/涡线在水平面内的位置，
     *             以及偏航控制计算中的偏航角速度积分更新。
     * 【参数】
     *   @param ZTilt  绕 Z 轴旋转角度，单位：弧度（rad）
     * 【使用案例】
     *   Vec3 vortex_pos(10.0, 5.0, 0.0);
     *   vortex_pos.RotZ(0.017453);  // 旋转1°（=π/180 rad）
     */
    void Vec3::RotZ(double ZTilt)
    {
        Vec3 OP;
        OP.x = x;
        OP.y = y;
        x = OP.x * std::cos(ZTilt) - OP.y * std::sin(ZTilt);
        y = OP.x * std::sin(ZTilt) + OP.y * std::cos(ZTilt);
    }

    /**
     * @brief 绕原点的 X 轴原地旋转（角度以弧度为单位）
     *
     * 【功能】在 YZ 平面内对当前向量进行旋转，不涉及 X 分量变化。
     *         弧度版本，适合高频调用的内核计算循环。
     * 【物理意义】在叶片结构动力学中，用于将叶片根部局部坐标系的
     *             力/弯矩向量转换到全局坐标，处理挥舞（flapwise）方向载荷。
     * 【参数】
     *   @param XTilt  绕 X 轴旋转角度，单位：弧度（rad）
     * 【使用案例】
     *   Vec3 flap_load(0.0, 500.0, 0.0);
     *   flap_load.RotX(0.0873);  // 转换5°锥角（precone）影响
     */
    void Vec3::RotX(double XTilt)
    {
        Vec3 OP;
        OP.y = y;
        OP.z = z;
        y = OP.y * std::cos(XTilt) - OP.z * std::sin(XTilt);
        z = OP.y * std::sin(XTilt) + OP.z * std::cos(XTilt);
    }

    /**
     * @brief 绕原点的 Y 轴原地旋转（角度以弧度为单位）
     *
     * 【功能】在 XZ 平面内对当前向量进行旋转，不涉及 Y 分量变化。
     *         弧度版本，适合高频调用的仿真时间推进循环。
     * 【物理意义】用于主轴仰角（tilt ~5°）导致的叶片旋转平面倾斜修正，
     *             以及涡流尾迹粒子在时间步内绕 Y 轴的位置更新。
     * 【参数】
     *   @param YTilt  绕 Y 轴旋转角度，单位：弧度（rad）
     * 【使用案例】
     *   Vec3 wake_particle(20.0, 0.0, 5.0);
     *   wake_particle.RotY(0.0873);  // 5°仰角修正
     */
    void Vec3::RotY(double YTilt)
    {
        Vec3 OP;
        OP.x = x;
        OP.z = z;
        x = OP.x * std::cos(YTilt) + OP.z * std::sin(YTilt);
        z = -OP.x * std::sin(YTilt) + OP.z * std::cos(YTilt);
    }

    /**
     * @brief 绕原点的 Y 轴旋转（角度以度为单位，原地修改）
     *
     * 【功能】与 RotY(rad) 功能相同，但输入角度单位为度（°），
     *         函数内部自动完成度到弧度转换。
     * 【物理意义】在用户界面层和文件解析层，角度多以度为单位输入，
     *             此函数用于仰角/锥角等参数的初始化接口。
     * 【参数】
     *   @param YTilt  绕 Y 轴旋转角度，单位：度（°）
     * 【使用案例】
     *   Vec3 hub_pos(0.0, 0.0, 90.0);
     *   hub_pos.RotateY(5.0);  // 按5°仰角初始化轮毂位置
     */
    void Vec3::RotateY(double YTilt)
    {
        YTilt *= PI_ / 180.0;
        double xo = x;
        double zo = z;
        x = xo * std::cos(YTilt) + zo * std::sin(YTilt);
        z = -xo * std::sin(YTilt) + zo * std::cos(YTilt);
    }
}