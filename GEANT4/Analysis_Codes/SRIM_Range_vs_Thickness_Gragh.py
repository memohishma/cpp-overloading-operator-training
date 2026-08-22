import matplotlib.pyplot as plt

# 1. تحديد الأزرق الكحلي الداكن الخص بالبحوث والمجلات العالمية
navy_blue = '#002B49'

# 2. إعداد الخطوط لتدعم Times New Roman و LaTeX المصمم للتمثيل العلمي الأكاديمي
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['Times New Roman', 'DejaVu Serif']
plt.rcParams['mathtext.fontset'] = 'stix'  # نمط متناسق كلياً مع Times New Roman للمصطلحات والرموز

# 3. بيانات الرسمة المطابقة
energy = [1.8, 1.9, 2.0, 2.1, 2.2]        # Proton Energy (MeV)
range_um = [135.1, 148.3, 161.8, 175.7 , 190.2]      # Range in Lithium (μm)

# 4. إنشاء الشكل بدقة عالية (300 DPI) وبدون عنوان علوي
fig, ax = plt.subplots(figsize=(7, 5), dpi=300)

# 5. رسم المنحنى بالأزرق الداكن
ax.plot(
    energy, range_um, 
    color=navy_blue, 
    linestyle='-', 
    linewidth=2.0, 
    marker='o', 
    markersize=7.5, 
    markerfacecolor=navy_blue, 
    markeredgecolor=navy_blue,
    label='Protons in Lithium'
)

# 6. إضافة الصناديق النصية مع استخدام رمز الميكرون العلمية مفسرة صحيحة
for x, y in zip(energy, range_um):
    annotation_text = f"{x:.1f} MeV\n{y} μm"
    ax.annotate(
        annotation_text,
        xy=(x, y),
        xytext=(-20, 22),
        textcoords='offset points',
        fontsize=10,
        fontweight='bold',
        color=navy_blue,
        ha='center',
        va='bottom',
        bbox=dict(
            boxstyle='round,pad=0.4',
            facecolor='#FAFAFA',
            edgecolor='#555555',
            linewidth=0.8,
            alpha=0.95
        )
    )

# 7. ضبط المحاور والعلامات التوضيحية
ax.set_xlim(1.7, 2.35)
ax.set_ylim(120, 210)

ax.set_xlabel('Proton Energy (MeV)', fontsize=13, labelpad=8)
ax.set_ylabel('Range in Lithium (μm)', fontsize=13, labelpad=8)

# ضبط بروز علامات الترقيم للداخل (Inward Ticks)
ax.tick_params(axis='both', which='major', labelsize=11, direction='in', top=True, right=True, length=5)

# شبكة خلفية خفيفة
ax.grid(True, linestyle='--', linewidth=0.5, color='#DDDDDD', alpha=0.7)

# مفتاح الرسم
ax.legend(loc='upper left', frameon=True, framealpha=0.9, edgecolor='#CCCCCC', fontsize=11)

plt.tight_layout()
plt.show()
