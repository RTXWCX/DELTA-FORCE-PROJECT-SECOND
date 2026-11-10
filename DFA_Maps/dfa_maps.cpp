// ============================================================================
//  dfa_maps.cpp —— 地图 + 难度模块实现
//  ============================================================================
//  【配置区】以下数据集中在本文件顶部，日常微调只改这里：
//   - 新增地图：kMaps 追加一行 { "图名", "特点" }，kMapScore 追加一行 5 个评分，
//               kAllowedDiffs 追加一行（-1 结尾，表示该图开放的难度列表）。
//   - 新增难度：kDiffs 追加一行，kDiffScore 追加一行 5 个评分，
//               并在相关地图的 kAllowedDiffs 行里加入该难度编号。
// ============================================================================
#include "dfa_maps.h"

// ---------- 地图 ----------
static const MapInfo kMaps[DFA_MAP_COUNT] = {
    { "零号大坝", "新手图：普通/机密无高门槛、有常规撤离点，物资点多；另有永夜（黑夜）玩法，低成本玩法友好" },
    { "长弓溪谷", "全图最大最开阔：狙击位多，好物资集中在少数点位，赶路风险高；无绝密" },
    { "巴克什",   "巷战密集、室内交战多：对近战与队伍协同要求高；难度从机密起步" },
    { "航天基地", "高强度公式化地图：物资点密集、露天刷新点多，是猛攻哥主战场；难度从机密起步" },
    { "AZ3",      "新地图（核危机主题）：开放普通 / 绝密 / 超临界三档，强度跨度大" },
    { "潮汐监狱", "仅开放绝密难度：监狱多层结构，高门槛高爆率，适合高资金与高熟练度玩家" },
};

// ---------- 各地图的核心物资区（输出「核心物资区」段落时使用） ----------
static const char* kLootZones[DFA_MAP_COUNT] = {
    /* 零号大坝 */ "行政辖区、主变电站、游客中心、水泥厂",
    /* 长弓溪谷 */ "坠机之地、钻石皇后酒店、超星车站、雷达站、阿米亚小镇、变电站、蓝港码头",
    /* 巴克什   */ "巴别塔、皇家博物馆、蓝汀旅馆、巴克什大浴场（其余物资点较多，分布较乱）",
    /* 航天基地 */ "离心机、蓝室、黑室、浮力室、中控区、码头",
    /* AZ3      */ "反应堆、老科学院、乏处理厂、应急火电站、压水堆、仿星器研究所、海水处理区",
    /* 潮汐监狱 */ "物资点密集、高爆点密集、枪战激烈，落地即高强度交火",
};

// ---------- 地图 + 难度的特别提示 ----------
// AZ3 超临界：随机刷新 2 个泄漏点，其中 1 个被激活并持续扩大辐射半径。
static const char kAZ3LeakNote[] =
    "超临界难度下，地图会随机刷新 2 个泄漏点，其中 1 个会被激活并持续向外扩大辐射半径；"
    "进图后先确认泄漏点方位，规划路线时留出绕行余量，别在辐射区里恋战。";

// ---------- 难度 ----------
static const DiffInfo kDiffs[DFA_DIFF_COUNT] = {
    { "普通", "无战备门槛，交战强度低，适合低成本试水",
      "零号大坝 / 长弓溪谷 / AZ3" },
    { "机密", "有战备门槛（零号大坝约 11w），物资与风险同步上升，主流玩法难度",
      "零号大坝 / 长弓溪谷 / 巴克什 / 航天基地" },
    { "绝密", "门槛与风险最高、爆率最高，只建议高资金与高熟练度玩家进入",
      "航天基地 / 巴克什 / AZ3 / 潮汐监狱" },
    { "永夜", "黑夜玩法：视野受限，隐蔽与听声辨位更重要，夜视装备价值高",
      "零号大坝（黑夜大坝）" },
    { "超临界", "凌驾于绝密之上的最高强度：战备门槛与交战烈度最高，只适合高资金、高熟练度、成建制队伍",
      "AZ3" },
};

// ---------- 各地图对五种玩法的加分（列顺序 = DfaStyle） ----------
static const int kMapScore[DFA_MAP_COUNT][DFA_STYLE_COUNT] = {
    /* 零号大坝 */ {  2,  2,  1,  0, -1 },
    /* 长弓溪谷 */ {  1,  1,  2,  1,  0 },
    /* 巴克什   */ {  0,  1,  1,  2,  2 },
    /* 航天基地 */ { -2,  0,  0,  2,  3 },
    /* AZ3      */ {  0,  1,  1,  2,  2 },
    /* 潮汐监狱 */ { -2,  0,  0,  1,  3 },
};

// ---------- 各难度对五种玩法的加分 ----------
static const int kDiffScore[DFA_DIFF_COUNT][DFA_STYLE_COUNT] = {
    /* 普通 */ {  2,  1,  1, -1, -2 },
    /* 机密 */ {  0,  2,  2,  1,  0 },
    /* 绝密 */ { -3, -1, -1,  1,  3 },
    /* 永夜 */ {  2,  3,  1,  1, -1 },
    /* 超临界 */ { -4, -2, -2,  1,  5 },
};

// ---------- 每张地图实际开放的难度（-1 表示结束） ----------
// 零号大坝：普通/机密/永夜（无绝密）
// 长弓溪谷：普通/机密（无绝密）
// 巴克什：机密起步 → 机密/绝密
// 航天基地：机密起步 → 机密/绝密
// AZ3：普通 / 绝密 / 超临界
// 潮汐监狱：仅绝密
static const int kAllowedDiffs[DFA_MAP_COUNT][4] = {
    /* 零号大坝 */ { DFA_DIFF_NORMAL, DFA_DIFF_SECRET, DFA_DIFF_NIGHT, -1 },
    /* 长弓溪谷 */ { DFA_DIFF_NORMAL, DFA_DIFF_SECRET, -1, -1 },
    /* 巴克什   */ { DFA_DIFF_SECRET, DFA_DIFF_TOP_SECRET, -1, -1 },
    /* 航天基地 */ { DFA_DIFF_SECRET, DFA_DIFF_TOP_SECRET, -1, -1 },
    /* AZ3      */ { DFA_DIFF_NORMAL, DFA_DIFF_TOP_SECRET, DFA_DIFF_SUPERCRITICAL, -1 },
    /* 潮汐监狱 */ { DFA_DIFF_TOP_SECRET, -1, -1, -1 },
};

int DFA_M_GetMapCount() { return DFA_MAP_COUNT; }

const MapInfo* DFA_M_GetMap(int idx) {
    if (idx < 0 || idx >= DFA_MAP_COUNT) return nullptr;
    return &kMaps[idx];
}

const int* DFA_M_GetMapStyleScores(int idx) {
    static const int kZero[DFA_STYLE_COUNT] = { 0, 0, 0, 0, 0 };
    if (idx < 0 || idx >= DFA_MAP_COUNT) return kZero;
    return kMapScore[idx];
}

int DFA_M_GetDiffCount() { return DFA_DIFF_COUNT; }

const DiffInfo* DFA_M_GetDiff(int idx) {
    if (idx < 0 || idx >= DFA_DIFF_COUNT) return nullptr;
    return &kDiffs[idx];
}

const int* DFA_M_GetDiffStyleScores(int idx) {
    static const int kZero[DFA_STYLE_COUNT] = { 0, 0, 0, 0, 0 };
    if (idx < 0 || idx >= DFA_DIFF_COUNT) return kZero;
    return kDiffScore[idx];
}

int DFA_M_GetAllowedDiffCount(int mapIdx) {
    if (mapIdx < 0 || mapIdx >= DFA_MAP_COUNT) return 0;
    int n = 0;
    while (n < 4 && kAllowedDiffs[mapIdx][n] >= 0) ++n;
    return n;
}

int DFA_M_GetAllowedDiff(int mapIdx, int slot) {
    if (mapIdx < 0 || mapIdx >= DFA_MAP_COUNT) return -1;
    if (slot < 0 || slot >= 4) return -1;
    return kAllowedDiffs[mapIdx][slot];
}

const char* DFA_M_GetLootZones(int mapIdx) {
    if (mapIdx < 0 || mapIdx >= DFA_MAP_COUNT) return nullptr;
    return kLootZones[mapIdx];
}

const char* DFA_M_GetSpecialNote(int mapIdx, int diffIdx) {
    // AZ3 超临界：随机泄漏点机制
    if (mapIdx == DFA_MAP_AZ3 && diffIdx == DFA_DIFF_SUPERCRITICAL) return kAZ3LeakNote;
    return nullptr;
}
