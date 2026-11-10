// ============================================================================
//  dfa_custom_weapons.h —— 自定义枪械模块（工程：DFA_CustomWeapons，静态库）
//  职责：扫描指定文件夹内的文本文档（*.txt / *.csv），解析玩家自备的枪械数据，
//        与内置 69 把枪一起参与菜单选择与评分。
//  依赖：DFA_Weapons（复用 WeaponClass 枚举、大类名称与默认评分表）
//  扩展：数据格式见文件底部注释；改解析规则只改 dfa_custom_weapons.cpp。
// ============================================================================
#ifndef DFA_CUSTOM_WEAPONS_H
#define DFA_CUSTOM_WEAPONS_H

#include "dfa_weapons.h"

#ifdef __cplusplus
extern "C" {
#endif

// 载入并解析指定文件夹（UTF-8 路径）内的全部 *.txt / *.csv 文件。
// 返回值：成功载入的枪械条数（>=0）；-1 表示文件夹不存在或无法访问。
// 注意：重复调用会先清空上一次的数据；解析失败的行走错误列表，不中断整体载入。
int DFA_CW_Load(const char* folderUtf8);

// 已载入的自定义枪械数量
int DFA_CW_GetCount();

// 第 idx 把自定义枪械（0 起）；越界返回 nullptr。
// 返回的 WeaponInfo 中 subType 字段为数据来源文件名，便于在界面上标注。
const WeaponInfo* DFA_CW_GetWeapon(int idx);

// 第 idx 把自定义枪械的 5 维评分（顺序同 DfaStyle）；越界返回 nullptr。
// 载入时已把「省略分数」的条目按所属大类默认分填好，调用方无需再兜底。
const int* DFA_CW_GetStyleScores(int idx);

// 本次实际扫描的文件夹（UTF-8）；从未载入时返回 ""
const char* DFA_CW_GetFolder();

// 默认文件夹：<exe 所在目录>\自定义枪械（UTF-8）
const char* DFA_CW_GetDefaultFolder();

// 本次扫描到的数据文件个数
int DFA_CW_GetFileCount();

// 解析错误条数 / 第 idx 条错误描述（形如 "我的枪械.txt:3 大类无法识别：XXX"）
int         DFA_CW_GetErrorCount();
const char* DFA_CW_GetError(int idx);

// 自定义枪械容量上限（当前 256），供界面提示
int DFA_CW_GetCapacity();

#ifdef __cplusplus
}
#endif

// ============================================================================
//  数据文件格式（每个文件可写多行，顺序即菜单顺序，按文件名升序依次读取）
//
//  行格式：  枪名 | 大类 | 跑刀,鼠鼠,正常玩,微交战,全装猛攻
//  分隔符：  | 或 , 或 全角，/、  均可；行首 # / ; / // 为注释。
//  分数：    5 个整数（-99~99），省略时套用该大类的默认评分。
//  大类：    突击步枪 / 冲锋枪 / 狙击步枪 / 射手步枪 / 霰弹枪 / 轻机枪 /
//            手枪 / 特殊武器 / 通用（也接受 gunRifle、SMG 等别名）
//  示例：    我的M4A1 | 突击步枪 | 1,1,4,5,5
//            测试狙   | 狙击步枪
// ============================================================================

#endif // DFA_CUSTOM_WEAPONS_H
