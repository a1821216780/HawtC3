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

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "FileParser.h"
#include "OtherHelper.h"

namespace Qahse::IO::IO
{
    class InputFileParser
    {
    public:
        // -------------------------- Common string/line helpers --------------------------
        static std::string ToUpperCopy(const std::string &s)
        {
            std::string out = s;
            std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c)
                           { return static_cast<char>(std::toupper(c)); });
            return out;
        }

        static bool IsBlankOrCommentLine(const std::string &line)
        {
            auto first = std::find_if_not(line.begin(), line.end(), [](unsigned char c)
                                          { return std::isspace(c) != 0; });
            if (first == line.end())
                return true;

            const std::string t(first, line.end());
            return t.rfind("--", 0) == 0 || t.rfind("//", 0) == 0 || t.rfind("#", 0) == 0;
        }

        static bool MatchKeywordInDataLine(const std::string &line, const std::string &keyword)
        {
            if (IsBlankOrCommentLine(line))
                return false;

            const std::string keyUpper = ToUpperCopy(keyword);
            std::istringstream iss(line);
            std::string token;
            while (iss >> token)
            {
                if (token == "--" || token == "//" || token == "#")
                    break;
                if (ToUpperCopy(token) == keyUpper)
                    return true;
            }
            return false;
        }

        // -------------------------- Legacy line search API --------------------------
        static int FindLineIndex(const std::vector<std::string> &lines,
                                 const std::string &keyword,
                                 const std::string &filepath,
                                 bool error = true,
                                 bool show = true)
        {
            const auto idxs = FindAllLineIndexes(lines, keyword, filepath, error, show);
            return idxs.empty() ? -1 : idxs.front();
        }

        static std::vector<int> FindAllLineIndexes(const std::vector<std::string> &lines,
                                                   const std::string &keyword,
                                                   const std::string &filepath,
                                                   bool error = true,
                                                   bool show = true)
        {
            auto result = OtherHelper::GetMatchingLineIndexes(lines, keyword, filepath, error, show);
            result.erase(std::remove(result.begin(), result.end(), -1), result.end());
            return result;
        }

        // -------------------------- Keyword readers (comment-safe) --------------------------
        static int FindKeywordLine(const std::vector<std::string> &lines,
                                   const std::string &keyword,
                                   const std::string &filepath,
                                   bool error = false,
                                   bool show = true)
        {
            for (int i = 0; i < static_cast<int>(lines.size()); ++i)
            {
                if (MatchKeywordInDataLine(lines[i], keyword))
                    return i;
            }

            // Keep old logging style when requested.
            if (error || show)
                (void)FindLineIndex(lines, keyword, filepath, error, show);

            return -1;
        }

        static std::string ReadKeywordToken(const std::vector<std::string> &lines,
                                            const std::string &keyword,
                                            const std::string &filepath,
                                            const std::string &defaultVal,
                                            bool error = false,
                                            bool show = true)
        {
            const int idx = FindKeywordLine(lines, keyword, filepath, error, show);
            if (idx < 0)
                return defaultVal;

            std::istringstream iss(lines[idx]);
            std::vector<std::string> tokens;
            std::string token;
            while (iss >> token)
            {
                if (token == "--" || token == "//" || token == "#")
                    break;
                tokens.push_back(token);
            }

            if (tokens.empty())
                return defaultVal;

            const std::string keyUpper = ToUpperCopy(keyword);
            for (size_t i = 0; i < tokens.size(); ++i)
            {
                if (ToUpperCopy(tokens[i]) != keyUpper)
                    continue;

                // "value keyword" format.
                if (i > 0)
                    return tokens[i - 1];

                // Fallback for "keyword value" format.
                if (i + 1 < tokens.size())
                    return tokens[i + 1];

                return defaultVal;
            }

            return tokens.front();
        }

        static double ReadKeywordDouble(const std::vector<std::string> &lines,
                                        const std::string &keyword,
                                        const std::string &filepath,
                                        double defaultVal,
                                        bool error = false,
                                        bool show = true)
        {
            const std::string token = ReadKeywordToken(lines, keyword, filepath, "", error, show);
            double out = 0.0;
            return TryParseDoubleToken(token, out) ? out : defaultVal;
        }

        static int ReadKeywordInt(const std::vector<std::string> &lines,
                                  const std::string &keyword,
                                  const std::string &filepath,
                                  int defaultVal,
                                  bool error = false,
                                  bool show = true)
        {
            const std::string token = ReadKeywordToken(lines, keyword, filepath, "", error, show);
            int out = 0;
            return TryParseIntToken(token, out) ? out : defaultVal;
        }

        static bool ReadKeywordBool(const std::vector<std::string> &lines,
                                    const std::string &keyword,
                                    const std::string &filepath,
                                    bool defaultVal,
                                    bool error = false,
                                    bool show = true)
        {
            const std::string token = ReadKeywordToken(lines, keyword, filepath, "", error, show);
            if (token.empty())
                return defaultVal;

            const std::string t = ToUpperCopy(token);
            if (t == "TRUE" || t == "1" || t == "YES" || t == "Y" || t == "ON")
                return true;
            if (t == "FALSE" || t == "0" || t == "NO" || t == "N" || t == "OFF")
                return false;
            return defaultVal;
        }

        // -------------------------- Primitive parsers --------------------------
        static double ParseDouble(const std::string &line)
        {
            std::istringstream iss(line);
            double v = 0.0;
            iss >> v;
            return v;
        }

        static int ParseInt(const std::string &line)
        {
            std::istringstream iss(line);
            int v = 0;
            iss >> v;
            return v;
        }

        static bool ParseBool(const std::string &line)
        {
            std::string token;
            std::istringstream iss(line);
            iss >> token;

            const std::string t = ToUpperCopy(token);
            return (t == "TRUE" || t == "1" || t == "YES" || t == "Y" || t == "ON");
        }

        static std::string ParseQuotedString(const std::string &line)
        {
            auto p1 = line.find_first_of("\"'");
            if (p1 == std::string::npos)
            {
                std::istringstream iss(line);
                std::string t;
                iss >> t;
                return t;
            }

            const char quote = line[p1];
            auto p2 = line.find(quote, p1 + 1);
            if (p2 == std::string::npos)
                p2 = line.size();
            return line.substr(p1 + 1, p2 - p1 - 1);
        }

        static std::vector<double> ParseDoubleArray(const std::string &line)
        {
            std::vector<double> result;
            std::istringstream iss(line);
            std::string token;

            while (iss >> token)
            {
                std::istringstream ts(token);
                std::string sub;
                while (std::getline(ts, sub, ','))
                {
                    if (sub.empty())
                        continue;

                    double v = 0.0;
                    if (!TryParseDoubleToken(sub, v))
                        return result;
                    result.push_back(v);
                }
            }
            return result;
        }

        // -------------------------- Legacy find+parse wrappers --------------------------
        static double FindAndParseDouble(const std::vector<std::string> &lines,
                                         const std::string &keyword,
                                         const std::string &filepath,
                                         double defaultVal = 0.0,
                                         bool error = true)
        {
            const int idx = FindLineIndex(lines, keyword, filepath, error);
            return (idx < 0) ? defaultVal : ParseDouble(lines[idx]);
        }

        static int FindAndParseInt(const std::vector<std::string> &lines,
                                   const std::string &keyword,
                                   const std::string &filepath,
                                   int defaultVal = 0,
                                   bool error = true)
        {
            const int idx = FindLineIndex(lines, keyword, filepath, error);
            return (idx < 0) ? defaultVal : ParseInt(lines[idx]);
        }

        static bool FindAndParseBool(const std::vector<std::string> &lines,
                                     const std::string &keyword,
                                     const std::string &filepath,
                                     bool defaultVal = false,
                                     bool error = true)
        {
            const int idx = FindLineIndex(lines, keyword, filepath, error);
            return (idx < 0) ? defaultVal : ParseBool(lines[idx]);
        }

        static std::string FindAndParseString(const std::vector<std::string> &lines,
                                              const std::string &keyword,
                                              const std::string &filepath,
                                              bool error = true)
        {
            const int idx = FindLineIndex(lines, keyword, filepath, error);
            return (idx < 0) ? std::string() : ParseQuotedString(lines[idx]);
        }

        static std::string ResolvePath(const std::string &basePath, const std::string &relPath)
        {
            namespace fs = std::filesystem;

            std::string cleaned = CleanPathToken(relPath);
            if (cleaned.empty())
                return cleaned;

            const fs::path p(cleaned);
            if (p.is_absolute())
                return p.lexically_normal().string();

            const fs::path base = fs::path(basePath).parent_path();
            return fs::absolute(base / p).lexically_normal().string();
        }

        virtual ~InputFileParser() = default;

        const std::string &errors() const { return m_errors; }
        bool hasErrors() const { return !m_errors.empty(); }

    protected:
        std::vector<std::string> m_lines;
        std::string m_filePath;
        std::string m_baseDir;
        std::string m_errors;

        bool loadFile(const std::string &filePath)
        {
            m_filePath = filePath;
            m_baseDir = std::filesystem::path(filePath).parent_path().string();
            if (!m_baseDir.empty() && m_baseDir.back() != '/' && m_baseDir.back() != '\\')
                m_baseDir += '/';

            m_lines = FileParser::FileContentToStringList(filePath);
            if (m_lines.empty())
            {
                m_errors += "\n[Error] Cannot read file: " + filePath;
                return false;
            }
            return true;
        }

        double readDouble(const std::string &keyword, double defaultVal = 0.0)
        {
            std::string val;
            if (!readValueText(keyword, val))
                return defaultVal;

            double result = 0.0;
            if (FileParser::TryParseDouble(FileParser::UnifyString(val), result))
                return result;

            m_errors += "\n[Warning] Cannot parse double for keyword: " + keyword;
            return defaultVal;
        }

        int readInt(const std::string &keyword, int defaultVal = 0)
        {
            std::string val;
            if (!readValueText(keyword, val))
                return defaultVal;

            int result = 0;
            if (FileParser::TryParseInt(FileParser::UnifyString(val), result))
                return result;

            m_errors += "\n[Warning] Cannot parse int for keyword: " + keyword;
            return defaultVal;
        }

        bool readBool(const std::string &keyword, bool defaultVal = false)
        {
            std::string val;
            if (!readValueText(keyword, val))
                return defaultVal;
            return FileParser::ParseBool(val);
        }

        std::string readString(const std::string &keyword, const std::string &defaultVal = "")
        {
            std::string val;
            if (!readValueText(keyword, val))
                return defaultVal;
            return stripQuotes(val);
        }

        std::vector<double> readDoubleList(const std::string &keyword, int expectedCount)
        {
            bool found = false;
            const auto tokens = FileParser::FindLineWithKeyword(keyword, m_lines, &m_errors, false, &found, true);

            std::vector<double> result;
            if (!found)
                return result;

            const std::string keyUpper = ToUpperCopy(keyword);
            for (const auto &tok : tokens)
            {
                if (ToUpperCopy(tok) == keyUpper)
                    break;

                double v = 0.0;
                if (FileParser::TryParseDouble(FileParser::UnifyString(tok), v))
                    result.push_back(v);
            }

            if (static_cast<int>(result.size()) < expectedCount)
            {
                m_errors += "\n[Warning] Expected " + std::to_string(expectedCount) +
                            " values for keyword: " + keyword +
                            ", got " + std::to_string(result.size());
            }
            return result;
        }

        Eigen::MatrixXd readMatrix(const std::string &keyword, int rows, int cols)
        {
            bool found = false;
            auto mat = FileParser::FindMatrixInFile(keyword, m_lines, rows, cols, &m_errors, false, &found);
            if (!found)
                return Eigen::MatrixXd::Zero(rows, cols);
            return mat;
        }

        std::vector<std::vector<double>> readNumericTable(const std::string &keyword, int cols)
        {
            bool found = false;
            return FileParser::FindNumericDataTable(keyword, m_lines, cols, &m_errors, false, &found);
        }

        std::vector<std::string> readFileListAfterKeyword(const std::string &keyword, int count)
        {
            std::vector<std::string> result;
            if (count <= 0)
                return result;

            const int idx = FindLineIndex(m_lines, keyword, m_filePath, false, false);
            if (idx < 0)
                return result;

            for (int k = 0; k < count; ++k)
            {
                const int lineIdx = idx + 1 + k;
                if (lineIdx >= static_cast<int>(m_lines.size()))
                    break;

                auto lineTokens = FileParser::TokenizeLine(m_lines[lineIdx]);
                if (!lineTokens.empty())
                    result.push_back(stripQuotes(lineTokens[0]));
            }
            return result;
        }

        std::string resolvePath(const std::string &relPath) const
        {
            if (relPath.empty())
                return relPath;

            std::filesystem::path p(relPath);
            if (p.is_absolute())
                return p.lexically_normal().string();

            return (std::filesystem::path(m_baseDir) / p).lexically_normal().string();
        }

        static std::string stripQuotes(const std::string &s)
        {
            if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
                return s.substr(1, s.size() - 2);
            return s;
        }

    private:
        static bool TryParseDoubleToken(const std::string &token, double &out)
        {
            try
            {
                size_t pos = 0;
                out = std::stod(token, &pos);
                while (pos < token.size() && std::isspace(static_cast<unsigned char>(token[pos])) != 0)
                    ++pos;
                return pos == token.size();
            }
            catch (...)
            {
                return false;
            }
        }

        static bool TryParseIntToken(const std::string &token, int &out)
        {
            try
            {
                size_t pos = 0;
                out = std::stoi(token, &pos);
                while (pos < token.size() && std::isspace(static_cast<unsigned char>(token[pos])) != 0)
                    ++pos;
                return pos == token.size();
            }
            catch (...)
            {
                return false;
            }
        }

        static std::string CleanPathToken(const std::string &path)
        {
            std::string cleaned = path;
            cleaned.erase(std::remove(cleaned.begin(), cleaned.end(), '\''), cleaned.end());
            cleaned.erase(std::remove(cleaned.begin(), cleaned.end(), '"'), cleaned.end());

            while (!cleaned.empty() && std::isspace(static_cast<unsigned char>(cleaned.front())) != 0)
                cleaned.erase(cleaned.begin());
            while (!cleaned.empty() && std::isspace(static_cast<unsigned char>(cleaned.back())) != 0)
                cleaned.pop_back();

            return cleaned;
        }

        bool readValueText(const std::string &keyword, std::string &out)
        {
            bool found = false;
            out = FileParser::FindValueInFile(keyword, m_lines, &m_errors, false, &found);
            return found;
        }
    };

} // namespace Qahse::IO::IO
