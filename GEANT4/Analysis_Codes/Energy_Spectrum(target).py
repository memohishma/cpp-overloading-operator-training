# ============================================================
#  7Li(p,n)7Be Neutron Energy Spectrum Plotter — Google Colab
# ============================================================
#  Upload your Geant4 output_nt_Detector_t*.csv files from the
#  browser, merge them, and plot the neutron energy spectrum
#  in keV with a KDE overlay and a stats box (mu, median, sigma, N).
#
#  HOW TO USE IN COLAB:
#   1. Paste this whole script into a Colab cell and run it.
#   2. A file picker will pop up — select ALL your
#      output_nt_Detector_t*.csv files at once (multi-select).
#   3. Edit the CONFIG block below to match your run
#      (beam energy, spread, units, plot title, etc).
# ============================================================

# ---- 0. Install/import ------------------------------------------------
import io
import re
import glob
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy.stats import gaussian_kde

try:
    from google.colab import files
    IN_COLAB = True
except ImportError:
    IN_COLAB = False

# ============================================================
#  CONFIG — edit these to match your simulation
# ============================================================
EP_MEAN_MEV   = 2.2          # proton beam energy (MeV)
EP_SPREAD_MEV = 0.2          # proton beam energy spread, +/- (MeV)

# Units of the kinetic-energy column INSIDE the CSV files.
# Leave as "auto" to detect from the "(keV)"/"(MeV)" tag in the
# column header comment (e.g. "PostStepEnergy (keV)"). Only used
# as a fallback if that tag isn't found.
INPUT_ENERGY_UNIT = "auto"   # "auto", "MeV", or "keV"

# Name of the kinetic energy column if you already know it.
# Leave as None to auto-detect from the CSV header comments.
# Your files have both "PreStepEnergy (keV)" and "PostStepEnergy (keV)" —
# auto-detect prefers PostStepEnergy. Set explicitly to override, e.g.:
# ENERGY_COLUMN = "PreStepEnergy (keV)"
ENERGY_COLUMN = None

# Show a shaded band / dotted line marking the proton beam energy
# band on the plot for reference (purely illustrative annotation).
SHOW_EP_ANNOTATION = True

TARGET_LABEL = "Target"      # e.g. "Target" or "Detector" for the title
OUTPUT_PNG   = "neutron_spectrum_keV.png"
# ============================================================


# ---- 1. Upload files from the browser ---------------------------------
def upload_csvs():
    """Opens a browser file picker in Colab and returns a dict
    {filename: raw_bytes} for every uploaded file."""
    if not IN_COLAB:
        raise RuntimeError(
            "google.colab not available — this upload step only works "
            "inside a Colab notebook. Run this script in Colab, or "
            "replace this function with local file paths."
        )
    print("Select ALL your output_nt_Detector_t*.csv files (multi-select).")
    uploaded = files.upload()
    return uploaded


# ---- 2. Parse a single Geant4 CSV ntuple --------------------------------
def parse_g4_csv(raw_bytes, filename):
    """
    Geant4's CSV ntuple writer emits header lines. Two variants exist:
        #column0 fEvent                      (indexed form)
    or
        #column int fEvent                   (type + name, no index —
        #column double PostStepEnergy (keV)   this is what your files use)
    Columns appear in the same left-to-right order as the data rows,
    so we just collect them in the order they're declared.
    followed by comma-separated data rows (no header row of names).
    """
    text = raw_bytes.decode("utf-8", errors="ignore")
    lines = text.splitlines()

    col_names = []       # ordered list, for the "#column type name" form
    col_names_indexed = {}  # dict, for the "#columnN name" form
    data_lines = []
    for line in lines:
        line = line.strip()
        if not line:
            continue
        if line.startswith("#"):
            m_indexed = re.match(r"#column(\d+)\s+(.+)", line)
            m_typed = re.match(r"#column\s+\S+\s+(.+)", line)
            if m_indexed:
                idx, name = m_indexed.groups()
                col_names_indexed[int(idx)] = name.strip()
            elif m_typed:
                col_names.append(m_typed.group(1).strip())
            continue
        data_lines.append(line)

    if not data_lines:
        print(f"  [!] No data rows found in {filename}, skipping.")
        return None

    data_str = "\n".join(data_lines)
    df = pd.read_csv(io.StringIO(data_str), header=None)

    if col_names_indexed:
        names = [col_names_indexed.get(i, f"col{i}") for i in range(df.shape[1])]
        df.columns = names
    elif col_names and len(col_names) == df.shape[1]:
        df.columns = col_names
    else:
        # No usable header comments found — fall back to generic names
        df.columns = [f"col{i}" for i in range(df.shape[1])]

    df["__source_file__"] = filename
    return df


# ---- 3. Auto-detect the kinetic-energy column ---------------------------
def find_energy_column(df, forced_name=None):
    if forced_name is not None:
        if forced_name in df.columns:
            return forced_name
        raise ValueError(
            f"ENERGY_COLUMN='{forced_name}' not found. "
            f"Available columns: {list(df.columns)}"
        )

    candidates = [c for c in df.columns if re.search(
        r"kin.?e|kinetic|energy|^e$|^ke$", c, re.IGNORECASE)]

    if not candidates:
        raise ValueError(
            "Could not auto-detect the energy column. "
            f"Available columns are: {list(df.columns)}. "
            "Set ENERGY_COLUMN explicitly in the CONFIG block."
        )

    # Prefer PostStepEnergy over PreStepEnergy (typically identical for a
    # scoring volume, but PostStep is the more conventional choice), then
    # anything with "kin" in it, then whatever else matched.
    def rank(name):
        n = name.lower()
        if "poststep" in n:
            return 0
        if "kin" in n:
            return 1
        if "prestep" in n:
            return 2
        return 3

    candidates.sort(key=rank)
    return candidates[0]


def detect_unit_from_column_name(col_name):
    """Looks for a '(keV)' or '(MeV)' tag inside the column name."""
    m = re.search(r"\((keV|MeV)\)", col_name, re.IGNORECASE)
    if m:
        return m.group(1)
    return None


# ---- 4. Load + merge all uploaded files ---------------------------------
def load_all(uploaded_dict):
    frames = []
    for fname, raw in uploaded_dict.items():
        if "Detector" not in fname:
            print(f"  [i] Skipping {fname} (not a Detector file).")
            continue
        df = parse_g4_csv(raw, fname)
        if df is not None:
            frames.append(df)
            print(f"  [+] Loaded {fname}: {len(df)} rows")
    if not frames:
        raise RuntimeError("No usable output_nt_Detector_t*.csv files found.")
    merged = pd.concat(frames, ignore_index=True)
    print(f"\nTotal merged rows (neutrons at detector): {len(merged)}")
    return merged


# ---- 5. Main plotting routine -------------------------------------------
def plot_spectrum(energies_keV, ep_mean_mev, ep_spread_mev,
                   target_label="Target", out_png="neutron_spectrum_keV.png"):
    n = len(energies_keV)
    mu = np.mean(energies_keV)
    med = np.median(energies_keV)
    sigma = np.std(energies_keV)

    fig, ax = plt.subplots(figsize=(10, 5.2))

    # --- histogram (step style, filled) ---
    counts, bin_edges, _ = ax.hist(
        energies_keV, bins=100, histtype="step",
        color="#1b3d5c", linewidth=1.3, label="Geant4 simulation"
    )
    ax.fill_between(
        0.5 * (bin_edges[:-1] + bin_edges[1:]), counts,
        step="mid", color="#a9c6de", alpha=0.35
    )

    # --- KDE overlay (Scott's rule, scipy default) ---
    kde = gaussian_kde(energies_keV, bw_method="scott")
    x_grid = np.linspace(energies_keV.min(), energies_keV.max(), 500)
    # scale KDE to match histogram bin width * N so it overlays the counts
    bin_width = bin_edges[1] - bin_edges[0]
    kde_y = kde(x_grid) * n * bin_width
    ax.plot(x_grid, kde_y, "--", color="crimson", linewidth=1.8,
            label="KDE (Scott's bandwidth)")

    # --- optional beam-energy reference annotation ---
    if SHOW_EP_ANNOTATION:
        ax.axvspan(0, 0, color="none")  # no-op placeholder (keeps legend order)
        # Illustrative legend-only entries (band/line reflect the proton
        # beam energy spread used in the run, shown for reference only —
        # adjust/remove if it doesn't apply to your x-axis quantity).
        ax.plot([], [], color="wheat", linewidth=8, alpha=0.6,
                label=f"$E_p$ = {ep_mean_mev} $\\pm$ {ep_spread_mev} MeV band")
        ax.plot([], [], ":", color="gray",
                label=f"$E_p$ = {ep_mean_mev} MeV")

    ax.set_xlabel("Neutron Kinetic Energy [keV]")
    ax.set_ylabel("Neutron Yield [n / keV / proton]")
    ax.set_title(
        f"$^{{7}}$Li(p,n)$^{{7}}$Be — Neutron Energy Spectrum ({target_label})\n"
        f"$E_p$ = {ep_mean_mev} $\\pm$ {ep_spread_mev} MeV  |  $N_n$ = {n:,}",
        fontsize=12
    )
    ax.legend(loc="upper right", fontsize=9, frameon=True)

    # Leave headroom above the data so the stats box has a clear
    # spot below the legend instead of overlapping it.
    ymin, ymax = ax.get_ylim()
    ax.set_ylim(0, ymax * 1.35)
    ax.set_xlim(0, energies_keV.max())

    stats_text = (
        f"$\\mu$ = {mu:,.1f} keV\n"
        f"$\\tilde{{E}}$ = {med:,.1f} keV\n"
        f"$\\sigma$ = {sigma:,.1f} keV\n"
        f"N = {n:,}"
    )
    ax.text(
        0.985, 0.72, stats_text, transform=ax.transAxes,
        fontsize=9, va="top", ha="right",
        bbox=dict(boxstyle="round", facecolor="white",
                  edgecolor="gray", alpha=0.9)
    )

    ax.grid(alpha=0.25)
    fig.tight_layout()
    fig.savefig(out_png, dpi=200)
    plt.show()
    print(f"\nSaved plot to: {out_png}")


# ============================================================
#  RUN
# ============================================================
if __name__ == "__main__":
    uploaded = upload_csvs()
    merged = load_all(uploaded)

    energy_col = find_energy_column(merged, ENERGY_COLUMN)
    print(f"\nUsing energy column: '{energy_col}'")

    energies = merged[energy_col].astype(float).values

    if INPUT_ENERGY_UNIT.lower() == "auto":
        detected_unit = detect_unit_from_column_name(energy_col)
        if detected_unit is None:
            raise ValueError(
                f"Couldn't detect a unit from column name '{energy_col}'. "
                "Set INPUT_ENERGY_UNIT explicitly to 'MeV' or 'keV'."
            )
        unit = detected_unit
        print(f"Auto-detected energy unit: {unit}")
    else:
        unit = INPUT_ENERGY_UNIT

    if unit.lower() == "mev":
        energies_keV = energies * 1000.0
    else:
        energies_keV = energies

    plot_spectrum(
        energies_keV,
        ep_mean_mev=EP_MEAN_MEV,
        ep_spread_mev=EP_SPREAD_MEV,
        target_label=TARGET_LABEL,
        out_png=OUTPUT_PNG,
    )
