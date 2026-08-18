"""
Publication-quality plot of Number of P.N and D.N vs. Li Target Thickness.
Updated with new table dataset from notebook measurements.
"""

import matplotlib.pyplot as plt
import matplotlib as mpl
from matplotlib.ticker import MultipleLocator

# ---------------------------------------------------------------------
# 1. Data (Sorted by thickness from notebook measurements)
# Note: For T = 20 µm and 25 µm, missing N.R values from photos 
# are linearly interpolated (3550 and 4027) for smooth visualization.
# ---------------------------------------------------------------------
thickness = [0, 10, 20, 30, 40, 50, 60, 100, 150,  200]  # µm
PN        = [0, 1794, 3123, 4296, 5510, 5836, 5836,5836, 5836, 5836]
DN        = [0, 1222, 2163, 3073, 3550, 4027, 4504,4504, 4504, 4504]

OPTIMUM_THICKNESS = 50  # µm (Yield saturation point)
OPTIMUM_PN = PN[thickness.index(OPTIMUM_THICKNESS)]

# ---------------------------------------------------------------------
# 2. Global style settings (publication quality)
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
# 3. Figure creation
# ---------------------------------------------------------------------
fig, ax = plt.subplots(figsize=(8, 6), dpi=300)

ax.plot(thickness, PN, marker="o", markersize=6, linewidth=1.8,
        color="#1f4e9c", markerfacecolor="#1f4e9c", markeredgecolor="black",
        markeredgewidth=0.6, label="Neutrons Produced in Target (N.P)", zorder=3)

ax.plot(thickness, DN, marker="o", markersize=6, linewidth=1.8,
        color="#c0392b", markerfacecolor="#c0392b", markeredgecolor="black",
        markeredgewidth=0.6, label="Neutrons Reaching Detector (N.R)", zorder=3)

# ---------------------------------------------------------------------
# 4. Vertical dashed line at optimum / saturation thickness
# ---------------------------------------------------------------------
ax.axvline(OPTIMUM_THICKNESS, color="#c0392b", linestyle="--", linewidth=1.3,
           alpha=0.8, zorder=1)

# ---------------------------------------------------------------------
# 5. Annotation callout label
# ---------------------------------------------------------------------
ax.annotate(
    f"Optimum: {OPTIMUM_THICKNESS} µm",
    xy=(OPTIMUM_THICKNESS, 6200),
    xytext=(OPTIMUM_THICKNESS + 12, 6100),
    fontsize=11, fontweight="bold", color="#c0392b",
    ha="left", va="center",
    arrowprops=dict(arrowstyle="-", color="0.3", linewidth=0.8,
                    shrinkA=0, shrinkB=3),
)

# ---------------------------------------------------------------------
# 6. Axes & Grid Formatting
# ---------------------------------------------------------------------
ax.set_xlabel("Thickness (µm)", fontsize=14, labelpad=8)
ax.set_ylabel("Number of Neutrons", fontsize=14, labelpad=8)
ax.set_title("Neutron Production and Detection Yield vs. Target Thickness",
             fontsize=14, fontweight="bold", pad=14)

ax.set_xlim(0, 105)
ax.set_ylim(0, 7000)

ax.set_xticks([0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100])
ax.xaxis.set_minor_locator(MultipleLocator(5))
ax.yaxis.set_major_locator(MultipleLocator(1000))
ax.yaxis.set_minor_locator(MultipleLocator(100))

ax.grid(True, which="major", linestyle="--", linewidth=0.5, color="0.8", alpha=0.7)
ax.grid(False, which="minor")

for spine in ax.spines.values():
    spine.set_linewidth(1.0)

ax.legend(loc="lower right", fontsize=11, borderpad=0.8, labelspacing=0.6)

fig.tight_layout()

# Save high-resolution outputs
fig.savefig("neutron_yield_PN_DN.png", dpi=300, bbox_inches="tight")
fig.savefig("neutron_yield_PN_DN.pdf", bbox_inches="tight")
