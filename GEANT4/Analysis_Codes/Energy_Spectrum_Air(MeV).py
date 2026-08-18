"""
Neutron Energy Spectrum: Air (Detector Interface)
==================================================
Extracts a Geant4 multi-threaded output archive (Air/free-space case),
merges the 'output_nt_Detector_t*.csv' thread files, and plots a
publication-quality, IAEA-shaded BNCT neutron energy spectrum
(step/column histogram style).

Designed to run in Google Colab or a local Jupyter/Python environment.
"""

import glob
import os
import re
import zipfile

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# ---------------------------------------------------------------
# 0. Colab detection & file upload handling
# ---------------------------------------------------------------
try:
    from google.colab import files as colab_files
    IN_COLAB = True
except ImportError:
    IN_COLAB = False


def upload_zip_colab(prompt_label):
    """
    Interactively upload a single zip archive in Google Colab.
    Returns the path to the uploaded zip file, or None outside Colab.
    """
    if not IN_COLAB:
        return None
    print(f"Please upload the {prompt_label} zip archive...")
    uploaded = colab_files.upload()
    if not uploaded:
        return None
    return list(uploaded.keys())[0]


def extract_zip(zip_path, extract_to):
    """Extract a zip archive into extract_to, returning that directory."""
    os.makedirs(extract_to, exist_ok=True)
    with zipfile.ZipFile(zip_path, "r") as zf:
        zf.extractall(extract_to)
    print(f"Extracted '{zip_path}' -> '{extract_to}'")
    return extract_to


# ---------------------------------------------------------------
# 1. Locate the Detector thread files inside an extracted folder
# ---------------------------------------------------------------
DETECTOR_FILENAME_PATTERN = "output_nt_Detector_t*.csv"


def _thread_index(path):
    """Extract the thread index from a filename, tolerating '(n)' suffixes."""
    match = re.search(r"_t(\d+)", os.path.basename(path))
    return int(match.group(1)) if match else -1


def find_detector_files(root_dir, pattern=DETECTOR_FILENAME_PATTERN):
    """
    Recursively search root_dir for Detector thread-output CSVs (ignoring
    any other ntuples present, e.g. BoneDose/BonePhantom/GammaOutput/
    TargetInterface), deduplicated by thread index, sorted by thread index.
    """
    all_matches = glob.glob(os.path.join(root_dir, "**", pattern), recursive=True)

    best_by_thread = {}
    for p in all_matches:
        idx = _thread_index(p)
        is_duplicate = bool(re.search(r"\(\d+\)\.csv$", p))
        if idx not in best_by_thread:
            best_by_thread[idx] = p
        else:
            current_is_dup = bool(re.search(r"\(\d+\)\.csv$", best_by_thread[idx]))
            if current_is_dup and not is_duplicate:
                best_by_thread[idx] = p

    file_list = [best_by_thread[idx] for idx in sorted(best_by_thread.keys())]
    if not file_list:
        raise FileNotFoundError(
            f"No files matching '{pattern}' found anywhere under '{root_dir}'."
        )
    print(f"  Found {len(file_list)} Detector thread file(s) under '{root_dir}'.")
    return file_list


# ---------------------------------------------------------------
# 2. Header parsing / energy-column & unit auto-detection
# ---------------------------------------------------------------
def detect_energy_column(columns):
    """Auto-detect the kinetic-energy column name from a list of headers."""
    candidates = [
        "E_k", "Ek", "KineticEnergy", "kineticEnergy",
        "Energy_MeV", "Energy_keV", "Energy_eV",
        "Energy", "energy", "E",
    ]
    for c in candidates:
        if c in columns:
            return c
    for col in columns:
        if re.search(r"kinet|energy", col, re.IGNORECASE):
            return col
    for col in columns:
        if col.strip().lower() == "e":
            return col
    raise ValueError(
        f"Could not auto-detect an energy column. Available columns: {list(columns)}"
    )


def detect_energy_unit_factor(column_name):
    """Infer the energy unit from the column name; return factor to MeV."""
    name_lower = column_name.lower()
    if "mev" in name_lower:
        return 1.0, "MeV"
    if "kev" in name_lower:
        return 1e-3, "keV"
    if "ev" in name_lower:
        return 1e-6, "eV"
    if "gev" in name_lower:
        return 1e3, "GeV"
    return 1.0, "MeV (assumed \u2014 no unit suffix found in column name)"


def _is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False


def _extract_header(lines):
    """
    Locate the true column-name header inside a Geant4 ntuple CSV.
    Supports both the 'tools::wcsv::ntuple' one-field-per-line style
    ('#column double Energy_eV') and the single comma-list style.
    """
    column_names = []
    comma_header = None
    data_start_idx = 0

    for i, line in enumerate(lines):
        stripped = line.strip()
        if not stripped:
            continue
        if stripped.startswith("#"):
            col_match = re.match(r"#column\s+\S+\s+(\S+)", stripped)
            if col_match:
                column_names.append(col_match.group(1))
                continue
            if not column_names:
                candidate = stripped.lstrip("#").strip()
                tokens = [t.strip() for t in candidate.split(",")]
                if len(tokens) > 1 and any(
                    not _is_number(t) and t != "" for t in tokens
                ):
                    comma_header = tokens
            continue
        else:
            data_start_idx = i
            break

    if column_names:
        return column_names, data_start_idx
    return comma_header, data_start_idx


def load_and_merge(file_list, label=""):
    """
    Load every per-thread Detector CSV, auto-detect the energy column and
    its unit, convert to MeV, and concatenate into one 1-D numpy array.
    """
    all_energies = []
    energy_col_name = None
    unit_factor = None
    unit_label = None

    for fpath in file_list:
        with open(fpath, "r") as f:
            lines = f.readlines()

        header_tokens, _ = _extract_header(lines)

        if header_tokens is not None:
            df = pd.read_csv(fpath, comment="#", header=None, names=header_tokens)
        else:
            df = pd.read_csv(fpath, comment="#")

        if df.empty:
            continue

        if energy_col_name is None:
            energy_col_name = detect_energy_column(df.columns)
            unit_factor, unit_label = detect_energy_unit_factor(energy_col_name)
            print(f"  [{label}] Detected energy column: '{energy_col_name}' "
                  f"(unit: {unit_label} -> MeV, factor={unit_factor:g})")
        elif energy_col_name not in df.columns:
            energy_col_name = detect_energy_column(df.columns)
            unit_factor, unit_label = detect_energy_unit_factor(energy_col_name)

        raw_energies = pd.to_numeric(df[energy_col_name], errors="coerce").dropna().values
        all_energies.append(raw_energies * unit_factor)

    if not all_energies:
        raise ValueError(f"No valid neutron energy data found for '{label}'.")

    merged = np.concatenate(all_energies)
    print(f"  [{label}] Total merged neutron entries: {merged.size}")
    return merged


def load_dataset(zip_path, extract_dir, label):
    """Full pipeline for one dataset: extract zip -> find Detector files -> merge."""
    root = extract_zip(zip_path, extract_dir)
    file_list = find_detector_files(root)
    energies_MeV = load_and_merge(file_list, label=label)
    return energies_MeV[energies_MeV > 0]  # log-binning needs strictly positive


# ---------------------------------------------------------------
# 3. Main pipeline
# ---------------------------------------------------------------
def main():
    # -------------------------------------------------------
    # Locate/upload the Air zip archive.
    # Edit this path if your file is already sitting in the working
    # directory (e.g. after mounting Google Drive) instead of being
    # uploaded interactively.
    # -------------------------------------------------------
    AIR_ZIP = "BSA_2_billion_events.zip"       # <-- edit if your filename differs

    if IN_COLAB and not os.path.exists(AIR_ZIP):
        uploaded_name = upload_zip_colab("AIR (free-space)")
        if uploaded_name:
            AIR_ZIP = uploaded_name

    if not os.path.exists(AIR_ZIP):
        raise FileNotFoundError(
            f"Air zip not found at '{AIR_ZIP}'. Edit AIR_ZIP in the script, "
            "upload it, or mount Google Drive and point to its path."
        )

    # -------------------------------------------------------
    # Extract + merge the Air dataset
    # -------------------------------------------------------
    print("Loading AIR dataset...")
    energies_air_MeV = load_dataset(AIR_ZIP, "extracted_air", label="Air")

    # -------------------------------------------------------
    # Logarithmic energy bins: 1e-9 MeV (1 meV) to 1e1 MeV (10 MeV)
    # -------------------------------------------------------
    energy_bins = np.logspace(-9, 1, 120)

    # -------------------------------------------------------
    # Publication-quality figure
    # -------------------------------------------------------
    plt.rcParams["font.family"] = "Times New Roman"
    plt.rcParams["font.size"] = 14

    fig, ax = plt.subplots(figsize=(10, 6), dpi=300, facecolor="white")
    ax.set_facecolor("white")

    # IAEA BNCT energy-region shading
    ax.axvspan(1e-9, 0.5e-6, color="red", alpha=0.15, label="Thermal (<0.5 eV)")
    ax.axvspan(0.5e-6, 10e-3, color="green", alpha=0.15, label="Epithermal (0.5 eV\u201310 keV)")
    ax.axvspan(10e-3, 1e1, color="orange", alpha=0.15, label="Fast (>10 keV)")

    # -------------------------------------------------------
    # Neutron energy spectrum (step/column histogram style)
    # -------------------------------------------------------
    ax.hist(energies_air_MeV, bins=energy_bins, histtype="step",
            linewidth=2, color="navy",
            label="Detector Interface \u2014 Air")

    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlim(1e-9, 1e1)

    ax.set_xlabel("Neutron Energy (MeV)")
    ax.set_ylabel("Neutron Flux / Counts (arbitrary units)")
    ax.set_title("Neutron Energy Spectrum at Detector Interface (Air)")

    ax.grid(True, which="major", linestyle="--", linewidth=0.6, alpha=0.4)
    ax.grid(True, which="minor", linestyle="--", linewidth=0.4, alpha=0.2)
    ax.minorticks_on()

    ax.legend(loc="upper right", fontsize=10, frameon=True,
              facecolor="white", framealpha=0.9)

    for spine in ax.spines.values():
        spine.set_linewidth(1.0)

    plt.tight_layout()

    output_name = "Neutron_Spectrum_Air.png"
    plt.savefig(output_name, dpi=300, facecolor="white", bbox_inches="tight")
    print(f"\nSaved figure: {output_name}")

    plt.show()

    if IN_COLAB:
        colab_files.download(output_name)


if __name__ == "__main__":
    main()
