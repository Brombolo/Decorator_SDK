/*
 * Haiku Decorator SDK — TabPainter.cpp
 * Carica PNG tramite BTranslationUtils e produce BBitmap blendato.
 * Include solo header pubblici Haiku.
 */

#include "TabPainter.h"

#include <TranslationUtils.h>
#include <algorithm>

using std::min;
using std::max;

TabPainter::TabPainter(const TabTextureConfig& config, const char* themeDir)
    : fTexture(nullptr), fConfig(config)
{
    if (fConfig.file.IsEmpty()) return;
    BString path(themeDir);
    path << "/" << fConfig.file;
    _Load(path.String());
}

TabPainter::~TabPainter() {
    delete fTexture;
}

bool TabPainter::IsValid() const {
    return fTexture != nullptr && fTexture->IsValid();
}

bool TabPainter::_Load(const char* path) {
    BBitmap* bmp = BTranslationUtils::GetBitmapFile(path);
    if (!bmp || !bmp->IsValid()) { delete bmp; return false; }

    // Converti in B_RGB32 se necessario
    if (bmp->ColorSpace() == B_RGB32 || bmp->ColorSpace() == B_RGBA32) {
        fTexture = bmp;
        return true;
    }

    BRect b = bmp->Bounds();
    BBitmap* conv = new (std::nothrow) BBitmap(b, B_RGB32);
    if (!conv || !conv->IsValid()) { delete bmp; delete conv; return false; }

    // Copia raw (BitmapStream non disponibile senza Translation Kit avanzato)
    memcpy(conv->Bits(), bmp->Bits(),
           min((size_t)conv->BitsLength(), (size_t)bmp->BitsLength()));
    delete bmp;
    fTexture = conv;
    return true;
}

uint8 TabPainter::_Clamp(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (uint8)v;
}

rgb_color TabPainter::_BlendPixel(rgb_color base, rgb_color tex,
                                   float opacity) const
{
    auto lerp8 = [](int a, int b, float t) -> uint8 {
        return (uint8)(a + (b - a) * t);
    };

    switch (fConfig.blend_mode) {
        case BLEND_MULTIPLY: {
            rgb_color m = {
                (uint8)((base.red   * tex.red)   / 255),
                (uint8)((base.green * tex.green) / 255),
                (uint8)((base.blue  * tex.blue)  / 255), 255 };
            return { lerp8(base.red,   m.red,   opacity),
                     lerp8(base.green, m.green, opacity),
                     lerp8(base.blue,  m.blue,  opacity), 255 };
        }
        case BLEND_OVERLAY: {
            auto ov = [](int b, int t) -> int {
                return b < 128 ? (2*b*t)/255 : 255-(2*(255-b)*(255-t))/255;
            };
            rgb_color o = { _Clamp(ov(base.red,   tex.red)),
                            _Clamp(ov(base.green, tex.green)),
                            _Clamp(ov(base.blue,  tex.blue)), 255 };
            return { lerp8(base.red,   o.red,   opacity),
                     lerp8(base.green, o.green, opacity),
                     lerp8(base.blue,  o.blue,  opacity), 255 };
        }
        case BLEND_SCREEN: {
            rgb_color s = {
                _Clamp(255-((255-base.red)  *(255-tex.red)  /255)),
                _Clamp(255-((255-base.green)*(255-tex.green)/255)),
                _Clamp(255-((255-base.blue) *(255-tex.blue) /255)), 255 };
            return { lerp8(base.red,   s.red,   opacity),
                     lerp8(base.green, s.green, opacity),
                     lerp8(base.blue,  s.blue,  opacity), 255 };
        }
        default: // BLEND_NORMAL
            return { lerp8(base.red,   tex.red,   opacity),
                     lerp8(base.green, tex.green, opacity),
                     lerp8(base.blue,  tex.blue,  opacity), 255 };
    }
}

BBitmap* TabPainter::CreateBlendedBitmap(const BRect& rect,
                                          rgb_color base_color) const
{
    if (!IsValid()) return nullptr;

    BBitmap* result = new (std::nothrow) BBitmap(rect.OffsetToCopy(0,0),
                                                  B_RGB32);
    if (!result || !result->IsValid()) { delete result; return nullptr; }

    BRect texBounds = fTexture->Bounds();
    int32 texW = texBounds.IntegerWidth()  + 1;
    int32 texH = texBounds.IntegerHeight() + 1;
    int32 outW  = rect.IntegerWidth()  + 1;
    int32 outH  = rect.IntegerHeight() + 1;

    uint8* texBits = (uint8*)fTexture->Bits();
    uint8* outBits = (uint8*)result->Bits();
    int32  texBPR  = fTexture->BytesPerRow();
    int32  outBPR  = result->BytesPerRow();

    for (int32 y = 0; y < outH; y++) {
        for (int32 x = 0; x < outW; x++) {
            int32 tx = fConfig.tile ? (x % texW) : (x * (texW-1) / max(outW-1, 1));
            int32 ty = fConfig.tile ? (y % texH) : (y * (texH-1) / max(outH-1, 1));

            uint8* tp = texBits + ty * texBPR + tx * 4;
            // Haiku B_RGB32 = BGRA
            rgb_color tex_px = { tp[2], tp[1], tp[0], 255 };
            rgb_color blended = _BlendPixel(base_color, tex_px, fConfig.opacity);

            uint8* op = outBits + y * outBPR + x * 4;
            op[0] = blended.blue;
            op[1] = blended.green;
            op[2] = blended.red;
            op[3] = 255;
        }
    }

    return result;
}
