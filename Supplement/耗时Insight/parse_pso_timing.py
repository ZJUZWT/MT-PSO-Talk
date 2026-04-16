# -*- coding: utf-8 -*-
import csv, os, sys
from pathlib import Path

BASE_DIR = Path(__file__).parent
MATCH_PREFIX = "Mine PSO:"

def find_scopes_csvs(base_dir):
    results = {}
    for root, dirs, files in os.walk(base_dir):
        for f in files:
            if f.endswith("Scopes.csv"):
                full_path = Path(root) / f
                rel = full_path.relative_to(base_dir)
                key = str(rel).replace(os.sep, "_").replace("/", "_").replace(" ", "_").replace(".csv", "")
                results[key] = full_path
    return results

def parse_csv(filepath):
    results = []
    if not filepath.exists():
        return results
    with open(filepath, "r", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            name = row.get("Name", "").strip()
            if name.startswith(MATCH_PREFIX):
                results.append(row)
    return results

def safe_float(val, default=0.0):
    try:
        v = float(val)
        if v != v: return default
        if abs(v) == float("inf"): return default
        return v
    except:
        return default

def safe_int(val, default=0):
    try: return int(val)
    except: return default

def fmt_row(name, count, total, avg, mn, mx, sd):
    return "  {:<55} {:>6} {:>12.6f} {:>10.3f} {:>10.3f} {:>10.3f} {:>10.3f}".format(name, count, total, avg, mn, mx, sd)

def fmt_hdr():
    return "  {:<55} {:>6} {:>12} {:>10} {:>10} {:>10} {:>10}".format("Scope Name", "Count", "Total(s)", "Avg(ms)", "Min(ms)", "Max(ms)", "StdDev(ms)")

def fmt_sep():
    return "  {} {} {} {} {} {} {}".format("-"*55, "-"*6, "-"*12, "-"*10, "-"*10, "-"*10, "-"*10)

def main():
    insight_dir = None
    for item in BASE_DIR.iterdir():
        if "Insight" in item.name and item.is_dir():
            insight_dir = item
            break
    if insight_dir is None:
        print("[Error] No Insight directory found"); sys.exit(1)

    CSV_FILES = find_scopes_csvs(insight_dir)
    if not CSV_FILES:
        print("[Error] No *Scopes.csv found"); sys.exit(1)

    print()
    print("=" * 140)
    print("  Mine PSO Timing Report")
    print("=" * 140)
    print("  Search dir: {}".format(insight_dir))
    print("  Found {} Scopes CSV files".format(len(CSV_FILES)))

    all_entries = {}

    for device_name, csv_path in sorted(CSV_FILES.items()):
        print()
        print("-"*100)
        print("  Device/API: {}".format(device_name))
        print("  CSV: {}".format(csv_path))
        print("-"*100)

        entries = parse_csv(csv_path)
        all_entries[device_name] = entries

        if not entries:
            print("  [None] No Mine PSO: entries found")
            continue

        print(fmt_hdr())
        print(fmt_sep())

        entries.sort(key=lambda e: safe_float(e.get("TotalDurationSeconds", "0")), reverse=True)

        total_dur_all = 0.0
        total_cnt_all = 0

        for entry in entries:
            name = entry.get("Name", "").strip()
            short_name = name[len(MATCH_PREFIX):].strip() if name.startswith(MATCH_PREFIX) else name
            count = safe_int(entry.get("Count", "0"))
            total_dur = safe_float(entry.get("TotalDurationSeconds", "0"))
            mean_dur = safe_float(entry.get("MeanDurationSeconds", "0"))
            min_dur = safe_float(entry.get("MinDurationSeconds", "0"))
            max_dur = safe_float(entry.get("MaxDurationSeconds", "0"))
            dev_dur = safe_float(entry.get("DeviationDurationSeconds", "0"))
            total_dur_all += total_dur
            total_cnt_all += count
            print(fmt_row(short_name, count, total_dur, mean_dur*1000, min_dur*1000, max_dur*1000, dev_dur*1000))

        print(fmt_sep())
        print("  {:<55} {:>6} {:>12.6f}".format("TOTAL", total_cnt_all, total_dur_all))

    # Cross-device comparison
    print()
    print()
    print("="*160)
    print("  Cross Device/API Comparison (by Scope Name)")
    print("="*160)

    scope_names = set()
    for entries in all_entries.values():
        for entry in entries:
            scope_names.add(entry.get("Name", "").strip())
    scope_names = sorted(scope_names)

    if scope_names:
        devices = sorted(CSV_FILES.keys())
        header = "  {:<50}".format("Scope Name")
        for d in devices:
            short_d = d.replace("_Scopes", "")[:30]
            header += " | {:>30}".format(short_d)
        print(header)
        sep = "  " + "-"*50
        for d in devices:
            sep += " | " + "-"*30
        print(sep)

        for scope in scope_names:
            short_name = scope[len(MATCH_PREFIX):].strip() if scope.startswith(MATCH_PREFIX) else scope
            row = "  {:<50}".format(short_name)
            for d in devices:
                found = None
                for entry in all_entries.get(d, []):
                    if entry.get("Name", "").strip() == scope:
                        found = entry
                        break
                if found:
                    count = safe_int(found.get("Count", "0"))
                    mean_ms = safe_float(found.get("MeanDurationSeconds", "0")) * 1000
                    row += " | {:>5}x avg={:>8.3f}ms".format(count, mean_ms)
                else:
                    row += " | {:>30}".format("N/A")
            print(row)

    # Export CSV
    output_csv = BASE_DIR / "pso_timing_summary.csv"
    with open(output_csv, "w", encoding="utf-8-sig", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["Device","ScopeName","Count","TotalDurationSeconds","MeanDurationMs","MinDurationMs","MaxDurationMs","DeviationDurationMs","FirstStartSeconds","FirstDurationSeconds","LastStartSeconds","LastDurationSeconds"])
        for device_name in sorted(all_entries.keys()):
            for entry in all_entries[device_name]:
                name = entry.get("Name", "").strip()
                short_name = name[len(MATCH_PREFIX):].strip() if name.startswith(MATCH_PREFIX) else name
                writer.writerow([device_name, short_name, safe_int(entry.get("Count","0")), safe_float(entry.get("TotalDurationSeconds","0")), safe_float(entry.get("MeanDurationSeconds","0"))*1000, safe_float(entry.get("MinDurationSeconds","0"))*1000, safe_float(entry.get("MaxDurationSeconds","0"))*1000, safe_float(entry.get("DeviationDurationSeconds","0"))*1000, safe_float(entry.get("FirstStartSeconds","0")), safe_float(entry.get("FirstDurationSeconds","0")), safe_float(entry.get("LastStartSeconds","0")), safe_float(entry.get("LastDurationSeconds","0"))])

    print()
    print()
    print("  Summary CSV exported: {}".format(output_csv))
    print("=" * 140)

if __name__ == "__main__":
    main()
