# 新剧本工作区

这里开始重建新的动画剧本，只保留当前 live 页面。

## 工作规则

1. 一次只做一页。
2. 先在本目录写清这一页的动画剧本。
3. 剧本确认后，再由 `Remotion` 实现这页动画。
4. 不再把旧 `01-09` 剧本和旧 `draw.io` 当作 live 来源。
5. 如果某份文档是几何草图镜像，请直接放在本目录，并在文件名里标明 `草图镜像-<sketch-id>`。
6. 正式动画页和草图镜像页都必须使用同一套 ledger 结构：
   - `Current screenshot`
   - `三层 Review 总表`
   - `节点剧本与 Review 表`
   - `边剧本与 Review 表`
7. 每一页剧本只管理自己的截图、facts、评分和 review，不做跨页累计传递。
8. 只要当前页和上一页存在连续语义，节点和边都必须记录：
   - 与上一页连续语义
   - 当前渲染载体
   - 连续语义 Review
   - 连续语义分数
9. 如果当前页和上一页是同一个语义 actor / route，默认必须用同一个元素 carrier 表达；如果故意打断连续性，文档里必须写明原因。
10. 剧本里的 Step 统一写紧凑别名：例如 `5`、`4_data`、`13_img`；不要混用 `page_05` / `page_04_data`。

## 你要编辑的文件

- 页模板：[`_page-template.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/_page-template.md)
- 第 01 页：[`01-第一页.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/01-第一页.md)
- 第 02 页：[`02-第二页.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/02-第二页.md)
- 第 03 页：[`03-第三页.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/03-第三页.md)
- 第 04 页：[`04-第四页.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/04-第四页.md)
- 第 05 页：[`05-第五页.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/05-第五页.md)
- 第 10 页正式动画：[`10-第十页-正式动画.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/10-第十页-正式动画.md)
- 第 11 页正式动画：[`11-第十一页-正式动画.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/11-第十一页-正式动画.md)
- 第 12 页正式动画：[`12-第十二页-正式动画.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/12-第十二页-正式动画.md)
- 第 13 页正式动画：[`13-第十三页-正式动画.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/13-第十三页-正式动画.md)
- 第 14 页正式动画：[`14-第十四页-正式动画.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/14-第十四页-正式动画.md)
- 第 15 页正式动画：[`15-第十五页-正式动画.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/15-第十五页-正式动画.md)
- 第 16 页正式动画：[`16-第十六页-正式动画.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/16-第十六页-正式动画.md)
- 第 17 页正式动画：[`17-第十七页-正式动画.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/17-第十七页-正式动画.md)
- 第 18 页正式动画：[`18-第十八页-正式动画.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/18-第十八页-正式动画.md)
- 第 19 页正式动画：[`19-第十九页-正式动画.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/19-第十九页-正式动画.md)
- 第 21 页正式动画：[`21-第二十一页-正式动画.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/21-第二十一页-正式动画.md)
- 第 22 页正式动画：[`22-第二十二页-正式动画.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/22-第二十二页-正式动画.md)
- 第 23 页正式动画：[`23-第二十三页-正式动画.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/23-第二十三页-正式动画.md)
- 第 24 页正式动画：[`24-第二十四页-正式动画.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/24-第二十四页-正式动画.md)
- 第 28 页正式动画：[`28-第二十八页-正式动画.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/28-第二十八页-正式动画.md)
- 第 29 页正式动画：[`29-第二十九页-正式动画.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/29-第二十九页-正式动画.md)
- 后续页优化备注草案：[`21-24页-后续优化方向备注草案.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/21-24页-后续优化方向备注草案.md)
- 第 11 页草图镜像：[`11-第十一页-草图镜像-page11-r1.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/11-第十一页-草图镜像-page11-r1.md)
- 第 12 页草图镜像：[`12-第十二页-草图镜像-page12-r1.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/12-第十二页-草图镜像-page12-r1.md)
- 第 13 页草图镜像：[`13-第十三页-草图镜像-page13-r1.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/13-第十三页-草图镜像-page13-r1.md)
- 第 14 页草图镜像：[`14-第十四页-草图镜像-page14-r1.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/14-第十四页-草图镜像-page14-r1.md)
- 第 15 页草图镜像：[`15-第十五页-草图镜像-page15-r1.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/15-第十五页-草图镜像-page15-r1.md)

## 当前状态

- 旧剧本已从 live 路径移除
- 旧 `draw.io` 已从 live 路径移除
- `SlideApp` 当前已经把正式故事推进到第 29 页（`24-27` 为策略占位页，`28-29` 为结尾页）
- `Docs/剧本` 当前 live 的正式页包括：`10-第十页-正式动画.md`、`11-第十一页-正式动画.md`、`12-第十二页-正式动画.md`、`13-第十三页-正式动画.md`、`14-第十四页-正式动画.md`、`15-第十五页-正式动画.md`、`16-第十六页-正式动画.md`、`17-第十七页-正式动画.md`、`18-第十八页-正式动画.md`、`19-第十九页-正式动画.md`、`21-第二十一页-正式动画.md`、`22-第二十二页-正式动画.md`、`23-第二十三页-正式动画.md`、`24-第二十四页-正式动画.md`、`28-第二十八页-正式动画.md`、`29-第二十九页-正式动画.md`
- `11-15` 的草图镜像页继续保留为历史几何镜像与对照，不再充当正式评分台账
- 草图镜像页和正式动画页共用同一个 `Docs/剧本/` 目录，只靠文件名区分

## 快速对齐检查

每次改 `embed.ts` 时间锚点或新增页面后，先跑两步再改剧本：

1. 过渡时长审计（全量 step）  
   `python3 scripts/slide-geometry-harness/audit_transition_timings.py`
2. 页面与剧本对齐审计（step / 文档覆盖 / 占位状态）  
   `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`

输出文件：

- `Docs/剧本/00-剧本与页面对齐审计.md`

这个文件是当前“剧本是否和页面对得上”的单一事实来源：

- `missing_doc`：页面有 step，但没有对应剧本；
- `timing_placeholder`：剧本里有时长审计章节，但还是占位；
- `no_shot`：缺当前截图；
- 动画异常以 `verdict != in_range` 为准。
