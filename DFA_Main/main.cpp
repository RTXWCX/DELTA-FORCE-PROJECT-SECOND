// ============================================================================
//  main.cpp —— 主程序（工程：DFA_Main，控制台应用）
//  职责：用户交互（菜单输入）、调用推荐引擎、输出完整建议
//  说明：本工程为「整合入口」，链接 5 个模块静态库：
//        DFA_Weapons / DFA_Maps / DFA_Economy / DFA_Styles / DFA_Engine
//  扩展：改数据去各模块 .cpp 的「配置区」；改输出文案直接改本文件。
// ============================================================================
#include "dfa_common.h"
#include "dfa_weapons.h"
#include "dfa_operators.h"
#include "dfa_maps.h"
#include "dfa_economy.h"
#include "dfa_styles.h"
#include "dfa_engine.h"

#include <windows.h>   // SetConsoleOutputCP / SetConsoleCP
#include <iostream>
#include <string>
#include <cstring>

// ============================ 工具函数 ============================

static void InitConsole() {
    SetConsoleOutputCP(65001);   // 控制台 UTF-8 显示中文
    SetConsoleCP(65001);
}

static int ReadInt(const char* prompt, int lo, int hi) {
    int v = 0;
    while (true) {
        std::cout << prompt;
        if (std::cin >> v) {
            if (v >= lo && v <= hi) return v;
            std::cout << "  >> 输入无效：请输入 " << lo << " ~ " << hi << " 之间的数字。\n";
        } else {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "  >> 输入无效：请输入数字。\n";
        }
    }
}

static long long ReadLL(const char* prompt) {
    long long v = 0;
    while (true) {
        std::cout << prompt;
        if (std::cin >> v) {
            if (v >= 0) return v;
            std::cout << "  >> 输入无效：存量不能为负。\n";
        } else {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "  >> 输入无效：请输入数字。\n";
        }
    }
}

static bool ReadYesNo(const char* prompt) {
    std::string s;
    while (true) {
        std::cout << prompt;
        std::cin >> s;
        if (s == "y" || s == "Y" || s == "1") return true;
        if (s == "n" || s == "N" || s == "0") return false;
        std::cout << "  >> 输入无效：请输入 y 或 n。\n";
    }
}

// ============================ 输出 ============================

static void PrintBanner() {
    std::cout << "=============================================================\n";
    std::cout << "      三角洲行动 · 烽火地带 玩法风格推荐器 v2.2（多模块）\n";
    std::cout << "  基于游戏内设定与玩家社区共识（NGA / B站 / 抖音百科）整理\n";
    std::cout << "=============================================================\n\n";
}

static void PrintProfile(const DfaProfile& p) {
    const WeaponInfo* w = DFA_W_GetWeapon(p.weaponIdx);
    const OperatorInfo* op = DFA_O_GetOperator(p.opIdx);
    const MapInfo*    m = DFA_M_GetMap(p.mapIdx);
    const DiffInfo*   d = DFA_M_GetDiff(p.diffIdx);
    int tier = DFA_E_WealthTier(p.wealthWan);

    std::cout << "---------------------- 你的输入档案 ----------------------\n";
    std::cout << "  擅长枪械  : " << (w ? w->name : "?") << "（" << (w ? DFA_W_GetClassName(w->cls) : "?")
              << " · " << (w ? w->subType : "?") << "）\n";
    std::cout << "  主玩干员  : " << (op ? op->code : "?") << "（" << (op ? op->name : "?") << " · "
              << (op ? DFA_O_GetRoleName(op->role) : "?") << "兵）\n";
    std::cout << "  常去地图  : " << (m ? m->name : "?") << "\n";
    std::cout << "  地图难度  : " << (d ? d->name : "?")
              << "（开放情况：" << (d ? d->availability : "?") << "）\n";
    std::cout << "  哈夫币存量: " << p.wealthWan << " 万（分档：" << DFA_E_GetTierName(tier) << "）\n";
    std::cout << "  固定队友  : " << (p.team ? "有" : "无") << "\n";
    std::cout << "---------------------------------------------------------\n\n";
}

static void PrintScores(const int scores[DFA_STYLE_COUNT]) {
    std::cout << "------------------------ 风格评分 ------------------------\n";
    std::cout << "  评分越高越契合；同分时优先推荐风险更低的玩法\n\n";
    for (int s = 0; s < DFA_STYLE_COUNT; ++s) {
        int vis = (int)std::strlen(DFA_S_GetName(s)) / 3 * 2;   // 中文对齐
        std::cout << "  " << DFA_S_GetName(s);
        for (int i = vis; i < 8; ++i) std::cout << " ";
        std::cout << ": " << scores[s] << " 分";
        if (scores[s] < 0) std::cout << "（不推荐）";
        std::cout << "\n";
    }
    std::cout << "---------------------------------------------------------\n\n";
}

static void PrintConclusion(int best, bool forced) {
    std::cout << "=============================================================\n";
    if (forced) {
        std::cout << "  ★ 强制建议（资金不足进入绝密风险极高）★\n";
    }
    std::cout << "  推荐玩法：\n";
    std::cout << "  ┌─────────────────────────────────────────────────┐\n";
    std::cout << "  │  【 " << DFA_S_GetName(best) << " 】  " << DFA_S_GetTag(best) << "  │\n";
    std::cout << "  └─────────────────────────────────────────────────┘\n";
    std::cout << "=============================================================\n\n";
}

static void PrintJudgement(const DfaProfile& p) {
    const WeaponInfo* w = DFA_W_GetWeapon(p.weaponIdx);
    const OperatorInfo* op = DFA_O_GetOperator(p.opIdx);
    const MapInfo*    m = DFA_M_GetMap(p.mapIdx);
    const DiffInfo*   d = DFA_M_GetDiff(p.diffIdx);
    int tier = DFA_E_WealthTier(p.wealthWan);

    const char* clsNote = nullptr;
    if (w) {
        switch (w->cls) {
            case WC_ASSAULT: clsNote = "万金油定位，远中近都能打，适合稳健与接战打法"; break;
            case WC_SMG:     clsNote = "近战爆发高，适合室内巷战，也是低成本玩法的性价比之选"; break;
            case WC_SNIPER:  clsNote = "擅长架点与远距离压制，适合选择性接战，近战偏弱"; break;
            case WC_DMR:     clsNote = "中远距离半自动火力，架点与持续输出平衡，近战一般"; break;
            case WC_SHOTGUN: clsNote = "近距离一击制胜，适合室内搜点，远距离乏力"; break;
            case WC_LMG:     clsNote = "持续火力压制，适合阵地推进，机动性一般"; break;
            case WC_PISTOL:  clsNote = "说明预算有限或主动降风险，低成本玩法更匹配"; break;
            case WC_SPECIAL: clsNote = "特殊武器（复合弓）安静隐蔽，适合摸人与低成本探索玩法"; break;
            default:         clsNote = "无特别倾向，玩法不受枪械类型限制"; break;
        }
    }

    std::cout << "------------------------ 判断依据 ------------------------\n";
    std::cout << "  1. 枪械  ：「" << (w ? w->name : "?") << "」属于"
              << (w ? DFA_W_GetClassName(w->cls) : "?") << "（二级分类："
              << (w ? w->subType : "?") << "），"
              << (clsNote ? clsNote : "未知") << "\n";
    if (op) {
        std::cout << "  2. 干员  ：「" << op->code << "」（" << op->name << " · "
                  << DFA_O_GetRoleName(op->role) << "兵）\n";
        std::cout << "      核心技能：" << op->skills << "\n";
        std::cout << "      适配提示：" << op->advice << "\n";
    }
    std::cout << "  3. 地图  ：" << (m ? m->name : "?") << " —— " << (m ? m->note : "?") << "\n";
    std::cout << "  4. 难度  ：" << (d ? d->name : "?") << " —— " << (d ? d->note : "?") << "\n";
    std::cout << "  5. 资金  ：" << p.wealthWan << " 万（" << DFA_E_GetTierName(tier)
              << "）—— " << DFA_E_GetTierNote(tier) << "\n";
    std::cout << "  6. 队友  ：" << (p.team ? "有固定队友，可执行协同战术，高对抗玩法（微交战/全装猛攻）收益更高"
                                           : "暂无固定队友，独狼更适合稳健或低成本玩法，接战更依赖个人判断") << "\n";
    std::cout << "---------------------------------------------------------\n\n";
}

static void PrintGuide(int best) {
    const StyleGuide* g = DFA_S_GetGuide(best);
    if (!g) return;
    std::cout << "------------------------ 玩法档案 ------------------------\n";
    std::cout << "  【装备建议】\n    " << g->loadout << "\n\n";
    std::cout << "  【战术要点】\n    " << g->tactics << "\n\n";
    std::cout << "  【经济规划】\n    " << g->economy << "\n\n";
    std::cout << "  【风险提醒】\n    " << g->risk << "\n";
    std::cout << "---------------------------------------------------------\n\n";
}

static void PrintWarnings(const DfaProfile& p, int best) {
    const OperatorInfo* op = DFA_O_GetOperator(p.opIdx);
    int tier = DFA_E_WealthTier(p.wealthWan);
    bool any = false;
    std::cout << "------------------------ 风险与提示 ----------------------\n";
    if (p.diffIdx == DFA_DIFF_TOP_SECRET && p.wealthWan < 500) {
        std::cout << "  [高危] 资金不足 500 万打绝密风险极高，建议先在机密练手攒钱。\n";
        any = true;
    }
    if (best == DFA_STYLE_FULL_PUSH && p.wealthWan < 500) {
        std::cout << "  [预警] 资金偏紧（<500万），全装一局成本 100w+，建议先降档到「微交战」。\n";
        any = true;
    }
    if (best == DFA_STYLE_FULL_PUSH && !p.team) {
        std::cout << "  [预警] 无固定队友单排全装猛攻风险高，建议先组到固定队再猛攻。\n";
        any = true;
    }
    if ((best == DFA_STYLE_KNIFE_RUN || best == DFA_STYLE_RAT) && tier == 3) {
        std::cout << "  [提示] 资金已雄厚仍跑刀/当鼠鼠效率偏低，可考虑「正常玩/微交战」提升收益。\n";
        any = true;
    }
    if (p.mapIdx == DFA_MAP_SPACE_BASE && best == DFA_STYLE_KNIFE_RUN) {
        std::cout << "  [提示] 航天基地是高强度图，跑刀收益差且易被清图队针对，慎选。\n";
        any = true;
    }
    if (p.mapIdx == DFA_MAP_TIDE_PRISON && best == DFA_STYLE_KNIFE_RUN) {
        std::cout << "  [提示] 潮汐监狱仅开放绝密，跑刀/低装玩法不适用，建议至少「正常玩」。\n";
        any = true;
    }
    if (p.diffIdx == DFA_DIFF_NIGHT) {
        std::cout << "  [提示] 永夜（黑夜）玩法视野受限，建议携带夜视仪并优先听声辨位。\n";
        any = true;
    }
    if (op && op->role == OP_ROLE_SUPPORT && !p.team) {
        std::cout << "  [提示] 支援干员独狼时团队价值有限，建议组固定队或改选进攻/侦察干员。\n";
        any = true;
    }
    if (p.diffIdx == DFA_DIFF_NIGHT && op && op->role == OP_ROLE_RECON) {
        std::cout << "  [提示] 侦察干员在永夜（黑夜）环境信息优势明显，收益更高。\n";
        any = true;
    }
    if (!any) {
        std::cout << "  当前条件下无明显风险预警，按推荐玩法执行即可。\n";
    }
    std::cout << "---------------------------------------------------------\n\n";
}

static void PrintFooter() {
    std::cout << "=============================================================\n";
    std::cout << "  说明：本程序依据游戏内设定与玩家社区共识编写，非官方数据。\n";
    std::cout << "  各模块数据均为独立工程中的「配置区」常量，修改后重新编译即可。\n";
    std::cout << "  工程结构：DFA_Weapons / DFA_Operators / DFA_Maps / DFA_Economy /\n";
    std::cout << "            DFA_Styles / DFA_Engine（引擎）/ DFA_Main（主程序整合入口）\n";
    std::cout << "=============================================================\n\n";
}

// ============================ 主流程 ============================

int main() {
    InitConsole();
    PrintBanner();

    bool again = true;
    while (again) {
        std::cout << "请依次输入以下 6 项信息（数字菜单或 y/n）：\n\n";

        // 【1/6】枪械（按大类分组展示，编号连续）
        int wCount = DFA_W_GetWeaponCount();
        std::cout << "【1/5】最擅长 / 精通的枪械（输入序号）：\n";
        {
            int lastCls = -1;
            for (int i = 0; i < wCount; ++i) {
                const WeaponInfo* w = DFA_W_GetWeapon(i);
                if (w->cls != lastCls) {
                    lastCls = w->cls;
                    std::cout << "  【" << DFA_W_GetClassName(w->cls) << "】\n";
                }
                std::cout << "    " << (i + 1) << ". " << w->name
                          << "（" << w->subType << "）\n";
            }
        }
        std::cout << "  请输入 1 ~ " << wCount << " 之间的序号：";
        int weapon = ReadInt("", 1, wCount) - 1;

        // 【2/6】干员（按兵种分组展示，编号连续）
        int oCount = DFA_O_GetOperatorCount();
        std::cout << "\n【2/6】主玩 / 精通的干员（输入序号）：\n";
        {
            int lastRole = -1;
            for (int i = 0; i < oCount; ++i) {
                const OperatorInfo* op = DFA_O_GetOperator(i);
                if (op->role != lastRole) {
                    lastRole = op->role;
                    std::cout << "  【" << DFA_O_GetRoleName(op->role) << "兵】\n";
                }
                std::cout << "    " << (i + 1) << ". " << op->code
                          << "（" << op->name << "）\n";
            }
        }
        std::cout << "  请输入 1 ~ " << oCount << " 之间的序号：";
        int opIdx = ReadInt("", 1, oCount) - 1;

        // 【3/6】地图
        int mCount = DFA_M_GetMapCount();
        std::cout << "\n【3/6】常去的地图（输入序号）：\n";
        for (int i = 0; i < mCount; ++i) {
            const MapInfo* m = DFA_M_GetMap(i);
            std::cout << "    " << (i + 1) << ". " << m->name << "\n";
        }
        std::cout << "  请输入 1 ~ " << mCount << " 之间的序号：";
        int map = ReadInt("", 1, mCount) - 1;

        // 【4/6】难度（只列出所选地图实际开放的难度）
        int diff = 0;
        {
            const MapInfo* mInfo = DFA_M_GetMap(map);
            int dCount = DFA_M_GetAllowedDiffCount(map);
            std::cout << "\n【4/6】常玩的难度（" << (mInfo ? mInfo->name : "?")
                      << " 开放难度，输入序号）：\n";
            for (int i = 0; i < dCount; ++i) {
                int did = DFA_M_GetAllowedDiff(map, i);
                const DiffInfo* d = DFA_M_GetDiff(did);
                std::cout << "    " << (i + 1) << ". " << (d ? d->name : "?") << "\n";
            }
            std::cout << "  请输入 1 ~ " << dCount << " 之间的序号：";
            int diffSlot = ReadInt("", 1, dCount) - 1;
            diff = DFA_M_GetAllowedDiff(map, diffSlot);   // 全局难度编号
        }

        // 【5/6】资金
        std::cout << "\n【5/6】当前哈夫币存量（单位：万，例如 500 表示 500 万哈夫币）：";
        long long wealth = ReadLL("  ");

        // 【6/6】队友
        std::cout << "\n【6/6】是否有固定组队队友？(y/n)：";
        bool team = ReadYesNo("  ");

        std::cout << "\n\n";

        DfaProfile p = { weapon, opIdx, map, diff, wealth, team };

        // ---- 评估 ----
        int scores[DFA_STYLE_COUNT];
        DFA_E_ComputeScores(&p, scores);
        int best = DFA_E_PickBest(scores);
        bool forced = DFA_E_ForceRule(&p, best);

        // ---- 输出 ----
        PrintProfile(p);
        PrintScores(scores);
        PrintConclusion(best, forced);
        PrintJudgement(p);
        PrintGuide(best);
        PrintWarnings(p, best);
        PrintFooter();

        std::cout << "是否重新评估？(y/n)：";
        again = ReadYesNo("  ");
        std::cout << "\n\n";
    }

    std::cout << "感谢使用，祝干员局局大红！按回车键退出...";
    std::cin.ignore(10000, '\n');
    std::cin.get();
    return 0;
}
