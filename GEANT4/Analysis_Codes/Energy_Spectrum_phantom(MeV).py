"""
Neutron Energy Spectrum at Phantom Interface
=============================================
Merges multi-threaded Geant4 ntuple output (output_nt_Detector_t0.csv ...
output_nt_Detector_t31.csv) and produces a publication-quality, IAEA-shaded
BNCT neutron energy spectrum plot.

Designed to run in Google Colab or a local Jupyter/Python environment.

Author: (BNCT spectral analysis pipeline)
"""

import glob
import re

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


def upload_files_colab():
    """
    Interactively upload the Geant4 thread-output CSV files when running
    inside Google Colab. No-op (returns immediately) outside Colab.
    """
    if not IN_COLAB:
        print("Not running in Google Colab — skipping upload prompt. "
              "Make sure your CSV files are already in the working directory.")
        return []
    print("Please upload your Geant4 output CSV files "
          "(output_nt_Detector_t0.csv ... t31.csv)...")
    uploaded = colab_files.upload()
    return list(uploaded.keys())


# ---------------------------------------------------------------
# 1. Locate and merge multi-threaded Geant4 CSV files
# ---------------------------------------------------------------
FILE_PATTERN = "output_nt_Detector_t*.csv"


def _thread_index(path):
    """
    Extract the thread index from a Geant4 output filename, tolerating
    browser/Colab duplicate-upload suffixes like ' (1)' before '.csv'
    (e.g. 'output_nt_Detector_t0 (1).csv'). Always returns an int so the
    sort key never mixes types.
    """
    match = re.search(r"_t(\d+)", path)
    return int(match.group(1)) if match else -1


def find_input_files(pattern=FILE_PATTERN):
    """
    Glob-match all per-thread Geant4 output files, sorted by thread index.
    Also drops duplicate re-uploads (files ending in ' (1).csv', ' (2).csv',
    etc.) that Colab creates when the same filename is uploaded more than
    once in a session, keeping only the original t<N>.csv for each thread.
    """
    all_matches = glob.glob(pattern)

    # Prefer the plain 'output_nt_Detector_t<N>.csv' over any '(n)'-suffixed
    # duplicate for the same thread index.
    best_by_thread = {}
    for p in all_matches:
        idx = _thread_index(p)
        is_duplicate = bool(re.search(r"\(\d+\)\.csv$", p))
        if idx not in best_by_thread:
            best_by_thread[idx] = p
        else:
            # Keep the non-duplicate-named file if one of the two is a "(n)" copy
            current_is_dup = bool(re.search(r"\(\d+\)\.csv$", best_by_thread[idx]))
            if current_is_dup and not is_duplicate:
                best_by_thread[idx] = p

    dropped = len(all_matches) - len(best_by_thread)
    if dropped > 0:
        print(f"Note: ignored {dropped} duplicate re-uploaded file(s) "
              f"(e.g. '... (1).csv').")

    file_list = [best_by_thread[idx] for idx in sorted(best_by_thread.keys())]

    if not file_list:
        raise FileNotFoundError(
            f"No files found matching pattern '{pattern}'. "
            "Make sure the Geant4 output_nt_Detector_t0.csv ... t31.csv "
            "files are in the working directory (or upload them first)."
        )
    print(f"Found {len(file_list)} thread output file(s).")
    return file_list


def detect_energy_column(columns):
    """
    Auto-detect the kinetic-energy column name from a list of column headers.
    Handles common Geant4 G4AnalysisManager / wcsv ntuple naming conventions
    (e.g. 'E_k', 'KineticEnergy', 'Ek', 'Energy_eV', 'Energy_MeV', or a bare
    'E' column).
    """
    candidates = [
        "E_k", "Ek", "KineticEnergy", "kineticEnergy",
        "Energy_MeV", "Energy_keV", "Energy_eV",
        "Energy", "energy", "E",
    ]
    for c in candidates:
        if c in columns:
            return c
    # Fallback: case-insensitive partial match against common keywords
    for col in columns:
        if re.search(r"kinet|energy", col, re.IGNORECASE):
            return col
    # Last resort: a column literally named 'e' or 'E' with any case
    for col in columns:
        if col.strip().lower() == "e":
            return col
    raise ValueError(
        f"Could not auto-detect an energy column. Available columns: {list(columns)}"
    )


def detect_energy_unit_factor(column_name):
    """
    Infer the energy unit encoded in the column name (Geant4 wcsv ntuples
    commonly suffix the unit directly onto the name, e.g. 'Energy_eV',
    'Energy_keV', 'Energy_MeV') and return the multiplicative factor needed
    to convert that column's raw values into MeV.
    """
    name_lower = column_name.lower()
    if "mev" in name_lower:
        return 1.0, "MeV"
    if "kev" in name_lower:
        return 1e-3, "keV"
    if "ev" in name_lower:  # matches '..._eV' but not 'MeV'/'keV' (checked above)
        return 1e-6, "eV"
    if "gev" in name_lower:
        return 1e3, "GeV"
    # No unit suffix found in the name — assume Geant4 default internal
    # unit for kinetic energy ntuples, which is MeV, unless proven otherwise.
    return 1.0, "MeV (assumed \u2014 no unit suffix found in column name)"


def _extract_header(lines):
    """
    Locate the true column-name header inside a Geant4 ntuple CSV.

    Two Geant4 CSV export styles are supported:

    1. tools::wcsv::ntuple style (one field per line):
           #class tools::wcsv::ntuple
           #title BSA_Output_Neutrons
           #separator 44
           #column int fEvent
           #column double Energy_eV
           #column double CosTheta
           ...
           0,0.0321909,0.8,...
       Here each '#column <type> <name>' line defines one field, in order.

    2. Single comma-list style:
           #class tools::histo::h1d
           #csv nt Detector
           #, E_k, Weight, ...
           1.234e-05, 1.0, ...
       Here the header is one '#'-prefixed comma-separated list of names.

    Returns (column_names_list_or_None, data_start_idx).
    """
    column_names = []
    comma_header = None
    data_start_idx = 0

    for i, line in enumerate(lines):
        stripped = line.strip()
        if not stripped:
            continue
        if stripped.startswith("#"):
            # Style 1: '#column <type> <name>'
            col_match = re.match(r"#column\s+\S+\s+(\S+)", stripped)
            if col_match:
                column_names.append(col_match.group(1))
                continue
            # Style 2: '#, name1, name2, ...' (only consider if style 1 not in use)
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


def _is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False


def load_and_merge(file_list):
    """
    Load every per-thread CSV, skip all '#'-prefixed Geant4 metadata/header
    lines, auto-detect the kinetic-energy column, and concatenate all
    entries (in MeV) into a single 1-D numpy array.
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
            # No usable '#' header found — try reading normally, still
            # skipping any leading '#' comment lines.
            df = pd.read_csv(fpath, comment="#")

        if df.empty:
            print(f"  Warning: {fpath} contained no data rows, skipping.")
            continue

        if energy_col_name is None:
            energy_col_name = detect_energy_column(df.columns)
            unit_factor, unit_label = detect_energy_unit_factor(energy_col_name)
            print(f"Detected energy column: '{energy_col_name}' "
                  f"(unit: {unit_label} -> converting to MeV, factor={unit_factor:g})")
        elif energy_col_name not in df.columns:
            # Column naming differs slightly between threads — re-detect.
            energy_col_name = detect_energy_column(df.columns)
            unit_factor, unit_label = detect_energy_unit_factor(energy_col_name)

        raw_energies = pd.to_numeric(df[energy_col_name], errors="coerce").dropna().values
        energies_MeV = raw_energies * unit_factor
        all_energies.append(energies_MeV)
        print(f"  {fpath}: {len(energies_MeV)} entries")

    if not all_energies:
        raise ValueError(
            "No valid neutron energy data could be extracted from the input files."
        )

    merged = np.concatenate(all_energies)
    print(f"\nTotal merged neutron entries: {merged.size}")
    return merged


# ---------------------------------------------------------------
# 2. Main pipeline
# ---------------------------------------------------------------
def main():
    # Upload (Colab only; no-op locally)
    upload_files_colab()

    # Locate & merge Geant4 thread outputs
    input_files = find_input_files()
    energies_MeV = load_and_merge(input_files)

    # Log-binning requires strictly positive energies
    energies_MeV = energies_MeV[energies_MeV > 0]

    # -------------------------------------------------------
    # Logarithmic energy bins: 1e-9 MeV (1 meV) to 1e1 MeV (10 MeV)
    # -------------------------------------------------------
    energy_bins = np.logspace(-9, 1, 120)

    # -------------------------------------------------------
    # Publication-quality figure
    # -------------------------------------------------------
    plt.rcParams["font.size"] = 12

    fig, ax = plt.subplots(figsize=(10, 6), dpi=300)

    # --- IAEA BNCT energy-region shading (drawn behind the spectrum) ---
    ax.axvspan(1e-9, 0.5e-6, color="red", alpha=0.15,
               label="Thermal (<0.5 eV)")
    ax.axvspan(0.5e-6, 10e-3, color="green", alpha=0.15,
               label="Epithermal (0.5 eV\u201310 keV)")
    ax.axvspan(10e-3, 1e1, color="orange", alpha=0.15,
               label="Fast (>10 keV)")

    # --- Neutron energy spectrum (step histogram) ---
    ax.hist(
        energies_MeV,
        bins=energy_bins,
        histtype="step",
        linewidth=2,
        color="navy",
        label="Neutron Spectrum",
    )

    # --- Axis scales ---
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlim(1e-9, 1e1)

    # --- Labels & title ---
    ax.set_xlabel("Neutron Energy (MeV)", fontsize=13)
    ax.set_ylabel("Neutron Flux / Counts (arbitrary units)", fontsize=13)
    ax.set_title(
        "Neutron Energy Spectrum at Phantom Interface (IAEA BNCT Evaluation)",
        fontsize=14, fontweight="bold",
    )

    # --- Grid (major + minor, dashed) ---
    ax.grid(True, which="major", linestyle="--", alpha=0.4)
    ax.grid(True, which="minor", linestyle="--", alpha=0.2)
    ax.minorticks_on()

    # --- Legend ---
    ax.legend(loc="upper right", fontsize=10, frameon=True,
              facecolor="white", framealpha=0.9)

    plt.tight_layout()

    # -------------------------------------------------------
    # Save output
    # -------------------------------------------------------
    output_name = "Neutron_Energy_Spectrum_Phantom.png"
    plt.savefig(output_name, dpi=300, facecolor="white")
    print(f"\nSaved figure: {output_name}")

    plt.show()

    # Auto-download in Colab
    if IN_COLAB:
        colab_files.download(output_name)


if __name__ == "__main__":
    main()
