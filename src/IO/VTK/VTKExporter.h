#ifndef QAHSE_VTK_EXPORTER_H
#define QAHSE_VTK_EXPORTER_H

// =============================================================================
// Qahse Core - VTK Exporter
// 使用 vtu11 库输出风力机仿真几何数据 (.vtu + .pvd)
// 支持: 叶片面板、塔架、机舱/轮毂、叶片结构梁、
//       行架/下部结构、系泊缆、浮式平台、尾迹涡线/涡粒子
// =============================================================================

#include <string>
#include <vector>

class StrModel;  // global namespace

namespace Qahse {

class Turbine;
class Simulation;
class LinearWave;

// VTK 输出类型
enum class VTKOutputType : int {
    Surface   = 1,   // 叶片面板曲面
    Wireframe = 2    // 叶片面板线框
};

// =============================================================================
// VTKExporter -- 仿真 VTK 文件输出器
// =============================================================================
class VTKExporter {
public:
    VTKExporter();
    ~VTKExporter();

    /// 设置输出参数
    void configure(const std::string& outputDir,
                   double dtVTK,
                   double simDt,
                   VTKOutputType type = VTKOutputType::Wireframe,
                   int sideNum = 12);

    /// 在仿真步中调用: 检查是否该输出, 若是则导出当前帧
    void exportIfNeeded(int timeStep, double time,
                        Turbine* turbine, Simulation* sim);

    /// 仿真结束后调用: 写出 .pvd 时间序列集合文件
    void finalize(const std::string& simName);

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool en) { m_enabled = en; }

private:
    void exportFrame(int timeStep, double time,
                     Turbine* turbine, Simulation* sim);

    // ---- 各部件导出 ----
    // 叶片气动面板 (QUAD)
    void exportBladePanels(const std::string& filename, Turbine* turbine);
    // 塔架梁 (LINE)
    void exportTower(const std::string& filename, ::StrModel* sm);
    // 机舱 + 轮毂 (QUAD/TRIANGLE 简化几何)
    void exportNacelleHub(const std::string& filename, Turbine* turbine);
    // 叶片/支撑杆/力矩管结构梁 (LINE)
    void exportRotorStruct(const std::string& filename, ::StrModel* sm);
    // 行架/下部结构 (LINE) — 海上 jacket/truss
    void exportSubstructure(const std::string& filename, ::StrModel* sm);
    // 系泊缆 (LINE)
    void exportMoorings(const std::string& filename, ::StrModel* sm);
    // 浮式平台 (QUAD 简化几何)
    void exportPlatform(const std::string& filename, ::StrModel* sm);
    // 波浪自由液面 (QUAD 网格)
    void exportWaveSurface(const std::string& filename,
                           Simulation* sim,
                           Turbine* turbine,
                           double time);
    // 尾迹涡线 (LINE)
    void exportWakeLines(const std::string& filename, Turbine* turbine);
    // 尾迹涡粒子 (VERTEX)
    void exportWakeParticles(const std::string& filename, Turbine* turbine);

    // 辅助: 按 BodyType 导出 m_Bodies 子集 (梁→LINE)
    void exportBodiesByType(const std::string& filename, ::StrModel* sm,
                            const std::vector<int>& bodyTypes);
    // 辅助: 按 BodyType 导出 m_RigidBodies 子集 (刚体→LINE)
    void exportRigidBodiesByType(const std::string& filename, ::StrModel* sm,
                                 const std::vector<int>& bodyTypes);

    // 配置
    bool           m_enabled    = false;
    std::string    m_outputDir;
    double         m_dtVTK      = 0.5;
    double         m_simDt      = 0.01;
    int            m_everyNth   = 1;
    VTKOutputType  m_type       = VTKOutputType::Wireframe;
    int            m_sideNum    = 12;

    // PVD 时间序列记录
    struct PVDEntry {
        double time;
        std::string bladeFile;
        std::string towerFile;
        std::string nacelleHubFile;
        std::string rotorStructFile;
        std::string substructureFile;
        std::string mooringFile;
        std::string platformFile;
        std::string waveFile;
        std::string wakeLineFile;
        std::string wakePartFile;
    };
    std::vector<PVDEntry> m_pvdEntries;
    int m_frameCount = 0;
};

} // namespace Qahse

#endif // QAHSE_VTK_EXPORTER_H
