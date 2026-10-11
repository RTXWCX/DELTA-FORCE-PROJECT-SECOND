// ============================================================================
//  dfa_operators.h —— 干员模块（工程：DFA_Operators，静态库）
//  职责：干员清单（代号/本名/兵种/核心技能/适配提示）、各兵种对五种玩法的评分
//  数据源：云马克（yunmaku.com）干员资料页（2026-09-24 调整后版本）
//  扩展：新增干员 → 只需在 dfa_operators.cpp 的 kOperators 数组加一行
// ============================================================================
#ifndef DFA_OPERATORS_H
#define DFA_OPERATORS_H

#include "dfa_common.h"

// 干员兵种
enum DfaRole {
    OP_ROLE_SUPPORT = 0,   // 支援兵
    OP_ROLE_ENGINEER,      // 工程兵
    OP_ROLE_RECON,         // 侦察兵
    OP_ROLE_ASSAULT,       // 突击兵
    OP_ROLE_COUNT
};

struct OperatorInfo {
    const char* code;      // 干员代号
    const char* name;      // 本名
    DfaRole     role;      // 兵种
    const char* skills;    // 核心技能摘要（干员特长/战术装备/战术道具）
    const char* advice;    // 玩法适配提示（输出判断依据用）
};

#ifdef __cplusplus
extern "C" {
#endif

// 干员清单
int               DFA_O_GetOperatorCount();
const OperatorInfo* DFA_O_GetOperator(int idx);   // 越界返回 nullptr

// 兵种名称（如「支援」）
const char*       DFA_O_GetRoleName(DfaRole r);

// 某兵种对五种玩法的评分（返回 5 元素数组，顺序见 dfa_common.h）
const int*        DFA_O_GetRoleScores(DfaRole r);

#ifdef __cplusplus
}
#endif

#endif // DFA_OPERATORS_H
