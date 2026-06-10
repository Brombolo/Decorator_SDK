#ifndef CONFIG_READER_H
#define CONFIG_READER_H

/*
 * Haiku Decorator SDK — ConfigReader.h
 * Legge theme.conf (TOML minimale) e popola una struttura ThemeConfig.
 */

#include "ThemeConfig.h"
#include <String.h>

class ConfigReader {
public:
    // Carica theme.conf dal percorso indicato.
    // Ritorna B_OK oppure un codice di errore Haiku.
    static status_t         Load(const char* path, ThemeConfig& out);

    // Restituisce il messaggio dell'ultimo errore di parsing.
    static const char*      LastError();

private:
    static BString          sLastError;

    // Parsing di sezioni e chiavi
    static bool             _ParseColor(const char* value, rgb_color& out);
    static bool             _ParseBool(const char* value, bool& out);
    static int32            _ParseInt(const char* value, int32 defaultVal);
    static float            _ParseFloat(const char* value, float defaultVal);

    static TabShape         _ParseTabShape(const char* value);
    static EffectType       _ParseEffectType(const char* value);
    static GradientDir      _ParseGradientDir(const char* value);
    static BlendMode        _ParseBlendMode(const char* value);
    static BorderStyle      _ParseBorderStyle(const char* value);
    static ButtonShape      _ParseButtonShape(const char* value);
    static ButtonIconStyle  _ParseButtonIconStyle(const char* value);
    static ButtonHoverEffect _ParseHoverEffect(const char* value);
    static ResizeCornerStyle _ParseResizeStyle(const char* value);
    static TitleAlignment   _ParseTitleAlignment(const char* value);
    static TitleTruncation  _ParseTruncation(const char* value);
    static ButtonPosition   _ParseButtonPosition(const char* value);

    // Calcola highlight/shadow automatici da un colore base (come Haiku tint)
    static rgb_color        _AutoHighlight(rgb_color base);
    static rgb_color        _AutoShadow(rgb_color base);
};

#endif // CONFIG_READER_H
