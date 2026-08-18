"""
Publication-quality plot comparing Neutron Production Yield (P.N) 
between Pencil Proton Beam (2.2 MeV) and Broad Proton Beam (2.2 ± 0.2 MeV) vs. Li Target Thickness.
"""

import matplotlib.pyplot as plt
import matplotlib as mpl
from matplotlib.ticker import MultipleLocator

# ---------------------------------------------------------------------
# 1. Datasets
# ---------------------------------------------------------------------
thickness_pencil = [0, 10, 20, 30, 40, 50, 60, 100, 200]  # µm
PN_pencil        = [0, 1794, 3123, 4296, 5510, 5836, 5836, 5836, 5836]

thickness_beam   = [0, 10, 20, 40, 50, 60, 70, 80, 90, 100, 120, 130, 140, 150, 200]  # µm
PN_beam          = [0, 1481, 2707, 4484, 5101, 5525, 5953, 6011, 6082, 6136, 6170, 6175, 6177, 6179, 6179]

OPT_PENCIL_T = 50  # µm
OPT_BEAM_T   = 140 # µm

# ---------------------------------------------------------------------
# 2. Style Settings
# ---------------------------------------------------------------------
mpl.rcParams.update({
    "font.family": "serif",
    "font.serif": ["Liberation Serif", "Times New Roman", "DejaVu Serif"],
    "font.size": 13,
    "axes.linewidth": 1.0,
    "xtick.direction": "in",
    "ytick.direction": "in",
    "xtick.major.size": 6,
    "ytick.major.size": 6,
    "xtick.minor.size": 3,
    "ytick.minor.size": 3,
    "xtick.major.width": 1.0,
    "ytick.major.width": 1.0,
    "xtick.top": True,
    "ytick.right": True,
    "axes.edgecolor": "black",
    "legend.frameon": True,
    "legend.edgecolor": "black",
    "legend.fancybox": False,
    "figure.facecolor": "white",
    "savefig.facecolor": "white",
})

# ---------------------------------------------------------------------
# 3. Figure & Plotting with Single Legend Box Including Energies
# ---------------------------------------------------------------------
fig, ax = plt.subplots(figsize=(8.5, 6), dpi=300)

ax.plot(thickness_pencil, PN_pencil, marker="o", markersize=6, linewidth=1.8,
        color="#1f4e9c", markerfacecolor="#1f4e9c", markeredgecolor="black",
        markeredgewidth=0.6, label="Pencil Proton Beam (2.2 MeV)", zorder=3)

ax.plot(thickness_beam, PN_beam, marker="s", markersize=6, linewidth=1.8,
        color="#27ae60", markerfacecolor="#27ae60", markeredgecolor="black",
        markeredgewidth=0.6, label="Broad Proton Beam (2.2 ± 0.2 MeV)", zorder=3)

# ---------------------------------------------------------------------
# 4. Saturation Lines
# ---------------------------------------------------------------------
ax.axvline(OPT_PENCIL_T, color="#1f4e9c", linestyle="--", linewidth=1.2, alpha=0.7, zorder=1)
ax.annotate(
    f"Pencil Saturation: {OPT_PENCIL_T} µm",
    xy=(OPT_PENCIL_T, 5836),
    xytext=(OPT_PENCIL_T + 8, 4800),
    fontsize=10, fontweight="bold", color="#1f4e9c",
    ha="left", va="center",
    arrowprops=dict(arrowstyle="->", color="#1f4e9c", linewidth=0.8, shrinkA=0, shrinkB=4)
)

ax.axvline(OPT_BEAM_T, color="#27ae60", linestyle="--", linewidth=1.2, alpha=0.7, zorder=1)
ax.annotate(
    f"Broad Saturation: {OPT_BEAM_T} µm",
    xy=(OPT_BEAM_T, 6177),
    xytext=(OPT_BEAM_T - 55, 6600),
    fontsize=10, fontweight="bold", color="#27ae60",
    ha="left", va="center",
    arrowprops=dict(arrowstyle="->", color="#27ae60", linewidth=0.8, shrinkA=0, shrinkB=4)
)

# ---------------------------------------------------------------------
# 5. Axes & Single Legend Box
# ---------------------------------------------------------------------
ax.set_xlabel("Li Target Thickness (µm)", fontsize=14, labelpad=8)
ax.set_ylabel("Neutrons Produced (P.N)", fontsize=14, labelpad=8)
ax.set_title("Comparison of Neutron Production Yield: Pencil vs. Broad Proton Beam",
             fontsize=13, fontweight="bold", pad=14)

ax.set_xlim(0, 170)
ax.set_ylim(0, 7200)

ax.set_xticks([0, 20, 40, 60, 80, 100, 120, 140, 160])
ax.xaxis.set_minor_locator(MultipleLocator(10))
ax.yaxis.set_major_locator(MultipleLocator(1000))
ax.yaxis.set_minor_locator(MultipleLocator(200))

ax.grid(True, which="major", linestyle="--", linewidth=0.5, color="0.8", alpha=0.7)
ax.grid(False, which="minor")

for spine in ax.spines.values():
    spine.set_linewidth(1.0)

# Single combined legend box
ax.legend(loc="lower right", fontsize=10.5, borderpad=0.8, labelspacing=0.6)

fig.tight_layout()

fig.savefig("neutron_yield_pencil_vs_beam.png", dpi=300, bbox_inches="tight")
fig.savefig("neutron_yield_pencil_vs_beam.pdf", bbox_inches="tight")
