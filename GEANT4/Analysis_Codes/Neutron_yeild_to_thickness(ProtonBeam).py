"""
Publication-quality plot of Number of P.N and D.N vs. Li Target Thickness.
Suitable for direct inclusion in a Master's thesis.
"""

import matplotlib.pyplot as plt
import matplotlib as mpl

# ---------------------------------------------------------------------
# 1. Data (exactly as provided — no modification, smoothing, or fitting)
# ---------------------------------------------------------------------
thickness = [0, 10, 20, 40, 50, 60, 70, 80, 90, 100, 120, 130, 140, 150, 160]   # mm
PN        = [0, 1481, 2707 , 4484, 5101, 5525, 5953, 6011, 6082, 6136, 6170, 6175, 6177, 6179, 6179]
DN        = [0, 1048, 1922, 3243, 3727, 4037, 4321, 4444, 4502, 4545, 4577, 4581, 4584, 4582, 4586]

# ---------------------------------------------------------------------
# 2. Global style settings (serif font, inward ticks, thin borders)
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
# 3. Figure
# ---------------------------------------------------------------------
fig, ax = plt.subplots(figsize=(7.5, 5.5), dpi=300)

ax.plot(thickness, PN, marker="o", markersize=6, linewidth=1.8,
        color="#1f4e9c", markerfacecolor="#1f4e9c", markeredgecolor="black",
        markeredgewidth=0.6, label="Neutrons Produced in Target")

ax.plot(thickness, DN, marker="o", markersize=6, linewidth=1.8,
        color="#c0392b", markerfacecolor="#c0392b", markeredgecolor="black",
        markeredgewidth=0.6, label="Neutrons Reaching Detector")

# ---------------------------------------------------------------------
# 4. Axes formatting (Updated to start both axes at 0)
# ---------------------------------------------------------------------
ax.set_xlabel("Thickness (mm)", fontsize=14, labelpad=8)
ax.set_ylabel("Number of Neutrons", fontsize=14, labelpad=8)
ax.set_title("Neutron Production and Detection Yield vs. Target Thickness",
             fontsize=14, fontweight="bold", pad=14)

# Set lower limits to 0 for both axes
ax.set_xlim(0, 165)
ax.set_ylim(0, 7000)

# Regular tick intervals starting at 0 mm
ax.set_xticks([0, 20, 40, 60, 80, 100, 120, 140, 160])
ax.minorticks_on()

ax.grid(True, which="major", linestyle="--", linewidth=0.5, color="0.8", alpha=0.7)
ax.grid(False, which="minor")

ax.legend(loc="lower right", fontsize=12, borderpad=0.8, labelspacing=0.6)

for spine in ax.spines.values():
    spine.set_linewidth(1.0)

fig.tight_layout()

# ---------------------------------------------------------------------
# 5. Save high-resolution output
# ---------------------------------------------------------------------
fig.savefig("/mnt/user-data/outputs/neutron_yield_PN_DN.png", dpi=300, bbox_inches="tight")
fig.savefig("/mnt/user-data/outputs/neutron_yield_PN_DN.pdf", bbox_inches="tight")

print("Saved: neutron_yield_PN_DN.png and neutron_yield_PN_DN.pdf")
