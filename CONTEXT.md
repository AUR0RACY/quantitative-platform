# CONTEXT.md — quantitative-platform

A Trailblazer-inspired quantitative trading platform. Free, self-use, built for Windows.

---

## 术语表

| 术语 | 定义 |
|------|------|
| **Bar** | 一根 K 线，包含 OHLC（开高低收）数据 |
| **逐 Bar 执行** | 策略代码在每根 Bar 上独立执行一次，平台自动遍历历史 |
| **CTP** | 中国期货市场统一接入接口（上期所/大商所/郑商所/中金所/广期所） |
| **SimNow** | CTP 官方仿真环境 |
| **虚拟盘** | 用实时行情模拟交易，无真实资金风险 |
| **回测** | 在历史数据上验证策略表现 |

---

## 核心决策

### 语言

- **策略编写 + 平台开发**：C++
- **避开开拓者专有语言（TBL）**

### Tech stack

| Layer | Choice |
|---|---|
| GUI Framework | Qt 6 |
| Charting | QCustomPlot (MIT) |
| Strategy compilation | MSVC → DLL hot-loading |
| Strategy parameter tuning | DLL exports metadata; params editable without recompile |
| Local storage | SQLite via SQLiteCpp |
| Distribution | Portable (zip archive, no installer) |

### 执行模型

- **逐 Bar 执行**（与开拓者一致，交易员无迁移成本）
- 放弃传统循环模型

### 行情源

| 市场 | 方案 | 费用 |
|------|------|------|
| 期货 | CTP（Level 1 + Level 2 + Tick） | 免费（需期货公司开通量化权限） |
| 期货仿真 | SimNow | 免费 |
| 数字货币 | OKX REST + WebSocket | 免费 |

### 账户模型

- **多账户、多期货公司同时连接**
- 一个交易者可管理多个基金账号

---

## 功能清单（MVP）

按操作频率排列：

1. **K 线图表** — 实时行情、技术指标叠加
2. **策略编辑器** — C++ 策略代码编写与编译
3. **回测引擎** — 历史数据回测 + 报告（收益曲线、最大回撤、夏普比率）
4. **虚拟盘** — SimNow 仿真环境验证
5. **实盘交易** — CTP 实盘账户连接，策略自动执行
6. **持仓与盈亏** — 实时持仓监控、P&L
7. **多账户切换** — 不同客户账户管理

### 不做（反开拓者模式）

- ❌ 复杂 UI、多入口
- ❌ 教程/新手引导
- ❌ 社区/社交功能
- ❌ 过多的菜单层级

---

## 目标用户

- 已在开拓者平台有经验的**专业期货交易员**
- 主要交易品种：**上期所白银**
- 策略复杂度：**中高**（多品种对冲、套利）
- 用户分布：**多个客户，在不同期货公司开户**

---

## UI 原则

- **简洁** — 页面清晰，无干扰
- **入口少** — 只暴露核心功能
- **上手快** — 对已经会用开拓者的交易员，切换成本趋近于零

---

## 待确认

- [ ] 策略示例库（是否需要内置几个经典策略模板？）
- [ ] 回测报告指标细节（除夏普/MDD外还需要什么？）
- [ ] 风险控制面板（手动止损、强平线、单日最大亏损限制？）

---

---
## 开发进度

| Phase | 内容 | 状态 |
|---|---|---|
| 1 | 项目骨架（CMake + Qt6 + C++20） | ✅ |
| 2 | 数据层（SQLite + Schema + CRUD） | ✅ |
| 3 | 回测引擎（逐 Bar 执行 + DLL 热加载） | ✅ |
| 4 | UI（K线图 + 策略编辑 + 账户面板） | ✅ |
| 5 | 实盘（CTP + SimNow + 下单） | 🔜 |
| 6 | 扩展（OKX + 蜡烛图 + 更多指标 + 报告） | ⏳ |

### Phase 2 成果

- **数据库**：SQLite3 via SQLiteCpp（FetchContent 自动拉取）
- **Schema**：instruments, bars, ticks, accounts 四张表
- **API**：Instrument CRUD / Bar 读写 / Tick 读写 / Account CRUD
- **测试**：10 个单元测试全部通过
- **Navbar v2**：行情 / 策略 / 回测 / 交易 / 账户（仪表盘，勾选多账户一键下单）

---

*最后更新：2026-06-13 | Phase 2 完成*
