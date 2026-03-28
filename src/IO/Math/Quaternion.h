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
#include "../../Params.h"
#include "Vec3.h"
#include "Vec3f.h"

namespace Qahse::IO::Math
{
    class Quaternion
    {
    public:
        Quaternion()
            : a(0.0), qx(0.0), qy(0.0), qz(0.0), ang_rad(0.0)
        {
            setmat();
        }

        void conj(Vec3 &vec)
        {
            v.x = vec.x;
            v.y = vec.y;
            v.z = vec.z;
            vec.x = 2.0 * ((m8 + m10) * v.x + (m6 - m4) * v.y + (m3 + m7) * v.z) + v.x;
            vec.y = 2.0 * ((m4 + m6) * v.x + (m5 + m10) * v.y + (m9 - m2) * v.z) + v.y;
            vec.z = 2.0 * ((m7 - m3) * v.x + (m2 + m9) * v.y + (m5 + m8) * v.z) + v.z;
        }

        void conj(Vec3f &vec)
        {
            v.x = vec.x;
            v.y = vec.y;
            v.z = vec.z;
            vec.x = static_cast<float>(2.0 * ((m8 + m10) * v.x + (m6 - m4) * v.y + (m3 + m7) * v.z) + v.x);
            vec.y = static_cast<float>(2.0 * ((m4 + m6) * v.x + (m5 + m10) * v.y + (m9 - m2) * v.z) + v.y);
            vec.z = static_cast<float>(2.0 * ((m7 - m3) * v.x + (m2 + m9) * v.y + (m5 + m8) * v.z) + v.z);
        }

        void conj(double &x, double &y, double &z)
        {
            v.x = x;
            v.y = y;
            v.z = z;
            x = 2.0 * ((m8 + m10) * v.x + (m6 - m4) * v.y + (m3 + m7) * v.z) + v.x;
            y = 2.0 * ((m4 + m6) * v.x + (m5 + m10) * v.y + (m9 - m2) * v.z) + v.y;
            z = 2.0 * ((m7 - m3) * v.x + (m2 + m9) * v.y + (m5 + m8) * v.z) + v.z;
        }

        void setmat()
        {
            m2 = a * qx;
            m3 = a * qy;
            m4 = a * qz;
            m5 = -qx * qx;
            m6 = qx * qy;
            m7 = qx * qz;
            m8 = -qy * qy;
            m9 = qy * qz;
            m10 = -qz * qz;
        }

        void set(double ang_deg, Vec3 const &v)
        {
            Vec3 n = v;
            n.Normalize();
            ang_rad = ang_deg * PI_ / 180.0;
            a = std::cos(ang_rad / 2.0);
            double sina = std::sin(ang_rad / 2.0);
            qx = n.x * sina;
            qy = n.y * sina;
            qz = n.z * sina;
            setmat();
        }

        void set(double ang_deg, Vec3f const &v)
        {
            Vec3f n = v;
            n.Normalize();
            ang_rad = ang_deg * PI_ / 180.0;
            a = std::cos(ang_rad / 2.0);
            double sina = std::sin(ang_rad / 2.0);
            qx = n.x * sina;
            qy = n.y * sina;
            qz = n.z * sina;
            setmat();
        }

    private:
        double ang_rad;
        double m2, m3, m4, m5, m6, m7, m8, m9, m10;
        Vec3 v;
        double a, qx, qy, qz;
    };
}
