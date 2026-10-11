// ============================================================================
//  dfa_weapons.h —— 枪械模块（工程：DFA_Weapons，静态库）
//  职责：枪械清单（含大类 + 二级分类）、各类别对五种玩法的评分
//  扩展：新增枪械 → 只需在 dfa_weapons.cpp 的 kWeapons 数组加一行
// ============================================================================
#ifndef DFA_WEAPONS_H
#define DFA_WEAPONS_H

#include "dfa_common.h"

// 枪械大类
enum WeaponClass {
    WC_ASSAULT = 0,   // 突击步枪
    WC_SMG,           // 冲锋枪
    WC_SNIPER,        // 狙击步枪
    WC_DMR,           // 射手步枪
    WC_SHOTGUN,       // 霰弹枪
    WC_LMG,           // 轻机枪
    WC_PISTOL,        // 手枪
    WC_SPECIAL,       // 特殊武器
    WC_GENERIC,       // 其他 / 通用
    WC_CLASS_COUNT
};

struct WeaponInfo {
    const char* name;      // 枪械名
    const char* subType;   // 二级分类（游戏内类型标签：gunRifle / gunSMG / gunSniper /
                           // gunMP / gunShotgun / gunLMG / gunPistol / special）
    WeaponClass cls;       // 大类
};

#ifdef __cplusplus
extern "C" {
#endif

// 枪械清单
int               DFA_W_GetWeaponCount();
const WeaponInfo* DFA_W_GetWeapon(int idx);       // 越界返回 nullptr

// 大类名称
const char*       DFA_W_GetClassName(WeaponClass c);

// 某大类枪械对五种玩法的评分（返回 5 元素数组，顺序见 dfa_common.h）
const int*        DFA_W_GetStyleScores(WeaponClass c);

#ifdef __cplusplus
}
#endif

#endif // DFA_WEAPONS_H
