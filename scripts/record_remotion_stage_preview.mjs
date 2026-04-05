#!/usr/bin/env node

import {spawn} from "node:child_process";
import {mkdir, rm} from "node:fs/promises";
import {createRequire} from "node:module";
import {dirname, join, resolve} from "node:path";
import {fileURLToPath} from "node:url";

const require = createRequire(import.meta.url);
const {chromium} = require("../SlideApp/node_modules/playwright");

const __dirname = dirname(fileURLToPath(import.meta.url));
const repoRoot = resolve(__dirname, "..");
const distDir = join(repoRoot, "SlideApp", "dist");
const outputDir = join(repoRoot, ".generated", "storyboard-previews", "remotion");
const tempVideoDir = join(outputDir, ".tmp-video");
const port = 4173;
const url = `http://127.0.0.1:${port}`;

const viewport = {width: 1760, height: 1000};
const mp4Path = join(outputDir, "remotion-stage-06-09-v1.mp4");
const gifPath = join(outputDir, "remotion-stage-06-09-v1.gif");
const STEP_SEQUENCE = [
  {id: "inline_material", frame: 234, settleMs: 1500},
  {id: "shared_code", frame: 270, settleMs: 1500},
  {id: "ue_pso", frame: 306, settleMs: 1700},
];

async function waitForServer(targetUrl) {
  const startedAt = Date.now();

  while (Date.now() - startedAt < 15000) {
    try {
      const response = await fetch(targetUrl);
      if (response.ok) {
        return;
      }
    } catch {}

    await new Promise((resolveDelay) => setTimeout(resolveDelay, 250));
  }

  throw new Error(`Timed out waiting for ${targetUrl}`);
}

function even(value) {
  const rounded = Math.round(value);
  return rounded % 2 === 0 ? rounded : rounded - 1;
}

async function waitForFrame(page, expectedFrame) {
  await page.waitForFunction(
    (frame) =>
      document.querySelector(".stage-runtime")?.getAttribute("data-current-frame") ===
      String(frame),
    expectedFrame,
    {timeout: 4000},
  );
}

async function main() {
  await mkdir(outputDir, {recursive: true});
  await rm(tempVideoDir, {force: true, recursive: true});
  await mkdir(tempVideoDir, {recursive: true});

  const server = spawn("python3", ["-m", "http.server", String(port), "-d", distDir], {
    cwd: repoRoot,
    stdio: "ignore",
  });

  try {
    await waitForServer(url);

    const browser = await chromium.launch({
      headless: true,
      args: ["--autoplay-policy=no-user-gesture-required"],
    });

    let rawVideoPath = "";

    try {
      const context = await browser.newContext({
        viewport,
        deviceScaleFactor: 1,
        recordVideo: {
          dir: tempVideoDir,
          size: viewport,
        },
      });

      const page = await context.newPage();
      await page.goto(url, {waitUntil: "networkidle"});

      await page.getByRole("button", {name: /show controls/i}).click();
      await page.getByLabel("Layout", {exact: true}).selectOption("canvas-priority");
      await page.getByLabel("Step", {exact: true}).selectOption("ue_mapping");
      await page.getByRole("button", {name: /hide controls/i}).click();
      await page.waitForTimeout(1200);

      const runtime = page.locator(".stage-runtime");
      const box = await runtime.boundingBox();
      if (!box) {
        throw new Error("Could not resolve .stage-runtime bounding box");
      }

      const crop = {
        x: even(box.x),
        y: even(box.y),
        width: even(box.width),
        height: even(box.height),
      };

      await runtime.click({position: {x: 30, y: 30}});
      await waitForFrame(page, 198);
      await page.waitForTimeout(500);

      for (const step of STEP_SEQUENCE) {
        await page.keyboard.press("ArrowRight");
        await waitForFrame(page, step.frame);
        await page.waitForTimeout(step.settleMs);
      }

      const video = page.video();
      await page.close();
      await context.close();

      if (!video) {
        throw new Error("Playwright did not expose a video handle");
      }

      rawVideoPath = await video.path();

      const cropFilter = `crop=${crop.width}:${crop.height}:${crop.x}:${crop.y}`;

      const ffmpegArgs = [
        "-y",
        "-i",
        rawVideoPath,
        "-vf",
        cropFilter,
        "-pix_fmt",
        "yuv420p",
        mp4Path,
      ];
      const ffmpeg = spawn("/opt/homebrew/bin/ffmpeg", ffmpegArgs, {
        cwd: repoRoot,
        stdio: "inherit",
      });
      await new Promise((resolveDone, rejectDone) => {
        ffmpeg.on("exit", (code) => {
          if (code === 0) {
            resolveDone();
            return;
          }
          rejectDone(new Error(`ffmpeg mp4 exited with ${code}`));
        });
      });

      const gif = spawn(
        "/opt/homebrew/bin/ffmpeg",
        [
          "-y",
          "-i",
          mp4Path,
          "-vf",
          "fps=24,scale=960:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse",
          gifPath,
        ],
        {
          cwd: repoRoot,
          stdio: "inherit",
        },
      );
      await new Promise((resolveDone, rejectDone) => {
        gif.on("exit", (code) => {
          if (code === 0) {
            resolveDone();
            return;
          }
          rejectDone(new Error(`ffmpeg gif exited with ${code}`));
        });
      });
    } finally {
      await browser.close();
    }

    console.log(mp4Path);
    console.log(gifPath);
  } finally {
    server.kill("SIGTERM");
    await rm(tempVideoDir, {force: true, recursive: true});
  }
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
