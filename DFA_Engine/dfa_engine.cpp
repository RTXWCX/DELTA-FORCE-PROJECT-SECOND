// ============================================================================
//  dfa_engine.cpp —— 推荐引擎模块实现
//  评分模型：总分 = 枪械类别分 + 干员兵种分 + 地图分 + 难度分 + 资金分档分 + 队友分
//  强制规则：绝密 + 资金 < 100 万 → 强制跑刀（防破产兜底）
//  扩展：新增规则 → 在本文件底部继续追加 ForceRule 条件即可。
// ============================================================================
#include "dfa_engine.h"
#include "dfa_weapons.h"
#include "dfa_maps.h"
#include "dfa_economy.h"
#include "dfa_operators.h"

void DFA_E_ComputeScores(const DfaProfile* p, int out[DFA_STYLE_COUNT]) {
    if (!p || !out) return;

    // 枪械类别（越界按通用处理）
    WeaponClass cls = WC_GENERIC;
    const WeaponInfo* w = DFA_W_GetWeapon(p->weaponIdx);
    if (w) cls = w->cls;
    const int* ws = DFA_W_GetStyleScores(cls);

    // 干员兵种（越界按 0 号处理）
    const OperatorInfo* op = DFA_O_GetOperator(p->opIdx < 0 || p->opIdx >= DFA_O_GetOperatorCount() ? 0 : p->opIdx);
    const int* os = DFA_O_GetRoleScores(op ? op->role : OP_ROLE_SUPPORT);

    // 地图 / 难度（越界按 0 号处理）
    const int* ms = DFA_M_GetMapStyleScores(p->mapIdx < 0 || p->mapIdx >= DFA_MAP_COUNT ? 0 : p->mapIdx);
    const int* ds = DFA_M_GetDiffStyleScores(p->diffIdx < 0 || p->diffIdx >= DFA_DIFF_COUNT ? 0 : p->diffIdx);

    // 资金分档 + 队友
    int tier = DFA_E_WealthTier(p->wealthWan);
    const int* hs = DFA_E_GetWealthStyleScores(tier);
    const int* ts = DFA_E_GetTeamStyleScores(p->team);

    for (int s = 0; s < DFA_STYLE_COUNT; ++s) {
        out[s] = ws[s] + os[s] + ms[s] + ds[s] + hs[s] + ts[s];
    }
}

int DFA_E_PickBest(const int scores[DFA_STYLE_COUNT]) {
    if (!scores) return 0;
    int best = 0;
    for (int s = 1; s < DFA_STYLE_COUNT; ++s) {
        if (scores[s] > scores[best]) best = s;   // 同分保留编号小（风险低）
    }
    return best;
}

bool DFA_E_ForceRule(const DfaProfile* p, int& best) {
    if (!p) return false;
    if (p->diffIdx == DFA_DIFF_TOP_SECRET && p->wealthWan < 100) {
        best = DFA_STYLE_KNIFE_RUN;   // 强制跑刀
        return true;
    }
    return false;
}
