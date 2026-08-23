  #################################################################################################
  #################################################################################################
  #################################################################################################
  # ============================================================
#  7Li(p,n)7Be Neutron Energy Spectrum Plotter — Publication Quality
# ============================================================
#  Merges existing output_nt_Detector_t*.csv files in the working
#  directory and plots the neutron energy spectrum in keV with
#  a KDE overlay and statistical annotations, styled for a
#  Master's thesis / journal submission figure.
# ============================================================

import glob
import io
import re
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator
import numpy as np
import pandas as pd
from scipy.stats import gaussian_kde

# ============================================================
#  CONFIG — edit these to match your simulation
# ============================================================
EP_MEAN_MEV = 2.2  # proton beam energy (MeV)
EP_SPREAD_MEV = 0.2  # proton beam energy spread, +/- (MeV)

# Units of the kinetic-energy column inside the CSV files.
# "auto", "MeV", or "keV"
INPUT_ENERGY_UNIT = "auto"

# Specific column name or None for auto-detection
ENERGY_COLUMN = None

TARGET_LABEL = "Detector"  # e.g. "Target" or "Detector"

# Physical target parameters for the plot subtitle
TARGET_MATERIAL = "Li"  # e.g. "Li", "LiF", or None
TARGET_THICKNESS_UM = 40  # thickness in micrometers, or None

# X-axis window limit (keV)
XLIM_MAX_KEV = 1000

OUTPUT_PNG = "neutron_spectrum_keV.png"
# ============================================================


# ============================================================
#  PUBLICATION STYLE — serif fonts, journal-appropriate sizing
# ============================================================
plt.rcParams.update({
    "font.family": "serif",
    "font.serif": ["Times New Roman", "DejaVu Serif", "Georgia", "serif"],
    "mathtext.fontset": "stix",  # serif-matching math rendering
    "axes.labelsize": 12,
    "axes.titlesize": 12,
    "xtick.labelsize": 10,
    "ytick.labelsize": 10,
    "legend.fontsize": 10,
})


def parse_g4_csv(filepath):
  """Parses a single Geant4 CSV ntuple file from disk, reading header comments

  and comma-separated data rows.
  """
  with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
    lines = f.readlines()

  col_names = []
  col_names_indexed = {}
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
    print(f"  [!] No data rows found in {filepath}, skipping.")
    return None

  data_str = "\n".join(data_lines)
  df = pd.read_csv(io.StringIO(data_str), header=None)

  if col_names_indexed:
    names = [col_names_indexed.get(i, f"col{i}") for i in range(df.shape[1])]
    df.columns = names
  elif col_names and len(col_names) == df.shape[1]:
    df.columns = col_names
  else:
    df.columns = [f"col{i}" for i in range(df.shape[1])]

  df["__source_file__"] = filepath
  return df


def find_energy_column(df, forced_name=None):
  """Auto-detects or validates the kinetic energy column."""
  if forced_name is not None:
    if forced_name in df.columns:
      return forced_name
    raise ValueError(
        f"ENERGY_COLUMN='{forced_name}' not found. "
        f"Available columns: {list(df.columns)}"
    )

  candidates = [
      c
      for c in df.columns
      if re.search(r"kin.?e|kinetic|energy|^e$|^ke$", c, re.IGNORECASE)
  ]

  if not candidates:
    raise ValueError(
        "Could not auto-detect the energy column. "
        f"Available columns are: {list(df.columns)}. "
        "Set ENERGY_COLUMN explicitly in the CONFIG block."
    )

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
  """Detects '(keV)' or '(MeV)' inside the column name."""
  m = re.search(r"\((keV|MeV)\)", col_name, re.IGNORECASE)
  if m:
    return m.group(1)
  return None


def load_local_csvs(pattern="output_nt_Detector_t* (4).csv"):
  """Finds and merges all matching CSV files from the local directory."""
  file_list = sorted(glob.glob(pattern))
  if not file_list:
    raise FileNotFoundError(
        f"No files matching '{pattern}' were found in the working directory."
    )

  frames = []
  print(f"Found {len(file_list)} CSV files matching pattern '{pattern}':")
  for fname in file_list:
    df = parse_g4_csv(fname)
    if df is not None:
      frames.append(df)
      print(f"  [+] Loaded {fname}: {len(df)} rows")

  if not frames:
    raise RuntimeError("No valid data could be loaded from the CSV files.")

  merged = pd.concat(frames, ignore_index=True)
  print(f"\nTotal merged rows (neutrons at detector): {len(merged)}")
  return merged


def plot_spectrum(
    energies_keV,
    ep_mean_mev,
    ep_spread_mev,
    target_label="Target",
    target_material=None,
    target_thickness_um=None,
    xlim_max_kev=1000,
    out_png="neutron_spectrum_keV.png",
):
  n = len(energies_keV)
  mu = np.mean(energies_keV)
  med = np.median(energies_keV)
  sigma = np.std(energies_keV)

  fig, ax = plt.subplots(figsize=(7.5, 4.8))

  # --- Histogram (step style, filled) ---
  counts, bin_edges, _ = ax.hist(
      energies_keV,
      bins=100,
      range=(0, xlim_max_kev),
      histtype="step",
      color="#1f4e9c",
      linewidth=1.2,
      label="Geant4 simulation",
  )
  ax.fill_between(
      0.5 * (bin_edges[:-1] + bin_edges[1:]),
      counts,
      step="mid",
      color="#1f4e9c",
      alpha=0.12,
  )

  # --- KDE overlay (Scott's rule) ---
  kde = gaussian_kde(energies_keV, bw_method="scott")
  x_grid = np.linspace(0, xlim_max_kev, 500)
  bin_width = bin_edges[1] - bin_edges[0]
  kde_y = kde(x_grid) * n * bin_width
  ax.plot(
      x_grid,
      kde_y,
      linestyle="--",
      color="#c0392b",
      linewidth=1.8,
      label="KDE (Scott's bandwidth)",
  )

  ax.set_xlabel("Neutron Kinetic Energy [keV]")
  ax.set_ylabel("Neutron Yield [n / keV / proton]")

  # --- Subtitle construction ---
  subtitle_parts = [f"$E_p$ = {ep_mean_mev} $\\pm$ {ep_spread_mev} MeV"]
  if target_thickness_um is not None and target_material:
    subtitle_parts.append(
        f"Target: {target_thickness_um:g} $\\mu$m {target_material}"
    )
  elif target_material:
    subtitle_parts.append(f"Target: {target_material}")
  subtitle_parts.append(f"$N_n$ = {n:,}")
  subtitle = "  |  ".join(subtitle_parts)

  ax.set_title(
      f"$^{{7}}$Li(p,n)$^{{7}}$Be — Neutron Energy Spectrum ({target_label})\n"
      f"{subtitle}",
      fontsize=12,
  )

  # --- Legend: top right inside the frame ---
  ax.legend(
      loc="upper right",
      frameon=True,
      fancybox=True,
      edgecolor="0.4",
      framealpha=0.9,
  )

  # --- Axis limits matching the target image ---
  ax.set_xlim(0, 500)
  ax.set_ylim(0, 250)

  # --- Ticks: minor ticks on, inward on all four sides ---
  ax.minorticks_on()
  ax.tick_params(
      axis="both", which="both", direction="in", top=True, right=True
  )
  ax.tick_params(which="major", length=6)
  ax.tick_params(which="minor", length=3)
  ax.xaxis.set_minor_locator(AutoMinorLocator())
  ax.yaxis.set_minor_locator(AutoMinorLocator())

  # --- Stats box ---
  stats_text = (
      f"$\\mu$ = {mu:,.1f} keV\n"
      f"$\\tilde{{E}}$ = {med:,.1f} keV\n"
      f"$\\sigma$ = {sigma:,.1f} keV\n"
      f"N = {n:,}"
  )
  ax.text(
      0.975,
      0.74,
      stats_text,
      transform=ax.transAxes,
      fontsize=10,
      va="top",
      ha="right",
      bbox=dict(
          boxstyle="round,pad=0.5",
          facecolor="white",
          edgecolor="0.4",
          linewidth=0.8,
          alpha=0.85,
      ),
  )

  fig.tight_layout()
  fig.savefig(out_png, dpi=300, bbox_inches="tight")
  plt.show()
  print(f"\nSaved plot to: {out_png}")


# ============================================================
#  EXECUTION
# ============================================================
if __name__ == "__main__":
  merged = load_local_csvs("output_nt_Detector_t* (4).csv")

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
      target_material=TARGET_MATERIAL,
      target_thickness_um=TARGET_THICKNESS_UM,
      xlim_max_kev=XLIM_MAX_KEV,
      out_png=OUTPUT_PNG,
  )
