#ifndef THEME_CONFIG_H
#define THEME_CONFIG_H

/*
 * Haiku Decorator SDK — ThemeConfig.h
 * Struttura dati che rappresenta la configurazione letta da theme.conf
 * Generato da ConfigReader::Load()
 */

#include <GraphicsDefs.h>   // rgb_color
#include <String.h>
#include <SupportDefs.h>

// ─── Enumerazioni ─────────────────────────────────────────────────────────────

enum TabShape {
    TAB_SHAPE_FLAT = 0,
    TAB_SHAPE_ROUNDED,
    TAB_SHAPE_SLANTED
    // TAB_SHAPE_WAVE rimosso: path Bézier incompatibile con il ciclo di
    // ridisegno sincrono di app_server.
};

enum EffectType {
    EFFECT_NONE = 0,
    EFFECT_GRADIENT,
    EFFECT_GLASS,
    EFFECT_TEXTURE,
    EFFECT_TINT_FROM_SYSTEM
};

enum GradientDir {
    GRAD_VERTICAL = 0,
    GRAD_HORIZONTAL,
    GRAD_DIAGONAL
};

enum BlendMode {
    BLEND_NORMAL = 0,
    BLEND_MULTIPLY,
    BLEND_OVERLAY,
    BLEND_SCREEN
};

enum BorderStyle {
    BORDER_FLAT = 0,
    BORDER_BEVELED,
    BORDER_INSET,
    BORDER_SHADOW
};

enum ButtonShape {
    BTN_CIRCLE = 0,
    BTN_SQUARE,
    BTN_ROUNDED_SQUARE,
    BTN_DIAMOND
};

enum ButtonIconStyle {
    ICON_SYMBOL = 0,
    ICON_DOT,
    ICON_CROSS_X,
    ICON_CLASSIC_BE,
    ICON_NONE
};

enum ButtonHoverEffect {
    HOVER_LIGHTEN = 0,
    HOVER_DARKEN,
    HOVER_GLOW,
    HOVER_SCALE,
    HOVER_NONE
};

enum ResizeCornerStyle {
    RESIZE_KNOB = 0,
    RESIZE_LINES,
    RESIZE_NONE
};

enum TitleAlignment {
    TITLE_LEFT = 0,
    TITLE_CENTER,
    TITLE_RIGHT
};

enum TitleTruncation {
    TRUNC_END = 0,
    TRUNC_MIDDLE
};

enum ButtonPosition {
    BTN_POS_LEFT = 0,
    BTN_POS_RIGHT
};

// ─── Colori del tab per uno stato (active / inactive / stack_tile) ─────────────

struct TabColorSet {
    rgb_color   color_start;
    rgb_color   color_end;
    rgb_color   text_color;
    rgb_color   border_color;
};

// ─── Configurazione effetto tab ───────────────────────────────────────────────

struct TabEffectConfig {
    EffectType  type            = EFFECT_NONE;
    GradientDir gradient_dir    = GRAD_VERTICAL;
    float       glass_opacity   = 0.25f;
    float       tint_amount     = 0.15f;
    bool        inner_glow      = false;
    rgb_color   inner_glow_color;
    bool        separator_line  = true;
    rgb_color   separator_color;
    bool        separator_auto  = true;     // true = colore calcolato auto
};

// ─── Configurazione texture tab ───────────────────────────────────────────────

struct TabTextureConfig {
    BString     file;
    bool        tile            = true;
    BlendMode   blend_mode      = BLEND_MULTIPLY;
    float       opacity         = 0.4f;
};

// ─── Configurazione completa tab ──────────────────────────────────────────────

struct TabConfig {
    int32           height          = 22;
    int32           font_size       = 12;
    bool            font_bold       = false;

    TabShape        shape           = TAB_SHAPE_FLAT;
    int32           corner_radius   = 0;
    float           slant_angle     = 15.0f;

    TabColorSet     active;
    TabColorSet     inactive;

    struct {
        rgb_color   color_start;
        rgb_color   color_end;
        rgb_color   text_color;
        rgb_color   indicator_color;
    } stack_tile;

    TabEffectConfig effect;
    TabTextureConfig texture;
};

// ─── Configurazione bordi ─────────────────────────────────────────────────────

struct BorderColorSet {
    rgb_color   color_base;
    rgb_color   color_highlight;
    rgb_color   color_shadow;
    bool        highlight_auto  = true;
    bool        shadow_auto     = true;
};

struct BorderConfig {
    int32           width           = 5;
    BorderStyle     style           = BORDER_BEVELED;
    BorderColorSet  active;
    BorderColorSet  inactive;

    struct {
        int32   radius      = 0;
        bool    anti_alias  = true;
    } corners;
};

// ─── Configurazione pulsanti ──────────────────────────────────────────────────

struct ButtonColorSet {
    rgb_color   color_normal;
    rgb_color   color_hover;
    rgb_color   color_pressed;
    rgb_color   icon_color;
};

struct ButtonsConfig {
    int32               size            = 12;
    int32               margin          = 5;
    int32               spacing         = 4;
    ButtonPosition      position        = BTN_POS_LEFT;
    ButtonShape         shape           = BTN_CIRCLE;
    int32               corner_radius   = 3;

    struct {
        ButtonIconStyle style        = ICON_SYMBOL;
        float           scale        = 0.6f;
        float           stroke_width = 1.5f;
    } icon;

    ButtonColorSet      close;
    ButtonColorSet      zoom;

    struct {
        ButtonHoverEffect   type            = HOVER_LIGHTEN;
        float               scale_factor    = 1.08f;
    } hover_effect;
};

// ─── Configurazione resize corner ─────────────────────────────────────────────

struct ResizeCornerConfig {
    ResizeCornerStyle   style       = RESIZE_KNOB;
    int32               size        = 18;
    rgb_color           color;
    bool                color_auto  = true;
};

// ─── Configurazione titolo ────────────────────────────────────────────────────

struct TitleConfig {
    TitleAlignment  alignment       = TITLE_CENTER;
    bool            shadow          = false;
    rgb_color       shadow_color;
    TitleTruncation truncation      = TRUNC_END;
};

// ─── Configurazione floating window ──────────────────────────────────────────

struct FloatingConfig {
    int32       tab_height      = 14;
    int32       border_width    = 3;
    rgb_color   tab_color;
    bool        tab_color_auto  = true;
};

// ─── Configurazione rendering ─────────────────────────────────────────────────

struct RenderingConfig {
    bool    scale_with_dpi  = true;
    int32   min_tab_height  = 16;
};

// ─── Struttura principale ─────────────────────────────────────────────────────

struct ThemeConfig {
    BString         name    = "MyDecorator";
    BString         author;
    BString         version = "1.0";

    TabConfig       tab;
    BorderConfig    border;
    ButtonsConfig   buttons;
    ResizeCornerConfig resize_corner;
    TitleConfig     title;
    FloatingConfig  floating;
    RenderingConfig rendering;
};

#endif // THEME_CONFIG_H
