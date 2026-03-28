#pragma once
#include "PCS_Types.h"
#include "FEM/PCS_FEM_Types.h"
#include "../IO/IO/InputFileParser.h"
#include "../IO/IO/StringHelper.h"
#include "../IO/IO/CheckError.h"
#include "../IO/Log/LogHelper.h"

#include <vector>
#include <string>
#include <sstream>
#include <tuple>
#include <algorithm>
#include <limits>
#include <Eigen/Dense>

namespace Qahse::PcsL
{
    using Qahse::IO::IO::CheckError;
    using Qahse::IO::IO::InputFileParser;
    using Qahse::IO::IO::StringHelper;
    using Qahse::IO::Log::LogHelper;

    inline std::vector<int> FindAllDataLineIndexes(const std::vector<std::string> &lines,
                                                   const std::string &keyword)
    {
        std::vector<int> result;
        for (int i = 0; i < static_cast<int>(lines.size()); ++i)
        {
            if (InputFileParser::MatchKeywordInDataLine(lines[i], keyword))
                result.push_back(i);
        }
        return result;
    }

    inline std::vector<CompositeMaterial> ReadMaterials(const std::string &matPath, int count, bool isFEM)
    {
        CheckError::Filexists(matPath);
        auto lines = StringHelper::ReadAllLines(matPath);

        std::vector<CompositeMaterial> mats(count);
        for (int i = 0; i < count; i++)
        {
            int row = 3 + i;
            if (row >= (int)lines.size())
            {
                LogHelper::ErrorLog("材料文件中材料数量不足: " + matPath, "", "", 20, "ReadMaterials");
                break;
            }
            std::istringstream iss(lines[row]);
            CompositeMaterial &m = mats[i];

            if (isFEM)
            {
                iss >> m.Id >> m.E1 >> m.E2 >> m.E3 >> m.G12 >> m.G13 >> m.G23 >> m.Nu12 >> m.Nu13 >> m.Nu23 >> m.Density;
                std::string name;
                if (iss >> name)
                    m.Name = name;
                iss >> m.S1t >> m.S2t >> m.S3t >> m.S1c >> m.S2c >> m.S3c >> m.S12s >> m.S23s >> m.S13s;
            }
            else
            {
                iss >> m.Id >> m.E1 >> m.E2 >> m.G12 >> m.Nu12 >> m.Density;
                std::string name;
                if (iss >> name)
                    m.Name = name;
            }
        }
        std::sort(mats.begin(), mats.end(), [](const CompositeMaterial &a, const CompositeMaterial &b)
                  { return a.Id < b.Id; });
        return mats;
    }

    inline Eigen::MatrixXd ReadMatrixBlock(const std::vector<std::string> &lines, int startLine, int numRows, int numCols)
    {
        Eigen::MatrixXd mat(numRows, numCols);
        for (int i = 0; i < numRows; i++)
        {
            std::istringstream iss(lines[startLine + i]);
            for (int j = 0; j < numCols; j++)
            {
                double v = 0;
                iss >> v;
                mat(i, j) = v;
            }
        }
        return mat;
    }

    inline Eigen::MatrixXd ReadMatrixBlockAuto(const std::vector<std::string> &lines, int startLine, int numRows)
    {
        std::istringstream probe(lines[startLine]);
        std::vector<double> vals;
        double v;
        while (probe >> v)
            vals.push_back(v);
        int ncol = (int)vals.size();
        Eigen::MatrixXd mat(numRows, ncol);
        for (int j = 0; j < ncol; j++)
            mat(0, j) = vals[j];
        for (int i = 1; i < numRows; i++)
        {
            std::istringstream iss(lines[startLine + i]);
            for (int j = 0; j < ncol; j++)
            {
                double val = 0;
                iss >> val;
                mat(i, j) = val;
            }
        }
        return mat;
    }

    inline AirfoilShape ReadAirfoilFile(const std::string &path)
    {
        CheckError::Filexists(path);
        auto lines = StringHelper::ReadAllLines(path);
        AirfoilShape af;
        af.N_af_nodes = InputFileParser::ParseInt(lines[0]);
        auto mat = ReadMatrixBlock(lines, 4, af.N_af_nodes, 2);
        af.X = mat.col(0);
        af.Y = mat.col(1);

        int splitIdx = 0;
        double xmax = af.X.maxCoeff();
        for (int i = 0; i < af.N_af_nodes; i++)
        {
            if (af.X(i) == xmax)
            {
                splitIdx = i;
                break;
            }
        }

        af.Xu = af.X.head(splitIdx + 1);
        af.Yu = af.Y.head(splitIdx + 1);
        int nLow = af.N_af_nodes - splitIdx;
        af.Xl = af.X.tail(nLow).reverse();
        af.Yl = af.Y.tail(nLow).reverse();
        return af;
    }

    inline std::tuple<SurfaceConfiguration, SurfaceConfiguration, SurfaceConfiguration> ReadLayupFile(const std::string &path, int Nweb)
    {
        CheckError::Filexists(path);
        auto lines = StringHelper::ReadAllLines(path);
        auto findAll = [&](const std::string &kw)
        {
            return FindAllDataLineIndexes(lines, kw);
        };

        SurfaceConfiguration top;
        {
            top.N_scts = InputFileParser::ReadKeywordInt(lines, "N_scts(1)", path, 0, true, true);
        }
        {
            auto xnodes = findAll("xsec_node");
            if (!xnodes.empty())
            {
                top.xsec_node = InputFileParser::ParseDoubleArray(lines[xnodes[0] + 1]);
            }
        }
        auto sectIdxs = findAll("Sect_num");
        top.Sector.resize(top.N_scts);
        for (int i = 0; i < top.N_scts; i++)
        {
            auto &sec = top.Sector[i];
            std::istringstream iss(lines[sectIdxs[i] + 1]);
            iss >> sec.Sect_num >> sec.N_laminas;
            sec.Laminae.resize(sec.N_laminas);
            for (int j = 0; j < sec.N_laminas; j++)
            {
                auto &lam = sec.Laminae[j];
                std::istringstream lss(lines[sectIdxs[i] + 6 + j]);
                lss >> lam.LayerId >> lam.PlyCount >> lam.Thickness >> lam.FiberOrientation >> lam.MaterialId;
                std::string disp;
                if (lss >> disp)
                    lam.MatDisp = disp;
            }
        }

        SurfaceConfiguration low;
        {
            low.N_scts = InputFileParser::ReadKeywordInt(lines, "N_scts(2)", path, 0, true, true);
        }
        {
            auto xnodes = findAll("xsec_node");
            if (xnodes.size() > 1)
            {
                low.xsec_node = InputFileParser::ParseDoubleArray(lines[xnodes[1] + 1]);
            }
        }
        low.Sector.resize(low.N_scts);
        for (int i = 0; i < low.N_scts; i++)
        {
            auto &sec = low.Sector[i];
            int si = top.N_scts + i;
            std::istringstream iss(lines[sectIdxs[si] + 1]);
            iss >> sec.Sect_num >> sec.N_laminas;
            sec.Laminae.resize(sec.N_laminas);
            for (int j = 0; j < sec.N_laminas; j++)
            {
                auto &lam = sec.Laminae[j];
                std::istringstream lss(lines[sectIdxs[si] + 6 + j]);
                lss >> lam.LayerId >> lam.PlyCount >> lam.Thickness >> lam.FiberOrientation >> lam.MaterialId;
                std::string disp;
                if (lss >> disp)
                    lam.MatDisp = disp;
            }
        }

        SurfaceConfiguration web;
        web.N_scts = Nweb;
        web.Sector.resize(Nweb);
        if (Nweb > 0)
        {
            auto webIdxs = findAll("web_num");
            auto wlamIdxs = findAll("wlam_num");
            for (int i = 0; i < Nweb; i++)
            {
                auto &sec = web.Sector[i];
                std::istringstream iss(lines[webIdxs[i] + 1]);
                iss >> sec.Sect_num >> sec.N_laminas;
                sec.Laminae.resize(sec.N_laminas);
                for (int j = 0; j < sec.N_laminas; j++)
                {
                    auto &lam = sec.Laminae[j];
                    std::istringstream lss(lines[wlamIdxs[i] + j + 1]);
                    lss >> lam.LayerId >> lam.PlyCount >> lam.Thickness >> lam.FiberOrientation >> lam.MaterialId;
                    std::string disp;
                    if (lss >> disp)
                        lam.MatDisp = disp;
                }
            }
        }
        return {top, low, web};
    }
}
