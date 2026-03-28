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
#include <cstddef>
#include <Eigen/Dense>

namespace Qahse::IO::Math
{

    class Vec3f;

    class Vec3
    {
    public:
        double x;
        double y;
        double z;

        // --- constructors ---

        Vec3()
            : x(0.0), y(0.0), z(0.0) {}

        Vec3(double xi, double yi, double zi)
            : x(xi), y(yi), z(zi) {}

        Vec3(Vec3f vecf);

        // Construct from Eigen vector
        Vec3(const Eigen::Vector3d &v)
            : x(v[0]), y(v[1]), z(v[2]) {}

        // --- Eigen interop (zero-copy map over x,y,z storage) ---

        Eigen::Map<Eigen::Vector3d> eigen()
        {
            return Eigen::Map<Eigen::Vector3d>(&x);
        }

        Eigen::Map<const Eigen::Vector3d> eigen() const
        {
            return Eigen::Map<const Eigen::Vector3d>(&x);
        }

        // --- operators (Eigen-accelerated) ---

        bool operator==(Vec3 const &V) const
        {
            return (eigen() - V.eigen()).squaredNorm() < 1e-9;
        }

        void operator+=(Vec3 const &T) { eigen() += T.eigen(); }
        void operator-=(Vec3 const &T) { eigen() -= T.eigen(); }
        void operator*=(double d) { eigen() *= d; }

        Vec3 operator*(double d) const
        {
            Vec3 r;
            r.eigen() = eigen() * d;
            return r;
        }
        Vec3 operator/(double d) const
        {
            Vec3 r;
            r.eigen() = eigen() / d;
            return r;
        }

        Vec3 operator+(Vec3 const &V) const
        {
            Vec3 r;
            r.eigen() = eigen() + V.eigen();
            return r;
        }
        Vec3 operator-(Vec3 const &V) const
        {
            Vec3 r;
            r.eigen() = eigen() - V.eigen();
            return r;
        }

        // cross product (Eigen SIMD-friendly)
        Vec3 operator*(Vec3 const &T) const
        {
            Vec3 r;
            r.eigen() = eigen().cross(T.eigen());
            return r;
        }

        // --- utilities (Eigen-accelerated) ---

        void Set(double x0, double y0, double z0)
        {
            x = x0;
            y = y0;
            z = z0;
        }
        void Set(Vec3 const &V) { eigen() = V.eigen(); }
        void Copy(Vec3 const &V) { eigen() = V.eigen(); }

        void Normalize()
        {
            double abs = VAbs();
            if (abs < 1.e-10)
                return;
            eigen() /= abs;
        }

        double VAbs() const { return eigen().norm(); }
        double dot(Vec3 const &V) const { return eigen().dot(V.eigen()); }
        Vec3 cross(Vec3 const &V) const { return eigen().cross(V.eigen()); }
        bool IsSame(Vec3 const &V) const
        {
            return (eigen() - V.eigen()).squaredNorm() < 1e-20;
        }
        static Vec3 cross(const Vec3 &a, const Vec3 &b)
        {
            return a.cross(b);
        }
        static Vec3 vnorm(const Vec3 &v)
        {
            double len = v.VAbs();
            return (len > 1e-15) ? v * (1.0 / len) : Vec3(0, 0, 0);
        }
        void Translate(Vec3 const &T) { eigen() += T.eigen(); }

        // rotation methods (implemented in Vec3.cpp)
        void Rotate(Vec3 const &R, double Angle);
        void Rotate(Vec3 &O, Vec3 const &R, double Angle);
        void RotateN(Vec3 n, double NTilt);
        void RotateX(Vec3 const &O, double XTilt);
        void RotateY(Vec3 const &O, double YTilt);
        void RotateZ(Vec3 const &O, double ZTilt);
        void RotateY(double YTilt);
        void RotX(double XTilt);
        void RotY(double YTilt);
        void RotZ(double ZTilt);
    };

    // Safety: Eigen::Map requires contiguous x,y,z storage
    static_assert(sizeof(Vec3) == 3 * sizeof(double),
                  "Vec3 must be tightly packed for Eigen::Map");
}
