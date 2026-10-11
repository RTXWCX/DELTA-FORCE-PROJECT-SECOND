// ============================================================================
//  dfa_economy.h —— 资金 + 队友模块（工程：DFA_Economy，静态库）
//  职责：哈夫币资金分档、分档对五种玩法的评分、固定队友评分
//  扩展：调整资金分档 → 改 dfa_economy.cpp 的 kWealthTiers 阈值
// ============================================================================
#ifndef DFA_ECONOMY_H
#define DFA_ECONOMY_H

#include "dfa_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// 资金分档（0~3）：紧张 / 小康 / 宽裕 / 雄厚
int DFA_E_WealthTier(long long wealthWan);

// 分档阈值（3 个，单位：万）；分档数量由 kWealthTierCount 决定
int         DFA_E_GetTierCount();
long long   DFA_E_GetTier(int i);                    // 越界返回 0
const char* DFA_E_GetTierName(int tier);             // 越界返回 "未知"
const char* DFA_E_GetTierNote(int tier);             // 越界返回 ""

// 某资金分档对五种玩法的评分（返回 5 元素数组）
const int*  DFA_E_GetWealthStyleScores(int tier);

// 固定队友评分（0 = 无固排，1 = 有固排）
const int*  DFA_E_GetTeamStyleScores(bool hasTeam);

#ifdef __cplusplus
}
#endif

#endif // DFA_ECONOMY_H
