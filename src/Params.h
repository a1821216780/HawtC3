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



// Physical constants
#define PI_                     3.14159265358979323846
#define KINVISCAIR              1.647e-05
#define DENSITYAIR              1.225
#define KINVISCWATER            1.307e-6
#define DENSITYWATER            1025
#define GRAVITY                 9.80665
#define TINYVAL                 1.0e-10
#define ZERO_MASS               1.0      // 原值 1e-5 导致 KKT 矩阵条件数 ~7e15, SparseQR 无法正确执行约束

// Airfoil discretization parameters
#define IQX                     302
#define IBX                     604

// Blade analysis parameters
#define MAXBLADESTATIONS        200

// Controller array sizes
#define arraySizeTUB            550
#define arraySizeBLADED         550
#define arraySizeDTU            100

