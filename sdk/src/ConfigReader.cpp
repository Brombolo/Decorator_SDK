/*
 * Haiku Decorator SDK — ConfigReader.cpp
 * Parser TOML minimale per theme.conf
 */

#include "ConfigReader.h"

#include <File.h>
#include <String.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <InterfaceDefs.h>  // tint_color, B_LIGHTEN_2_TINT, etc.

BString ConfigReader::sLastError = "";

// ─── Utilità stringa ──────────────────────────────────────────────────────────

static void trim(char* s) {
    // trim leading
    char* p = s;
    while (*p == ' ' || *p == '\t') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    // trim trailing
    int len = strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t'
                    || s[len-1] == '\r' || s[len-1] == '\n'))
        s[--len] = '\0';
    // remove inline comment
    bool in_string = false;
    for (int i = 0; s[i]; i++) {
        if (s[i] == '"') in_string = !in_string;
        if (!in_string && s[i] == '#') { s[i] = '\0'; break; }
    }
    // re-trim trailing after comment removal
    len = strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t'))
        s[--len] = '\0';
}

static void strip_quotes(char* s) {
    int len = strlen(s);
    if (len >= 2 && s[0] == '"' && s[len-1] == '"') {
        memmove(s, s + 1, len - 2);
        s[len - 2] = '\0';
    }
}

// ─── Parsing tipi base ────────────────────────────────────────────────────────

bool ConfigReader::_ParseColor(const char* value, rgb_color& out) {
    if (!value || value[0] != '#') return false;
    const char* hex = value + 1;
    int len = strlen(hex);
    unsigned int r, g, b, a = 255;
    if (len == 6) {
        if (sscanf(hex, "%02x%02x%02x", &r, &g, &b) != 3) return false;
    } else if (len == 8) {
        if (sscanf(hex, "%02x%02x%02x%02x", &r, &g, &b, &a) != 4) return false;
    } else {
        return false;
    }
    out.red   = (uint8)r;
    out.green = (uint8)g;
    out.blue  = (uint8)b;
    out.alpha = (uint8)a;
    return true;
}

bool ConfigReader::_ParseBool(const char* value, bool& out) {
    if (strcasecmp(value, "true") == 0  || strcmp(value, "1") == 0) { out = true;  return true; }
    if (strcasecmp(value, "false") == 0 || strcmp(value, "0") == 0) { out = false; return true; }
    return false;
}

int32 ConfigReader::_ParseInt(const char* value, int32 defaultVal) {
    if (!value || !*value) return defaultVal;
    char* end;
    long v = strtol(value, &end, 10);
    if (end == value) return defaultVal;
    return (int32)v;
}

float ConfigReader::_ParseFloat(const char* value, float defaultVal) {
    if (!value || !*value) return defaultVal;
    char* end;
    float v = strtof(value, &end);
    if (end == value) return defaultVal;
    return v;
}

// ─── Parsing enumerazioni ─────────────────────────────────────────────────────

TabShape ConfigReader::_ParseTabShape(const char* v) {
    if (!v) return TAB_SHAPE_FLAT;
    if (strcmp(v, "rounded") == 0) return TAB_SHAPE_ROUNDED;
    if (strcmp(v, "slanted") == 0) return TAB_SHAPE_SLANTED;
    if (strcmp(v, "wave")    == 0) return TAB_SHAPE_WAVE;
    return TAB_SHAPE_FLAT;
}

EffectType ConfigReader::_ParseEffectType(const char* v) {
    if (!v) return EFFECT_NONE;
    if (strcmp(v, "gradient")         == 0) return EFFECT_GRADIENT;
    if (strcmp(v, "glass")            == 0) return EFFECT_GLASS;
    if (strcmp(v, "texture")          == 0) return EFFECT_TEXTURE;
    if (strcmp(v, "tint_from_system") == 0) return EFFECT_TINT_FROM_SYSTEM;
    return EFFECT_NONE;
}

GradientDir ConfigReader::_ParseGradientDir(const char* v) {
    if (!v) return GRAD_VERTICAL;
    if (strcmp(v, "horizontal") == 0) return GRAD_HORIZONTAL;
    if (strcmp(v, "diagonal")   == 0) return GRAD_DIAGONAL;
    return GRAD_VERTICAL;
}

BlendMode ConfigReader::_ParseBlendMode(const char* v) {
    if (!v) return BLEND_NORMAL;
    if (strcmp(v, "multiply") == 0) return BLEND_MULTIPLY;
    if (strcmp(v, "overlay")  == 0) return BLEND_OVERLAY;
    if (strcmp(v, "screen")   == 0) return BLEND_SCREEN;
    return BLEND_NORMAL;
}

BorderStyle ConfigReader::_ParseBorderStyle(const char* v) {
    if (!v) return BORDER_BEVELED;
    if (strcmp(v, "flat")    == 0) return BORDER_FLAT;
    if (strcmp(v, "inset")   == 0) return BORDER_INSET;
    if (strcmp(v, "shadow")  == 0) return BORDER_SHADOW;
    return BORDER_BEVELED;
}

ButtonShape ConfigReader::_ParseButtonShape(const char* v) {
    if (!v) return BTN_CIRCLE;
    if (strcmp(v, "square")         == 0) return BTN_SQUARE;
    if (strcmp(v, "rounded_square") == 0) return BTN_ROUNDED_SQUARE;
    if (strcmp(v, "diamond")        == 0) return BTN_DIAMOND;
    return BTN_CIRCLE;
}

ButtonIconStyle ConfigReader::_ParseButtonIconStyle(const char* v) {
    if (!v) return ICON_SYMBOL;
    if (strcmp(v, "dot")        == 0) return ICON_DOT;
    if (strcmp(v, "cross_x")    == 0) return ICON_CROSS_X;
    if (strcmp(v, "classic_be") == 0) return ICON_CLASSIC_BE;
    if (strcmp(v, "none")       == 0) return ICON_NONE;
    return ICON_SYMBOL;
}

ButtonHoverEffect ConfigReader::_ParseHoverEffect(const char* v) {
    if (!v) return HOVER_LIGHTEN;
    if (strcmp(v, "darken") == 0) return HOVER_DARKEN;
    if (strcmp(v, "glow")   == 0) return HOVER_GLOW;
    if (strcmp(v, "scale")  == 0) return HOVER_SCALE;
    if (strcmp(v, "none")   == 0) return HOVER_NONE;
    return HOVER_LIGHTEN;
}

ResizeCornerStyle ConfigReader::_ParseResizeStyle(const char* v) {
    if (!v) return RESIZE_KNOB;
    if (strcmp(v, "lines") == 0) return RESIZE_LINES;
    if (strcmp(v, "none")  == 0) return RESIZE_NONE;
    return RESIZE_KNOB;
}

TitleAlignment ConfigReader::_ParseTitleAlignment(const char* v) {
    if (!v) return TITLE_CENTER;
    if (strcmp(v, "left")  == 0) return TITLE_LEFT;
    if (strcmp(v, "right") == 0) return TITLE_RIGHT;
    return TITLE_CENTER;
}

TitleTruncation ConfigReader::_ParseTruncation(const char* v) {
    if (!v) return TRUNC_END;
    if (strcmp(v, "middle") == 0) return TRUNC_MIDDLE;
    return TRUNC_END;
}

ButtonPosition ConfigReader::_ParseButtonPosition(const char* v) {
    if (!v) return BTN_POS_LEFT;
    if (strcmp(v, "right") == 0) return BTN_POS_RIGHT;
    return BTN_POS_LEFT;
}

// ─── Colori automatici ────────────────────────────────────────────────────────

rgb_color ConfigReader::_AutoHighlight(rgb_color base) {
    return tint_color(base, B_LIGHTEN_2_TINT);
}

rgb_color ConfigReader::_AutoShadow(rgb_color base) {
    return tint_color(base, B_DARKEN_3_TINT);
}

// ─── Caricamento principale ───────────────────────────────────────────────────

status_t ConfigReader::Load(const char* path, ThemeConfig& cfg) {
    FILE* f = fopen(path, "r");
    if (!f) {
        sLastError = "Impossibile aprire il file: ";
        sLastError += path;
        return B_ENTRY_NOT_FOUND;
    }

    char line[1024];
    char section[256] = "";     // es. "tab.active"
    int  lineNum = 0;

    while (fgets(line, sizeof(line), f)) {
        lineNum++;
        trim(line);

        // Righe vuote e commenti
        if (line[0] == '\0' || line[0] == '#') continue;

        // ─── Sezione [xxx] ────────────────────────────────────────────────────
        if (line[0] == '[') {
            char* close = strchr(line, ']');
            if (!close) {
                sLastError.SetToFormat("Linea %d: sezione non chiusa", lineNum);
                fclose(f);
                return B_BAD_DATA;
            }
            *close = '\0';
            strncpy(section, line + 1, sizeof(section) - 1);
            trim(section);
            continue;
        }

        // ─── Coppia chiave = valore ───────────────────────────────────────────
        char* eq = strchr(line, '=');
        if (!eq) continue;

        char key[256] = {};
        char val[512] = {};
        strncpy(key, line, (size_t)(eq - line));
        strncpy(val, eq + 1, sizeof(val) - 1);
        trim(key);
        trim(val);
        strip_quotes(val);

        // Chiave composta: "section.key"
        char full[512];
        if (section[0])
            snprintf(full, sizeof(full), "%s.%s", section, key);
        else
            strncpy(full, key, sizeof(full));

// Macro locali per ridurre la verbosità del parsing
#define INT_KEY(field, k)  if (strcmp(full, k) == 0) { field = _ParseInt(val, field); continue; }
#define FLOAT_KEY(field, k) if (strcmp(full, k) == 0) { field = _ParseFloat(val, field); continue; }
#define BOOL_KEY(field, k) if (strcmp(full, k) == 0) { _ParseBool(val, field); continue; }
#define COLOR_KEY(field, flag_field, k) \
    if (strcmp(full, k) == 0) { \
        if (val[0] != '\0') { flag_field = false; _ParseColor(val, field); } \
        continue; \
    }
#define COLOR_SIMPLE(field, k) \
    if (strcmp(full, k) == 0) { _ParseColor(val, field); continue; }
#define STR_KEY(field, k)  if (strcmp(full, k) == 0) { field = val; continue; }

        // ── meta ──────────────────────────────────────────────────────────────
        STR_KEY(cfg.name,    "meta.name")
        STR_KEY(cfg.author,  "meta.author")
        STR_KEY(cfg.version, "meta.version")

        // ── tab ───────────────────────────────────────────────────────────────
        INT_KEY(cfg.tab.height,        "tab.height")
        INT_KEY(cfg.tab.font_size,     "tab.font_size")
        BOOL_KEY(cfg.tab.font_bold,    "tab.font_bold")
        INT_KEY(cfg.tab.corner_radius, "tab.corner_radius")
        FLOAT_KEY(cfg.tab.slant_angle, "tab.slant_angle")
        if (strcmp(full, "tab.shape") == 0) { cfg.tab.shape = _ParseTabShape(val); continue; }

        // tab.active
        COLOR_SIMPLE(cfg.tab.active.color_start,  "tab.active.color_start")
        COLOR_SIMPLE(cfg.tab.active.color_end,    "tab.active.color_end")
        COLOR_SIMPLE(cfg.tab.active.text_color,   "tab.active.text_color")
        COLOR_SIMPLE(cfg.tab.active.border_color, "tab.active.border_color")

        // tab.inactive
        COLOR_SIMPLE(cfg.tab.inactive.color_start,  "tab.inactive.color_start")
        COLOR_SIMPLE(cfg.tab.inactive.color_end,    "tab.inactive.color_end")
        COLOR_SIMPLE(cfg.tab.inactive.text_color,   "tab.inactive.text_color")
        COLOR_SIMPLE(cfg.tab.inactive.border_color, "tab.inactive.border_color")

        // tab.stack_tile
        COLOR_SIMPLE(cfg.tab.stack_tile.color_start,     "tab.stack_tile.color_start")
        COLOR_SIMPLE(cfg.tab.stack_tile.color_end,       "tab.stack_tile.color_end")
        COLOR_SIMPLE(cfg.tab.stack_tile.text_color,      "tab.stack_tile.text_color")
        COLOR_SIMPLE(cfg.tab.stack_tile.indicator_color, "tab.stack_tile.indicator_color")

        // tab.effect
        if (strcmp(full, "tab.effect.type")         == 0) { cfg.tab.effect.type         = _ParseEffectType(val);  continue; }
        if (strcmp(full, "tab.effect.gradient_dir") == 0) { cfg.tab.effect.gradient_dir = _ParseGradientDir(val); continue; }
        FLOAT_KEY(cfg.tab.effect.glass_opacity,   "tab.effect.glass_opacity")
        FLOAT_KEY(cfg.tab.effect.tint_amount,      "tab.effect.tint_amount")
        BOOL_KEY(cfg.tab.effect.inner_glow,        "tab.effect.inner_glow")
        COLOR_SIMPLE(cfg.tab.effect.inner_glow_color, "tab.effect.inner_glow_color")
        BOOL_KEY(cfg.tab.effect.separator_line,    "tab.effect.separator_line")
        if (strcmp(full, "tab.effect.separator_color") == 0) {
            if (val[0] != '\0') {
                _ParseColor(val, cfg.tab.effect.separator_color);
                cfg.tab.effect.separator_auto = false;
            }
            continue;
        }

        // tab.texture
        STR_KEY(cfg.tab.texture.file, "tab.texture.file")
        BOOL_KEY(cfg.tab.texture.tile, "tab.texture.tile")
        if (strcmp(full, "tab.texture.blend_mode") == 0) { cfg.tab.texture.blend_mode = _ParseBlendMode(val); continue; }
        FLOAT_KEY(cfg.tab.texture.opacity, "tab.texture.opacity")

        // ── border ────────────────────────────────────────────────────────────
        INT_KEY(cfg.border.width, "border.width")
        if (strcmp(full, "border.style") == 0) { cfg.border.style = _ParseBorderStyle(val); continue; }

        COLOR_SIMPLE(cfg.border.active.color_base, "border.active.color_base")
        COLOR_KEY(cfg.border.active.color_highlight, cfg.border.active.highlight_auto, "border.active.color_highlight")
        COLOR_KEY(cfg.border.active.color_shadow,    cfg.border.active.shadow_auto,    "border.active.color_shadow")

        COLOR_SIMPLE(cfg.border.inactive.color_base, "border.inactive.color_base")
        COLOR_KEY(cfg.border.inactive.color_highlight, cfg.border.inactive.highlight_auto, "border.inactive.color_highlight")
        COLOR_KEY(cfg.border.inactive.color_shadow,    cfg.border.inactive.shadow_auto,    "border.inactive.color_shadow")

        INT_KEY(cfg.border.corners.radius, "border.corners.radius")
        BOOL_KEY(cfg.border.corners.anti_alias, "border.corners.anti_alias")

        // Risolvi auto-colori bordo
        if (cfg.border.active.highlight_auto)
            cfg.border.active.color_highlight = _AutoHighlight(cfg.border.active.color_base);
        if (cfg.border.active.shadow_auto)
            cfg.border.active.color_shadow = _AutoShadow(cfg.border.active.color_base);
        if (cfg.border.inactive.highlight_auto)
            cfg.border.inactive.color_highlight = _AutoHighlight(cfg.border.inactive.color_base);
        if (cfg.border.inactive.shadow_auto)
            cfg.border.inactive.color_shadow = _AutoShadow(cfg.border.inactive.color_base);

        // ── buttons ───────────────────────────────────────────────────────────
        INT_KEY(cfg.buttons.size,          "buttons.size")
        INT_KEY(cfg.buttons.margin,        "buttons.margin")
        INT_KEY(cfg.buttons.spacing,       "buttons.spacing")
        INT_KEY(cfg.buttons.corner_radius, "buttons.corner_radius")
        if (strcmp(full, "buttons.position") == 0) { cfg.buttons.position = _ParseButtonPosition(val); continue; }
        if (strcmp(full, "buttons.shape")    == 0) { cfg.buttons.shape    = _ParseButtonShape(val);    continue; }

        if (strcmp(full, "buttons.icon.style") == 0) { cfg.buttons.icon.style = _ParseButtonIconStyle(val); continue; }
        FLOAT_KEY(cfg.buttons.icon.scale,        "buttons.icon.scale")
        FLOAT_KEY(cfg.buttons.icon.stroke_width,  "buttons.icon.stroke_width")

        COLOR_SIMPLE(cfg.buttons.close.color_normal,  "buttons.close.color_normal")
        COLOR_SIMPLE(cfg.buttons.close.color_hover,   "buttons.close.color_hover")
        COLOR_SIMPLE(cfg.buttons.close.color_pressed, "buttons.close.color_pressed")
        COLOR_SIMPLE(cfg.buttons.close.icon_color,    "buttons.close.icon_color")

        COLOR_SIMPLE(cfg.buttons.zoom.color_normal,  "buttons.zoom.color_normal")
        COLOR_SIMPLE(cfg.buttons.zoom.color_hover,   "buttons.zoom.color_hover")
        COLOR_SIMPLE(cfg.buttons.zoom.color_pressed, "buttons.zoom.color_pressed")
        COLOR_SIMPLE(cfg.buttons.zoom.icon_color,    "buttons.zoom.icon_color")

        if (strcmp(full, "buttons.hover_effect.type") == 0) { cfg.buttons.hover_effect.type = _ParseHoverEffect(val); continue; }
        FLOAT_KEY(cfg.buttons.hover_effect.scale_factor, "buttons.hover_effect.scale_factor")

        // ── resize_corner ─────────────────────────────────────────────────────
        if (strcmp(full, "resize_corner.style") == 0) { cfg.resize_corner.style = _ParseResizeStyle(val); continue; }
        INT_KEY(cfg.resize_corner.size, "resize_corner.size")
        if (strcmp(full, "resize_corner.color") == 0) {
            if (val[0] != '\0') {
                _ParseColor(val, cfg.resize_corner.color);
                cfg.resize_corner.color_auto = false;
            }
            continue;
        }

        // ── title ─────────────────────────────────────────────────────────────
        if (strcmp(full, "title.alignment")  == 0) { cfg.title.alignment  = _ParseTitleAlignment(val); continue; }
        if (strcmp(full, "title.truncation") == 0) { cfg.title.truncation = _ParseTruncation(val);     continue; }
        BOOL_KEY(cfg.title.shadow, "title.shadow")
        COLOR_SIMPLE(cfg.title.shadow_color, "title.shadow_color")

        // ── floating ──────────────────────────────────────────────────────────
        INT_KEY(cfg.floating.tab_height,    "floating.tab_height")
        INT_KEY(cfg.floating.border_width,  "floating.border_width")
        if (strcmp(full, "floating.tab_color") == 0) {
            if (val[0] != '\0') {
                _ParseColor(val, cfg.floating.tab_color);
                cfg.floating.tab_color_auto = false;
            }
            continue;
        }

        // ── rendering ─────────────────────────────────────────────────────────
        BOOL_KEY(cfg.rendering.scale_with_dpi, "rendering.scale_with_dpi")
        INT_KEY(cfg.rendering.min_tab_height,  "rendering.min_tab_height")

#undef INT_KEY
#undef FLOAT_KEY
#undef BOOL_KEY
#undef COLOR_KEY
#undef COLOR_SIMPLE
#undef STR_KEY
    }

    fclose(f);

    // Risoluzione auto-colori post-caricamento
    if (cfg.resize_corner.color_auto)
        cfg.resize_corner.color = cfg.border.active.color_base;
    if (cfg.floating.tab_color_auto)
        cfg.floating.tab_color = cfg.tab.inactive.color_start;
    if (cfg.tab.effect.separator_auto)
        cfg.tab.effect.separator_color = tint_color(cfg.tab.active.color_start, B_DARKEN_2_TINT);

    return B_OK;
}

const char* ConfigReader::LastError() {
    return sLastError.String();
}
