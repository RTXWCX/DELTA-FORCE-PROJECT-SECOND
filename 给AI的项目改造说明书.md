# 三角洲行动 · 玩法风格推荐器 v2.3 —— 项目改造说明书（喂给 AI 用）

> 这份文档的作用：把项目结构、数据契约、评分算法、扩展方法和硬性约束讲清楚，让 AI（DeepSeek / ChatGPT 等）在不看全部源码的情况下也能准确改代码，不会乱改接口、不会破坏数组顺序、不会写出编译不过的代码。
>
> **配套用法**：先把本文档整段贴给 AI，再按需贴对应模块的 `.h` 和 `.cpp` 原文。涉及跨模块改动时，把 `dfa_common.h` + 相关几个模块一起贴。

---

## 0. 项目一句话概括

一个 Windows 控制台 C++17 程序。用户输入 6 项信息（枪械、干员、地图、难度、哈夫币存量、有无固定队友），程序用 6 张固定权重表做**加权求和**，给 5 种玩法（跑刀 / 当鼠鼠 / 正常玩 / 微交战 / 全装猛攻）各打一个分，取最高分作为推荐，并输出判断依据、玩法档案、风险提示。

设计原则：**数据与逻辑分离**。所有战术数据都是各模块 `.cpp` 顶部的静态常量数组（"配置区"），评分逻辑集中在 `DFA_Engine`，界面在 `main.cpp`。改数据不改算法，改算法不动数据。

---

## 1. 技术栈与构建

| 项 | 值 |
|---|---|
| 语言标准 | C++17（`stdcpp17`，`ConformanceMode` 开启） |
| 平台 | Visual Studio 2026（工具集 `v145`），解决方案 `DeltaForceAdvisor.sln` |
| 目标 | x64 / Release（也配了 Win32，但只维护 x64） |
| 编译选项 | `/utf-8`；`RuntimeLibrary = MultiThreaded`（**静态 CRT**，产物是单个 exe，目标机不需要装 VC 运行库） |
| 字符集 | `CharacterSet = NotSet`（不是 Unicode 工程，字符串就是普通 `const char*` + UTF-8 字节） |
| 源码编码 | UTF-8 **无 BOM**，必须保持，不要存成 ANSI/GBK，否则中文全乱 |
| 输出路径 | `bin\$(Platform)\$(Configuration)\DFA_Main.exe` |

命令行构建（已验证可用）：

```
"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ^
  "…\DeltaForceAdvisor.sln" /t:Rebuild /p:Configuration=Release /p:Platform=x64 /m /v:minimal
```

控制台中文显示：`main.cpp` 里 `InitConsole()` 调 `SetConsoleOutputCP(65001)` + `SetConsoleCP(65001)`，把控制台切到 UTF-8。这段别删。

---

## 2. 模块结构

8 个工程：7 个静态库 + 1 个可执行程序。依赖方向单向，不允许出现反向引用或环形依赖。

```
Common\dfa_common.h          公共枚举（玩法风格），所有模块共享
        ▲
        │ include
DFA_Weapons\    枪械清单 + 按"枪械大类"评分      ─┐
DFA_CustomWeapons\ 从 txt 文件夹读私人枪械（依赖 Weapons） ─┤
DFA_Operators\  干员清单 + 按"兵种"评分          ─┤
DFA_Maps\       地图/难度清单 + 三张评分表        ─┼─→ DFA_Engine\ 加权求和 + 选最优 + 强制规则
DFA_Economy\    资金分档 + 队友评分              ─┤        ▲
DFA_Styles\     玩法名称/标语/攻略文案           ─┘        │ 链接
                                                          DFA_Main\ main.cpp 交互与输出
```

| 模块 | 文件 | 职责 | 配置区位置 |
|---|---|---|---|
| 公共 | `Common\dfa_common.h` | `enum DfaStyle`（5 种玩法 + `DFA_STYLE_COUNT`） | 期望顺序：跑刀 0 / 鼠鼠 1 / 正常玩 2 / 微交战 3 / 全装猛攻 4 |
| 枪械 | `DFA_Weapons\dfa_weapons.h/.cpp` | 69 把枪；9 个大类各自的 5 维分数；大类名解析 | `dfa_weapons.cpp` 顶部 `kWeapons` / `kWeaponScore` / 末尾 `kClassAliases` |
| 自定义枪械 | `DFA_CustomWeapons\dfa_custom_weapons.h/.cpp` | 扫描「自定义枪械」文件夹的 .txt/.csv，把私人枪械读进内存（最多 256 条） | 函数体 + 文件尾的格式注释 |
| 干员 | `DFA_Operators\dfa_operators.h/.cpp` | 18 名干员；4 个兵种各自的 5 维分数 | `dfa_operators.cpp` 顶部 `kOperators` / `kRoleScore` |
| 地图 | `DFA_Maps\dfa_maps.h/.cpp` | 6 张图、5 种难度；地图分 + 难度分 + 每图开放难度 + 核心物资区文案 | `dfa_maps.cpp` 顶部 `kMaps` / `kDiffs` / `kMapScore` / `kDiffScore` / `kAllowedDiffs` / `kLootZones` |
| 经济 | `DFA_Economy\dfa_economy.h/.cpp` | 资金 4 档 + 队友 2 态，各 5 维分数 | `dfa_economy.cpp` 顶部 `kWealthTiers` / `kWealthScore` / `kTeamScore` |
| 玩法 | `DFA_Styles\dfa_styles.h/.cpp` | 玩法名、标语、4 段攻略文案 | `dfa_styles.cpp` 顶部 `kStyleNames` / `kStyleTags` / `kGuides` |
| 引擎 | `DFA_Engine\dfa_engine.h/.cpp` | 加权求和、取最优、强制兜底规则 | 函数体本身 |
| 主程序 | `DFA_Main\main.cpp` | 6 步输入、7 段输出、风险提示文案 | 各 `PrintXxx` 函数 |

**命名坑提醒**：`DFA_Economy` 和 `DFA_Engine` 两个模块的函数前缀**都是 `DFA_E_`**（`DFA_E_WealthTier` 属经济模块，`DFA_E_ComputeScores` 属引擎模块）。改代码时按头文件判断归属，别搞混。

---

## 3. 核心数据契约（改动时不许破坏）

```cpp
// dfa_common.h
enum DfaStyle {
    DFA_STYLE_KNIFE_RUN = 0,   // 跑刀
    DFA_STYLE_RAT,             // 当鼠鼠
    DFA_STYLE_NORMAL,          // 正常玩
    DFA_STYLE_LIGHT_COMBAT,    // 微交战
    DFA_STYLE_FULL_PUSH,       // 全装猛攻
    DFA_STYLE_COUNT            // = 5
};

// 枪械
enum WeaponClass { WC_ASSAULT=0, WC_SMG, WC_SNIPER, WC_DMR, WC_SHOTGUN,
                   WC_LMG, WC_PISTOL, WC_SPECIAL, WC_GENERIC, WC_CLASS_COUNT };
struct WeaponInfo { const char* name; const char* subType; WeaponClass cls; };

// 干员
enum DfaRole { OP_ROLE_SUPPORT=0, OP_ROLE_ENGINEER, OP_ROLE_RECON, OP_ROLE_ASSAULT, OP_ROLE_COUNT };
struct OperatorInfo { const char* code; const char* name; DfaRole role;
                      const char* skills; const char* advice; };

// 地图 / 难度
enum DfaMapId { DFA_MAP_ZERO_DAM=0, DFA_MAP_LONG_BOW, DFA_MAP_BAKSHI,
                DFA_MAP_SPACE_BASE, DFA_MAP_AZ3, DFA_MAP_TIDE_PRISON, DFA_MAP_COUNT };
enum DfaDifficulty { DFA_DIFF_NORMAL=0, DFA_DIFF_SECRET, DFA_DIFF_TOP_SECRET,
                     DFA_DIFF_NIGHT, DFA_DIFF_SUPERCRITICAL, DFA_DIFF_COUNT };
struct MapInfo  { const char* name; const char* note; };
struct DiffInfo { const char* name; const char* note; const char* availability; };

// 玩法文案
struct StyleGuide { const char* loadout; const char* tactics;
                    const char* economy; const char* risk; };

// 用户输入档案（引擎入口唯一参数）
struct DfaProfile {
    int weaponIdx;      // 统一菜单下标：0..内置数-1 = kWeapons；自定义 = 内置数 + DFA_CW 序号
    int opIdx;          // kOperators 下标，0 起
    int mapIdx;         // DfaMapId
    int diffIdx;        // DfaDifficulty（注意：是全局难度编号，不是"该图第几个难度"）
    long long wealthWan;// 哈夫币，单位"万"
    bool team;          // 有无固定队友
};
```

**几条硬规则**

1. 所有数组**顺序即编号**。菜单序号 = 数组下标 + 1。中间插入一行会让后面所有编号平移，用户凭序号输入的旧习惯会失效。
2. 枚举里 `XXX_COUNT` 必须是最后一项，所有表都以它为维度。
3. 所有 `GetXxx(idx)` 越界返回 `nullptr` 或全 0 数组，**不允许抛异常、不允许直接下标访问崩掉**。
4. 字符串一律 `const char*`，不要改成 `std::string`（会破坏 `extern "C"` 链接和静态库接口）。
5. 头文件接口都包在 `extern "C"` 里，新增接口也必须在里面。
6. `weaponIdx` 不是"永远是内置下标"：引擎先比较 `weaponIdx < DFA_W_GetWeaponCount()`，小于走内置，大于等于则按 `weaponIdx - 内置数` 去取 `DFA_CW_GetStyleScores`。任何新代码要用枪械数据，都走 `main.cpp` 的 `WeaponAt(idx)`，不要直接 `DFA_W_GetWeapon`。

---

## 4. 评分模型（唯一算法，务必保持这个形状）

```cpp
// dfa_engine.cpp —— 总分 = 六路分项相加，无乘法、无归一化、无随机
out[s] = 枪械大类分[s] + 兵种分[s] + 地图分[s] + 难度分[s] + 资金档分[s] + 队友分[s];

// 选最优：分数最高者；同分保留下标更小者（下标小的玩法风险更低）
int best = 0;
for (int s = 1; s < DFA_STYLE_COUNT; ++s)
    if (scores[s] > scores[best]) best = s;

// 强制兜底：绝密 + 资金 < 100 万 → 无条件改成跑刀，并置 forced = true
if (diffIdx == DFA_DIFF_TOP_SECRET && wealthWan < 100) { best = DFA_STYLE_KNIFE_RUN; return true; }
```

列顺序永远是：**跑刀 / 鼠鼠 / 正常玩 / 微交战 / 全装猛攻**。

### 4.1 枪械大类分 `kWeaponScore[WC_CLASS_COUNT][5]`

| 大类 | 跑刀 | 鼠鼠 | 正常玩 | 微交战 | 全装猛攻 |
|---|---|---|---|---|---|
| 突击步枪 | 0 | 0 | 3 | 4 | 4 |
| 冲锋枪 | 1 | 2 | 2 | 3 | 3 |
| 狙击步枪 | 0 | 1 | 2 | 4 | 1 |
| 射手步枪 | 0 | 1 | 2 | 3 | 1 |
| 霰弹枪 | 2 | 2 | 1 | 1 | 1 |
| 轻机枪 | 0 | 0 | 2 | 3 | 2 |
| 手枪 | 3 | 3 | -1 | -2 | -3 |
| 特殊武器 | 2 | 3 | 1 | 1 | -2 |
| 通用 | 0 | 0 | 1 | 1 | 1 |

### 4.2 兵种分 `kRoleScore[OP_ROLE_COUNT][5]`

| 兵种 | 跑刀 | 鼠鼠 | 正常玩 | 微交战 | 全装猛攻 |
|---|---|---|---|---|---|
| 支援 | 0 | 2 | 3 | 1 | 1 |
| 工程 | 1 | 1 | 2 | 2 | 2 |
| 侦察 | 2 | 3 | 2 | 1 | 0 |
| 突击 | 0 | 0 | 2 | 4 | 4 |

### 4.3 地图分 `kMapScore[DFA_MAP_COUNT][5]`

| 地图 | 跑刀 | 鼠鼠 | 正常玩 | 微交战 | 全装猛攻 |
|---|---|---|---|---|---|
| 零号大坝 | 2 | 2 | 1 | 0 | -1 |
| 长弓溪谷 | 1 | 1 | 2 | 1 | 0 |
| 巴克什 | 0 | 1 | 1 | 2 | 2 |
| 航天基地 | -2 | 0 | 0 | 2 | 3 |
| AZ3 | 0 | 1 | 1 | 2 | 2 |
| 潮汐监狱 | -2 | 0 | 0 | 1 | 3 |

### 4.4 难度分 `kDiffScore[DFA_DIFF_COUNT][5]`

| 难度 | 跑刀 | 鼠鼠 | 正常玩 | 微交战 | 全装猛攻 |
|---|---|---|---|---|---|
| 普通 | 2 | 1 | 1 | -1 | -2 |
| 机密 | 0 | 2 | 2 | 1 | 0 |
| 绝密 | -3 | -1 | -1 | 1 | 3 |
| 永夜 | 2 | 3 | 1 | 1 | -1 |
| 超临界 | -4 | -2 | -2 | 1 | 5 |

### 4.5 资金档分 `kWealthScore[4][5]`（阈值 `kWealthTiers = {100, 500, 1500}`，单位万）

| 档 | 区间 | 跑刀 | 鼠鼠 | 正常玩 | 微交战 | 全装猛攻 |
|---|---|---|---|---|---|---|
| 紧张 | <100 | 4 | 3 | -2 | -3 | -4 |
| 小康 | 100~500 | 0 | 1 | 2 | 0 | -1 |
| 宽裕 | 500~1500 | -2 | -1 | 2 | 2 | 1 |
| 雄厚 | ≥1500 | -4 | -3 | 0 | 2 | 3 |

### 4.6 队友分 `kTeamScore[2][5]`

| 状态 | 跑刀 | 鼠鼠 | 正常玩 | 微交战 | 全装猛攻 |
|---|---|---|---|---|---|
| 无固排 | 2 | 2 | 1 | 0 | -2 |
| 有固排 | -2 | -2 | 1 | 2 | 3 |

### 4.7 每图开放难度 `kAllowedDiffs[DFA_MAP_COUNT][4]`

第二维固定 **4 列**，`-1` 是结束哨兵，因此**每张图最多只能列 3 个难度**。

| 地图 | 列 0 | 列 1 | 列 2 | 列 3 |
|---|---|---|---|---|
| 零号大坝 | 普通 | 机密 | 永夜 | -1 |
| 长弓溪谷 | 普通 | 机密 | -1 | -1 |
| 巴克什 | 机密 | 绝密 | -1 | -1 |
| 航天基地 | 机密 | 绝密 | -1 | -1 |
| AZ3 | 普通 | 绝密 | 超临界 | -1 |
| 潮汐监狱 | 绝密 | -1 | -1 | -1 |

---

## 5. 模块 API 清单

| 签名 | 语义 | 越界行为 |
|---|---|---|
| `int DFA_W_GetWeaponCount()` | 枪械总数（`sizeof` 动态算，不是宏） | — |
| `const WeaponInfo* DFA_W_GetWeapon(int idx)` | 取第 idx 把枪 | 返回 `nullptr` |
| `const char* DFA_W_GetClassName(WeaponClass c)` | 大类中文名 | 返回 `"未知"` |
| `const int* DFA_W_GetStyleScores(WeaponClass c)` | 该大类 5 维分 | 返回全 0 数组 |
| `int DFA_W_FindClass(const char* name)` | 大类名（中文全名或 `gunRifle` 等别名）→ `WeaponClass` | 未知返回 `-1` |
| `int DFA_CW_Load(const char* folderUtf8)` | 扫描文件夹并载入私人枪械，返回条数；重复调用先清空 | `-1` 表示目录不可用 |
| `int DFA_CW_GetCount()` / `DFA_CW_GetCapacity()` | 已载入条数 / 容量上限（256） | `0` / `256` |
| `const WeaponInfo* DFA_CW_GetWeapon(int idx)` | 第 idx 把自定义枪 | `nullptr` |
| `const int* DFA_CW_GetStyleScores(int idx)` | 该枪 5 维分（省略项已用大类默认值补全） | `nullptr` |
| `const char* DFA_CW_GetFolder()` / `DFA_CW_GetDefaultFolder()` | 当前数据文件夹 / 默认 `<exe>\自定义枪械` | 空字符串 |
| `int DFA_CW_GetFileCount()` / `GetErrorCount()` / `const char* DFA_CW_GetError(int)` | 实际读取的文件数 / 错误条数 / 第 i 条错误说明 | `0` / `0` / `nullptr` |
| `int DFA_O_GetOperatorCount()` | 干员总数（动态） | — |
| `const OperatorInfo* DFA_O_GetOperator(int idx)` | 取第 idx 名干员 | `nullptr` |
| `const char* DFA_O_GetRoleName(DfaRole r)` | 兵种中文名（不带"兵"字，界面自己拼"兵"） | `"未知"` |
| `const int* DFA_O_GetRoleScores(DfaRole r)` | 该兵种 5 维分 | 全 0 数组 |
| `int DFA_M_GetMapCount()` / `GetMap(int)` / `GetMapStyleScores(int)` | 地图三件套 | `DFA_MAP_COUNT` / `nullptr` / 全 0 |
| `int DFA_M_GetDiffCount()` / `GetDiff(int)` / `GetDiffStyleScores(int)` | 难度三件套 | `DFA_DIFF_COUNT` / `nullptr` / 全 0 |
| `int DFA_M_GetAllowedDiffCount(int mapIdx)` | 该图开放的难度个数 | 0 |
| `int DFA_M_GetAllowedDiff(int mapIdx, int slot)` | 第 slot 个开放难度的**全局难度编号** | `-1` |
| `const char* DFA_M_GetLootZones(int mapIdx)` | 该图核心物资区文案（`kLootZones`，供输出「核心物资区」段落） | `nullptr` |
| `const char* DFA_M_GetSpecialNote(int mapIdx, int diffIdx)` | 地图 + 难度的特别提示（目前仅 AZ3 超临界：随机泄漏点 / 辐射扩散） | `nullptr` |
| `int DFA_E_WealthTier(long long wan)` | 资金→档位 0~3 | 恒返回有效档位 |
| `int DFA_E_GetTierCount()` / `GetTier(i)` / `GetTierName(t)` / `GetTierNote(t)` | 档位四件套 | `4` / `0` / `"未知"` / `""` |
| `const int* DFA_E_GetWealthStyleScores(int tier)` | 档位 5 维分 | 全 0 |
| `const int* DFA_E_GetTeamStyleScores(bool)` | 队友 5 维分 | 恒有效 |
| `const char* DFA_S_GetName(int)` / `GetTag(int)` | 玩法名 / 标语 | `"未知"` / `""` |
| `const StyleGuide* DFA_S_GetGuide(int)` | 玩法档案 | `nullptr` |
| `void DFA_E_ComputeScores(const DfaProfile*, int out[5])` | 算总分写进 out | 入参空指针直接 return |
| `int DFA_E_PickBest(const int scores[5])` | 最优玩法下标 | 空指针返回 0 |
| `bool DFA_E_ForceRule(const DfaProfile*, int& best)` | 强制兜底，触发时改写 best | 空指针返回 false |

---

## 6. 主流程（main.cpp）

输入（`main()` 里 while 循环，支持反复评估）：

1. 枪械：内置 69 把按 `cls` 分组打印，后接【自定义枪械】分组，序号全局连续；`ReadInt("", 1, TotalWeaponCount()) - 1`（`TotalWeaponCount` = 内置 + `DFA_CW_GetCount()`，`WeaponAt` 负责分派）
   - 启动时先 `DFA_CW_Load(folder)` 并打印状态（文件夹缺失 / 扫描 N 文件未解析 / 载入 M 条 / 前 5 条错误）
   - 文件夹优先级：`--weapon-dir <目录>`（或 `=` 形式）→ 环境变量 `DFA_WEAPON_DIR` → `<exe目录>\自定义枪械`
2. 干员：按 `role` 分组打印，序号连续；`ReadInt("", 1, oCount) - 1`
3. 地图：`ReadInt("", 1, mCount) - 1`
4. 难度：只列 `DFA_M_GetAllowedDiff(map, slot)`，用户输 slot（1 起），再转成全局 `diffIdx`
5. 资金：`ReadLL`，拒绝负数
6. 队友：`ReadYesNo`，接受 `y/Y/1` 与 `n/N/0`

然后 `DfaProfile p = { weapon, opIdx, map, diff, wealth, team };` → `ComputeScores` → `PickBest` → `ForceRule`。

输出 7 段，顺序固定在 `main()` 尾部：`PrintProfile` → `PrintScores` → `PrintConclusion` → `PrintJudgement` → `PrintGuide` → `PrintWarnings` → `PrintFooter`。

`PrintWarnings` 是硬编码的条件集合，阈值直接写在函数里：绝密且资金<500 万报警、全装且资金<500 万降档、全装且无固排、跑刀/鼠鼠但资金雄厚、航天基地跑刀、潮汐监狱跑刀、永夜视野、支援兵独狼、永夜+侦察兵。

---

## 7. 常见改造任务 → 精确改法

### 7.1 新增一把枪（最常做）
只改 `dfa_weapons.cpp` 的 `kWeapons`，一行：

```cpp
{ "枪名", "gunRifle", WC_ASSAULT },
```

- 第 2 项是游戏内二级分类标签：`gunRifle / gunSMG / gunSniper / gunMP / gunShotgun / gunLMG / gunPistol / special / other`
- 第 3 项决定分数，选对大类即可，**不需要单独给这把枪写分数**
- 菜单序号自动 +1，`DFA_W_GetWeaponCount()` 自动更新，**没有任何地方需要改数字**

### 7.2 新增干员
只改 `dfa_operators.cpp` 的 `kOperators`，一行：

```cpp
{ "代号", "本名", OP_ROLE_ASSAULT, "技能1·技能2·技能3·技能4", "适配提示（一句话，十几字）" },
```

注意：干员**没有独立分数**，只有所属兵种的统一分。若确实要做"每个干员单独分"，那是破坏性改造（见 7.6）。

### 7.3 新增地图
三处都要动：
1. `dfa_maps.h` 的 `enum DfaMapId` 在 `DFA_MAP_COUNT` 前插一项
2. `dfa_maps.cpp` 的 `kMaps` 加一行 `{ "图名", "特点" }`
3. `kMapScore` 加一行 5 个数；`kAllowedDiffs` 加一行（≤3 个难度 + `-1`）
4. `kLootZones` 加一行核心物资区文案（输出时按玩家所选地图展示；不填也要补一个占位字符串，保证行数对齐）

`kMaps` 声明是 `kMaps[DFA_MAP_COUNT]`，维度会自动对齐，但**行数少一行会编译报错**（这是好事，能兜住漏改）。

### 7.4 新增难度
1. `dfa_maps.h` 的 `enum DfaDifficulty` 在 `DFA_DIFF_COUNT` 前插一项（建议追加在末尾，避免已有难度编号整体平移）
2. `dfa_maps.cpp` 的 `kDiffs` 加一行（含 `availability` 文案）、`kDiffScore` 加一行
3. 在相关地图的 `kAllowedDiffs` 行里插入该难度编号

⚠️ `kAllowedDiffs` 第二维只有 4 列，**某图已经列了 3 个难度就塞不下第 4 个**，得先把列宽改大（例如 `[DFA_MAP_COUNT][6]`）并同步 `DFA_M_GetAllowedDiffCount` 里的硬编码 `4`。

### 7.5 新增一种玩法（大改，最容易翻车）
必须同步动这些地方，一处不漏：
1. `dfa_common.h`：`DfaStyle` 在 `DFA_STYLE_COUNT` 前追加
2. `kWeaponScore`、`kRoleScore`、`kMapScore`、`kDiffScore`、`kWealthScore`、`kTeamScore` **六张表每一行都要多一列**
3. `dfa_styles.cpp`：`kStyleNames`、`kStyleTags`、`kGuides` 各加一项
4. `main.cpp`：`PrintWarnings` 里针对具体玩法的条件要复查；`PrintScores` 的列对齐逻辑依赖"玩法名是 3 个汉字"（`strlen/3*2` 算显示宽度），新玩法名不是 3 字时对齐会歪
5. 检查所有 `[DFA_STYLE_COUNT]` 维度的数组声明是否自动跟着变（大部分是，`kWealthScore` 是 `[][DFA_STYLE_COUNT]`，行数由初始化列表决定，安全）

### 7.6 给每名干员加独立分数（破坏性改造）
现状是"兵种统一分"，要做"干员个性分"需要：
1. `OperatorInfo` 加 `int score[5]` 或新增 `const int kOpScore[][5]` 与 `kOperators` **顺序严格一一对应**
2. `dfa_operators.h` 新增接口 `const int* DFA_O_GetOperatorScores(int idx)`
3. `dfa_engine.cpp` 把 `os = DFA_O_GetRoleScores(op->role)` 换成干员分（或两者相加）
4. 18 条干员每条都要补 5 个数，漏一条就编译报数量不匹配

### 7.7 调整权重（调参最常做）
只改对应表里的数字。建议一次只动一个维度，改完用第 9 节的用例回归。

### 7.8 新增风险提示 / 强制规则
- 提示类：在 `PrintWarnings` 里加 `if (...)`，记得 `any = true`
- 硬规则类：在 `DFA_E_ForceRule` 里追加条件，注意**顺序**：先命中的先改写 `best`

### 7.9 添加私人枪械（不改代码，只放 txt）
程序启动会扫描「自定义枪械」文件夹并自动录入，**不需要改任何源码、不需要重编译**：

1. 在 `DFA_Main.exe` 同级新建文件夹 `自定义枪械`（或用 `--weapon-dir <目录>` / 环境变量 `DFA_WEAPON_DIR` 指向别处）
2. 放入 `.txt` 或 `.csv`，每行一把枪：

```
枪名 | 大类 | 跑刀,鼠鼠,正常玩,微交战,全装猛攻
我的M4A1 | 突击步枪 | 1,1,4,5,5
测试狙 | 狙击步枪
```

- 分隔符 `|` `,` 以及全角 `｜` `，`、顿号都认；行首 `#` `;` `//` 为注释
- 大类写中文全名或别名（`突击步枪` / `步枪` / `ar` / `gunRifle` …），别名表在 `dfa_weapons.cpp` 的 `kClassAliases`
- 5 个评分可整段省略 → 直接用该大类默认分；写了就必须正好 5 个、范围 -99~99
- 只扫描文件夹第一层、按文件名升序；子目录与 `.md` 等非 txt/csv 忽略；编码支持 UTF-8（推荐）、UTF-16（带 BOM）、GBK
- 与内置或其他文件重名会被跳过并报错；上限 256 条；错误最多列 64 条
- 详细格式见 `自定义枪械\格式说明.md`，模板见 `自定义枪械\枪械数据模板.txt`

---

## 8. 红线与已知坑（让 AI 一定要遵守）

1. **不要引入硬编码的条目总数**（如 `#define OP_COUNT 17`）。所有数量都用 `sizeof(array)/sizeof(array[0])` 或 `XXX_COUNT` 枚举。历史上就是因为数量被写死，新增干员后菜单死活不出现。
2. **不要改数组顺序去"整理"**，序号会全体平移；要加就加在末尾或对应分组的末尾。
3. **不要跨模块直接访问 `static` 数组**（它们都是文件内 `static`），必须走 `Get` 接口。
4. **不要在静态库里抛异常或 `exit()`**，越界一律返回 `nullptr` / 全 0 / `-1`。
5. **不要用 `std::string` / `std::vector` 改接口签名**，`extern "C"` 链接会断。
6. **不要给字符串加 `std::wstring` 或改 `CharacterSet`**，会连带改掉整个控制台编码方案。
7. **`diffIdx` 是全局难度编号，不是"第几个难度"**，混用会导致评分取错行。
8. **修改后必须重新编译**。改了 `.cpp` 不重编译，跑的还是旧 exe，现象表现为"改了没反应"——这是本项目历史上真实踩过的坑。
9. **源文件保存为 UTF-8**，不要 BOM 以外的编码；用记事本另存时注意别存成 ANSI。
10. `kAllowedDiffs` 列宽 4 是写死的，加难度前先确认没超。
11. 中文对齐用 `strlen/3*2` 估算显示宽度，只对纯 3 字节汉字有效，加英文/数字玩法名会歪。
12. `DFA_Economy` 与 `DFA_Engine` 共用 `DFA_E_` 前缀，改函数前先确认是哪个头文件的。
13. 私人枪械**不要塞进 `kWeapons`**：它是运行时数据，排在菜单尾部；塞进内置数组会平移全部编号，还会让缺少 txt 的机器显示不存在的数据。
14. `DFA_CW_*` 返回的是内部静态存储的借用指针，**不要 `delete`/`free`**；重复 `DFA_CW_Load` 会让上一次拿到的指针全部失效，界面层每次现取即可。

---

## 9. 验收用例（改完必须跑这几组）

输入固定写法（每行一项，最后两个 `n` 是"不重新评估"和"回车退出"）：

| # | 枪械 | 干员 | 地图 | 难度 | 资金(万) | 队友 | 期望总分（跑刀/鼠鼠/正常/微交战/全装） | 期望推荐 |
|---|---|---|---|---|---|---|---|---|
| 1 | 1 = KC17 | 18 = 野火 | 1 = 零号大坝 | 1 = 普通 | 100 | n | 6 / 6 / 10 / 7 / 2 | 正常玩 |
| 2 | 1 = KC17 | 18 = 野火 | 6 = 潮汐监狱 | 1 = 绝密 | 50 | n | — | 强制跑刀（触发 `forced`） |

用例 1 的分数吻合（已实测）：

```
跑刀 0+0+2+2+0+2 = 6
鼠鼠 0+0+2+1+1+2 = 6
正常 3+2+1+1+2+1 = 10
微交战 4+4+0-1+0+0 = 7
全装 4+4-1-2-1-2 = 2
```

另外必查：菜单条数与实际数据一致（当前应为 **枪械 1~69（无自定义数据时）、干员 1~18、地图 1~6、零号大坝难度 1~3**）。若 `自定义枪械` 文件夹里有数据，枪械菜单会从 70 起继续编号，条数 = 69 + `DFA_CW_GetCount()`，且自定义枪应参与评分（见 7.9）。

---

## 10. 给 AI 的提示词模板（可直接复制）

**模板 A：跨模块理解**
> 下面是一个 C++17 控制台项目的说明文档和部分源码。请先复述：①模块依赖关系 ②评分公式 ③数组顺序与菜单编号的对应规则。确认理解后再等我提需求，不要现在改代码。

**模板 B：加数据**
> 按说明书第 7.1 节，在 `kWeapons` 里新增 3 把突击步枪：`XXX / YYY / ZZZ`。要求：只输出修改后的完整 `kWeapons` 数组所在文件片段，不给整份文件；不要改动任何函数、不要改数组顺序、不要引入新的数字常量。

**模板 C：加玩法（高风险）**
> 按第 7.5 节新增第 6 种玩法「蹲撤离点」，排在「全装猛攻」之后。请列出所有必须同步修改的位置清单，逐条给出改前/改后代码差异；重点检查六张权重表每一行是否都补了第 6 列，以及 `main.cpp` 的对齐和风险提示是否需要调整。

**模板 D：改算法**
> 把加权求和改成加权平均（各维度权重不同，权重表新增一张）。要求：`DfaProfile` 结构不变、所有 `Get` 接口签名不变、越界行为不变；给出 `dfa_engine.cpp` 的完整新版本，并说明同分处理规则是否仍成立。

**模板 E：让 AI 自检**
> 你改完后，请逐条对照说明书第 8 节 14 条红线自检，明确回答每一条是否满足；再按第 9 节用例 1 手算五个分数，与 6/6/10/7/2 比对。

---

## 11. 附录：编号速查

- 玩法：0 跑刀 / 1 鼠鼠 / 2 正常玩 / 3 微交战 / 4 全装猛攻
- 兵种：0 支援 / 1 工程 / 2 侦察 / 3 突击
- 枪械大类：0 突击步枪 / 1 冲锋枪 / 2 狙击步枪 / 3 射手步枪 / 4 霰弹枪 / 5 轻机枪 / 6 手枪 / 7 特殊武器 / 8 通用
- 地图：0 零号大坝 / 1 长弓溪谷 / 2 巴克什 / 3 航天基地 / 4 AZ3 / 5 潮汐监狱
- 难度：0 普通 / 1 机密 / 2 绝密 / 3 永夜
- 资金档：0 紧张(<100) / 1 小康(100~500) / 2 宽裕(500~1500) / 3 雄厚(≥1500)
- 队伍：0 无固排 / 1 有固排
- 自定义枪械：菜单编号从 70 起（= 内置 69 + `DFA_CW` 序号 + 1），运行时最多 256 条

当前数据规模：枪械 69、干员 18（支援 4 / 工程 5 / 侦察 4 / 突击 5）、地图 6、难度 5、玩法 5、兵种 4；另有运行时自定义枪械（上限 256，默认 0 条）。
