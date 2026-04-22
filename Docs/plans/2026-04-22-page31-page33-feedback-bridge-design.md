# Page 31-33 Feedback Bridge Design

## Goal

把当前 late-tail 结尾重新拆成 3 页：

- `page_31`：讲仓库里真实在跑的 harness 回路
- `page_32`：用“反馈系统 -> 人的学习 -> 非代码能力”做桥
- `page_33`：用《逍遥游》与推荐做最终收束

## Approved Direction

### Page 31

- 不再把重点放在 `geometryReviewArtifact -> geometryMetrics -> geometryScorePolicy` 这条内部链路本身。
- 改成观众更容易一眼看懂的 live harness 流程：
  - `Hook 进入`
  - `网页数据评分`
  - `网页图片评分`
  - `回执循环`
  - `通过则停止 / 不通过继续`
- 右侧辅助卡只负责点名真实实现抓手，不抢主循环：
  - `workflow_gate.py`
  - `front Edge probe`
  - `browser-api capture`
  - `blind critics`

### Page 32

- 这一页不继续堆工程名词，也不直接跳到《逍遥游》。
- 只负责承上启下：
  - 承上：`反馈系统`
  - 启下：`人的学习`、`纯代码能力边际收益下降`、`看似无用的知识未来可能显出价值`
- 页面气质偏停顿与思考，不做复杂图解。

### Page 33

- 恢复成单独终页。
- 复用当前 merged final page 的成熟结构：
  - 上半《逍遥游》引句
  - 下半左右两组推荐入口

## Narrative Rationale

- `page_31` 负责“客观事实”
- `page_32` 负责“感性桥”
- `page_33` 负责“最终收束”

这样可以把用户想讲的这条线拆干净：

`harness -> feedback system -> 人怎么学习 -> 非代码能力 -> 无用之用`

## Layout Decision

### Page 31

- 保留左主右辅的双区结构。
- 左侧主卡画 4 节点顺时针回环。
- 右侧辅助卡用 token + 两条短句解释“真实评分来源”和“停 / 继续”。
- 不再让公式链路占主角。

### Page 32

- 采用无框大字停顿页。
- 上方一个题眼。
- 中部 4 到 6 行大字正文。
- 底部一行橙色收束句，显式点出“不断回执、不断修正、螺旋上升”。

### Page 33

- 直接复用当前 merged epilogue 的视觉骨架。
- 不额外加新结构，避免结尾再引入陌生版式。

## Timeline Decision

- 恢复 `page_33` 到 storyboard、timeline、formal review、workflow gate 的 canonical step 序列。
- `page_31 -> page_32` 维持轻 handoff。
- 新增 `page_32 -> page_33` handoff。
- 总时长需要顺延，不能把新增终页硬塞进当前时长里。

## Verification Targets

- `page_31` 屏幕上能直接读出“真实网页评分 + 图片评分 + 回执 + 停 / 继续”。
- `page_32` 不再像工程说明页，而像桥页。
- `page_33` 恢复成单独终页。
- `audit_transition_timings.py`、`audit_storyboard_sync.py`、`review:mechanical -- --from page_31` 都通过。
