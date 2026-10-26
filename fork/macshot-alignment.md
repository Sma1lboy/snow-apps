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

### 2.1 一个开关：`ui_profile = snow | macshot`

- 默认 `snow`。第一版是编译开关；以后要做成运行时切换的设置项，再动 `configurationschema.cpp`。上游行为完全不变，以后也可以拿去和上游讨论合并（上游 README：功能 PR 需要先开讨论）。
- 所有 fork 逻辑都通过 `UiProfile` 读取，不散落在各处 `if`。

### 2.2 目录

```
fork/                                     # 文档、合并脚本（上游永远不碰）
snow_shot/src/presentation/profile/       # 新增：UiProfile、macshot 布局策略、token 覆盖
snow_shot/src/presentation/postcapture/   # 新增：浮动缩略图 U5
```

### 2.3 接缝（唯一需要改上游文件的地方）

每处只加几行，统一标记 `// FORK(macshot):`，用 `git grep "FORK(macshot)"` 就能列全。

提交数是上游全部历史（仓库从 2026-09-06 开始，约 7 周、954 次提交），越高越容易冲突。

| 接缝 | 文件 | 上游提交数 | 做法 |
|---|---|---|---|
| 主题 token | 不改上游：启动后调用 `adqt::theme::ThemeManager::setConfig/setSeed`，工具栏单独用 `setScopeOverride(toolbarWindow, …)`（`ant_design_qt/.../theme/theme_manager.h:33`） | 0 | 圆角 10–12、灰色主色、`backgroundOpacity < 1` 半透明。只换样式，不动布局 |
| 工具栏定位 | `src/presentation/toolbar/screenshottoolbarplacement.h`，调用点 `toolbar/screenshottoolbarpresenter.cpp:192-198` | 1 / 17 | 把 `screenshotToolbarPlacement()` 换成可注入的策略；macshot 策略 = 水平居中，放不下再翻到上方 |
| 面板常量和绘制 | `include/.../screenshottoolbarmainpanel.h:33-39`（ControlSize 32、PanelRadius 8…），`tools/screenshottoolbarmainpanel.cpp:53-61, 262-300` | 8 / 9 | 收成一个 `ToolbarSkin` 结构，默认值等于现状 |
| 工具栏窗口工厂 | `overlay/screenshotoverlayuihost.cpp:429`（`new ScreenshotToolbarWindow(...)`） | 19 | 改成走工厂。macshot 的"底部绘图 + 右侧竖排动作"结构在这里换，新工具栏实现 `ScreenshotToolbarCommandSink` 接口（`include/.../screenshottoolbarcommands.h:23`） |
| 导出完成 | `services/screenshotexportcoordinator.cpp` | 9 | 发一个信号，浮动缩略图订阅 |
| 构建 | 新建 `cmake/SnowShotSkinMacshot.cmake`，参照已有的 `cmake/SnowShotMini.cmake` 双版本写法；`snow_shot/CMakeLists.txt` 只加一行 include | 431（只加 1 行） | 编译期开关 `SNOW_SHOT_UI_SKIN=macshot` |

**刻意不碰**（上游一周改好几次，冲突会很痛）：
- `snow_shot/CMakeLists.txt`：431 次（只允许上面那 1 行 include）
- `storage/configurationschema.cpp`：204 次。所以 `ui_profile` 这个设置项**不往这里加**，第一版先用编译开关或单独的配置文件
- `services/screenshotcontroller.cpp`：160 次
- `tools/screenshottoolpalette.cpp`：127 次，约 1.09 万行，而且主栏只支持横排
- `tools/screenshottoolpalettestylecontrols.cpp`：47 次，约 7.5k 行，是所有工具子选项行
- 图标 SVG（`components/icons/resources/`）：60 次。要换图标就加一套新文件名，不要原地覆盖

需要改它们的行为时，优先找已有的配置项或信号。

**取舍**：右侧竖排动作栏（U2）没有现成接缝，只能走工厂换整个工具栏窗口。难点是 7.5k 行的子选项行要么复用要么重写，所以 U2 放在第一版最后做。

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

1. `UiProfile` + `cmake/SnowShotSkinMacshot.cmake` 编译开关（先不进设置页，避开 `configurationschema.cpp`）
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
