# 新剧本工作区

这里开始重建新的动画剧本，只保留当前 live 页面。

## 工作规则

1. 一次只做一页。
2. 先在本目录写清这一页的动画剧本。
3. 剧本确认后，再由 `Remotion` 实现这页动画。
4. 不再把旧 `01-09` 剧本和旧 `draw.io` 当作 live 来源。
5. 如果某份文档是几何草图镜像，请直接放在本目录，并在文件名里标明 `草图镜像-<sketch-id>`。
6. 每一页剧本只管理自己的截图、facts、评分和 review，不做跨页累计传递。

## 你要编辑的文件

- 页模板：[`_page-template.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/_page-template.md)
- 第 01 页：[`01-第一页.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/01-第一页.md)
- 第 02 页：[`02-第二页.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/02-第二页.md)
- 第 03 页：[`03-第三页.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/03-第三页.md)
- 第 04 页：[`04-第四页.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/04-第四页.md)
- 第 14 页草图镜像：[`14-第十四页-草图镜像-page14-contract-r1.md`](/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/14-第十四页-草图镜像-page14-contract-r1.md)

## 当前状态

- 旧剧本已从 live 路径移除
- 旧 `draw.io` 已从 live 路径移除
- `SlideApp` 当前 live 到第 04 页
- live 页包括：`01-第一页.md`、`02-第二页.md`、`03-第三页.md`、`04-第四页.md`
- 草图镜像页和正式动画页共用同一个 `Docs/剧本/` 目录，只靠文件名区分
