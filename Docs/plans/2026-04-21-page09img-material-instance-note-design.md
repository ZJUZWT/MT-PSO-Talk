# Page09img Material Instance Note Design

**Goal:** Move the faint Material/MaterialInstance explanation out of the middle comparison card on `page_09_img`, and replace it with a more factual sample note that explains the parent material, the two matching instances, and the Static Bool preservation caveat.

**Design:**
- Keep the existing `page_09_img` evidence layout and transition grammar unchanged.
- Remove the old in-card helper line `M=Material，M-I*=Material Instance` from the center `.uexp` comparison card.
- Add a new outside note above the three top evidence cards so the explanation reads like a page-level legend instead of card-local copy.
- Split the note into one main legend line plus two smaller footnote lines:
  - main legend: parent material and the two matching instances
  - footnote: both instances flip the same Static Bool so UE keeps the corresponding ShaderCode
- Sync the same factual explanation into storyboard notes and the page script markdown.

**Why this shape:**
- It honors the user's request to move the faint explanation outside and above the cards.
- It avoids compressing the center comparison card.
- It keeps the explanation visibly secondary while making the sample setup auditable.
