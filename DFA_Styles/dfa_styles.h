// ============================================================================
//  dfa_styles.h —— 玩法档案模块（工程：DFA_Styles，静态库）
//  职责：五种玩法的名称、定位标语与完整档案（装备/战术/经济/风险）
//  扩展：新增玩法 → 在 dfa_common.h 的 DfaStyle 末尾追加，并在 dfa_styles.cpp
//        的 kStyleNames / kStyleTags / kGuides 各追加一项；同时给各模块的
//        评分表每一行追加一列。
// ============================================================================
#ifndef DFA_STYLES_H
#define DFA_STYLES_H

#include "dfa_common.h"

struct StyleGuide {
    const char* loadout;   // 装备建议
    const char* tactics;   // 战术要点
    const char* economy;   // 经济规划
    const char* risk;      // 风险提醒
};

#ifdef __cplusplus
extern "C" {
#endif

const char*    DFA_S_GetName(int style);
const char*    DFA_S_GetTag(int style);
const StyleGuide* DFA_S_GetGuide(int style);   // 越界返回 nullptr

#ifdef __cplusplus
}
#endif

#endif // DFA_STYLES_H
