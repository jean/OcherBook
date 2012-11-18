#ifndef OCHER_UX_FONT_ENGINE_H
#define OCHER_UX_FONT_ENGINE_H

#include "clc/data/Hashtable.h"
#include "ocher/ux/fb/FbTypes.h"


/**
 */
struct GlyphFace
{
    // TODO: bitfield
    uint8_t faceId;
    uint8_t points;
    uint8_t underline;
    uint8_t bold;
    uint8_t italic;

    int16_t bearingY;
    int16_t ascender;
    int16_t descender;
    int16_t lineHeight;
} __attribute__((packed));

struct GlyphDescr
{
    uint32_t c;
    // TODO: bitfield
    uint8_t faceId;
    uint8_t points;
    uint8_t underline;
    uint8_t bold;
    uint8_t italic;
} __attribute__((packed));

class GlyphCache
{
public:
    GlyphCache();
    ~GlyphCache();

    void put(GlyphDescr* f, Glyph* g);
    Glyph* get(GlyphDescr* f);

    clc::Hashtable m_cache;  ///< Maps GlyphDescr -> Glyph
};

/**
 * Converts glyphs to bitmaps.
 */
class FontEngine
{
public:
    FontEngine();
    ~FontEngine();

    void setFont();
    void setSize(unsigned int points);
    void setBold(int b);
    void setUnderline(int u);
    void setItalic(int i);
    void apply();
    int dirty;
    GlyphFace m_cur;
    GlyphFace m_next;

    /**
     * 
     */
    void plotString(const char* str, unsigned int len, Glyph** glyphs, Rect* boundingBox);

    /**
     * Renders a string, in the current font.
     * @param str  Data to render
     * @param len
     * @param pen  Position of starting baseline, relative to r.  Updated.
     * @param r  Containing rectangle (possibly clip rectangle)
     * @param flags
     *      BLIT   Actually blit to framebuffer, vs just measuring
     *      WRAP   Word wrap?
     *      XCLIP  Render clipped char, or Stop if char does not fit.  N/A if WRAP.
     *      YCLIP  Render clipped char, or Stop if char does not fit.
     * @return offset of first unrendered character (==len if all fit).  May also stop early if len
     *      splits a multi-byte character.
     */
#define FE_NOBLIT 1
#define FE_WRAP   2
#define FE_XCLIP  4
#define FE_YCLIP  8
    unsigned int renderString(const char* str, unsigned int len, Pos* pen, const Rect* r, unsigned int flags);

    GlyphCache m_cache;
};

#endif
