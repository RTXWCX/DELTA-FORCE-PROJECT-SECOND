// ============================================================================
//  dfa_economy.cpp —— 资金 + 队友模块实现
//  ============================================================================
//  【配置区】以下数据集中在本文件顶部，日常微调只改这里：
//   - 调整资金分档：改 kWealthTiers 三个阈值（单位：万），并同步 kWealthTierNames /
//     kWealthTierNotes / kWealthScore 到对应档数。
//   - 调整队友加成：改 kTeamScore 两行。
// ============================================================================
#include "dfa_economy.h"

// ---------- 资金分档阈值（单位：万） ----------
// 规则：< T0 → 紧张；T0~T1 → 小康；T1~T2 → 宽裕；>= T2 → 雄厚
static const long long kWealthTiers[3] = { 100, 500, 1500 };
static const int kTierCount = (int)(sizeof(kWealthTiers) / sizeof(kWealthTiers[0])) + 1;

static const char* kWealthTierNames[] = {
    "紧张（<100万）", "小康（100~500万）", "宽裕（500~1500万）", "雄厚（≥1500万）"
};
static const char* kWealthTierNotes[] = {
    "资金紧张，应以低成本玩法保本攒钱，切忌高投入",
    "可稳定支撑机密起装（约 40~60w）成本，属于稳健积累期",
    "可承担中高端玩法试错，具备接战型玩法的资金基础",
    "资金雄厚，具备猛攻玩法的容错空间（社区观点：资产 1000 万+ 再考虑高投入）"
};

// ---------- 资金分档对五种玩法的加分（列顺序 = DfaStyle） ----------
static const int kWealthScore[][DFA_STYLE_COUNT] = {
    /* 紧张   */ {  4,  3, -2, -3, -4 },
    /* 小康   */ {  0,  1,  2,  0, -1 },
    /* 宽裕   */ { -2, -1,  2,  2,  1 },
    /* 雄厚   */ { -4, -3,  0,  2,  3 },
};

// ---------- 固定队友评分（0 = 无固排，1 = 有固排） ----------
static const int kTeamScore[2][DFA_STYLE_COUNT] = {
    /* 无固排 */ {  2,  2,  1,  0, -2 },
    /* 有固排 */ { -2, -2,  1,  2,  3 },
};

int DFA_E_WealthTier(long long wan) {
    if (wan < kWealthTiers[0]) return 0;
    if (wan < kWealthTiers[1]) return 1;
    if (wan < kWealthTiers[2]) return 2;
    return 3;
}

int DFA_E_GetTierCount() { return kTierCount; }

long long DFA_E_GetTier(int i) {
    if (i < 0 || i >= kTierCount - 1) return 0;
    return kWealthTiers[i];
}

const char* DFA_E_GetTierName(int tier) {
    if (tier < 0 || tier >= kTierCount) return "未知";
    return kWealthTierNames[tier];
}

const char* DFA_E_GetTierNote(int tier) {
    if (tier < 0 || tier >= kTierCount) return "";
    return kWealthTierNotes[tier];
}

const int* DFA_E_GetWealthStyleScores(int tier) {
    static const int kZero[DFA_STYLE_COUNT] = { 0, 0, 0, 0, 0 };
    if (tier < 0 || tier >= kTierCount) return kZero;
    return kWealthScore[tier];
}

const int* DFA_E_GetTeamStyleScores(bool hasTeam) {
    return kTeamScore[hasTeam ? 1 : 0];
}
