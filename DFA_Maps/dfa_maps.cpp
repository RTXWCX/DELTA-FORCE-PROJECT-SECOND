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
    { "AZ3",      "新地图（核危机主题）：仅开放普通与绝密两档，强度跨度大" },
    { "潮汐监狱", "仅开放绝密难度：监狱多层结构，高门槛高爆率，适合高资金与高熟练度玩家" },
};

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
};

// ---------- 每张地图实际开放的难度（-1 表示结束） ----------
// 零号大坝：普通/机密/永夜（无绝密）
// 长弓溪谷：普通/机密（无绝密）
// 巴克什：机密起步 → 机密/绝密
// 航天基地：机密起步 → 机密/绝密
// AZ3：仅普通 + 绝密
// 潮汐监狱：仅绝密
static const int kAllowedDiffs[DFA_MAP_COUNT][4] = {
    /* 零号大坝 */ { DFA_DIFF_NORMAL, DFA_DIFF_SECRET, DFA_DIFF_NIGHT, -1 },
    /* 长弓溪谷 */ { DFA_DIFF_NORMAL, DFA_DIFF_SECRET, -1, -1 },
    /* 巴克什   */ { DFA_DIFF_SECRET, DFA_DIFF_TOP_SECRET, -1, -1 },
    /* 航天基地 */ { DFA_DIFF_SECRET, DFA_DIFF_TOP_SECRET, -1, -1 },
    /* AZ3      */ { DFA_DIFF_NORMAL, DFA_DIFF_TOP_SECRET, -1, -1 },
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
