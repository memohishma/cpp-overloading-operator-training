"""
Publication-quality plot of Number of P.N and D.N vs. Li Target Thickness (um).
Features custom markers (circles for production, squares for detection) and 
an annotation box indicating the optimum thickness.
"""

import matplotlib.pyplot as plt
import matplotlib as mpl

# ---------------------------------------------------------------------
# 1. Data (Thickness in micrometers)
# ---------------------------------------------------------------------
Thickness = [0, 10, 20, 40, 60, 70, 80, 90, 100, 120, 140, 160]  # um
PN = [0, 1481, 2707, 4484, 5101, 5525, 5953, 6011, 6082, 6136, 6170, 6175]
DN = [0, 1048, 1922, 3243, 3727, 4037, 4321, 4444, 4502, 4545, 4577, 4581]

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

# Line 1: Produced Neutrons (Circle markers)
ax.plot(Thickness, PN, marker="o", markersize=6, linewidth=1.8,
        color="#1f4e9c", markerfacecolor="#1f4e9c", markeredgecolor="black",
        markeredgewidth=0.6, label="Neutrons Produced in Target", zorder=2)

# Line 2: Detected Neutrons (Square markers)
ax.plot(Thickness, DN, marker="s", markersize=6, linewidth=1.8,
        color="#c0392b", markerfacecolor="#c0392b", markeredgecolor="black",
        markeredgewidth=0.6, label="Neutrons Reaching Detector", zorder=2)



# ---------------------------------------------------------------------
# 5. Axes formatting
# ---------------------------------------------------------------------
ax.set_xlabel(r"Target Thickness ($\mu\mathrm{m}$)", fontsize=14, labelpad=8)
ax.set_ylabel("Number of Neutrons", fontsize=14, labelpad=8)
ax.set_title(r"Neutron Yield vs. Li Target Thickness for Divergent Proton Beam ($E_p = 2.2 \pm 0.2\ \mathrm{MeV}$)",
             fontsize=13, fontweight="bold", pad=14)

# Set lower limits to 0 for both axes
ax.set_xlim(0, 165)
ax.set_ylim(0, 7200)

# Ticks
ax.set_xticks([0, 20, 40, 60, 80, 100, 120, 140, 160])
ax.minorticks_on()

# Grid
ax.grid(True, which="major", linestyle="--", linewidth=0.5, color="0.8", alpha=0.7, zorder=1)
ax.grid(False, which="minor")

# Legend
ax.legend(loc="lower right", fontsize=10.5, borderpad=0.8, labelspacing=0.6)

for spine in ax.spines.values():
    spine.set_linewidth(1.0)

fig.tight_layout()
plt.show()

########################################################################################################################################
#with optimum thickness

"""
Publication-quality plot of Number of P.N and D.N vs. Li Target Thickness (um).
Features an annotation box and arrow indicating the selected optimum thickness.
Y-axis max limit set to 7000.
"""

import matplotlib.pyplot as plt
import matplotlib as mpl

# ---------------------------------------------------------------------
# 1. Data (Thickness in micrometers)
# ---------------------------------------------------------------------
Thickness = [0, 10, 20, 40, 60, 70, 80, 90, 100, 120, 140, 160]  # um
PN = [0, 1481, 2707, 4484, 5101, 5525, 5953, 6011, 6082, 6136, 6170, 6175]
DN = [0, 1048, 1922, 3243, 3727, 4037, 4321, 4444, 4502, 4545, 4577, 4581]

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

# Line 1: PN
ax.plot(Thickness, PN, marker="o", markersize=6, linewidth=1.8,
        color="#1f4e9c", markerfacecolor="#1f4e9c", markeredgecolor="black",
        markeredgewidth=0.6, label="Neutrons Produced in Target", zorder=2)

# Line 2: DN
ax.plot(Thickness, DN, marker="s", markersize=6, linewidth=1.8,
        color="#c0392b", markerfacecolor="#c0392b", markeredgecolor="black",
        markeredgewidth=0.6, label="Neutrons Reaching Detector", zorder=2)

# ---------------------------------------------------------------------
# 4. Annotation Box with Arrow pointing to 80 um
# ---------------------------------------------------------------------
ax.annotate("Optimum Thickness\n(80 \u03bcm)",
            xy=(80, 5953), xytext=(42, 6300),
            arrowprops=dict(facecolor="black", edgecolor="black", arrowstyle="->", lw=1.2, shrinkB=6),
            bbox=dict(boxstyle="round,pad=0.5", facecolor="#fff9c4", edgecolor="black", lw=0.8),
            fontsize=10.5, ha="center", va="center", zorder=5)

# ---------------------------------------------------------------------
# 5. Axes formatting
# ---------------------------------------------------------------------
ax.set_xlabel(r"Target Thickness ($\mu\mathrm{m}$)", fontsize=14, labelpad=8)
ax.set_ylabel("Number of Neutrons", fontsize=14, labelpad=8)
ax.set_title("Neutron Yield vs. Li Target Thickness for Divergent Proton Beam ($E_p = 2.2 \pm 0.2\ \mathrm{MeV}$)", fontsize=14, fontweight="bold", pad=14)

ax.set_xlim(0, 165)
ax.set_ylim(0, 7000)  # حد المحور الأعلى 7000

ax.set_xticks([0, 20, 40, 60, 80, 100, 120, 140, 160])
ax.minorticks_on()

ax.grid(True, which="major", linestyle="--", linewidth=0.5, color="0.8", alpha=0.7, zorder=1)
ax.grid(False, which="minor")

ax.legend(loc="lower right", fontsize=10.5, borderpad=0.8, labelspacing=0.6)

for spine in ax.spines.values():
    spine.set_linewidth(1.0)

fig.tight_layout()
plt.show()

##################################################################################################################################
#with range optimum

"""
Publication-quality plot of Number of P.N and D.N vs. Li Target Thickness (um).
Suitable for direct inclusion in a Master's thesis.
"""

import matplotlib.pyplot as plt
import matplotlib as mpl

# ---------------------------------------------------------------------
# 1. Data (Thickness converted to micrometers)
# ---------------------------------------------------------------------
Thickness = [0, 10, 20, 40, 60, 70, 80, 90, 100, 120, 140, 160] # um
PN = [0, 1481, 2707, 4484, 5101, 5525, 5953, 6011, 6082, 6136, 6170, 6175]
DN = [0, 1048, 1922, 3243, 3727, 4037, 4321, 4444, 4502, 4545, 4577, 4581]

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

# Shaded region for optimal thickness range (80-100 um) - Uniform font size text
ax.axvspan(80, 100, color="#fff9c4", alpha=0.8, zorder=0, label="Optimal Thickness Range (80-100 \u03bcm)")

# Line 1: PN (Square markers)
ax.plot(Thickness, PN, marker="s", markersize=6, linewidth=1.8,
        color="#1f4e9c", markerfacecolor="#1f4e9c", markeredgecolor="black",
        markeredgewidth=0.6, label="Neutrons Produced in Target", zorder=2)

# Line 2: DN (Square markers)
ax.plot(Thickness, DN, marker="s", markersize=6, linewidth=1.8,
        color="#c0392b", markerfacecolor="#c0392b", markeredgecolor="black",
        markeredgewidth=0.6, label="Neutrons Reaching Detector", zorder=2)

# ---------------------------------------------------------------------
# 4. Axes formatting
# ---------------------------------------------------------------------
ax.set_xlabel(r"Target Thickness ($\mu\mathrm{m}$)", fontsize=14, labelpad=8)
ax.set_ylabel("Number of Neutrons", fontsize=14, labelpad=8)
ax.set_title("Neutron Yield vs. Li Target Thickness for Divergent Proton Beam ($E_p = 2.2 \pm 0.2\ \mathrm{MeV}$)", fontsize=14, fontweight="bold", pad=14)

ax.set_xlim(0, 165)
ax.set_ylim(0, 7000)

ax.set_xticks([0, 20, 40, 60, 80, 100, 120, 140, 160])
ax.minorticks_on()

ax.grid(True, which="major", linestyle="--", linewidth=0.5, color="0.8", alpha=0.7, zorder=1)
ax.grid(False, which="minor")

ax.legend(loc="lower right", fontsize=10.5, borderpad=0.8, labelspacing=0.6)

for spine in ax.spines.values():
    spine.set_linewidth(1.0)

fig.tight_layout()
plt.show()
