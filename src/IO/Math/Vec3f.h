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
#include <cstdint>
#include <cstddef>
#include "Vec3.h"

namespace Qahse::IO::Math
{

    class Vec3i
    {
    public:
        int16_t x;
        int16_t y;
        int16_t z;

        Vec3i() : x(0), y(0), z(0) {}
    };

    class Vec3f
    {
    public:
        float x;
        float y;
        float z;

        // --- constructors ---

        Vec3f()
            : x(0.0f), y(0.0f), z(0.0f) {}

        Vec3f(float xi, float yi, float zi)
            : x(xi), y(yi), z(zi) {}

        // Construct from Eigen vector
        Vec3f(const Eigen::Vector3f &v)
            : x(v[0]), y(v[1]), z(v[2]) {}

        // --- Eigen interop (zero-copy map over x,y,z storage) ---

        Eigen::Map<Eigen::Vector3f> eigen()
        {
            return Eigen::Map<Eigen::Vector3f>(&x);
        }

        Eigen::Map<const Eigen::Vector3f> eigen() const
        {
            return Eigen::Map<const Eigen::Vector3f>(&x);
        }

        // --- operators (Eigen-accelerated) ---

        bool operator==(Vec3f const &V) const
        {
            return (eigen() - V.eigen()).squaredNorm() < 1e-9f;
        }

        // Cross-type assignment from double-precision Vec3
        void operator=(Vec3 const &T)
        {
            x = static_cast<float>(T.x);
            y = static_cast<float>(T.y);
            z = static_cast<float>(T.z);
        }

        void operator+=(Vec3f const &T) { eigen() += T.eigen(); }
        void operator+=(Vec3 const &T)
        {
            x += static_cast<float>(T.x);
            y += static_cast<float>(T.y);
            z += static_cast<float>(T.z);
        }
        void operator-=(Vec3f const &T) { eigen() -= T.eigen(); }
        void operator-=(Vec3 const &T)
        {
            x -= static_cast<float>(T.x);
            y -= static_cast<float>(T.y);
            z -= static_cast<float>(T.z);
        }
        void operator*=(float d) { eigen() *= d; }

        Vec3f operator*(float d) const
        {
            Vec3f r;
            r.eigen() = eigen() * d;
            return r;
        }
        Vec3f operator/(float d) const
        {
            Vec3f r;
            r.eigen() = eigen() / d;
            return r;
        }

        Vec3f operator+(Vec3f const &V) const
        {
            Vec3f r;
            r.eigen() = eigen() + V.eigen();
            return r;
        }
        Vec3f operator-(Vec3f const &V) const
        {
            Vec3f r;
            r.eigen() = eigen() - V.eigen();
            return r;
        }

        // cross product (Eigen SIMD-friendly)
        Vec3f operator*(Vec3f const &T) const
        {
            Vec3f r;
            r.eigen() = eigen().cross(T.eigen());
            return r;
        }

        // --- utilities (Eigen-accelerated) ---

        void Set(float x0, float y0, float z0)
        {
            x = x0;
            y = y0;
            z = z0;
        }
        void Set(Vec3f const &V) { eigen() = V.eigen(); }
        void Copy(Vec3f const &V) { eigen() = V.eigen(); }

        void Normalize()
        {
            float abs = VAbs();
            if (abs < 1.e-10f)
                return;
            eigen() /= abs;
        }

        float VAbs() const { return eigen().norm(); }
        float dot(Vec3f const &V) const { return eigen().dot(V.eigen()); }

        bool IsSame(Vec3f const &V) const
        {
            return (eigen() - V.eigen()).squaredNorm() < 1e-9f;
        }

        void Translate(Vec3f const &T) { eigen() += T.eigen(); }

        // rotation methods (implemented in Vec3f.cpp)
        void Rotate(Vec3f &O, Vec3f const &R, double Angle);
        void RotateN(Vec3f n, float NTilt);
        void RotateX(Vec3f const &O, float XTilt);
        void RotateY(Vec3f const &O, float YTilt);
        void RotateZ(Vec3f const &O, float ZTilt);
        void RotateY(float YTilt);
        void RotZ(float ZTilt);
        void RotY(float YTilt);
    };

    // Safety: Eigen::Map requires contiguous x,y,z storage
    static_assert(sizeof(Vec3f) == 3 * sizeof(float),
                  "Vec3f must be tightly packed for Eigen::Map");
}
