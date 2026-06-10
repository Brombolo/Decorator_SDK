#!/usr/bin/env python3
"""
Haiku Decorator SDK — validate_theme.py
Valida theme.conf prima della compilazione.
Segnala errori chiari con indicazione della riga.
"""

import sys
import re
import os

# ─── Regole di validazione ────────────────────────────────────────────────────

RULES = {
    "tab.height":               ("int",   14,   48),
    "tab.font_size":            ("int",   8,    18),
    "tab.corner_radius":        ("int",   0,    12),
    "tab.slant_angle":          ("float", 5.0,  45.0),
    "tab.shape":                ("enum",  ["flat", "rounded", "slanted"]),
    "tab.effect.type":          ("enum",  ["none", "gradient", "glass", "texture", "tint_from_system"]),
    "tab.effect.gradient_dir":  ("enum",  ["vertical", "horizontal", "diagonal"]),
    "tab.effect.glass_opacity": ("float", 0.0, 1.0),
    "tab.effect.tint_amount":   ("float", 0.0, 1.0),
    "tab.texture.blend_mode":   ("enum",  ["normal", "multiply", "overlay", "screen"]),
    "tab.texture.opacity":      ("float", 0.0, 1.0),

    "border.width":             ("int",   1,    12),
    "border.style":             ("enum",  ["flat", "beveled", "inset", "shadow"]),
    "border.corners.radius":    ("int",   0,    8),

    "buttons.size":             ("int",   8,    20),
    "buttons.margin":           ("int",   1,    20),
    "buttons.spacing":          ("int",   0,    20),
    "buttons.corner_radius":    ("int",   0,    8),
    "buttons.position":         ("enum",  ["left", "right"]),
    "buttons.shape":            ("enum",  ["circle", "square", "rounded_square", "diamond"]),
    "buttons.icon.style":       ("enum",  ["symbol", "dot", "cross_x", "classic_be", "none"]),
    "buttons.icon.scale":       ("float", 0.4,  0.9),
    "buttons.hover_effect.type":("enum",  ["lighten", "darken", "glow", "scale", "none"]),
    "buttons.hover_effect.scale_factor": ("float", 0.5, 2.0),

    "resize_corner.style":      ("enum",  ["knob", "lines", "none"]),
    "resize_corner.size":       ("int",   8,    40),

    "title.alignment":          ("enum",  ["left", "center", "right"]),
    "title.truncation":         ("enum",  ["end", "middle"]),

    "floating.tab_height":      ("int",   8,    32),
    "floating.border_width":    ("int",   1,    8),

    "rendering.min_tab_height": ("int",   8,    32),
}

COLOR_KEYS = [k for k in [
    "tab.active.color_start", "tab.active.color_end",
    "tab.active.text_color",  "tab.active.border_color",
    "tab.inactive.color_start", "tab.inactive.color_end",
    "tab.inactive.text_color",  "tab.inactive.border_color",
    "tab.stack_tile.color_start", "tab.stack_tile.color_end",
    "tab.stack_tile.text_color",  "tab.stack_tile.indicator_color",
    "tab.effect.inner_glow_color", "tab.effect.separator_color",
    "border.active.color_base",   "border.active.color_highlight",
    "border.active.color_shadow",
    "border.inactive.color_base", "border.inactive.color_highlight",
    "border.inactive.color_shadow",
    "buttons.close.color_normal",  "buttons.close.color_hover",
    "buttons.close.color_pressed", "buttons.close.icon_color",
    "buttons.zoom.color_normal",   "buttons.zoom.color_hover",
    "buttons.zoom.color_pressed",  "buttons.zoom.icon_color",
    "resize_corner.color",
    "title.shadow_color",
    "floating.tab_color",
]]

COLOR_RE = re.compile(r'^#[0-9a-fA-F]{6}([0-9a-fA-F]{2})?$')

# ─── Parser minimale TOML ─────────────────────────────────────────────────────

def parse_conf(path):
    """Ritorna dict { "section.key": (value, line_number) }"""
    result = {}
    section = ""
    with open(path, "r", encoding="utf-8") as f:
        for lineno, raw in enumerate(f, 1):
            line = raw.strip()
            # Rimuovi commento inline
            in_str = False
            clean = []
            for ch in line:
                if ch == '"': in_str = not in_str
                if not in_str and ch == '#': break
                clean.append(ch)
            line = ''.join(clean).strip()

            if not line:
                continue
            if line.startswith('[') and line.endswith(']'):
                section = line[1:-1].strip()
                continue
            if '=' not in line:
                continue
            key, _, val = line.partition('=')
            key = key.strip()
            val = val.strip().strip('"')
            full_key = f"{section}.{key}" if section else key
            result[full_key] = (val, lineno)
    return result

# ─── Validazione ─────────────────────────────────────────────────────────────

def validate(path):
    errors   = []
    warnings = []

    if not os.path.exists(path):
        print(f"ERRORE: file non trovato: {path}")
        sys.exit(1)

    data = parse_conf(path)

    for full_key, (val, lineno) in data.items():
        # Colori
        if full_key in COLOR_KEYS:
            if val == "":
                continue  # colore auto, ok
            if not COLOR_RE.match(val):
                errors.append(
                    f"  Riga {lineno}: '{full_key}' = '{val}' "
                    f"→ formato non valido. Usa #RRGGBB o #RRGGBBAA")
            continue

        # Regole tipizzate
        if full_key not in RULES:
            continue
        rule = RULES[full_key]

        if rule[0] == "enum":
            allowed = rule[1]
            if val not in allowed:
                errors.append(
                    f"  Riga {lineno}: '{full_key}' = '{val}' "
                    f"→ valore non valido. Ammessi: {', '.join(allowed)}")

        elif rule[0] == "int":
            lo, hi = rule[1], rule[2]
            try:
                n = int(val)
                if not (lo <= n <= hi):
                    errors.append(
                        f"  Riga {lineno}: '{full_key}' = {n} "
                        f"→ fuori range [{lo}, {hi}]")
            except ValueError:
                errors.append(
                    f"  Riga {lineno}: '{full_key}' = '{val}' "
                    f"→ deve essere un numero intero")

        elif rule[0] == "float":
            lo, hi = rule[1], rule[2]
            try:
                n = float(val)
                if not (lo <= n <= hi):
                    errors.append(
                        f"  Riga {lineno}: '{full_key}' = {n} "
                        f"→ fuori range [{lo:.1f}, {hi:.1f}]")
            except ValueError:
                errors.append(
                    f"  Riga {lineno}: '{full_key}' = '{val}' "
                    f"→ deve essere un numero decimale (es. 0.5)")

    # Avvisi logici
    if "tab.shape" in data and data["tab.shape"][0] == "rounded":
        if "tab.corner_radius" in data and int(data["tab.corner_radius"][0]) == 0:
            warnings.append(
                "  Avviso: tab.shape = 'rounded' ma tab.corner_radius = 0 "
                "(nessun arrotondamento visibile)")

    if "tab.effect.type" in data and data["tab.effect.type"][0] == "texture":
        if "tab.texture.file" not in data or data["tab.texture.file"][0] == "":
            errors.append(
                "  Errore: tab.effect.type = 'texture' richiede tab.texture.file")

    return errors, warnings

# ─── Main ─────────────────────────────────────────────────────────────────────

if __name__ == "__main__":
    path = sys.argv[1] if len(sys.argv) > 1 else "theme.conf"
    errors, warnings = validate(path)

    for w in warnings:
        print(w)

    if errors:
        print(f"\n{len(errors)} errore/i nel file di configurazione:\n")
        for e in errors:
            print(e)
        sys.exit(1)

    sys.exit(0)
