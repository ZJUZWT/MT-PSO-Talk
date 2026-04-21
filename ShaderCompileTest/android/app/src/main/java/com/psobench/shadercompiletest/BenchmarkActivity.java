package com.psobench.shadercompiletest;

import android.app.Activity;
import android.content.res.AssetManager;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import android.graphics.Color;
import android.graphics.Typeface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.TextView;

/**
 * BenchmarkActivity – Main UI for the PSO Shader Compile benchmark.
 *
 * Provides a simple UI with:
 *   - A slider to choose iteration count (1–500)
 *   - A "Run Benchmark" button
 *   - A monospace text area showing results
 */
public class BenchmarkActivity extends Activity {

    // Native methods
    private native String  runBenchmark(AssetManager assetManager, int iterations);
    private native boolean isVulkanSupported();

    static {
        System.loadLibrary("shader_compile_test");
    }

    private TextView    resultText;
    private TextView    iterLabel;
    private Button      runButton;
    private ProgressBar progressBar;
    private SeekBar     iterSlider;
    private int         iterations = 100;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // ── Build UI programmatically (no XML layout needed) ───────
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setBackgroundColor(Color.parseColor("#1a1a2e"));
        root.setPadding(dp(24), dp(48), dp(24), dp(24));

        // Title
        TextView title = new TextView(this);
        title.setText("🔬 PSO Shader Benchmark");
        title.setTextColor(Color.parseColor("#e94560"));
        title.setTextSize(TypedValue.COMPLEX_UNIT_SP, 24);
        title.setTypeface(Typeface.DEFAULT_BOLD);
        title.setGravity(Gravity.CENTER);
        root.addView(title, lp(true));

        // Subtitle
        TextView subtitle = new TextView(this);
        subtitle.setText("Vulkan PSO vs OpenGL ES – Dead Code Elimination Test");
        subtitle.setTextColor(Color.parseColor("#a0a0b0"));
        subtitle.setTextSize(TypedValue.COMPLEX_UNIT_SP, 13);
        subtitle.setGravity(Gravity.CENTER);
        subtitle.setPadding(0, dp(4), 0, dp(24));
        root.addView(subtitle, lp(true));

        // Vulkan support check
        boolean vulkanOk = false;
        try {
            vulkanOk = isVulkanSupported();
        } catch (Exception e) {
            // Vulkan not available
        }

        if (!vulkanOk) {
            TextView warn = new TextView(this);
            warn.setText("⚠️ Vulkan is NOT supported on this device!");
            warn.setTextColor(Color.parseColor("#ff6b6b"));
            warn.setTextSize(TypedValue.COMPLEX_UNIT_SP, 16);
            warn.setGravity(Gravity.CENTER);
            warn.setPadding(0, 0, 0, dp(16));
            root.addView(warn, lp(true));
        }

        // Iteration slider label
        iterLabel = new TextView(this);
        iterLabel.setText("Draw calls: " + iterations);
        iterLabel.setTextColor(Color.WHITE);
        iterLabel.setTextSize(TypedValue.COMPLEX_UNIT_SP, 15);
        root.addView(iterLabel, lp(true));

        // Iteration slider
        iterSlider = new SeekBar(this);
        iterSlider.setMax(499);  // 1 to 500
        iterSlider.setProgress(iterations - 1);
        iterSlider.setPadding(0, dp(8), 0, dp(16));
        iterSlider.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                iterations = progress + 1;
                iterLabel.setText("Draw calls: " + iterations);
            }
            @Override public void onStartTrackingTouch(SeekBar seekBar) {}
            @Override public void onStopTrackingTouch(SeekBar seekBar) {}
        });
        root.addView(iterSlider, lp(true));

        // Run button
        runButton = new Button(this);
        runButton.setText("▶  Run Benchmark");
        runButton.setTextColor(Color.WHITE);
        runButton.setBackgroundColor(Color.parseColor("#0f3460"));
        runButton.setTextSize(TypedValue.COMPLEX_UNIT_SP, 16);
        runButton.setPadding(dp(16), dp(12), dp(16), dp(12));
        LinearLayout.LayoutParams btnLp = lp(true);
        btnLp.bottomMargin = dp(16);
        root.addView(runButton, btnLp);

        // Progress bar
        progressBar = new ProgressBar(this);
        progressBar.setVisibility(View.GONE);
        root.addView(progressBar, lp(true));

        // Result text in a scrollable area
        ScrollView scroll = new ScrollView(this);
        scroll.setFillViewport(true);

        resultText = new TextView(this);
        resultText.setTypeface(Typeface.MONOSPACE);
        resultText.setTextColor(Color.parseColor("#16c79a"));
        resultText.setTextSize(TypedValue.COMPLEX_UNIT_SP, 12);
        resultText.setBackgroundColor(Color.parseColor("#11111d"));
        resultText.setPadding(dp(12), dp(12), dp(12), dp(12));
        resultText.setText("Press 'Run Benchmark' to start.\n\n"
            + "This test measures GPU execution time for:\n"
            + "  • Vulkan:    PSO compiled with colorWriteMask=0\n"
            + "  • OpenGL ES: glColorMask(0,0,0,0) set at draw time\n\n"
            + "Expected: Vulkan GPU time << GLES GPU time\n"
            + "(proving cross-stage Dead Code Elimination by PSO compiler)");

        scroll.addView(resultText);

        LinearLayout.LayoutParams scrollLp = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, 0, 1.0f);
        root.addView(scroll, scrollLp);

        setContentView(root);

        // ── Button click handler ───────────────────────────────────
        final boolean finalVulkanOk = vulkanOk;
        runButton.setOnClickListener(v -> {
            if (!finalVulkanOk) {
                resultText.setText("ERROR: Vulkan is not supported on this device.\n"
                    + "Cannot run the PSO comparison benchmark.");
                return;
            }
            startBenchmark();
        });

        // ── Auto-run mode (triggered via adb shell am start --ei) ──
        // Usage: adb shell am start -n com.psobench.shadercompiletest/.BenchmarkActivity
        //        --ez autorun true --ei iterations 500
        if (getIntent().getBooleanExtra("autorun", false) && finalVulkanOk) {
            int autoIters = getIntent().getIntExtra("iterations", 100);
            iterations = Math.max(1, Math.min(autoIters, 10000));
            iterLabel.setText("Draw calls: " + iterations);
            iterSlider.setProgress(Math.min(iterations - 1, 499));
            // Delay slightly to let the UI settle
            new Handler(Looper.getMainLooper()).postDelayed(this::startBenchmark, 500);
        }
    }

    private void startBenchmark() {
        runButton.setEnabled(false);
        iterSlider.setEnabled(false);
        progressBar.setVisibility(View.VISIBLE);
        resultText.setText("Running benchmark with " + iterations + " draw calls...\n"
            + "Please wait, this may take a few seconds.\n");

        final int iters = iterations;
        final AssetManager am = getAssets();

        // Run benchmark on a background thread
        new Thread(() -> {
            String result;
            try {
                result = runBenchmark(am, iters);
            } catch (Exception e) {
                result = "ERROR: " + e.getMessage();
            }

            // Save result to file for adb pull
            saveResultToFile(result);

            final String finalResult = result;
            new Handler(Looper.getMainLooper()).post(() -> {
                resultText.setText(finalResult);
                progressBar.setVisibility(View.GONE);
                runButton.setEnabled(true);
                iterSlider.setEnabled(true);
            });
        }).start();
    }

    private void saveResultToFile(String result) {
        try {
            File outFile = new File(getFilesDir(), "benchmark_result.txt");
            FileOutputStream fos = new FileOutputStream(outFile);
            OutputStreamWriter writer = new OutputStreamWriter(fos);
            writer.write(result);
            writer.close();
            fos.close();
        } catch (Exception e) {
            // Ignore file write errors
        }
    }

    // ── Helper: dp to pixels ───────────────────────────────────────
    private int dp(int dp) {
        return (int) TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_DIP, dp, getResources().getDisplayMetrics());
    }

    private LinearLayout.LayoutParams lp(boolean matchWidth) {
        return new LinearLayout.LayoutParams(
            matchWidth ? LinearLayout.LayoutParams.MATCH_PARENT
                       : LinearLayout.LayoutParams.WRAP_CONTENT,
            LinearLayout.LayoutParams.WRAP_CONTENT);
    }
}
