/*
// Qahse Core - VTK Exporter Implementation
// 使用 vtu11 库输出风力机仿真几何数据
// 支持海上+陆上: 叶片/塔架/机舱/轮毂/行架/系泊/浮台/尾迹
// ===
*/
#include "VTKExporter.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "VTKExporter.h"
#include "vtu11-cpp17.hpp"

#include "Qahse/Core/Turbine/Turbine.h"
#include "Qahse/Core/Turbine/TurbineSimData.h"
#include "Qahse/Core/Simulation/Simulation.h"
#include "Qahse/Core/Ocean/LinearWave.h"
#include "Qahse/Core/StructModel/StrModel.h"
#include "Qahse/Core/StructModel/StrObjects.h"
#include "Qahse/Core/Aero/VortexPanel.h"
#include "Qahse/Core/Aero/VortexLine.h"
#include "Qahse/Core/Aero/VortexNode.h"
#include "Qahse/Core/Aero/VortexParticle.h"
#include "Qahse/Core/Math/Vec3.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <algorithm>

namespace fs = std::filesystem;

namespace Qahse {


// Helper: generate tube surface (QUAD strips + optional triangle-fan end caps)
// Uses per-node deterministic frames: the frame at each node depends ONLY on
// the tangent direction at that node, ensuring that adjacent members sharing
// a joint produce identical rings (no fold/crease at junctions).
// Reference priority: Y → Z → X  (Y-first avoids flip for near-vertical columns)
// capStart/capEnd control whether triangle-fan caps are generated at each end.
// =============================================================================
static void generateTubeGeometry(
    std::vector<double>& pts,
    std::vector<vtu11::VtkIndexType>& conn,
    std::vector<vtu11::VtkIndexType>& offs,
    std::vector<vtu11::VtkCellType>& cellTypes,
    std::vector<double>& cellData,
    double dataVal,
    vtu11::VtkIndexType& ptIdx,
    const std::vector<Vec3>& line,
    const std::vector<double>& radii,
    int nSides,
    bool capStart = true,
    bool capEnd   = true)
{
    size_t N = line.size();
    if (N < 2 || radii.size() != N) return;

    // Reference directions in priority order: Y → Z → X
    static const Vec3 refDirs[3] = { Vec3(0,1,0), Vec3(0,0,1), Vec3(1,0,0) };

    for (size_t k = 0; k < N; k++) {
        Vec3 tangent;
        if (k == 0)        tangent = line[1] - line[0];
        else if (k == N-1) tangent = line[N-1] - line[N-2];
        else               tangent = line[k+1] - line[k-1];
        double tLen = tangent.VAbs();
        if (tLen < 1e-15) tangent = Vec3(0, 0, 1);
        else              tangent = tangent * (1.0 / tLen);

        Vec3 right(0,0,0);
        for (int ref = 0; ref < 3; ref++) {
            right = Vec3::cross(tangent, refDirs[ref]);
            if (right.VAbs() > 0.1) break;
        }
        right = Vec3::vnorm(right);
        Vec3 up = Vec3::vnorm(Vec3::cross(tangent, right));

        double r = radii[k];
        for (int i = 0; i < nSides; i++) {
            double angle = 2.0 * M_PI * i / nSides;
            Vec3 p = line[k] + right * (r * std::cos(angle))
                              + up    * (r * std::sin(angle));
            pts.push_back(p.x); pts.push_back(p.y); pts.push_back(p.z);
        }
    }

    // Side faces (QUAD strips)
    for (size_t k = 0; k + 1 < N; k++) {
        auto base0 = ptIdx + static_cast<vtu11::VtkIndexType>(k * nSides);
        auto base1 = ptIdx + static_cast<vtu11::VtkIndexType>((k + 1) * nSides);
        for (int i = 0; i < nSides; i++) {
            int next = (i + 1) % nSides;
            conn.push_back(base0 + i);
            conn.push_back(base0 + next);
            conn.push_back(base1 + next);
            conn.push_back(base1 + i);
            offs.push_back(static_cast<vtu11::VtkIndexType>(conn.size()));
            cellTypes.push_back(9); // VTK_QUAD
            cellData.push_back(dataVal);
        }
    }

    // End caps (triangle fans) — only where requested
    int nCapPts = 0;
    if (capStart) {
        vtu11::VtkIndexType centerIdx = ptIdx + static_cast<vtu11::VtkIndexType>(N * nSides + nCapPts);
        pts.push_back(line[0].x); pts.push_back(line[0].y); pts.push_back(line[0].z);
        nCapPts++;
        auto ring = ptIdx;
        for (int i = 0; i < nSides; i++) {
            int next = (i + 1) % nSides;
            conn.push_back(centerIdx);
            conn.push_back(ring + next);
            conn.push_back(ring + i);
            offs.push_back(static_cast<vtu11::VtkIndexType>(conn.size()));
            cellTypes.push_back(5); // VTK_TRIANGLE
            cellData.push_back(dataVal);
        }
    }
    if (capEnd) {
        vtu11::VtkIndexType centerIdx = ptIdx + static_cast<vtu11::VtkIndexType>(N * nSides + nCapPts);
        pts.push_back(line[N-1].x); pts.push_back(line[N-1].y); pts.push_back(line[N-1].z);
        nCapPts++;
        auto ring = ptIdx + static_cast<vtu11::VtkIndexType>((N - 1) * nSides);
        for (int i = 0; i < nSides; i++) {
            int next = (i + 1) % nSides;
            conn.push_back(centerIdx);
            conn.push_back(ring + i);
            conn.push_back(ring + next);
            offs.push_back(static_cast<vtu11::VtkIndexType>(conn.size()));
            cellTypes.push_back(5); // VTK_TRIANGLE
            cellData.push_back(dataVal);
        }
    }

    ptIdx += static_cast<vtu11::VtkIndexType>(N * nSides + nCapPts);
}

// =============================================================================
VTKExporter::VTKExporter() {}
VTKExporter::~VTKExporter() {}

// =============================================================================
void VTKExporter::configure(const std::string& outputDir,
                            double dtVTK,
                            double simDt,
                            VTKOutputType type,
                            int sideNum)
{
    m_outputDir = outputDir;
    m_dtVTK     = dtVTK;
    m_simDt     = simDt;
    m_type      = type;
    m_sideNum   = sideNum;

    m_everyNth = std::max(1, static_cast<int>(std::round(dtVTK / simDt)));

    fs::create_directories(m_outputDir);
    const char* dirs[] = {
        "blade", "tower", "nacelle_hub", "rotor_struct",
        "substructure", "mooring", "platform",
        "wave_surface", "wake_line", "wake_particle"
    };
    for (auto d : dirs)
        fs::create_directories(m_outputDir + "/" + d);

    m_pvdEntries.clear();
    m_frameCount = 0;
    m_enabled = true;

    std::cout << "[VTK] Enabled: output every " << m_everyNth
              << " steps (dt_vtk=" << dtVTK << "s), dir=" << m_outputDir << std::endl;
}

// =============================================================================
void VTKExporter::exportIfNeeded(int timeStep, double time,
                                  Turbine* turbine, Simulation* sim)
{
    if (!m_enabled) return;
    if (timeStep <= 0) return;
    if (timeStep % m_everyNth != 0 && timeStep != 1) return;

    exportFrame(timeStep, time, turbine, sim);
}

// =============================================================================
void VTKExporter::exportFrame(int timeStep, double time,
                               Turbine* turbine, Simulation* sim)
{
    if (!turbine || !turbine->m_simData) return;

    std::ostringstream ss;
    ss << std::setw(6) << std::setfill('0') << m_frameCount;
    std::string idx = ss.str();

    ::StrModel* sm = turbine->m_StrModel;
    PVDEntry entry;
    entry.time = time;

    // 1) 叶片气动面板
    entry.bladeFile = "blade/blade_" + idx + ".vtu";
    exportBladePanels(m_outputDir + "/" + entry.bladeFile, turbine);

    if (sm) {
        // 2) 塔架
        entry.towerFile = "tower/tower_" + idx + ".vtu";
        exportTower(m_outputDir + "/" + entry.towerFile, sm);

        // 3) 机舱 + 轮毂
        entry.nacelleHubFile = "nacelle_hub/nacelle_hub_" + idx + ".vtu";
        exportNacelleHub(m_outputDir + "/" + entry.nacelleHubFile, turbine);

        // 4) 叶片/支撑杆/力矩管结构梁
        entry.rotorStructFile = "rotor_struct/rotor_struct_" + idx + ".vtu";
        exportRotorStruct(m_outputDir + "/" + entry.rotorStructFile, sm);

        // 5) 行架/下部结构 (海上)
        if (sm->isSubStructure) {
            entry.substructureFile = "substructure/sub_" + idx + ".vtu";
            exportSubstructure(m_outputDir + "/" + entry.substructureFile, sm);
        }

        // 6) 系泊缆 (海上)
        if (sm->isFloating || !sm->m_Cables.empty()) {
            entry.mooringFile = "mooring/mooring_" + idx + ".vtu";
            exportMoorings(m_outputDir + "/" + entry.mooringFile, sm);
        }

        // 7) 浮式平台 (海上)
        if (sm->isFloating) {
            entry.platformFile = "platform/platform_" + idx + ".vtu";
            exportPlatform(m_outputDir + "/" + entry.platformFile, sm);
        }
    }

    // 8) 波浪自由液面
    if (sim && sim->m_linearWave && sim->m_bIsOffshore) {
        entry.waveFile = "wave_surface/wave_" + idx + ".vtu";
        exportWaveSurface(m_outputDir + "/" + entry.waveFile, sim, turbine, time);
    }

    // 9) 尾迹涡线
    if (!turbine->m_simData->m_WakeLine.empty()) {
        entry.wakeLineFile = "wake_line/wake_line_" + idx + ".vtu";
        exportWakeLines(m_outputDir + "/" + entry.wakeLineFile, turbine);
    }

    // 10) 尾迹涡粒子/节点
    if (!turbine->m_simData->m_WakeParticles.empty() ||
        !turbine->m_simData->m_WakeNode.empty()) {
        entry.wakePartFile = "wake_particle/wake_part_" + idx + ".vtu";
        exportWakeParticles(m_outputDir + "/" + entry.wakePartFile, turbine);
    }

    m_pvdEntries.push_back(entry);
    m_frameCount++;
}

// =============================================================================
// 叶片面板 → VTU (QUAD + CellData: Gamma, CL, CD, AoA, BladeID)
// =============================================================================
void VTKExporter::exportBladePanels(const std::string& filename, Turbine* turbine)
{
    auto* sd = turbine->m_simData;
    const auto& panels = sd->m_BladePanel;
    if (panels.empty()) return;

    size_t nCells  = panels.size() + sd->m_StrutPanel.size();

    std::vector<double>              points;
    std::vector<vtu11::VtkIndexType> connectivity;
    std::vector<vtu11::VtkIndexType> offsets;
    std::vector<vtu11::VtkCellType>  types;
    std::vector<double>              gammaData, clData, cdData, aoaData, bladeIdData;

    points.reserve(nCells * 4 * 3);
    connectivity.reserve(nCells * 4);
    offsets.reserve(nCells);
    types.reserve(nCells);

    auto addPanel = [&](VortexPanel* p) {
        vtu11::VtkIndexType base = static_cast<vtu11::VtkIndexType>(points.size() / 3);
        points.push_back(p->pLA->x); points.push_back(p->pLA->y); points.push_back(p->pLA->z);
        points.push_back(p->pTA->x); points.push_back(p->pTA->y); points.push_back(p->pTA->z);
        points.push_back(p->pTB->x); points.push_back(p->pTB->y); points.push_back(p->pTB->z);
        points.push_back(p->pLB->x); points.push_back(p->pLB->y); points.push_back(p->pLB->z);

        connectivity.push_back(base); connectivity.push_back(base+1);
        connectivity.push_back(base+2); connectivity.push_back(base+3);
        offsets.push_back(base + 4);
        types.push_back(9); // VTK_QUAD

        gammaData.push_back(p->m_Gamma);
        clData.push_back(p->m_CL);
        cdData.push_back(p->m_CD);
        aoaData.push_back(p->m_AoA);
        bladeIdData.push_back(static_cast<double>(p->fromBlade));
    };

    for (auto* p : panels)           addPanel(p);
    for (auto* p : sd->m_StrutPanel) addPanel(p);

    if (types.empty()) return;

    vtu11::Vtu11UnstructuredMesh mesh { points, connectivity, offsets, types };

    std::vector<vtu11::DataSetInfo> dataInfo = {
        { "Gamma",    vtu11::DataSetType::CellData, 1 },
        { "CL",       vtu11::DataSetType::CellData, 1 },
        { "CD",       vtu11::DataSetType::CellData, 1 },
        { "AoA",      vtu11::DataSetType::CellData, 1 },
        { "BladeID",  vtu11::DataSetType::CellData, 1 },
    };
    std::vector<vtu11::DataSetData> dataArrays = {
        gammaData, clData, cdData, aoaData, bladeIdData
    };

    vtu11::writeVtu(filename, mesh, dataInfo, dataArrays, "RawBinary");
}

// =============================================================================
// 辅助: 按 BodyType 导出 m_Bodies 子集 (柔性梁 → LINE)
// =============================================================================
void VTKExporter::exportBodiesByType(const std::string& filename, ::StrModel* sm,
                                      const std::vector<int>& bodyTypes)
{
    std::vector<double>              points;
    std::vector<vtu11::VtkIndexType> connectivity;
    std::vector<vtu11::VtkIndexType> offsets;
    std::vector<vtu11::VtkCellType>  types;
    std::vector<double>              bodyTypeData, bodyIdData;

    vtu11::VtkIndexType ptIdx = 0;

    for (Body* body : sm->m_Bodies) {
        bool match = false;
        for (int bt : bodyTypes)
            if (body->Btype == bt) { match = true; break; }
        if (!match || body->Nodes.size() < 2) continue;

        for (size_t n = 0; n < body->Nodes.size(); n++) {
            auto pos = body->Nodes[n]->GetPos();
            points.push_back(pos.x());
            points.push_back(pos.y());
            points.push_back(pos.z());
        }

        for (size_t n = 0; n + 1 < body->Nodes.size(); n++) {
            connectivity.push_back(ptIdx + static_cast<vtu11::VtkIndexType>(n));
            connectivity.push_back(ptIdx + static_cast<vtu11::VtkIndexType>(n + 1));
            offsets.push_back(static_cast<vtu11::VtkIndexType>(connectivity.size()));
            types.push_back(3); // VTK_LINE
            bodyTypeData.push_back(static_cast<double>(body->Btype));
            bodyIdData.push_back(static_cast<double>(body->ID));
        }

        ptIdx += static_cast<vtu11::VtkIndexType>(body->Nodes.size());
    }

    if (types.empty()) return;

    vtu11::Vtu11UnstructuredMesh mesh { points, connectivity, offsets, types };

    std::vector<vtu11::DataSetInfo> dataInfo = {
        { "BodyType", vtu11::DataSetType::CellData, 1 },
        { "BodyID",   vtu11::DataSetType::CellData, 1 },
    };
    std::vector<vtu11::DataSetData> dataArrays = { bodyTypeData, bodyIdData };

    vtu11::writeVtu(filename, mesh, dataInfo, dataArrays, "RawBinary");
}

// =============================================================================
// 辅助: 按 BodyType 导出 m_RigidBodies 子集 (刚体梁 → LINE)
// =============================================================================
void VTKExporter::exportRigidBodiesByType(const std::string& filename, ::StrModel* sm,
                                           const std::vector<int>& bodyTypes)
{
    std::vector<double>              points;
    std::vector<vtu11::VtkIndexType> connectivity;
    std::vector<vtu11::VtkIndexType> offsets;
    std::vector<vtu11::VtkCellType>  types;
    std::vector<double>              bodyTypeData, bodyIdData;

    vtu11::VtkIndexType ptIdx = 0;

    for (RigidBody* rb : sm->m_RigidBodies) {
        bool match = false;
        for (int bt : bodyTypes)
            if (rb->Btype == bt) { match = true; break; }
        if (!match || rb->Nodes.size() < 2) continue;

        for (size_t n = 0; n < rb->Nodes.size(); n++) {
            auto pos = rb->Nodes[n]->GetPos();
            points.push_back(pos.x());
            points.push_back(pos.y());
            points.push_back(pos.z());
        }

        for (size_t n = 0; n + 1 < rb->Nodes.size(); n++) {
            connectivity.push_back(ptIdx + static_cast<vtu11::VtkIndexType>(n));
            connectivity.push_back(ptIdx + static_cast<vtu11::VtkIndexType>(n + 1));
            offsets.push_back(static_cast<vtu11::VtkIndexType>(connectivity.size()));
            types.push_back(3);
            bodyTypeData.push_back(static_cast<double>(rb->Btype));
            bodyIdData.push_back(static_cast<double>(rb->ID));
        }

        ptIdx += static_cast<vtu11::VtkIndexType>(rb->Nodes.size());
    }

    if (types.empty()) return;

    vtu11::Vtu11UnstructuredMesh mesh { points, connectivity, offsets, types };

    std::vector<vtu11::DataSetInfo> dataInfo = {
        { "BodyType", vtu11::DataSetType::CellData, 1 },
        { "BodyID",   vtu11::DataSetType::CellData, 1 },
    };
    std::vector<vtu11::DataSetData> dataArrays = { bodyTypeData, bodyIdData };

    vtu11::writeVtu(filename, mesh, dataInfo, dataArrays, "RawBinary");
}

// =============================================================================
// 塔架 → VTU (tube surface QUAD, 使用实际锥形直径)
// =============================================================================
void VTKExporter::exportTower(const std::string& filename, ::StrModel* sm)
{
    std::vector<double>              pts;
    std::vector<vtu11::VtkIndexType> conn;
    std::vector<vtu11::VtkIndexType> offs;
    std::vector<vtu11::VtkCellType>  types;
    std::vector<double>              bodyIdData;
    vtu11::VtkIndexType ptIdx = 0;

    for (Body* body : sm->m_Bodies) {
        if (body->Btype != TOWER || body->Nodes.size() < 2) continue;

        std::vector<Vec3>   centerLine;
        std::vector<double> radii;
        for (size_t n = 0; n < body->Nodes.size(); n++) {
            auto p = body->Nodes[n]->GetPos();
            centerLine.emplace_back(p.x(), p.y(), p.z());
        }

        size_t nN = body->Nodes.size();
        size_t nE = body->Elements.size();
        radii.resize(nN, 1.0);
        if (nE > 0) {
            radii[0] = body->Elements[0]->diameter * 0.5;
            for (size_t i = 1; i + 1 < nN; i++) {
                size_t e0 = std::min(i - 1, nE - 1);
                size_t e1 = std::min(i,     nE - 1);
                radii[i] = (body->Elements[e0]->diameter
                          + body->Elements[e1]->diameter) * 0.25;
            }
            radii[nN - 1] = body->Elements[nE - 1]->diameter * 0.5;
        }
        for (auto& r : radii) if (r < 0.01) r = 1.0;

        generateTubeGeometry(pts, conn, offs, types, bodyIdData,
                             static_cast<double>(body->ID), ptIdx,
                             centerLine, radii, m_sideNum);
    }

    if (types.empty()) return;

    vtu11::Vtu11UnstructuredMesh mesh { pts, conn, offs, types };
    std::vector<vtu11::DataSetInfo> dataInfo = {
        { "BodyID", vtu11::DataSetType::CellData, 1 },
    };
    std::vector<vtu11::DataSetData> dataArrays = { bodyIdData };
    vtu11::writeVtu(filename, mesh, dataInfo, dataArrays, "RawBinary");
}

// =============================================================================
// 叶片/支撑杆/力矩管结构梁 → VTU (LINE)
// =============================================================================
void VTKExporter::exportRotorStruct(const std::string& filename, ::StrModel* sm)
{
    exportBodiesByType(filename, sm, { BLADE, STRUT, TORQUETUBE });
}

// =============================================================================
// 行架/下部结构 → VTU (tube surface QUAD, 使用实际构件直径)
// 包含 m_Bodies 中 SUBSTRUCTURE/SUBJOINT 类型
// 以及 m_RigidBodies 中的刚性构件
// Collinear same-diameter members are merged into single tubes to eliminate
// overlapping geometry (Z-fighting) at shared joints.
// =============================================================================
void VTKExporter::exportSubstructure(const std::string& filename, ::StrModel* sm)
{
    // --- Phase 1: collect all tube segments ---
    struct TubeSeg {
        std::vector<Vec3>   cl;
        std::vector<double> radii;
        double id;
    };
    std::vector<TubeSeg> segs;

    // 柔性体
    for (Body* body : sm->m_Bodies) {
        if (body->Btype != SUBSTRUCTURE && body->Btype != SUBJOINT) continue;
        if (body->Nodes.size() < 2) continue;

        TubeSeg seg;
        seg.id = static_cast<double>(body->ID);
        for (size_t n = 0; n < body->Nodes.size(); n++) {
            auto p = body->Nodes[n]->GetPos();
            seg.cl.emplace_back(p.x(), p.y(), p.z());
        }
        size_t nN = body->Nodes.size();
        size_t nE = body->Elements.size();
        seg.radii.resize(nN, 0.5);
        if (nE > 0) {
            seg.radii[0] = body->Elements[0]->diameter * 0.5;
            for (size_t i = 1; i + 1 < nN; i++) {
                size_t e0 = std::min(i - 1, nE - 1);
                size_t e1 = std::min(i,     nE - 1);
                seg.radii[i] = (body->Elements[e0]->diameter
                              + body->Elements[e1]->diameter) * 0.25;
            }
            seg.radii[nN - 1] = body->Elements[nE - 1]->diameter * 0.5;
        }
        for (auto& r : seg.radii) if (r < 0.01) r = 0.5;
        segs.push_back(std::move(seg));
    }

    // 刚体构件
    for (RigidBody* rb : sm->m_RigidBodies) {
        if (rb->Btype != SUBSTRUCTURE && rb->Btype != SUBJOINT) continue;
        if (rb->Nodes.size() < 2) continue;

        TubeSeg seg;
        seg.id = static_cast<double>(rb->ID);
        for (size_t n = 0; n < rb->Nodes.size(); n++) {
            auto p = rb->Nodes[n]->GetPos();
            seg.cl.emplace_back(p.x(), p.y(), p.z());
        }
        double r = rb->diameter * 0.5;
        if (r < 0.01) r = 0.5;
        seg.radii.assign(rb->Nodes.size(), r);
        segs.push_back(std::move(seg));
    }

    // --- Phase 2: merge collinear same-diameter chains ---
    // Two segments are merged if they share an endpoint (within tolerance)
    // AND have matching radii at the junction AND are roughly collinear.
    const double posTol = 0.2;   // position match tolerance [m]
    const double radTol = 0.5;   // radius match tolerance [m]
    const double cosLim = 0.95;  // collinearity: cos(angle) > 0.95 ≈ 18°

    auto segTangent = [](const TubeSeg& s, bool atEnd) -> Vec3 {
        size_t n = s.cl.size();
        Vec3 t = atEnd ? (s.cl[n-1] - s.cl[n-2]) : (s.cl[1] - s.cl[0]);
        double len = t.VAbs();
        return (len > 1e-10) ? t * (1.0 / len) : Vec3(0,0,1);
    };
    auto dist = [](const Vec3& a, const Vec3& b) {
        return (a - b).VAbs();
    };
    auto dotv = [](const Vec3& a, const Vec3& b) {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    };

    bool didMerge = true;
    while (didMerge) {
        didMerge = false;
        for (size_t i = 0; i < segs.size() && !didMerge; i++) {
            auto& A = segs[i];
            if (A.cl.empty()) continue;
            for (size_t j = i + 1; j < segs.size() && !didMerge; j++) {
                auto& B = segs[j];
                if (B.cl.empty()) continue;

                // Try 4 endpoint combinations: A.end↔B.start, A.end↔B.end,
                // A.start↔B.start, A.start↔B.end
                int mode = -1; // 0: A+B, 1: A+rev(B), 2: rev(B)+A via B+A, 3: B+rev(A) via rev(A.start↔B.end)
                double rA = 0, rB = 0;
                Vec3 tA, tB;

                if (dist(A.cl.back(), B.cl.front()) < posTol) {
                    rA = A.radii.back(); rB = B.radii.front();
                    tA = segTangent(A, true); tB = segTangent(B, false);
                    if (std::abs(rA - rB) < radTol && std::abs(dotv(tA, tB)) > cosLim)
                        mode = 0; // append B to A
                }
                if (mode < 0 && dist(A.cl.back(), B.cl.back()) < posTol) {
                    rA = A.radii.back(); rB = B.radii.back();
                    tA = segTangent(A, true); tB = segTangent(B, true);
                    if (std::abs(rA - rB) < radTol && std::abs(dotv(tA, tB)) > cosLim)
                        mode = 1; // append reversed B to A
                }
                if (mode < 0 && dist(A.cl.front(), B.cl.front()) < posTol) {
                    rA = A.radii.front(); rB = B.radii.front();
                    tA = segTangent(A, false); tB = segTangent(B, false);
                    if (std::abs(rA - rB) < radTol && std::abs(dotv(tA, tB)) > cosLim)
                        mode = 2; // prepend reversed B to A
                }
                if (mode < 0 && dist(A.cl.front(), B.cl.back()) < posTol) {
                    rA = A.radii.front(); rB = B.radii.back();
                    tA = segTangent(A, false); tB = segTangent(B, true);
                    if (std::abs(rA - rB) < radTol && std::abs(dotv(tA, tB)) > cosLim)
                        mode = 3; // prepend B to A
                }

                if (mode < 0) continue;

                // Perform merge
                if (mode == 0) {
                    // A.end ↔ B.start: append B (skip first point)
                    A.cl.insert(A.cl.end(), B.cl.begin() + 1, B.cl.end());
                    A.radii.insert(A.radii.end(), B.radii.begin() + 1, B.radii.end());
                } else if (mode == 1) {
                    // A.end ↔ B.end: append reversed B (skip last→first duplicate)
                    for (int k = (int)B.cl.size() - 2; k >= 0; k--) {
                        A.cl.push_back(B.cl[k]);
                        A.radii.push_back(B.radii[k]);
                    }
                } else if (mode == 2) {
                    // A.start ↔ B.start: prepend reversed B (skip first→first duplicate)
                    std::vector<Vec3> newCl;
                    std::vector<double> newR;
                    for (int k = (int)B.cl.size() - 1; k >= 1; k--) {
                        newCl.push_back(B.cl[k]);
                        newR.push_back(B.radii[k]);
                    }
                    newCl.insert(newCl.end(), A.cl.begin(), A.cl.end());
                    newR.insert(newR.end(), A.radii.begin(), A.radii.end());
                    A.cl = std::move(newCl);
                    A.radii = std::move(newR);
                } else { // mode == 3
                    // A.start ↔ B.end: prepend B (skip last→first duplicate)
                    std::vector<Vec3> newCl(B.cl.begin(), B.cl.end() - 1);
                    std::vector<double> newR(B.radii.begin(), B.radii.end() - 1);
                    newCl.insert(newCl.end(), A.cl.begin(), A.cl.end());
                    newR.insert(newR.end(), A.radii.begin(), A.radii.end());
                    A.cl = std::move(newCl);
                    A.radii = std::move(newR);
                }

                B.cl.clear();
                B.radii.clear();
                didMerge = true;
            }
        }
    }

    // --- Phase 2b: simplify straight tubes to 2 endpoints ---
    // Merged collinear tubes may still have closely-spaced intermediate nodes
    // (e.g., 0.06m cap member nodes) that cause frame computation noise.
    // For straight, nearly-constant-radius tubes, reduce to just start + end.
    for (auto& seg : segs) {
        if (seg.cl.size() <= 2) continue;
        Vec3 dir = seg.cl.back() - seg.cl.front();
        double totalLen = dir.VAbs();
        if (totalLen < 1e-10) continue;
        Vec3 udir = dir * (1.0 / totalLen);

        bool isStraight = true;
        bool constRadius = true;
        double rFirst = seg.radii.front();
        double rLast  = seg.radii.back();
        for (size_t k = 1; k + 1 < seg.cl.size(); k++) {
            Vec3 toK = seg.cl[k] - seg.cl.front();
            double proj = toK.x * udir.x + toK.y * udir.y + toK.z * udir.z;
            Vec3 perp = toK - udir * proj;
            if (perp.VAbs() > 0.1) { isStraight = false; break; }
            // Check radius varies significantly from linear interpolation
            double t = proj / totalLen;
            double rExpect = rFirst * (1.0 - t) + rLast * t;
            if (std::abs(seg.radii[k] - rExpect) > 0.3) { constRadius = false; break; }
        }
        if (isStraight && constRadius) {
            Vec3 pStart = seg.cl.front();
            Vec3 pEnd   = seg.cl.back();
            seg.cl.clear();
            seg.cl.push_back(pStart);
            seg.cl.push_back(pEnd);
            seg.radii = { rFirst, rLast };
        }
    }

    // --- Phase 3: determine cap visibility ---
    // At each endpoint, only the tube with the LARGEST radius gets a cap.
    // This prevents Z-fighting at diameter transitions (e.g., D=24m base ↔ D=12m upper).
    struct EndPt { Vec3 pos; double radius; size_t segIdx; bool isStart;
        EndPt(const Vec3& p, double r, size_t s, bool st) : pos(p), radius(r), segIdx(s), isStart(st) {}
    };
    std::vector<EndPt> endpoints;
    for (size_t s = 0; s < segs.size(); s++) {
        if (segs[s].cl.size() < 2) continue;
        endpoints.push_back(EndPt(segs[s].cl.front(), segs[s].radii.front(), s, true));
        endpoints.push_back(EndPt(segs[s].cl.back(),  segs[s].radii.back(),  s, false));
    }

    // For each segment, default caps to true
    std::vector<bool> capStartVec(segs.size(), true);
    std::vector<bool> capEndVec(segs.size(), true);

    for (size_t i = 0; i < endpoints.size(); i++) {
        for (size_t j = i + 1; j < endpoints.size(); j++) {
            if (endpoints[i].segIdx == endpoints[j].segIdx) continue;
            if ((endpoints[i].pos - endpoints[j].pos).VAbs() > posTol) continue;
            // Two different segments share this endpoint — suppress the smaller cap
            auto& epA = endpoints[i];
            auto& epB = endpoints[j];
            if (epA.radius <= epB.radius) {
                if (epA.isStart) capStartVec[epA.segIdx] = false;
                else             capEndVec[epA.segIdx]   = false;
            }
            if (epB.radius <= epA.radius) {
                if (epB.isStart) capStartVec[epB.segIdx] = false;
                else             capEndVec[epB.segIdx]   = false;
            }
        }
    }

    // --- Phase 4: generate VTK geometry from merged segments ---
    std::vector<double>              pts;
    std::vector<vtu11::VtkIndexType> conn;
    std::vector<vtu11::VtkIndexType> offs;
    std::vector<vtu11::VtkCellType>  types;
    std::vector<double>              memberIdData;
    vtu11::VtkIndexType ptIdx = 0;

    for (size_t s = 0; s < segs.size(); s++) {
        if (segs[s].cl.size() < 2) continue;
        generateTubeGeometry(pts, conn, offs, types, memberIdData,
                             segs[s].id, ptIdx, segs[s].cl, segs[s].radii,
                             m_sideNum, capStartVec[s], capEndVec[s]);
    }

    if (types.empty()) return;

    vtu11::Vtu11UnstructuredMesh mesh { pts, conn, offs, types };
    std::vector<vtu11::DataSetInfo> dataInfo = {
        { "MemberID", vtu11::DataSetType::CellData, 1 },
    };
    std::vector<vtu11::DataSetData> dataArrays = { memberIdData };
    vtu11::writeVtu(filename, mesh, dataInfo, dataArrays, "RawBinary");
}

// =============================================================================
// 系泊缆 → VTU (LINE)
// =============================================================================
void VTKExporter::exportMoorings(const std::string& filename, ::StrModel* sm)
{
    std::vector<double>              points;
    std::vector<vtu11::VtkIndexType> connectivity;
    std::vector<vtu11::VtkIndexType> offsets;
    std::vector<vtu11::VtkCellType>  types;
    std::vector<double>              cableIdData;

    vtu11::VtkIndexType ptIdx = 0;

    for (Cable* cable : sm->m_Cables) {
        if (cable->Nodes.size() < 2) continue;

        for (size_t n = 0; n < cable->Nodes.size(); n++) {
            auto pos = cable->Nodes[n]->GetPos();
            points.push_back(pos.x()); points.push_back(pos.y()); points.push_back(pos.z());
        }

        for (size_t n = 0; n + 1 < cable->Nodes.size(); n++) {
            connectivity.push_back(ptIdx + static_cast<vtu11::VtkIndexType>(n));
            connectivity.push_back(ptIdx + static_cast<vtu11::VtkIndexType>(n + 1));
            offsets.push_back(static_cast<vtu11::VtkIndexType>(connectivity.size()));
            types.push_back(3);
            cableIdData.push_back(static_cast<double>(cable->ID));
        }

        ptIdx += static_cast<vtu11::VtkIndexType>(cable->Nodes.size());
    }

    if (types.empty()) return;

    vtu11::Vtu11UnstructuredMesh mesh { points, connectivity, offsets, types };
    std::vector<vtu11::DataSetInfo> dataInfo = {
        { "CableID", vtu11::DataSetType::CellData, 1 },
    };
    std::vector<vtu11::DataSetData> dataArrays = { cableIdData };
    vtu11::writeVtu(filename, mesh, dataInfo, dataArrays, "RawBinary");
}

// =============================================================================
// 机舱 + 轮毂 → VTU
// 机舱: 简化长方体, 从偏航节点延伸到轮毂
// 轮毂: 简化圆锥 (TRIANGLE 扇面)
// =============================================================================
void VTKExporter::exportNacelleHub(const std::string& filename, Turbine* turbine)
{
    ::StrModel* sm = turbine->m_StrModel;
    if (!sm) return;

    // Tower-top position and axis from structural model
    Vec3 towerTop;
    double towerTopR = 0;
    Vec3 towerAxis(0, 0, 1);
    bool hasTowerTop = false;
    for (Body* body : sm->m_Bodies) {
        if (body->Btype == TOWER && !body->Nodes.empty()) {
            auto p = body->Nodes.back()->GetPos();
            towerTop = Vec3(p.x(), p.y(), p.z());
            towerAxis = body->Nodes.back()->coordS.X;  // TOWER type: coordS.X = beam axis
            hasTowerTop = true;
            if (!body->Elements.empty())
                towerTopR = body->Elements.back()->diameter * 0.5;
            break;
        }
    }
    if (!hasTowerTop && sm->m_YawNodeFree) {
        auto p = sm->m_YawNodeFree->GetPos();
        towerTop = Vec3(p.x(), p.y(), p.z());
        hasTowerTop = true;
        towerTopR = turbine->m_towerTopRadius;
    }

    // Shaft direction from hub fixed node (CONNECTOR type: coordS.X = shaft dir)
    Vec3 shaftDir(1, 0, 0);
    Vec3 hubPos;
    bool hasHub = (sm->m_HubNodeFixed != nullptr);
    if (hasHub) {
        auto p = sm->m_HubNodeFixed->GetPos();
        hubPos = Vec3(p.x(), p.y(), p.z());
        shaftDir = sm->m_HubNodeFixed->coordS.X;
    }

    if (!hasTowerTop && !hasHub) return;
    if (!hasHub) hubPos = towerTop + Vec3(5, 0, 0);
    if (!hasTowerTop) { towerTop = hubPos - Vec3(5, 0, 0); towerTopR = 1.5; }

    double taLen = towerAxis.VAbs();
    if (taLen > 1e-10) towerAxis = towerAxis * (1.0 / taLen);
    double sdLen = shaftDir.VAbs();
    if (sdLen > 1e-10) shaftDir = shaftDir * (1.0 / sdLen);

    // Nacelle forward = shaft direction projected onto plane ⊥ tower axis
    double dotUp = shaftDir.x * towerAxis.x + shaftDir.y * towerAxis.y + shaftDir.z * towerAxis.z;
    Vec3 nacFwd = shaftDir - towerAxis * dotUp;
    double nacFwdLen = nacFwd.VAbs();
    if (nacFwdLen < 1e-6) nacFwd = Vec3(1, 0, 0);
    else nacFwd = nacFwd * (1.0 / nacFwdLen);

    Vec3 right = Vec3::cross(nacFwd, towerAxis);
    double rLen = right.VAbs();
    if (rLen > 1e-10) right = right * (1.0 / rLen);

    // Nacelle sizing
    double overHang = std::abs(turbine->m_overHang);
    if (overHang < 1.0) overHang = 5.0;
    double nacLength = overHang * 2.5;
    double crossSize = towerTopR;
    if (crossSize < 0.1) crossSize = turbine->m_towerTopRadius;
    if (crossSize < 0.5) crossSize = overHang * 0.4;
    double hw = crossSize;
    double hh = crossSize * 0.8;

    // Nacelle box: sits on tower-top, extends along nacFwd
    Vec3 nacStart = towerTop - nacFwd * (nacLength * 0.25);
    Vec3 nacEnd   = towerTop + nacFwd * (nacLength * 0.75);

    std::vector<double>              points;
    std::vector<vtu11::VtkIndexType> connectivity;
    std::vector<vtu11::VtkIndexType> offsets;
    std::vector<vtu11::VtkCellType>  types;
    std::vector<double>              compData;

    auto addPt = [&](const Vec3& p) {
        points.push_back(p.x); points.push_back(p.y); points.push_back(p.z);
    };

    // Box corners: bottom at tower-top level, top extends upward along towerAxis
    Vec3 c[8];
    c[0] = nacStart + right * hw + towerAxis * (hh * 2.0);
    c[1] = nacStart - right * hw + towerAxis * (hh * 2.0);
    c[2] = nacStart - right * hw;
    c[3] = nacStart + right * hw;
    c[4] = nacEnd   + right * hw + towerAxis * (hh * 2.0);
    c[5] = nacEnd   - right * hw + towerAxis * (hh * 2.0);
    c[6] = nacEnd   - right * hw;
    c[7] = nacEnd   + right * hw;
    for (int i = 0; i < 8; i++) addPt(c[i]);

    vtu11::VtkIndexType faces[6][4] = {
        {0, 1, 5, 4}, {3, 7, 6, 2}, {0, 4, 7, 3},
        {1, 2, 6, 5}, {0, 3, 2, 1}, {4, 5, 6, 7},
    };
    for (int f = 0; f < 6; f++) {
        for (int v = 0; v < 4; v++) connectivity.push_back(faces[f][v]);
        offsets.push_back(static_cast<vtu11::VtkIndexType>(connectivity.size()));
        types.push_back(9);
        compData.push_back(0.0);
    }

    // Hub cone: use shaft direction directly from structural node
    Vec3 hubRight = Vec3::cross(shaftDir, towerAxis);
    double hrLen = hubRight.VAbs();
    if (hrLen > 1e-10) hubRight = hubRight * (1.0 / hrLen);
    Vec3 hubUp = Vec3::cross(hubRight, shaftDir);
    double huLen = hubUp.VAbs();
    if (huLen > 1e-10) hubUp = hubUp * (1.0 / huLen);

    int nSides = m_sideNum;
    double hubRadius = crossSize * 0.6;
    double hubLength = hubRadius * 1.8;
    Vec3 hubApex = hubPos + shaftDir * hubLength;

    vtu11::VtkIndexType apexIdx = static_cast<vtu11::VtkIndexType>(points.size() / 3);
    addPt(hubApex);

    vtu11::VtkIndexType ringStart = apexIdx + 1;
    for (int i = 0; i < nSides; i++) {
        double angle = 2.0 * M_PI * i / nSides;
        addPt(hubPos + hubRight * (hubRadius * std::cos(angle))
                      + hubUp    * (hubRadius * std::sin(angle)));
    }
    for (int i = 0; i < nSides; i++) {
        connectivity.push_back(apexIdx);
        connectivity.push_back(ringStart + i);
        connectivity.push_back(ringStart + (i + 1) % nSides);
        offsets.push_back(static_cast<vtu11::VtkIndexType>(connectivity.size()));
        types.push_back(5);
        compData.push_back(1.0);
    }

    vtu11::VtkIndexType centerIdx = static_cast<vtu11::VtkIndexType>(points.size() / 3);
    addPt(hubPos);
    for (int i = 0; i < nSides; i++) {
        connectivity.push_back(centerIdx);
        connectivity.push_back(ringStart + (i + 1) % nSides);
        connectivity.push_back(ringStart + i);
        offsets.push_back(static_cast<vtu11::VtkIndexType>(connectivity.size()));
        types.push_back(5);
        compData.push_back(1.0);
    }

    vtu11::Vtu11UnstructuredMesh mesh { points, connectivity, offsets, types };
    std::vector<vtu11::DataSetInfo> dataInfo = {
        { "Component", vtu11::DataSetType::CellData, 1 },
    };
    std::vector<vtu11::DataSetData> dataArrays = { compData };
    vtu11::writeVtu(filename, mesh, dataInfo, dataArrays, "RawBinary");
}

// =============================================================================
// 浮式平台 → VTU
// 如果已有详细下部结构输出则跳过 (substructure 已包含完整几何)
// =============================================================================
void VTKExporter::exportPlatform(const std::string& filename, ::StrModel* sm)
{
    if (!sm->isFloating) return;
    // 当存在下部结构时, substructure 输出已包含完整平台几何
    if (sm->isSubStructure) return;

    // 获取浮体位置
    Vec3 floatPos(0, 0, 0);
    bool hasPos = false;

    if (sm->floaterNP) {
        auto p = sm->floaterNP->GetPos();
        floatPos = Vec3(p.x(), p.y(), p.z());
        hasPos = true;
    } else if (!sm->potFlowBodyData.empty() && sm->potFlowBodyData[0].floaterHYDRO) {
        auto p = sm->potFlowBodyData[0].floaterHYDRO->GetPos();
        floatPos = Vec3(p.x(), p.y(), p.z());
        hasPos = true;
    } else if (!sm->potFlowBodyData.empty() && sm->potFlowBodyData[0].floaterMASS) {
        auto p = sm->potFlowBodyData[0].floaterMASS->GetPos();
        floatPos = Vec3(p.x(), p.y(), p.z());
        hasPos = true;
    }

    if (!hasPos) return;

    // 简化圆柱平台: 半径和高度启发估计
    double radius = 5.0;   // 默认 5m
    double height = 10.0;  // 默认 10m

    // 如果有 substructure 偏移信息, 用它来估计
    if (sm->subOffset.VAbs() > 0.1) {
        radius = std::max(radius, sm->subOffset.VAbs() * 0.5);
    }

    int nSides = m_sideNum;
    Vec3 upDir(0, 0, 1);

    std::vector<double>              points;
    std::vector<vtu11::VtkIndexType> connectivity;
    std::vector<vtu11::VtkIndexType> offsets;
    std::vector<vtu11::VtkCellType>  types;
    std::vector<double>              compData;

    Vec3 topCenter = floatPos + upDir * (height * 0.5);
    Vec3 botCenter = floatPos - upDir * (height * 0.5);

    auto addPt = [&](const Vec3& p) {
        points.push_back(p.x); points.push_back(p.y); points.push_back(p.z);
    };

    // 顶面圆环
    vtu11::VtkIndexType topStart = 0;
    for (int i = 0; i < nSides; i++) {
        double a = 2.0 * M_PI * i / nSides;
        addPt(topCenter + Vec3(radius * std::cos(a), radius * std::sin(a), 0));
    }
    // 底面圆环
    vtu11::VtkIndexType botStart = static_cast<vtu11::VtkIndexType>(nSides);
    for (int i = 0; i < nSides; i++) {
        double a = 2.0 * M_PI * i / nSides;
        addPt(botCenter + Vec3(radius * std::cos(a), radius * std::sin(a), 0));
    }
    // 顶面中心
    vtu11::VtkIndexType topCenterIdx = static_cast<vtu11::VtkIndexType>(points.size() / 3);
    addPt(topCenter);
    // 底面中心
    vtu11::VtkIndexType botCenterIdx = topCenterIdx + 1;
    addPt(botCenter);

    // 侧面 QUAD
    for (int i = 0; i < nSides; i++) {
        vtu11::VtkIndexType i0 = topStart + i;
        vtu11::VtkIndexType i1 = topStart + (i + 1) % nSides;
        vtu11::VtkIndexType i2 = botStart + (i + 1) % nSides;
        vtu11::VtkIndexType i3 = botStart + i;
        connectivity.push_back(i0); connectivity.push_back(i1);
        connectivity.push_back(i2); connectivity.push_back(i3);
        offsets.push_back(static_cast<vtu11::VtkIndexType>(connectivity.size()));
        types.push_back(9); // VTK_QUAD
        compData.push_back(0.0); // side
    }

    // 顶面三角扇
    for (int i = 0; i < nSides; i++) {
        vtu11::VtkIndexType i0 = topStart + i;
        vtu11::VtkIndexType i1 = topStart + (i + 1) % nSides;
        connectivity.push_back(topCenterIdx); connectivity.push_back(i0); connectivity.push_back(i1);
        offsets.push_back(static_cast<vtu11::VtkIndexType>(connectivity.size()));
        types.push_back(5); // VTK_TRIANGLE
        compData.push_back(1.0); // top
    }

    // 底面三角扇
    for (int i = 0; i < nSides; i++) {
        vtu11::VtkIndexType i0 = botStart + i;
        vtu11::VtkIndexType i1 = botStart + (i + 1) % nSides;
        connectivity.push_back(botCenterIdx); connectivity.push_back(i1); connectivity.push_back(i0);
        offsets.push_back(static_cast<vtu11::VtkIndexType>(connectivity.size()));
        types.push_back(5);
        compData.push_back(2.0); // bottom
    }

    vtu11::Vtu11UnstructuredMesh mesh { points, connectivity, offsets, types };
    std::vector<vtu11::DataSetInfo> dataInfo = {
        { "FaceType", vtu11::DataSetType::CellData, 1 },
    };
    std::vector<vtu11::DataSetData> dataArrays = { compData };
    vtu11::writeVtu(filename, mesh, dataInfo, dataArrays, "RawBinary");
}

// =============================================================================
// 波浪自由液面 -> VTU (QUAD + PointData: Eta)
// 说明:
// - 网格范围优先由结构模型 XY 包围盒估算，保证覆盖平台与系泊附近区域
// - 若缺少结构节点，退化为以轮毂/原点为中心的默认范围
// =============================================================================
void VTKExporter::exportWaveSurface(const std::string& filename,
                                    Simulation* sim,
                                    Turbine* turbine,
                                    double time)
{
    if (!sim || !sim->m_linearWave || !sim->m_bIsOffshore) return;

    double minX =  1e100, maxX = -1e100;
    double minY =  1e100, maxY = -1e100;
    bool hasBounds = false;

    auto updateBounds = [&](double x, double y) {
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
        hasBounds = true;
    };

    ::StrModel* sm = turbine ? turbine->m_StrModel : nullptr;
    if (sm) {
        for (Body* body : sm->m_Bodies) {
            for (const auto& node : body->Nodes) {
                auto p = node->GetPos();
                updateBounds(p.x(), p.y());
            }
        }
        for (RigidBody* rb : sm->m_RigidBodies) {
            for (const auto& node : rb->Nodes) {
                auto p = node->GetPos();
                updateBounds(p.x(), p.y());
            }
        }
        for (Cable* cable : sm->m_Cables) {
            for (const auto& node : cable->Nodes) {
                auto p = node->GetPos();
                updateBounds(p.x(), p.y());
            }
        }
    }

    if (!hasBounds) {
        double cx = 0.0, cy = 0.0;
        if (sm && sm->m_HubNodeFixed) {
            auto p = sm->m_HubNodeFixed->GetPos();
            cx = p.x();
            cy = p.y();
        }
        minX = cx - 200.0; maxX = cx + 200.0;
        minY = cy - 200.0; maxY = cy + 200.0;
    }

    double spanX = maxX - minX;
    double spanY = maxY - minY;
    if (spanX < 60.0) { minX -= (60.0 - spanX) * 0.5; maxX += (60.0 - spanX) * 0.5; spanX = 60.0; }
    if (spanY < 60.0) { minY -= (60.0 - spanY) * 0.5; maxY += (60.0 - spanY) * 0.5; spanY = 60.0; }

    double maxSpan = (spanX > spanY) ? spanX : spanY;
    double margin = 0.2 * maxSpan;
    if (margin < 40.0) margin = 40.0;
    minX -= margin; maxX += margin;
    minY -= margin; maxY += margin;
    spanX = maxX - minX;
    spanY = maxY - minY;

    int nx = static_cast<int>(std::round(spanX / 10.0)) + 1;
    int ny = static_cast<int>(std::round(spanY / 10.0)) + 1;
    if (nx < 25) nx = 25;
    if (ny < 25) ny = 25;
    if (nx > 121) nx = 121;
    if (ny > 121) ny = 121;
    double dx = spanX / static_cast<double>(nx - 1);
    double dy = spanY / static_cast<double>(ny - 1);

    std::vector<double>              points;
    std::vector<vtu11::VtkIndexType> connectivity;
    std::vector<vtu11::VtkIndexType> offsets;
    std::vector<vtu11::VtkCellType>  types;
    std::vector<double>              etaPointData;

    points.reserve(static_cast<size_t>(nx) * static_cast<size_t>(ny) * 3);
    etaPointData.reserve(static_cast<size_t>(nx) * static_cast<size_t>(ny));
    connectivity.reserve(static_cast<size_t>(nx - 1) * static_cast<size_t>(ny - 1) * 4);
    offsets.reserve(static_cast<size_t>(nx - 1) * static_cast<size_t>(ny - 1));
    types.reserve(static_cast<size_t>(nx - 1) * static_cast<size_t>(ny - 1));

    auto pointIndex = [nx](int i, int j) -> vtu11::VtkIndexType {
        return static_cast<vtu11::VtkIndexType>(j * nx + i);
    };

    for (int j = 0; j < ny; ++j) {
        double y = minY + static_cast<double>(j) * dy;
        for (int i = 0; i < nx; ++i) {
            double x = minX + static_cast<double>(i) * dx;
            double eta = sim->m_linearWave->GetElevation(Vec3(x, y, 0.0), static_cast<float>(time));
            points.push_back(x);
            points.push_back(y);
            points.push_back(eta);
            etaPointData.push_back(eta);
        }
    }

    for (int j = 0; j < ny - 1; ++j) {
        for (int i = 0; i < nx - 1; ++i) {
            vtu11::VtkIndexType p00 = pointIndex(i, j);
            vtu11::VtkIndexType p10 = pointIndex(i + 1, j);
            vtu11::VtkIndexType p11 = pointIndex(i + 1, j + 1);
            vtu11::VtkIndexType p01 = pointIndex(i, j + 1);
            connectivity.push_back(p00);
            connectivity.push_back(p10);
            connectivity.push_back(p11);
            connectivity.push_back(p01);
            offsets.push_back(static_cast<vtu11::VtkIndexType>(connectivity.size()));
            types.push_back(9); // VTK_QUAD
        }
    }

    if (types.empty()) return;

    vtu11::Vtu11UnstructuredMesh mesh { points, connectivity, offsets, types };
    std::vector<vtu11::DataSetInfo> dataInfo = {
        { "Eta", vtu11::DataSetType::PointData, 1 },
    };
    std::vector<vtu11::DataSetData> dataArrays = { etaPointData };
    vtu11::writeVtu(filename, mesh, dataInfo, dataArrays, "RawBinary");
}

// =============================================================================
// 尾迹涡线 → VTU
// CellData: Gamma, CoreSize, Length, LineVelocity(3), LineSpeed,
//           FromBlade, FromStation, FromTimeStep, IsTrailing, IsShed
// PointData: NodeVelocity(3), NodeSpeed
// =============================================================================
void VTKExporter::exportWakeLines(const std::string& filename, Turbine* turbine)
{
    auto* sd = turbine->m_simData;
    const auto& lines = sd->m_WakeLine;
    if (lines.empty()) return;

    size_t nCells = lines.size();

    std::vector<double>              points;
    std::vector<vtu11::VtkIndexType> connectivity;
    std::vector<vtu11::VtkIndexType> offsets;
    std::vector<vtu11::VtkCellType>  types;
    std::vector<double>              gammaData, coreSizeData, lengthData;
    std::vector<double>              lineVelData, lineSpeedData;
    std::vector<double>              fromBladeData, fromStationData, fromStepData;
    std::vector<double>              isTrailingData, isShedData;
    std::vector<double>              nodeVelData, nodeSpeedData;

    points.reserve(nCells * 2 * 3);

    vtu11::VtkIndexType ptIdx = 0;
    for (size_t i = 0; i < nCells; i++) {
        VortexLine* l = lines[i];

        points.push_back(l->pL->x); points.push_back(l->pL->y); points.push_back(l->pL->z);
        points.push_back(l->pT->x); points.push_back(l->pT->y); points.push_back(l->pT->z);

        Vec3 vL = l->pL ? l->pL->velocity : Vec3(0, 0, 0);
        Vec3 vT = l->pT ? l->pT->velocity : Vec3(0, 0, 0);
        nodeVelData.push_back(vL.x); nodeVelData.push_back(vL.y); nodeVelData.push_back(vL.z);
        nodeVelData.push_back(vT.x); nodeVelData.push_back(vT.y); nodeVelData.push_back(vT.z);
        nodeSpeedData.push_back(vL.VAbs());
        nodeSpeedData.push_back(vT.VAbs());

        connectivity.push_back(ptIdx); connectivity.push_back(ptIdx + 1);
        offsets.push_back(ptIdx + 2);
        types.push_back(3);

        gammaData.push_back(l->Gamma);
        coreSizeData.push_back(std::sqrt(std::abs(l->coreSizeSquared)));
        lengthData.push_back((l->pT && l->pL) ? ((*l->pT - *l->pL).VAbs()) : l->Length);

        lineVelData.push_back(l->velocity.x);
        lineVelData.push_back(l->velocity.y);
        lineVelData.push_back(l->velocity.z);
        lineSpeedData.push_back(l->velocity.VAbs());

        fromBladeData.push_back(static_cast<double>(l->fromBlade));
        fromStationData.push_back(static_cast<double>(l->fromStation));
        fromStepData.push_back(static_cast<double>(l->fromTimestep));
        isTrailingData.push_back(l->isTrailing ? 1.0 : 0.0);
        isShedData.push_back(l->isShed ? 1.0 : 0.0);

        ptIdx += 2;
    }

    vtu11::Vtu11UnstructuredMesh mesh { points, connectivity, offsets, types };
    std::vector<vtu11::DataSetInfo> dataInfo = {
        { "Gamma",    vtu11::DataSetType::CellData, 1 },
        { "CoreSize", vtu11::DataSetType::CellData, 1 },
        { "Length",   vtu11::DataSetType::CellData, 1 },
        { "LineVelocity", vtu11::DataSetType::CellData, 3 },
        { "LineSpeed", vtu11::DataSetType::CellData, 1 },
        { "FromBlade", vtu11::DataSetType::CellData, 1 },
        { "FromStation", vtu11::DataSetType::CellData, 1 },
        { "FromTimeStep", vtu11::DataSetType::CellData, 1 },
        { "IsTrailing", vtu11::DataSetType::CellData, 1 },
        { "IsShed", vtu11::DataSetType::CellData, 1 },
        { "NodeVelocity", vtu11::DataSetType::PointData, 3 },
        { "NodeSpeed", vtu11::DataSetType::PointData, 1 },
    };
    std::vector<vtu11::DataSetData> dataArrays = {
        gammaData, coreSizeData, lengthData,
        lineVelData, lineSpeedData,
        fromBladeData, fromStationData, fromStepData,
        isTrailingData, isShedData,
        nodeVelData, nodeSpeedData
    };
    vtu11::writeVtu(filename, mesh, dataInfo, dataArrays, "RawBinary");
}

// =============================================================================
// 尾迹涡粒子/节点 → VTU (VERTEX)
// PointData: PositionVelocity(3), Speed, Alpha(3), AlphaMag, Gamma, CoreSize,
//            Volume, Length, Dist, FromTimeStep, FromStation, IsTrail, SourceType
// 说明: 当 m_WakeParticles 为空但 m_WakeNode 非空时, 自动回退输出节点点云
// =============================================================================
void VTKExporter::exportWakeParticles(const std::string& filename, Turbine* turbine)
{
    auto* sd = turbine->m_simData;
    const auto& particles = sd->m_WakeParticles;
    const auto& wakeNodes = sd->m_WakeNode;
    if (particles.empty() && wakeNodes.empty()) return;

    size_t nPoints = particles.empty() ? wakeNodes.size() : particles.size();

    std::vector<double>              points;
    std::vector<vtu11::VtkIndexType> connectivity;
    std::vector<vtu11::VtkIndexType> offsets;
    std::vector<vtu11::VtkCellType>  types;
    std::vector<double>              velData, velXData, velYData, velZData, speedData;
    std::vector<double>              alphaData, alphaMagData, gammaData, coreSizeData;
    std::vector<double>              volumeData, lengthData, distData;
    std::vector<double>              fromStepData, fromStationData, isTrailData, sourceTypeData;

    points.reserve(nPoints * 3);

    if (!particles.empty()) {
        for (size_t i = 0; i < nPoints; i++) {
            VortexParticle* p = particles[i];

            points.push_back(p->position.x);
            points.push_back(p->position.y);
            points.push_back(p->position.z);

            connectivity.push_back(static_cast<vtu11::VtkIndexType>(i));
            offsets.push_back(static_cast<vtu11::VtkIndexType>(i + 1));
            types.push_back(1);

            velData.push_back(p->position_dt.x);
            velData.push_back(p->position_dt.y);
            velData.push_back(p->position_dt.z);
            velXData.push_back(p->position_dt.x);
            velYData.push_back(p->position_dt.y);
            velZData.push_back(p->position_dt.z);
            speedData.push_back(std::sqrt(
                p->position_dt.x * p->position_dt.x +
                p->position_dt.y * p->position_dt.y +
                p->position_dt.z * p->position_dt.z));

            alphaData.push_back(p->alpha.x);
            alphaData.push_back(p->alpha.y);
            alphaData.push_back(p->alpha.z);
            alphaMagData.push_back(std::sqrt(
                p->alpha.x * p->alpha.x +
                p->alpha.y * p->alpha.y +
                p->alpha.z * p->alpha.z));

            gammaData.push_back(p->Gamma);
            coreSizeData.push_back(p->coresize);
            volumeData.push_back(p->volume);
            lengthData.push_back(p->length);
            distData.push_back(p->dist);
            fromStepData.push_back(static_cast<double>(p->fromTimestep));
            fromStationData.push_back(static_cast<double>(p->fromStation));
            isTrailData.push_back(p->isTrail ? 1.0 : 0.0);
            sourceTypeData.push_back(1.0); // 1=VortexParticle
        }
    } else {
        for (size_t i = 0; i < nPoints; i++) {
            VortexNode* n = wakeNodes[i];

            points.push_back(n->x);
            points.push_back(n->y);
            points.push_back(n->z);

            connectivity.push_back(static_cast<vtu11::VtkIndexType>(i));
            offsets.push_back(static_cast<vtu11::VtkIndexType>(i + 1));
            types.push_back(1);

            velData.push_back(n->velocity.x);
            velData.push_back(n->velocity.y);
            velData.push_back(n->velocity.z);
            velXData.push_back(n->velocity.x);
            velYData.push_back(n->velocity.y);
            velZData.push_back(n->velocity.z);
            speedData.push_back(n->velocity.VAbs());

            alphaData.push_back(0.0);
            alphaData.push_back(0.0);
            alphaData.push_back(0.0);
            alphaMagData.push_back(0.0);

            gammaData.push_back(0.0);
            coreSizeData.push_back(0.0);
            volumeData.push_back(0.0);
            lengthData.push_back(0.0);
            distData.push_back(0.0);
            fromStepData.push_back(static_cast<double>(n->fromTimestep));
            fromStationData.push_back(static_cast<double>(n->fromStation));
            isTrailData.push_back(0.0);
            sourceTypeData.push_back(2.0); // 2=WakeNode fallback
        }
    }

    vtu11::Vtu11UnstructuredMesh mesh { points, connectivity, offsets, types };
    std::vector<vtu11::DataSetInfo> dataInfo = {
        { "PositionVelocity", vtu11::DataSetType::PointData, 3 },
        { "PositionVelocityX", vtu11::DataSetType::PointData, 1 },
        { "PositionVelocityY", vtu11::DataSetType::PointData, 1 },
        { "PositionVelocityZ", vtu11::DataSetType::PointData, 1 },
        { "Speed", vtu11::DataSetType::PointData, 1 },
        { "Alpha", vtu11::DataSetType::PointData, 3 },
        { "AlphaMag", vtu11::DataSetType::PointData, 1 },
        { "Gamma", vtu11::DataSetType::PointData, 1 },
        { "CoreSize", vtu11::DataSetType::PointData, 1 },
        { "Volume", vtu11::DataSetType::PointData, 1 },
        { "Length", vtu11::DataSetType::PointData, 1 },
        { "Dist", vtu11::DataSetType::PointData, 1 },
        { "FromTimeStep", vtu11::DataSetType::PointData, 1 },
        { "FromStation", vtu11::DataSetType::PointData, 1 },
        { "IsTrail", vtu11::DataSetType::PointData, 1 },
        { "SourceType", vtu11::DataSetType::PointData, 1 },
    };
    std::vector<vtu11::DataSetData> dataArrays = {
        velData, velXData, velYData, velZData, speedData,
        alphaData, alphaMagData, gammaData, coreSizeData,
        volumeData, lengthData, distData,
        fromStepData, fromStationData, isTrailData, sourceTypeData
    };
    vtu11::writeVtu(filename, mesh, dataInfo, dataArrays, "RawBinary");

    // if (particles.empty() && !wakeNodes.empty()) {
    //     std::cout << "[VTK] wake_particle fallback: exported " << wakeNodes.size()
    //               << " wake nodes (no VPM particles in current wake mode)" << std::endl;
    // }
}

// =============================================================================
// finalize -- 写出 .pvd 时间序列集合文件
// =============================================================================
void VTKExporter::finalize(const std::string& simName)
{
    if (!m_enabled || m_pvdEntries.empty()) return;

    auto writePVD = [&](const std::string& pvdName,
                        const std::function<std::string(const PVDEntry&)>& getFile) {
        // 检查是否有任何非空条目
        bool hasData = false;
        for (const auto& e : m_pvdEntries)
            if (!getFile(e).empty()) { hasData = true; break; }
        if (!hasData) return;

        std::string pvdPath = m_outputDir + "/" + pvdName + ".pvd";
        std::ofstream f(pvdPath);
        if (!f.is_open()) return;

        f << "<?xml version=\"1.0\"?>\n";
        f << "<VTKFile type=\"Collection\" version=\"0.1\">\n";
        f << "  <Collection>\n";
        for (const auto& e : m_pvdEntries) {
            std::string file = getFile(e);
            if (!file.empty()) {
                f << "    <DataSet timestep=\"" << std::fixed << std::setprecision(6)
                  << e.time << "\" file=\"" << file << "\"/>\n";
            }
        }
        f << "  </Collection>\n";
        f << "</VTKFile>\n";
    };

    writePVD(simName + "_blade",
             [](const PVDEntry& e) { return e.bladeFile; });
    writePVD(simName + "_tower",
             [](const PVDEntry& e) { return e.towerFile; });
    writePVD(simName + "_nacelle_hub",
             [](const PVDEntry& e) { return e.nacelleHubFile; });
    writePVD(simName + "_rotor_struct",
             [](const PVDEntry& e) { return e.rotorStructFile; });
    writePVD(simName + "_substructure",
             [](const PVDEntry& e) { return e.substructureFile; });
    writePVD(simName + "_mooring",
             [](const PVDEntry& e) { return e.mooringFile; });
    writePVD(simName + "_platform",
             [](const PVDEntry& e) { return e.platformFile; });
    writePVD(simName + "_wave_surface",
             [](const PVDEntry& e) { return e.waveFile; });
    writePVD(simName + "_wake_line",
             [](const PVDEntry& e) { return e.wakeLineFile; });
    writePVD(simName + "_wake_particle",
             [](const PVDEntry& e) { return e.wakePartFile; });

    std::cout << "[VTK] Finalized: " << m_frameCount << " frames written to "
              << m_outputDir << std::endl;
}

} // namespace Qahse
