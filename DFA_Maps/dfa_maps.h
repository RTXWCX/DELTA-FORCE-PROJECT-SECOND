// ============================================================================
//  dfa_maps.h —— 地图 + 难度模块（工程：DFA_Maps，静态库）
//  职责：地图清单、难度清单、各地图/难度对五种玩法的评分、
//        以及「每张地图实际开放的难度」（难度按地图限定）
//  扩展：新增地图/难度 → 在 dfa_maps.cpp 对应数组加一行 + 评分表加一行；
//        改某图的难度开放 → 改 kAllowedDiffs 表中对应行
// ============================================================================
#ifndef DFA_MAPS_H
#define DFA_MAPS_H

#include "dfa_common.h"

// 地图编号
enum DfaMapId {
    DFA_MAP_ZERO_DAM = 0,   // 零号大坝
    DFA_MAP_LONG_BOW,       // 长弓溪谷
    DFA_MAP_BAKSHI,         // 巴克什
    DFA_MAP_SPACE_BASE,     // 航天基地
    DFA_MAP_AZ3,            // AZ3
    DFA_MAP_TIDE_PRISON,    // 潮汐监狱
    DFA_MAP_COUNT
};

// 难度编号
enum DfaDifficulty {
    DFA_DIFF_NORMAL = 0,    // 普通
    DFA_DIFF_SECRET,        // 机密
    DFA_DIFF_TOP_SECRET,    // 绝密
    DFA_DIFF_NIGHT,         // 永夜（黑夜玩法）
    DFA_DIFF_COUNT
};

struct MapInfo {
    const char* name;
    const char* note;       // 地图特点（用于输出判断依据）
};

struct DiffInfo {
    const char* name;
    const char* note;         // 难度特点（用于输出判断依据）
    const char* availability; // 开放情况说明（哪些地图开放该难度）
};

#ifdef __cplusplus
extern "C" {
#endif

// 地图
int      DFA_M_GetMapCount();
const MapInfo* DFA_M_GetMap(int idx);           // 越界返回 nullptr
const int*     DFA_M_GetMapStyleScores(int idx);

// 难度
int      DFA_M_GetDiffCount();
const DiffInfo* DFA_M_GetDiff(int idx);         // 越界返回 nullptr
const int*     DFA_M_GetDiffStyleScores(int idx);

// 指定地图实际开放的难度（slot 从 0 起，返回 DfaDifficulty 值；越界返回 -1）
int      DFA_M_GetAllowedDiffCount(int mapIdx);
int      DFA_M_GetAllowedDiff(int mapIdx, int slot);

#ifdef __cplusplus
}
#endif

#endif // DFA_MAPS_H
