# Snow Shot × macshot：交互对齐计划（fork 叠层）

目标：在 `Sma1lboy/snow-apps` 上做一层"macshot 风格"的 UI，**不另起一个软件**。
原则：上游（`mg-chao/snow-shot`）照常合并；我们的改动尽量是**新增文件** + 少量有标记的接缝。

- 上游：`upstream` = https://github.com/mg-chao/snow-shot （基线 `b4a7467f6`，1.4.2）
- 参考：https://github.com/sw33tLie/macshot （GPLv3，和 snow_shot 的 GPL-3.0-or-later 兼容）

---

## 1. 结论先说

功能上 Snow Shot 已经比 macshot 多（选区、标注、录屏、OCR、钉图、MCP 都更深）。
**差的是交互形态**，主要是下面 5 条，其次才是少数几个功能。

| # | macshot 的交互 | Snow Shot 现状 | 落点 |
|---|---|---|---|
| U1 | 工具栏**居中**贴在选区下方 | 右对齐到选区右下角 | `toolbar/screenshottoolbarplacement.h`（低频改动） |
| U2 | "画什么"在底部横排，"拿结果做什么"在选区**右侧竖排**（关闭/移动/编辑器/复制/保存/分享/上传/钉图/OCR/翻译/滚动/录屏） | 绘图工具和动作在同一条横栏里 | 已有 `ScreenshotToolbarLayoutKind{DrawingTools, ActionTools}`（`include/snow_shot/storage/settingsadapters.h:81`），可以拆成两个面板 |
| U3 | 当前工具的子选项是工具栏下方第二行，和主栏等宽、视觉上连成一块 | 已有子工具栏行，样式偏 Ant Design | 只换皮：主题 token |
| U4 | 选区顶部居中的尺寸胶囊 `1742 × 979`，8 个蓝色圆点控制柄 | 独立的选区信息面板（`DisplayMode::SizeOnly` 已存在） | 选区信息面板定位 + 控制柄样式 |
| U5 | 截完弹出**浮动缩略图**（复制/保存/钉图/编辑/上传，自动消失） | 没有，只有托盘气泡 | 新文件，挂在导出完成回调上 |

视觉统一（深色半透明面板、圆角 10px、蓝色强调、图标线宽一致）走主题 token，不碰控件代码。

## 2. 叠层设计

### 2.1 一个开关：`interface/ui_profile = snow | macshot`

- 运行时切换，默认 `snow`。上游行为完全不变，以后也可以拿去和上游讨论合并（上游 README：功能 PR 需要先开讨论）。
- 所有 fork 逻辑都通过 `UiProfile` 读取，不散落在各处 `if`。

### 2.2 目录

```
fork/                                     # 文档、合并脚本（上游永远不碰）
snow_shot/src/presentation/profile/       # 新增：UiProfile、macshot 布局策略、token 覆盖
snow_shot/src/presentation/postcapture/   # 新增：浮动缩略图 U5
```

### 2.3 接缝（唯一需要改上游文件的地方）

每处只加几行，统一标记 `// FORK(macshot):`，用 `git grep "FORK(macshot)"` 就能列全。

| 接缝 | 文件 | 近 6 个月上游提交数 | 做法 |
|---|---|---|---|
| 工具栏定位 | `src/presentation/toolbar/screenshottoolbarplacement.h` | 低（`toolbar/` 共 71，此文件很少） | `screenshotToolbarPlacement()` 先问 `UiProfile` 要策略；macshot 策略=水平居中，放不下再翻到上方 |
| 主题 token | `src/presentation/styles/thememanager.cpp` | 9 | 加一个 `macshot` 预设，覆盖 `colorPrimary` / `colorBgBase` / 圆角 / 面板透明度 |
| 工具栏面板皮肤 | `src/presentation/tools/screenshottoolbarmainpanel.cpp` | 9 | 已有 skin 机制（`interface/toolbar_skin_path`），按 profile 选皮肤 |
| 动作栏拆分 | `src/presentation/tools/screenshotfloatingtoolpalettewindow.cpp` | 27 | 按 `LayoutKind::ActionTools` 拆出第二个竖排窗口，定位到选区右侧 |
| 导出完成 | `src/presentation/services/screenshotexportcoordinator.cpp` | 9 | 发一个信号，浮动缩略图订阅 |

**刻意不碰**（上游改得太频繁，冲突会很痛）：
- `tools/screenshottoolpalette.cpp`：近 6 个月 127 次提交，约 1.09 万行
- `overlay/`：151 次
- `selection/`：79 次
- `ant_design_qt/`：102 次

需要改它们的行为时，优先找已有的配置项或信号。

### 2.4 分支与同步

- `main` 只做 fast-forward 跟随 `upstream/main`，不放自己的提交。
- `macshot-ui` 放全部 fork 改动，定期 `git merge upstream/main`。
- `fork/sync-upstream.sh`（待写）：fetch → merge → 列出 `FORK(macshot)` 接缝里有冲突的文件。

## 3. 功能差距（非交互，按价值排序）

macshot 有、Snow Shot 没有或不完整的：

| 优先级 | 功能 | 现状 | 难度 / 说明 |
|---|---|---|---|
| P1 | 浮动缩略图 | 无 | 中，新文件（= U5） |
| P1 | 表情 / 贴纸 / 插入图片、Cmd+V 粘贴图片为图层 | 引擎里没有图片元素 | **高**：要改 `snow_draw_engine_qt` 的 Rust 引擎，最好先和上游讨论 |
| P1 | 美化（渐变背景、窗口框、内边距、阴影） | 只有选区阴影/圆角 | 中，导出时后处理，可以全部放在新文件里 |
| P2 | 绘制时按住 Space 平移图形 | 无 | 低，但在引擎 `creation_workflow.rs` 里 |
| P2 | 选区边缘吸附到颜色边界 | 无（只有窗口/元素吸附） | 中 |
| P2 | 序号标记的指向锥 | 无 | 低，引擎 `text/serial.rs` |
| P2 | 文字下划线 / 删除线 | 无 | 低 |
| P2 | 遮挡：纯色模式 | 用实心矩形代替 | 低 |
| P2 | 全局调整（对比度 / 饱和度 / 锐度）+ 滤镜预设 | 只有区域亮度、灰度 | 中 |
| P3 | 自动打码 PII、人脸检测 | 有"自动滤镜"（按 UI 区域识别） | 中，可以在 OCR 结果上加正则 |
| P3 | 历史下拉面板 | 是主窗口里的一页 | 中 |
| P3 | 背景移除、HEIC、Google Drive / imgbb、AI 搜图 | 无 | 按需 |

Snow Shot 独有、保留不动：MCP server、表格 / 公式 / 图片转 Markdown 识别、原图翻译、自由 / 折线选区和选区加减、桌面画布、钉图分组、拼图 / 切图 / 打印 / PDF、水印 / 气泡 / 角度 / 橡皮擦工具、录屏摄像头和按键叠加、DXGI/WGC 采集后端。

## 4. 建议的第一版（只做交互，不碰引擎）

1. `UiProfile` + 设置项（新文件 + 设置页一行）
2. U1 居中定位
3. U3 / 视觉：`macshot` 主题 token + 工具栏皮肤
4. U4 尺寸胶囊放到选区顶部居中
5. U2 动作栏拆到右侧竖排
6. U5 浮动缩略图

做完这些，截图时的观感就和 macshot 基本一致；表情 / 贴纸和美化放第二版。

## 附：证据位置

- 工具枚举：`include/snow_shot/presentation/screenshotinteractionstate.h:14`、`screenshottoolpalette.h:73`、`snow_draw_engine_qt/include/snow_draw_engine_qt/snow_canvas_types.h:24`
- 当前定位算法：`src/presentation/core/screenshotgeometry.cpp:1121`（`anchoredToolbarPlacement`，右对齐，先下后上）
- 子工具栏：`src/presentation/tools/screenshottoolpalettestylecontrols.cpp`
- 选区信息面板：`src/presentation/selection/screenshotselectiontoolbarwidget.cpp`
- 编辑器窗口：`src/presentation/imageediting/imageeditingwindow.cpp`
- 历史：`src/presentation/components/screenshothistorypagewidget.cpp`；可重新编辑的标注：`include/snow_shot/presentation/screenshothistorytypes.h:40`
