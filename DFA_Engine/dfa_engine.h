// ============================================================================
//  dfa_engine.h —— 推荐引擎模块（工程：DFA_Engine，静态库）
//  职责：汇总各模块评分、选出最佳玩法、执行强制兜底规则
//  说明：本模块依赖 DFA_Weapons / DFA_Maps / DFA_Economy / DFA_Operators 数据模块
// ============================================================================
#ifndef DFA_ENGINE_H
#define DFA_ENGINE_H

#include "dfa_common.h"

// 用户输入档案（字段含义见各模块头文件）
struct DfaProfile {
    int        weaponIdx;   // 枪械菜单编号（0 起）
    int        opIdx;       // 干员菜单编号（0 起，见 DFA_Operators 模块）
    int        mapIdx;      // 地图编号（DfaMapId）
    int        diffIdx;     // 难度编号（DfaDifficulty）
    long long  wealthWan;   // 哈夫币存量（单位：万）
    bool       team;        // 是否有固定队友
};

#ifdef __cplusplus
extern "C" {
#endif

// 计算五种玩法的得分（分数越高越契合；out 需为 5 元素数组）
void DFA_E_ComputeScores(const DfaProfile* p, int out[DFA_STYLE_COUNT]);

// 选出得分最高的风格；同分取编号小（风险更低）的
int  DFA_E_PickBest(const int scores[DFA_STYLE_COUNT]);

// 强制兜底规则：绝密 + 资金不足 100 万 → 强制推荐跑刀（返回 true 表示触发）
// 触发时 best 被改写为跑刀编号
bool DFA_E_ForceRule(const DfaProfile* p, int& best);

#ifdef __cplusplus
}
#endif

#endif // DFA_ENGINE_H
