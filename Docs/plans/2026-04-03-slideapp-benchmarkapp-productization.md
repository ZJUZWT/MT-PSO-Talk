# SlideApp And BenchmarkApp Productization Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Reorganize the repository into product-style `SlideApp`, `BenchmarkApp`, and `Docs` roots, remove obsolete animation experiments and temporary media, and make the slide app publishable to GitHub Pages.

**Architecture:** Promote the current Remotion-based workbench into a standalone `SlideApp` package with all of its runtime and shared storyboard code colocated under that directory. Promote `PSO-compile-test` into `BenchmarkApp` with the existing CMake layout preserved. Keep `Docs` focused on written material and durable references only, and add a root-level Pages workflow that builds and publishes `SlideApp/dist`.

**Tech Stack:** Vite, React, Remotion Player, TypeScript, Vitest, GitHub Actions, CMake, C++.

---

### Task 1: Capture the target repository layout in tests and docs

**Files:**
- Create: `Docs/plans/2026-04-03-slideapp-benchmarkapp-productization.md`
- Modify: `SlideApp/package.json`
- Modify: `SlideApp/vite.config.ts`
- Test: `SlideApp/src/App.test.tsx`

**Step 1: Write the failing test**

Add a repository-shape regression to `SlideApp/src/App.test.tsx` or a small config-focused test so the app proves it exposes a stable Pages base and still renders after relocation.

**Step 2: Run test to verify it fails**

Run: `npm --prefix SlideApp run test -- --run src/App.test.tsx`
Expected: FAIL because the new `SlideApp` path and/or Pages assumptions are not wired yet.

**Step 3: Write minimal implementation**

Create the new directory shape and update the app package/config so the relocated app builds from `SlideApp`.

**Step 4: Run test to verify it passes**

Run: `npm --prefix SlideApp run test -- --run src/App.test.tsx`
Expected: PASS

**Step 5: Commit**

```bash
git add Docs/plans/2026-04-03-slideapp-benchmarkapp-productization.md SlideApp
git commit -m "refactor: promote slide app to root product package"
```

### Task 2: Promote the current slide workbench into `SlideApp`

**Files:**
- Create: `SlideApp/**`
- Delete: `Docs/prototypes/workbench/**`
- Delete: `Docs/prototypes/remotion-demo/**`
- Delete: `Docs/prototypes/shared/**`
- Modify: root `.gitignore`
- Test: `SlideApp/src/App.test.tsx`

**Step 1: Write the failing test**

Add a test expectation that the relocated app still renders the default PSO page and retains the rail and notes behavior after imports are rewritten.

**Step 2: Run test to verify it fails**

Run: `npm --prefix SlideApp run test -- --run src/App.test.tsx`
Expected: FAIL while imports still point to the old `Docs/prototypes/*` paths.

**Step 3: Write minimal implementation**

Copy only the currently used workbench, Remotion composition, and shared storyboard files into `SlideApp`, then rewrite imports so the app is self-contained.

**Step 4: Run test to verify it passes**

Run: `npm --prefix SlideApp run test -- --run src/App.test.tsx`
Expected: PASS

**Step 5: Commit**

```bash
git add SlideApp .gitignore
git commit -m "refactor: colocate slide app runtime sources"
```

### Task 3: Promote `PSO-compile-test` into `BenchmarkApp`

**Files:**
- Create: `BenchmarkApp/**`
- Delete: `PSO-compile-test/**`
- Modify: root docs that reference the benchmark location
- Test: `BenchmarkApp/CMakeLists.txt`

**Step 1: Write the failing test**

Use a configure/build smoke check that expects the benchmark root to exist at `BenchmarkApp`.

**Step 2: Run test to verify it fails**

Run: `cmake -S BenchmarkApp -B /tmp/mt-pso-benchmark-check`
Expected: FAIL before the directory move happens.

**Step 3: Write minimal implementation**

Move the benchmark project intact, preserving its internal `Test` layout and existing include paths.

**Step 4: Run test to verify it passes**

Run: `cmake -S BenchmarkApp -B /tmp/mt-pso-benchmark-check`
Expected: configure succeeds

**Step 5: Commit**

```bash
git add BenchmarkApp
git commit -m "refactor: rename benchmark app root"
```

### Task 4: Remove obsolete animation experiments and temporary media

**Files:**
- Delete: `Docs/prototypes/motion-canvas-demo/**`
- Delete: `Docs/prototypes/manim-demo/**`
- Delete: `Docs/prototypes/.smoke-artifacts/**`
- Delete: `Docs/PSO-video-demo-contact-sheet.png`
- Delete: `Docs/PSO-video-demo-frames/**`
- Delete: `Docs/PSO-video-demo-poster-crop.png`
- Delete: `Docs/PSO-video-demo-poster.png`
- Delete: `Docs/PSO-video-demo-segments/**`
- Delete: `Docs/PSO-video-demo.gif`
- Delete: `Docs/PSO-video-demo.mp4`
- Modify: `Docs/prototypes/README.md` or replacement product docs if needed
- Test: `git status --short`

**Step 1: Write the failing test**

Create a lightweight repo hygiene check by listing the obsolete paths that should disappear.

**Step 2: Run test to verify it fails**

Run: `test -e Docs/PSO-video-demo.mp4 && exit 1 || exit 0`
Expected: FAIL while the temporary media still exists.

**Step 3: Write minimal implementation**

Delete outdated experiment folders and generated videos/images that no longer belong in the cleaned product repo.

**Step 4: Run test to verify it passes**

Run: `test -e Docs/PSO-video-demo.mp4 && exit 1 || exit 0`
Expected: PASS

**Step 5: Commit**

```bash
git add -A Docs
git commit -m "chore: remove obsolete animation prototypes and media"
```

### Task 5: Add root scripts and GitHub Pages publishing

**Files:**
- Create: `package.json`
- Create: `.github/workflows/deploy-slideapp.yml`
- Modify: `SlideApp/vite.config.ts`
- Modify: `SlideApp/package.json`
- Modify: `SlideApp/index.html` if needed
- Create or Modify: `README.md`
- Test: `SlideApp/dist/**`

**Step 1: Write the failing test**

Add a Pages config regression so the built app no longer assumes `/` as its base path.

**Step 2: Run test to verify it fails**

Run: `npm --prefix SlideApp run build`
Expected: current build succeeds locally but still emits root-based asset assumptions for project-page hosting.

**Step 3: Write minimal implementation**

Add a Vite `base` strategy for GitHub Pages, root scripts to build/test `SlideApp`, and a workflow that uploads `SlideApp/dist` as the Pages artifact.

**Step 4: Run test to verify it passes**

Run: `npm --prefix SlideApp run build`
Expected: PASS with a publishable `dist`

**Step 5: Commit**

```bash
git add package.json .github/workflows/deploy-slideapp.yml SlideApp README.md
git commit -m "build: add slide app pages deployment"
```

### Task 6: Run full verification across the new structure

**Files:**
- Test: `SlideApp/src/**/*.test.ts*`
- Test: `BenchmarkApp/**`

**Step 1: Run targeted app tests**

Run: `npm --prefix SlideApp run test -- --run src/App.test.tsx`
Expected: PASS

**Step 2: Run full slide app test suite**

Run: `npm --prefix SlideApp run test`
Expected: PASS

**Step 3: Run slide app production build**

Run: `npm --prefix SlideApp run build`
Expected: PASS

**Step 4: Run benchmark configure smoke check**

Run: `cmake -S BenchmarkApp -B /tmp/mt-pso-benchmark-check`
Expected: PASS

**Step 5: Commit final verification-friendly cleanup**

```bash
git add -A
git commit -m "chore: finalize repository productization"
```
