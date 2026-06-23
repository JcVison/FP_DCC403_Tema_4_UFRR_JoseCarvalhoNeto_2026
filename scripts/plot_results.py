#!/usr/bin/env python3
import csv
import os
import matplotlib.pyplot as plt

CSV_PATH = os.path.join('docs', 'resultados.csv')
OUT_PATH = os.path.join('docs', 'resultados.png')

data = {}

with open(CSV_PATH, newline='') as f:
    reader = csv.DictReader(f)
    for row in reader:
        policy = row['policy']
        page = int(row['page_kb'])
        phys = int(row['physical_kb'])
        rate = float(row['fault_rate'])
        data.setdefault(page, {}).setdefault(policy, []).append((phys, rate))

pages = sorted(data.keys())
policies = ['lru', 'fifo', 'random']

fig, axes = plt.subplots(1, len(pages), figsize=(5 * len(pages), 4), sharey=True)
if len(pages) == 1:
    axes = [axes]

for ax, page in zip(axes, pages):
    for policy in policies:
        pts = sorted(data.get(page, {}).get(policy, []))
        if not pts:
            continue
        xs = [p for p, _ in pts]
        ys = [r for _, r in pts]
        ax.plot(xs, ys, marker='o', label=policy.upper())
    ax.set_xscale('log', base=2)
    ax.set_xticks([128,256,512,1024,2048])
    ax.get_xaxis().set_major_formatter(plt.ScalarFormatter())
    ax.set_title(f'Page = {page} KB')
    ax.set_xlabel('Physical (KB)')
    ax.grid(True, linestyle='--', alpha=0.4)

axes[0].set_ylabel('Fault Rate (%)')
axes[-1].legend(loc='upper right')
plt.tight_layout()
plt.savefig(OUT_PATH, dpi=150)
print(f'Gráfico salvo em {OUT_PATH}')
