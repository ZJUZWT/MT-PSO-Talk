type PlaybackFrameInput = {
  fromFrame: number;
  toFrame: number;
  durationMs: number;
  elapsedMs: number;
};

const clamp01 = (value: number) => Math.max(0, Math.min(1, value));

export function resolvePlaybackProgress(input: {
  durationMs: number;
  elapsedMs: number;
}) {
  if (input.durationMs <= 0) {
    return 1;
  }

  return clamp01(input.elapsedMs / input.durationMs);
}

export function resolvePlaybackFrame(input: PlaybackFrameInput) {
  if (input.durationMs <= 0 || input.fromFrame === input.toFrame) {
    return input.toFrame;
  }

  const progress = resolvePlaybackProgress({
    durationMs: input.durationMs,
    elapsedMs: input.elapsedMs,
  });

  return Math.round(
    input.fromFrame + (input.toFrame - input.fromFrame) * progress,
  );
}
