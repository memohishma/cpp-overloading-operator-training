# ============================================================
#  7Li(p,n)7Be Neutron Beam Spot Profile Plotter
# ============================================================
#  Merges existing output_nt_Detector_t*.csv files in the working
#  directory and plots the 2D neutron beam spot profile (X-Y position
#  density at the detector) as a smooth heatmap with 1/2/3-sigma
#  reference circles and a centroid marker.
# ============================================================

import glob
import io
import re
import matplotlib.pyplot as plt
from matplotlib.patches import Circle
import numpy as np
import pandas as pd
from scipy.stats import gaussian_kde

# ============================================================
#  CONFIG — edit these to match your simulation
# ============================================================
EP_MEAN_MEV = 2.2  # proton beam energy (MeV)
EP_SPREAD_MEV = 0.2  # proton beam energy spread, +/- (MeV)
DTHETA_MRAD = 10.0  # angular spread shown in the subtitle (mrad)
TARGET_LABEL = "Detector"  # e.g. "Target", "Detector", "Combined Targets"

# Physical target parameters for the plot subtitle
TARGET_MATERIAL = "Li"  # e.g. "Li", "LiF", or None
TARGET_THICKNESS_UM = 60  # thickness in micrometers, or None

# Column names for the transverse position (auto-detected if None).
X_COLUMN = None
Y_COLUMN = None

# Units of fX/fY inside the CSV (Geant4 default is meters).
INPUT_POS_UNIT = "m"  # "m" or "cm"

# Show the legend box and the bottom-left stats box?
SHOW_LEGEND = False
SHOW_STATS_BOX = False

# Zoom the plot to a window around the beam spot (in cm), or None
# to auto-scale to the data range.
WINDOW_HALF_WIDTH_CM = None  # e.g. 0.0015, or None for auto
ZOOM_SIGMA = 3  # window half-width = ZOOM_SIGMA * sigma_r when
# WINDOW_HALF_WIDTH_CM is None. Lower = tighter
# zoom on the spot, higher = wider view.

OUTPUT_PNG = "beam_spot_profile.png"
# ============================================================


# ---- 1. Parse a single Geant4 CSV ntuple --------------------------------
def parse_g4_csv(filepath):
  """Handles both Geant4 CSV header styles:

      #column0 fEvent                       (indexed form)
      #column double PostStepEnergy (keV)   (type + name form)
  """
  with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
    lines = f.readlines()

  col_names_indexed = {}
  col_names = []
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

  df = pd.read_csv(io.StringIO("\n".join(data_lines)), header=None)

  if col_names_indexed:
    names = [col_names_indexed.get(i, f"col{i}") for i in range(df.shape[1])]
    df.columns = names
  elif col_names and len(col_names) == df.shape[1]:
    df.columns = col_names
  else:
    df.columns = [f"col{i}" for i in range(df.shape[1])]

  df["__source_file__"] = filepath
  return df


# ---- 2. Auto-detect X/Y position columns --------------------------------
def find_pos_columns(df, forced_x=None, forced_y=None):
  if forced_x and forced_y:
    if forced_x in df.columns and forced_y in df.columns:
      return forced_x, forced_y
    raise ValueError(
        f"X_COLUMN/Y_COLUMN not found. Available: {list(df.columns)}"
    )

  def normalize(col):
    # strip everything except letters, lowercase -> "fX (m)" -> "fxm"
    return re.sub(r"[^a-zA-Z]", "", col).lower()

  x_names = {"x", "fx", "posx", "xpos", "positionx"}
  y_names = {"y", "fy", "posy", "ypos", "positiony"}

  x_candidates = [
      c
      for c in df.columns
      if normalize(c).rstrip("m") in x_names or normalize(c) in x_names
  ]
  y_candidates = [
      c
      for c in df.columns
      if normalize(c).rstrip("m") in y_names or normalize(c) in y_names
  ]

  if not x_candidates or not y_candidates:
    raise ValueError(
        "Could not auto-detect X/Y position columns. "
        f"Available columns: {list(df.columns)}. "
        "Set X_COLUMN / Y_COLUMN explicitly in the CONFIG block."
    )
  return x_candidates[0], y_candidates[0]


def detect_unit_from_column_name(col_name, default):
  m = re.search(r"\((m|cm|mm)\)", col_name, re.IGNORECASE)
  return m.group(1).lower() if m else default


# ---- 3. Load + merge local CSV files ------------------------------------
def load_local_csvs(pattern="output_nt_Detector_t*.csv"):
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
    raise RuntimeError("No usable output_nt_Detector_t*.csv files found.")

  merged = pd.concat(frames, ignore_index=True)
  print(f"\nTotal merged rows (neutrons at detector): {len(merged)}")
  return merged


# ---- 4. Main plotting routine -------------------------------------------
def plot_beam_spot(
    x_cm,
    y_cm,
    ep_mean_mev,
    ep_spread_mev,
    dtheta_mrad,
    target_label="Detector",
    target_material=None,
    target_thickness_um=None,
    out_png="beam_spot_profile.png",
):
  n = len(x_cm)
  cx, cy = np.mean(x_cm), np.mean(y_cm)
  sx, sy = np.std(x_cm), np.std(y_cm)
  fwhm_x = 2.3548 * sx
  fwhm_y = 2.3548 * sy
  sigma_r = 0.5 * (sx + sy)  # average radial sigma, for the reference circles

  if WINDOW_HALF_WIDTH_CM is not None:
    half = WINDOW_HALF_WIDTH_CM
  else:
    half = ZOOM_SIGMA * sigma_r

  # --- smooth 2D density via KDE ---
  grid_n = 200
  xs = np.linspace(cx - half, cx + half, grid_n)
  ys = np.linspace(cy - half, cy + half, grid_n)
  XX, YY = np.meshgrid(xs, ys)
  kde = gaussian_kde(np.vstack([x_cm, y_cm]))
  ZZ = kde(np.vstack([XX.ravel(), YY.ravel()])).reshape(XX.shape)
  ZZ = ZZ * n  # scale from density to approximate counts

  fig, ax = plt.subplots(figsize=(8.2, 7.6))
  im = ax.pcolormesh(XX, YY, ZZ, cmap="hot", shading="auto")
  ax.contour(XX, YY, ZZ, levels=8, colors="black", linewidths=0.3, alpha=0.35)

  cbar = fig.colorbar(im, ax=ax, pad=0.02)
  cbar.set_label("Neutron Counts", fontsize=11)

  # --- 1/2/3 sigma reference circles ---
  styles = [
      (1, "-", "steelblue"),
      (2, "--", "steelblue"),
      (3, ":", "steelblue"),
  ]
  for k, ls, color in styles:
    circ = Circle(
        (cx, cy),
        k * sigma_r,
        fill=False,
        edgecolor=color,
        linestyle=ls,
        linewidth=1.4,
        label=f"{k}$\\sigma$" if k == 1 else f"{k}$\\sigma$",
    )
    ax.add_patch(circ)

  ax.plot(
      cx,
      cy,
      "+",
      color="black",
      markersize=16,
      markeredgewidth=2.5,
      label=f"Centroid ({cx:.3f}, {cy:.3f}) cm",
  )

  ax.set_xlim(cx - half, cx + half)
  ax.set_ylim(cy - half, cy + half)
  ax.set_aspect("equal")

  # Axes labels
  ax.set_xlabel("X Position [cm]", fontsize=13, labelpad=8)
  ax.set_ylabel("Y Position [cm]", fontsize=13, labelpad=8)
  ax.tick_params(axis="both", which="major", labelsize=10.5)

  # Subtitle construction incorporating target thickness
  subtitle_parts = [f"$E_p$ = {ep_mean_mev} $\\pm$ {ep_spread_mev} MeV"]
  if target_thickness_um is not None and target_material:
    subtitle_parts.append(
        f"Target: {target_thickness_um:g} $\\mu$m {target_material}"
    )
  elif target_material:
    subtitle_parts.append(f"Target: {target_material}")
  subtitle_parts.append(f"$\\Delta\\theta$ = {dtheta_mrad} mrad")
  subtitle_parts.append(f"$N_n$ = {n:,}")
  subtitle = "  |  ".join(subtitle_parts)

  # Reduced title and subtitle font sizes to prevent overlapping with colorbar
  ax.set_title(
      f"$^{{7}}$Li(p,n)$^{{7}}$Be — Neutron Beam Spot Profile ({target_label})\n"
      f"{subtitle}",
      fontsize=11,
      pad=10,
  )

  if SHOW_LEGEND:
    ax.legend(loc="upper right", fontsize=8, frameon=True)

  if SHOW_STATS_BOX:
    stats_text = (
        f"Centroid: ({cx:.4f}, {cy:.4f}) cm\n"
        f"$\\sigma_X$ = {sx:.4f} cm    FWHM$_X$ = {fwhm_x:.4f} cm\n"
        f"$\\sigma_Y$ = {sy:.4f} cm    FWHM$_Y$ = {fwhm_y:.4f} cm\n"
        f"N = {n:,}"
    )
    ax.text(
        0.02,
        0.02,
        stats_text,
        transform=ax.transAxes,
        fontsize=8,
        va="bottom",
        ha="left",
        bbox=dict(
            boxstyle="round", facecolor="white", edgecolor="gray", alpha=0.85
        ),
    )

  fig.tight_layout()
  fig.savefig(out_png, dpi=200)
  plt.show()
  print(f"\nSaved plot to: {out_png}")
  print(f"Centroid: ({cx:.4f}, {cy:.4f}) cm")
  print(f"sigma_X = {sx:.4f} cm, sigma_Y = {sy:.4f} cm")
  print(f"FWHM_X = {fwhm_x:.4f} cm, FWHM_Y = {fwhm_y:.4f} cm")


# ============================================================
#  EXECUTION
# ============================================================
if __name__ == "__main__":
  merged = load_local_csvs("output_nt_Detector_t*.csv")

  x_col, y_col = find_pos_columns(merged, X_COLUMN, Y_COLUMN)
  print(f"\nUsing position columns: X='{x_col}', Y='{y_col}'")

  x_vals = merged[x_col].astype(float).values
  y_vals = merged[y_col].astype(float).values

  x_unit = detect_unit_from_column_name(x_col, INPUT_POS_UNIT)
  y_unit = detect_unit_from_column_name(y_col, INPUT_POS_UNIT)

  unit_to_cm = {"m": 100.0, "cm": 1.0, "mm": 0.1}
  x_cm = x_vals * unit_to_cm.get(x_unit, 1.0)
  y_cm = y_vals * unit_to_cm.get(y_unit, 1.0)
  print(
      f"Position units detected: X in {x_unit}, Y in {y_unit} -> converted to"
      " cm"
  )

  plot_beam_spot(
      x_cm,
      y_cm,
      ep_mean_mev=EP_MEAN_MEV,
      ep_spread_mev=EP_SPREAD_MEV,
      dtheta_mrad=DTHETA_MRAD,
      target_label=TARGET_LABEL,
      target_material=TARGET_MATERIAL,
      target_thickness_um=TARGET_THICKNESS_UM,
      out_png=OUTPUT_PNG,
  )
