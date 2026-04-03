# SlideApp Public Submodule Migration Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Split `SlideApp` into its own public GitHub repository, publish it with GitHub Pages, and reattach it to the private `MT-PSO-Talk` repo as a submodule at the same path.

**Architecture:** Create a new standalone repository whose root is the current `SlideApp` app. Add a self-contained Pages workflow there, then replace the tracked `SlideApp/` directory in the private repo with a git submodule pointing at the new public repository. Keep root scripts in the private repo targeting the submodule path so day-to-day development can still start from the monorepo root.

**Tech Stack:** Git, Git submodules, GitHub CLI, GitHub Pages, Vite, React, Remotion, npm.

---

### Task 1: Snapshot the current SlideApp into a standalone repository workspace

**Files:**
- Create: external local repo workspace `../MT-PSO-Talk-SlideApp`
- Copy from: `SlideApp/**`
- Verify: `SlideApp/package.json`

**Step 1: Prepare the standalone workspace**

Create a sibling directory outside the private repo and copy the current `SlideApp` contents into it.

**Step 2: Verify the copied app has the expected package root**

Run: `test -f ../MT-PSO-Talk-SlideApp/package.json`
Expected: PASS

**Step 3: Initialize git metadata**

Run: `git init -b main`
Expected: local standalone repository is ready for commits

**Step 4: Commit the baseline**

Run:
```bash
git add -A
git commit -m "feat: initialize slide app"
```

### Task 2: Add public-repo-specific repository files and Pages workflow

**Files:**
- Create: `../MT-PSO-Talk-SlideApp/.github/workflows/deploy.yml`
- Create: `../MT-PSO-Talk-SlideApp/README.md`
- Modify: `../MT-PSO-Talk-SlideApp/package.json` if workflow-facing scripts need adjustment
- Modify: `../MT-PSO-Talk-SlideApp/vite.config.ts` only if repo-name Pages base handling needs clarification

**Step 1: Add a repo-root Pages workflow**

Create a workflow that installs dependencies, builds with `BASE_PATH=/${{ github.event.repository.name }}/`, uploads the Pages artifact, and deploys it.

**Step 2: Add a repository README**

Document local development, Pages publishing, and the relationship to `MT-PSO-Talk`.

**Step 3: Verify local standalone app behavior**

Run:
```bash
npm ci
npm test
npm run build
BASE_PATH=/MT-PSO-Talk-SlideApp/ npm run build:pages
```
Expected: all commands PASS

**Step 4: Commit the standalone repo setup**

Run:
```bash
git add -A
git commit -m "build: add github pages deployment"
```

### Task 3: Create the public GitHub repository and push the standalone app

**Files:**
- Remote repo: `ZJUZWT/MT-PSO-Talk-SlideApp`

**Step 1: Create the public repository**

Run:
```bash
gh repo create ZJUZWT/MT-PSO-Talk-SlideApp --public --source=. --remote=origin --push
```
Expected: remote repository exists and current commits are pushed

**Step 2: Enable GitHub Pages**

Run:
```bash
gh api -X POST repos/ZJUZWT/MT-PSO-Talk-SlideApp/pages -f build_type=workflow
```
Expected: Pages site is created for workflow deployment

**Step 3: Watch the first deployment**

Run:
```bash
gh run list --repo ZJUZWT/MT-PSO-Talk-SlideApp --workflow deploy.yml --limit 1
gh run watch --repo ZJUZWT/MT-PSO-Talk-SlideApp <run-id> --exit-status
```
Expected: deployment completes successfully

### Task 4: Replace the private repo's tracked SlideApp directory with a submodule

**Files:**
- Delete: `.github/workflows/deploy-slideapp.yml`
- Delete: tracked `SlideApp/**` files from the private repo index
- Create: `.gitmodules`
- Create: git submodule entry at `SlideApp`
- Modify: `README.md`

**Step 1: Remove the old in-repo SlideApp tracking**

Run:
```bash
git rm -r SlideApp
git rm .github/workflows/deploy-slideapp.yml
```
Expected: the private repo is ready to replace the directory with a submodule

**Step 2: Add the public repo as a submodule at the same path**

Run:
```bash
git submodule add https://github.com/ZJUZWT/MT-PSO-Talk-SlideApp.git SlideApp
```
Expected: `SlideApp` reappears as a git submodule and `.gitmodules` is created

**Step 3: Update private repo documentation**

Document that `SlideApp` is public, lives in a submodule, and requires `git submodule update --init --recursive` after clone.

**Step 4: Verify root workflows still work**

Run:
```bash
npm --prefix SlideApp ci
npm run slide:test
npm run slide:build
npm run benchmark:configure
git submodule status
```
Expected: all commands PASS and the submodule status points to the pushed public commit

**Step 5: Commit the private repo migration**

Run:
```bash
git add -A
git commit -m "build: move slide app to public submodule"
```

### Task 5: Push the private repo update and verify both repositories

**Files:**
- Remote repo: `ZJUZWT/MT-PSO-Talk`
- Remote repo: `ZJUZWT/MT-PSO-Talk-SlideApp`

**Step 1: Push the private repo**

Run:
```bash
git push origin main
```
Expected: the submodule pointer and docs update are published

**Step 2: Verify the public Pages URL**

Expected URL: `https://zjuzwt.github.io/MT-PSO-Talk-SlideApp/`

**Step 3: Summarize the final developer workflow**

Document the two-step commit flow:
- commit/push inside `SlideApp/`
- commit/push the updated submodule pointer in `MT-PSO-Talk`
