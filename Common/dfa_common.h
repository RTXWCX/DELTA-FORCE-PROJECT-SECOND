// ============================================================================
//  dfa_common.h —— 模块公共头（所有工程共享，勿改风格编号顺序）
//  三角洲行动 · 烽火地带 玩法风格推荐器（多工程模块化版）
// ============================================================================
#ifndef DFA_COMMON_H
#define DFA_COMMON_H

// 五种玩法风格统一编号（所有模块共用，新增玩法时在末尾追加）
enum DfaStyle {
    DFA_STYLE_KNIFE_RUN = 0,   // 跑刀（风险最低）
    DFA_STYLE_RAT,             // 当鼠鼠
    DFA_STYLE_NORMAL,          // 正常玩
    DFA_STYLE_LIGHT_COMBAT,    // 微交战
    DFA_STYLE_FULL_PUSH,       // 全装猛攻（风险最高）
    DFA_STYLE_COUNT            // 风格总数（=5）
};

#endif // DFA_COMMON_H
