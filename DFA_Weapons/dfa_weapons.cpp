// ============================================================================
//  dfa_weapons.cpp —— 枪械模块实现
//  ============================================================================
//  【配置区】以下数据集中在本文件顶部，日常微调只改这里：
//   - 新增枪械：kWeapons 数组末尾追加 { "枪名", "二级分类标签", 大类 }，菜单自动出现。
//   - 调整枪械对玩法的加成：改 kWeaponScore 表中对应大类的一行。
//   - 新增大类：在 enum WeaponClass 追加一项，并在 kWeaponClassNames / kWeaponScore 各补一项。
// ============================================================================
#include "dfa_weapons.h"

// ---------- 枪械大类名称（顺序 = WeaponClass） ----------
static const char* kWeaponClassNames[WC_CLASS_COUNT] = {
    "突击步枪", "冲锋枪", "狙击步枪", "射手步枪", "霰弹枪", "轻机枪", "手枪", "特殊武器", "通用"
};

// ---------- 枪械清单（菜单顺序即编号；二级分类为游戏内类型标签） ----------
static const WeaponInfo kWeapons[] = {
    // ===== 突击步枪类（gunRifle） =====
    { "KC17",            "gunRifle",   WC_ASSAULT },
    { "MK47",            "gunRifle",   WC_ASSAULT },
    { "M4A1",            "gunRifle",   WC_ASSAULT },
    { "AKM",             "gunRifle",   WC_ASSAULT },
    { "QBZ95-1",         "gunRifle",   WC_ASSAULT },
    { "AKS-74U",         "gunRifle",   WC_ASSAULT },
    { "ASH-12",          "gunRifle",   WC_ASSAULT },
    { "K416",            "gunRifle",   WC_ASSAULT },
    { "M16A4",           "gunRifle",   WC_ASSAULT },
    { "AUG",             "gunRifle",   WC_ASSAULT },
    { "M7",              "gunRifle",   WC_ASSAULT },
    { "SG552",           "gunRifle",   WC_ASSAULT },
    { "AK-12",           "gunRifle",   WC_ASSAULT },
    { "SCAR-H",          "gunRifle",   WC_ASSAULT },
    { "G3",              "gunRifle",   WC_ASSAULT },
    { "PTR-32",          "gunRifle",   WC_ASSAULT },
    { "CAR-15",          "gunRifle",   WC_ASSAULT },
    { "AS Val",          "gunRifle",   WC_ASSAULT },
    { "腾龙",            "gunRifle",   WC_ASSAULT },
    { "K437",            "gunRifle",   WC_ASSAULT },
    { "MCX LT",          "gunRifle",   WC_ASSAULT },
    { "AR57",            "gunRifle",   WC_ASSAULT },
    { "RM277",           "gunRifle",   WC_ASSAULT },
    { "MDR",             "gunRifle",   WC_ASSAULT },
    // ===== 冲锋枪类（gunSMG） =====
    { "MK4",             "gunSMG",     WC_SMG },
    { "QCQ171",          "gunSMG",     WC_SMG },
    { "MP5",             "gunSMG",     WC_SMG },
    { "P90",             "gunSMG",     WC_SMG },
    { "Vector",          "gunSMG",     WC_SMG },
    { "UZI",             "gunSMG",     WC_SMG },
    { "野牛",            "gunSMG",     WC_SMG },
    { "SMG-45",          "gunSMG",     WC_SMG },
    { "SR-3M",           "gunSMG",     WC_SMG },
    { "勇士",            "gunSMG",     WC_SMG },
    { "MP7",             "gunSMG",     WC_SMG },
    { "汤姆逊",          "gunSMG",     WC_SMG },
    // ===== 狙击步枪类（gunSniper） =====
    { "SV-98",           "gunSniper",  WC_SNIPER },
    { "R93",             "gunSniper",  WC_SNIPER },
    { "M700",            "gunSniper",  WC_SNIPER },
    { "AWM",             "gunSniper",  WC_SNIPER },
    { "M82",             "gunSniper",  WC_SNIPER },
    // ===== 射手步枪类（gunMP） =====
    { "Mini-14",         "gunMP",      WC_DMR },
    { "VSS",             "gunMP",      WC_DMR },
    { "SVD",             "gunMP",      WC_DMR },
    { "M14",             "gunMP",      WC_DMR },
    { "SKS",             "gunMP",      WC_DMR },
    { "SR-25",           "gunMP",      WC_DMR },
    { "PSG-1",           "gunMP",      WC_DMR },
    { "杠杆式步枪",      "gunMP",      WC_DMR },
    { "SR9",             "gunMP",      WC_DMR },
    { "SVCH",            "gunMP",      WC_DMR },
    // ===== 霰弹枪类（gunShotgun） =====
    { "M1014",           "gunShotgun", WC_SHOTGUN },
    { "S12K",            "gunShotgun", WC_SHOTGUN },
    { "M870",            "gunShotgun", WC_SHOTGUN },
    { "725双管",         "gunShotgun", WC_SHOTGUN },
    { "FS-12",           "gunShotgun", WC_SHOTGUN },
    // ===== 机枪类（gunLMG） =====
    { "PKM",             "gunLMG",     WC_LMG },
    { "M249",            "gunLMG",     WC_LMG },
    { "M250",            "gunLMG",     WC_LMG },
    { "QJB201",          "gunLMG",     WC_LMG },
    // ===== 手枪类（gunPistol） =====
    { "QSZ92G",          "gunPistol",  WC_PISTOL },
    { "357左轮",         "gunPistol",  WC_PISTOL },
    { "沙漠之鹰",        "gunPistol",  WC_PISTOL },
    { "G18",             "gunPistol",  WC_PISTOL },
    { "93R",             "gunPistol",  WC_PISTOL },
    { "G17",             "gunPistol",  WC_PISTOL },
    { "M1911",           "gunPistol",  WC_PISTOL },
    // ===== 特殊武器类（special） =====
    { "复合弓",          "special",    WC_SPECIAL },
    // ===== 兜底选项 =====
    { "其他 / 通用",     "other",      WC_GENERIC },
};

// ---------- 各大类对五种玩法的加分 ----------
// 列顺序 = DfaStyle：跑刀 / 鼠鼠 / 正常玩 / 微交战 / 全装猛攻
static const int kWeaponScore[WC_CLASS_COUNT][DFA_STYLE_COUNT] = {
    /* 突击步枪 */ {  0,  0,  3,  4,  4 },
    /* 冲锋枪   */ {  1,  2,  2,  3,  3 },
    /* 狙击步枪 */ {  0,  1,  2,  4,  1 },
    /* 射手步枪 */ {  0,  1,  2,  3,  1 },
    /* 霰弹枪   */ {  2,  2,  1,  1,  1 },
    /* 轻机枪   */ {  0,  0,  2,  3,  2 },
    /* 手枪     */ {  3,  3, -1, -2, -3 },
    /* 特殊武器 */ {  2,  3,  1,  1, -2 },
    /* 通用     */ {  0,  0,  1,  1,  1 },
};

int DFA_W_GetWeaponCount() {
    return (int)(sizeof(kWeapons) / sizeof(kWeapons[0]));
}

const WeaponInfo* DFA_W_GetWeapon(int idx) {
    if (idx < 0 || idx >= DFA_W_GetWeaponCount()) return nullptr;
    return &kWeapons[idx];
}

const char* DFA_W_GetClassName(WeaponClass c) {
    if (c < 0 || c >= WC_CLASS_COUNT) return "未知";
    return kWeaponClassNames[c];
}

const int* DFA_W_GetStyleScores(WeaponClass c) {
    static const int kZero[DFA_STYLE_COUNT] = { 0, 0, 0, 0, 0 };
    if (c < 0 || c >= WC_CLASS_COUNT) return kZero;
    return kWeaponScore[c];
}
