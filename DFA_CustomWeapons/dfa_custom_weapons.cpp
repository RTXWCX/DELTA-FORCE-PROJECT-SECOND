// ============================================================================
//  dfa_custom_weapons.cpp —— 自定义枪械模块实现（工程：DFA_CustomWeapons）
//
//  设计要点：
//  1) 只依赖 Win32 + 标准库，不引入第三方解析器；路径全程走宽字符，支持中文目录。
//  2) 编码自动识别：UTF-8(BOM/无 BOM)、UTF-16(BOM)、本机 ANSI(GBK) 兜底。
//  3) 单行解析失败只记错误，不中断整个文件；错误最多保留 64 条供界面展示。
//  4) 载入后数据顺序 = 文件名升序 + 文件内行序，菜单直接按此顺序编号。
// ============================================================================
#include "dfa_custom_weapons.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <string>
#include <vector>

namespace {

constexpr int          kMaxWeapons   = 256;
constexpr int          kMaxErrors    = 64;
constexpr long long    kMaxFileBytes = 4ll * 1024 * 1024;   // 单文件 4MB 上限

struct CustomWeapon {
    std::string name;                        // 枪名（UTF-8）
    std::string source;                      // 来源文件名（UTF-8）
    WeaponInfo  info;                        // 对外视图，指针指向上面两个字符串
    int         scores[DFA_STYLE_COUNT];     // 已按大类默认分补全
};

CustomWeapon g_items[kMaxWeapons];
int          g_count      = 0;
int          g_fileCount  = 0;
std::string  g_folder;
std::string  g_defaultFolder;
std::string  g_errors[kMaxErrors];
int          g_errorCount = 0;
int          g_errorTotal = 0;

// ---------------------------------------------------------------- 编码工具

void AddError(const std::string& msg)
{
    ++g_errorTotal;
    if (g_errorCount < kMaxErrors) {
        g_errors[g_errorCount++] = msg;
    }
}

bool WideToUtf8(const std::wstring& w, std::string& out)
{
    out.clear();
    if (w.empty()) return true;
    const int n = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, w.c_str(),
                                        static_cast<int>(w.size()), nullptr, 0, nullptr, nullptr);
    if (n <= 0) return false;
    out.resize(static_cast<size_t>(n));
    return ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, w.c_str(),
                                 static_cast<int>(w.size()), &out[0], n, nullptr, nullptr) == n;
}

bool Utf8ToWide(const std::string& s, std::wstring& out)
{
    out.clear();
    if (s.empty()) return true;
    const int n = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(),
                                       static_cast<int>(s.size()), nullptr, 0);
    if (n <= 0) return false;
    out.resize(static_cast<size_t>(n));
    return ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(),
                                 static_cast<int>(s.size()), &out[0], n) == n;
}

bool AcpToUtf8(const std::string& in, std::string& out)
{
    out.clear();
    if (in.empty()) return true;
    const int n = ::MultiByteToWideChar(CP_ACP, 0, in.c_str(), static_cast<int>(in.size()), nullptr, 0);
    if (n <= 0) return false;
    std::wstring w(static_cast<size_t>(n), L'\0');
    if (::MultiByteToWideChar(CP_ACP, 0, in.c_str(), static_cast<int>(in.size()), &w[0], n) != n) return false;
    return WideToUtf8(w, out);
}

// 严格校验 UTF-8（拒绝超长编码、代理区、越界码点）
bool IsValidUtf8(const std::string& s)
{
    const size_t n = s.size();
    size_t i = 0;
    while (i < n) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        if (c < 0x80) { ++i; continue; }
        int len = 0;
        unsigned int cp = 0;
        if ((c & 0xE0) == 0xC0)      { len = 2; cp = c & 0x1Fu; }
        else if ((c & 0xF0) == 0xE0) { len = 3; cp = c & 0x0Fu; }
        else if ((c & 0xF8) == 0xF0) { len = 4; cp = c & 0x07u; }
        else return false;
        if (i + static_cast<size_t>(len) > n) return false;
        for (int k = 1; k < len; ++k) {
            const unsigned char cc = static_cast<unsigned char>(s[i + static_cast<size_t>(k)]);
            if ((cc & 0xC0) != 0x80) return false;
            cp = (cp << 6) | (cc & 0x3Fu);
        }
        if (len == 2 && cp < 0x80u)     return false;
        if (len == 3 && cp < 0x800u)    return false;
        if (len == 4 && cp < 0x10000u)  return false;
        if (cp > 0x10FFFFu)             return false;
        if (cp >= 0xD800u && cp <= 0xDFFFu) return false;
        i += static_cast<size_t>(len);
    }
    return true;
}

// 把文件字节流解码为 UTF-8；失败时给出中文原因
bool DecodeToUtf8(const std::string& raw, std::string& out, std::string& why)
{
    const size_t n = raw.size();
    const unsigned char* p = reinterpret_cast<const unsigned char*>(raw.data());
    out.clear();
    why.clear();

    // UTF-32 BOM（先于 UTF-16 判断，因为 FF FE 00 00 也以 FF FE 开头）
    if (n >= 4 && ((p[0] == 0xFF && p[1] == 0xFE && p[2] == 0x00 && p[3] == 0x00) ||
                   (p[0] == 0x00 && p[1] == 0x00 && p[2] == 0xFE && p[3] == 0xFF))) {
        why = "暂不支持 UTF-32 编码，请另存为 UTF-8";
        return false;
    }

    if (n >= 3 && p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF) {
        out.assign(raw, 3, std::string::npos);
        if (!IsValidUtf8(out)) { why = "UTF-8 BOM 之后的内容不是合法 UTF-8"; out.clear(); return false; }
        return true;
    }

    if (n >= 2 && p[0] == 0xFF && p[1] == 0xFE) {          // UTF-16 LE
        if ((n - 2) % 2 != 0) { why = "UTF-16 文件字节数为奇数，已损坏"; return false; }
        const size_t chars = (n - 2) / 2;
        std::wstring w(chars, L'\0');
        std::memcpy(&w[0], raw.data() + 2, chars * 2);
        if (!WideToUtf8(w, out)) { why = "UTF-16 内容包含非法代理项"; out.clear(); return false; }
        return true;
    }
    if (n >= 2 && p[0] == 0xFE && p[1] == 0xFF) {          // UTF-16 BE
        if ((n - 2) % 2 != 0) { why = "UTF-16 文件字节数为奇数，已损坏"; return false; }
        const size_t chars = (n - 2) / 2;
        std::wstring w(chars, L'\0');
        for (size_t i = 0; i < chars; ++i) {
            const unsigned char hi = p[2 + i * 2];
            const unsigned char lo = p[2 + i * 2 + 1];
            w[i] = static_cast<wchar_t>((static_cast<unsigned int>(hi) << 8) | lo);
        }
        if (!WideToUtf8(w, out)) { why = "UTF-16 内容包含非法代理项"; out.clear(); return false; }
        return true;
    }

    if (IsValidUtf8(raw)) { out = raw; return true; }       // 无 BOM 的 UTF-8 / 纯 ASCII

    // 兜底按本机代码页（中文系统即 GBK）解释
    if (AcpToUtf8(raw, out)) return true;
    why = "编码无法识别（既不是 UTF-8/UTF-16，也无法按本机 GBK 解码）";
    return false;
}

// ---------------------------------------------------------------- 文本工具

bool IsAsciiSpace(unsigned char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

// 去掉首尾的 ASCII 空白与全角空格 U+3000
std::string Trim(const std::string& s)
{
    size_t b = 0;
    size_t e = s.size();
    for (;;) {
        if (b < e && IsAsciiSpace(static_cast<unsigned char>(s[b]))) { ++b; continue; }
        if (b + 2 < e && static_cast<unsigned char>(s[b]) == 0xE3 &&
            static_cast<unsigned char>(s[b + 1]) == 0x80 &&
            static_cast<unsigned char>(s[b + 2]) == 0x80) { b += 3; continue; }
        break;
    }
    for (;;) {
        if (e > b && IsAsciiSpace(static_cast<unsigned char>(s[e - 1]))) { --e; continue; }
        if (e >= b + 3 && static_cast<unsigned char>(s[e - 3]) == 0xE3 &&
            static_cast<unsigned char>(s[e - 2]) == 0x80 &&
            static_cast<unsigned char>(s[e - 1]) == 0x80) { e -= 3; continue; }
        break;
    }
    return s.substr(b, e - b);
}

// 把 | ， 、 ｜ 等分隔符统一成半角逗号
std::string NormalizeSeparators(const std::string& s)
{
    std::string out;
    out.reserve(s.size());
    const size_t n = s.size();
    for (size_t i = 0; i < n;) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        if (c == '|' || c == ',') { out.push_back(','); ++i; continue; }
        if (c == 0xEF && i + 2 < n) {
            const unsigned char c1 = static_cast<unsigned char>(s[i + 1]);
            const unsigned char c2 = static_cast<unsigned char>(s[i + 2]);
            if (c1 == 0xBD && c2 == 0x9C) { out.push_back(','); i += 3; continue; }  // ｜
            if (c1 == 0xBC && c2 == 0x8C) { out.push_back(','); i += 3; continue; }  // ，
        }
        if (c == 0xE3 && i + 2 < n &&
            static_cast<unsigned char>(s[i + 1]) == 0x80 &&
            static_cast<unsigned char>(s[i + 2]) == 0x81) { out.push_back(','); i += 3; continue; } // 、
        out.push_back(static_cast<char>(c));
        ++i;
    }
    return out;
}

std::vector<std::string> Split(const std::string& s, char sep)
{
    std::vector<std::string> parts;
    size_t start = 0;
    for (;;) {
        const size_t pos = s.find(sep, start);
        if (pos == std::string::npos) {
            parts.push_back(s.substr(start));
            break;
        }
        parts.push_back(s.substr(start, pos - start));
        start = pos + 1;
    }
    return parts;
}

bool IsComment(const std::string& t)
{
    if (t.empty()) return false;
    if (t[0] == '#' || t[0] == ';') return true;
    if (t.size() >= 2 && t[0] == '/' && t[1] == '/') return true;
    if (t.compare(0, 3, "\xEF\xBC\x83") == 0) return true;   // ＃
    if (t.compare(0, 3, "\xEF\xBC\x9B") == 0) return true;   // ；
    return false;
}

// 紧凑截断，避免把多字节字符切成半截
std::string Truncate(const std::string& s, size_t maxBytes)
{
    if (s.size() <= maxBytes) return s;
    size_t cut = maxBytes;
    while (cut > 0 && (static_cast<unsigned char>(s[cut]) & 0xC0) == 0x80) --cut;
    return s.substr(0, cut) + "…";
}

bool ParseInt(const std::string& t, int& out)
{
    if (t.empty()) return false;
    errno = 0;
    char* end = nullptr;
    const long v = std::strtol(t.c_str(), &end, 10);
    if (errno != 0 || end == t.c_str() || end == nullptr || *end != '\0') return false;
    if (v < -99 || v > 99) return false;
    out = static_cast<int>(v);
    return true;
}

char LowerAscii(char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

bool EqualsAsciiCI(const std::string& a, const std::string& b)
{
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (LowerAscii(a[i]) != LowerAscii(b[i])) return false;
    }
    return true;
}

// ---------------------------------------------------------------- 文件读取

bool ReadWholeFile(const std::wstring& path, std::string& out, std::string& why)
{
    out.clear();
    HANDLE h = ::CreateFileW(path.c_str(), GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                             nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) { why = "无法打开文件（可能被占用）"; return false; }

    LARGE_INTEGER size;
    if (!::GetFileSizeEx(h, &size)) { ::CloseHandle(h); why = "无法读取文件大小"; return false; }
    if (size.QuadPart > kMaxFileBytes) {
        ::CloseHandle(h);
        why = "文件超过 4MB，已跳过";
        return false;
    }

    out.reserve(static_cast<size_t>(size.QuadPart) + 1);
    char buf[64 * 1024];
    for (;;) {
        DWORD got = 0;
        if (!::ReadFile(h, buf, static_cast<DWORD>(sizeof(buf)), &got, nullptr)) {
            ::CloseHandle(h);
            out.clear();
            why = "读取文件内容失败";
            return false;
        }
        if (got == 0) break;
        out.append(buf, got);
    }
    ::CloseHandle(h);
    return true;
}

struct ScannedFile {
    std::wstring fullPath;
    std::string  nameUtf8;
};

bool CollectFiles(const std::wstring& dir, std::vector<ScannedFile>& files)
{
    WIN32_FIND_DATAW fd;
    const std::wstring pattern = dir + L"\\*";
    HANDLE h = ::FindFirstFileW(pattern.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return false;

    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (fd.cFileName[0] == L'~') continue;                 // Office 临时文件
        const wchar_t* dot = std::wcsrchr(fd.cFileName, L'.');
        if (dot == nullptr) continue;
        if (_wcsicmp(dot, L".txt") != 0 && _wcsicmp(dot, L".csv") != 0) continue;

        ScannedFile f;
        f.fullPath = dir + L"\\" + fd.cFileName;
        if (!WideToUtf8(fd.cFileName, f.nameUtf8)) continue;    // 文件名无法转 UTF-8 就跳过
        files.push_back(f);
    } while (::FindNextFileW(h, &fd));
    ::FindClose(h);

    std::sort(files.begin(), files.end(), [](const ScannedFile& a, const ScannedFile& b) {
        return ::CompareStringOrdinal(a.fullPath.c_str(), -1, b.fullPath.c_str(), -1, TRUE) == CSTR_LESS_THAN;
    });
    return true;
}

// ---------------------------------------------------------------- 解析

bool NameExists(const std::string& name)
{
    for (int i = 0; i < DFA_W_GetWeaponCount(); ++i) {
        const WeaponInfo* w = DFA_W_GetWeapon(i);
        if (w != nullptr && w->name != nullptr && EqualsAsciiCI(name, w->name)) return true;
    }
    for (int i = 0; i < g_count; ++i) {
        if (EqualsAsciiCI(name, g_items[i].name)) return true;
    }
    return false;
}

void ProcessLine(const std::string& line, const std::string& fileName, int lineNo)
{
    const std::string t = Trim(line);
    if (t.empty() || IsComment(t)) return;

    const std::string prefix = fileName + ":" + std::to_string(lineNo) + " ";

    std::vector<std::string> fields = Split(NormalizeSeparators(t), ',');
    for (size_t i = 0; i < fields.size(); ++i) fields[i] = Trim(fields[i]);
    while (!fields.empty() && fields.back().empty()) fields.pop_back();

    if (fields.size() < 2) {
        AddError(prefix + "格式应为「枪名 | 大类 | 5 个评分」");
        return;
    }

    const std::string name = fields[0];
    if (name.empty()) {
        AddError(prefix + "枪名为空");
        return;
    }

    const int cls = DFA_W_FindClass(fields[1].c_str());
    if (cls < 0) {
        AddError(prefix + "大类无法识别：" + fields[1] + "（可用：突击步枪/冲锋枪/狙击步枪/射手步枪/霰弹枪/轻机枪/手枪/特殊武器/通用）");
        return;
    }

    std::vector<int> scores;
    for (size_t i = 2; i < fields.size(); ++i) {
        int v = 0;
        if (!ParseInt(fields[i], v)) {
            AddError(prefix + "评分必须是 -99~99 的整数，实际为：" + Truncate(fields[i], 32));
            return;
        }
        scores.push_back(v);
    }
    if (!scores.empty() && scores.size() != static_cast<size_t>(DFA_STYLE_COUNT)) {
        AddError(prefix + "评分需写 5 个（跑刀,鼠鼠,正常玩,微交战,全装猛攻），实际 " +
                 std::to_string(scores.size()) + " 个");
        return;
    }

    if (NameExists(name)) {
        AddError(prefix + "枪名与已有数据重复，已跳过：" + name);
        return;
    }
    if (g_count >= kMaxWeapons) {
        AddError(prefix + "自定义枪械已达上限 " + std::to_string(kMaxWeapons) + " 条，本条被忽略");
        return;
    }

    CustomWeapon& item = g_items[g_count];
    item.name   = name;
    item.source = fileName;
    item.info.name    = item.name.c_str();
    item.info.subType = item.source.c_str();
    item.info.cls     = static_cast<WeaponClass>(cls);

    const int* def = DFA_W_GetStyleScores(static_cast<WeaponClass>(cls));
    for (int s = 0; s < DFA_STYLE_COUNT; ++s) {
        item.scores[s] = (def != nullptr) ? def[s] : 0;
    }
    if (!scores.empty()) {
        for (int s = 0; s < DFA_STYLE_COUNT; ++s) item.scores[s] = scores[static_cast<size_t>(s)];
    }
    ++g_count;
}

void ProcessFile(const ScannedFile& file)
{
    std::string raw;
    std::string why;
    if (!ReadWholeFile(file.fullPath, raw, why)) {
        AddError(file.nameUtf8 + "：" + why);
        return;
    }

    std::string text;
    if (!DecodeToUtf8(raw, text, why)) {
        AddError(file.nameUtf8 + "：" + why);
        return;
    }

    size_t pos = 0;
    int lineNo = 0;
    for (;;) {
        const size_t nl = text.find('\n', pos);
        const std::string line = (nl == std::string::npos) ? text.substr(pos)
                                                           : text.substr(pos, nl - pos);
        ++lineNo;
        ProcessLine(line, file.nameUtf8, lineNo);
        if (nl == std::string::npos) break;
        pos = nl + 1;
    }
}

// ---------------------------------------------------------------- 默认目录

std::wstring ExeDirectory()
{
    std::wstring path(MAX_PATH, L'\0');
    for (;;) {
        const DWORD n = ::GetModuleFileNameW(nullptr, &path[0], static_cast<DWORD>(path.size()));
        if (n == 0) return std::wstring();
        if (n < path.size()) { path.resize(n); break; }
        path.resize(path.size() * 2);
    }
    const size_t slash = path.find_last_of(L"\\/");
    if (slash != std::wstring::npos) path.resize(slash);
    return path;
}

} // namespace

// ============================================================================
//  对外接口
// ============================================================================

extern "C" {

int DFA_CW_Load(const char* folderUtf8)
{
    try {
        for (int i = 0; i < g_count; ++i) g_items[i] = CustomWeapon();
        g_count      = 0;
        g_fileCount  = 0;
        g_errorCount = 0;
        g_errorTotal = 0;
        g_folder.clear();

        if (folderUtf8 == nullptr || *folderUtf8 == '\0') return -1;

        std::wstring dir;
        if (!Utf8ToWide(folderUtf8, dir)) return -1;
        while (!dir.empty() && (dir.back() == L'\\' || dir.back() == L'/')) dir.pop_back();
        if (dir.empty()) return -1;

        const DWORD attr = ::GetFileAttributesW(dir.c_str());
        if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY) == 0) return -1;

        if (!WideToUtf8(dir, g_folder)) g_folder = folderUtf8;

        std::vector<ScannedFile> files;
        if (!CollectFiles(dir, files)) return -1;
        g_fileCount = static_cast<int>(files.size());
        for (size_t i = 0; i < files.size(); ++i) ProcessFile(files[i]);
        return g_count;
    } catch (...) {
        AddError("载入过程中出现内存异常，已尽可能保留已解析的数据");
        return g_count;
    }
}

int DFA_CW_GetCount()
{
    return g_count;
}

const WeaponInfo* DFA_CW_GetWeapon(int idx)
{
    if (idx < 0 || idx >= g_count) return nullptr;
    return &g_items[idx].info;
}

const int* DFA_CW_GetStyleScores(int idx)
{
    if (idx < 0 || idx >= g_count) return nullptr;
    return g_items[idx].scores;
}

const char* DFA_CW_GetFolder()
{
    return g_folder.c_str();
}

const char* DFA_CW_GetDefaultFolder()
{
    if (g_defaultFolder.empty()) {
        const std::wstring dir = ExeDirectory();
        std::wstring target = dir.empty() ? std::wstring(L"自定义枪械") : (dir + L"\\自定义枪械");
        if (!WideToUtf8(target, g_defaultFolder)) g_defaultFolder.clear();
    }
    return g_defaultFolder.c_str();
}

int DFA_CW_GetFileCount()
{
    return g_fileCount;
}

int DFA_CW_GetErrorCount()
{
    return g_errorCount;
}

const char* DFA_CW_GetError(int idx)
{
    if (idx < 0 || idx >= g_errorCount) return nullptr;
    return g_errors[idx].c_str();
}

int DFA_CW_GetCapacity()
{
    return kMaxWeapons;
}

} // extern "C"
