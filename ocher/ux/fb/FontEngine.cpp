#include <stddef.h>
#include <ctype.h>

#include "clc/support/Logger.h"
#include "ocher/settings/Settings.h"
#include "ocher/ux/Factory.h"
#include "ocher/ux/fb/FontEngine.h"

#define LOG_NAME "ocher.ux.FontEngine"


#if 0
/**
 * Similar to memcpy
 * @param dst  Alpha bitmap
 * @param color  Color to apply to alpha of dst + src
 */
void* alphamemcpy(uint8_t* dst, const uint8_t* src, size_t n, uint8_t color);

static void invcpy(unsigned char* dst, unsigned char* src, size_t n)
{
#if 0
    // Assuming src is word aligned.
    // dst may not be aligned.

#else
    // TODO  invert then memcpy is total hack.  do it right:
    // respect alignment, invert while copying.  (Or is it possible
    // to have FreeType output inverted?)
    for (size_t i = 0; i < n; ++i) {
        src[i] = ~src[i];
    }
    memcpy(dst, src, n);
#endif
}
#endif

void invert(void* _p, size_t n)
{
    // TODO: optimize: words, ...
    unsigned char* p = (unsigned char*)_p;
    for (size_t i = 0; i < n; ++i) {
        p[i] = ~p[i];
    }
}

GlyphCache::GlyphCache()
{
}

GlyphCache::~GlyphCache()
{
}

void GlyphCache::put(GlyphDescr* f, Glyph* g)
{
    m_cache.put(f, sizeof(*f), g);
}

Glyph* GlyphCache::get(GlyphDescr* f)
{
    return (Glyph*)m_cache.get(f, sizeof(*f));
}

FontEngine::FontEngine()
{
    m_next.faceId = 0;
    m_next.points = settings.fontPoints;
    m_next.underline = 0;
    m_next.bold = 0;
    m_next.italic = 0;
    memset(&m_cur, 0xff, sizeof(m_cur));
    dirty = 1;
}

FontEngine::~FontEngine()
{
}

static int utf8ToUtf32(const char* _p, uint32_t* u32)
{
    const unsigned char* p = (const unsigned char*)_p;
    int len = 1;
    uint32_t c = *p;
    if (c >= 0x7f) {
        if ((c & 0xe0) == 0xc0) {
            c = ((c & 0x1f) << 6) | (p[1] & 0x3f);
            len++;
        } else if ((c & 0xf0) == 0xe0) {
            c = ((c & 0x0f) << 12) | ((p[1] & 0x3f) << 6) | (p[2] & 0x3f);
            len += 2;
        } else if ((c & 0xf8) == 0xf0) {
            c = ((c & 0x07) << 18) | ((p[1] & 0x3f) << 12) | ((p[2] & 0x3f) << 6) | (p[3] & 0x3f);
            len += 3;
        } else if ((c & 0xfc) == 0xf8) {
            c = ((c & 0x03) << 24) | ((p[1] & 0x3f) << 18) | ((p[2] & 0x3f) << 12) | ((p[3] & 0x3f) << 6) | (p[4] & 0x3f);
            len += 4;
        } else if ((c & 0xfe) == 0xfc) {
            c = ((c & 0x01) << 30) | ((p[1] & 0x3f) << 24) | ((p[2] & 0x3f) << 18) | ((p[3] & 0x3f) << 12) | ((p[4] & 0x3f) << 6) | (p[5] & 0x3f);
            len += 5;
        } else {
            // out of sync?
            c = 0;
        }
    }
    *u32 = c;
    return len;
}

void FontEngine::setFont()
{
    // TODO
}

void FontEngine::setSize(unsigned int points)
{
    m_next.points = points;
    dirty = 1;
}

void FontEngine::setBold(int bold)
{
    m_next.bold = bold;
    dirty = 1;
}

void FontEngine::setUnderline(int underline)
{
    m_next.underline = underline;
    dirty = 1;
}

void FontEngine::setItalic(int italic)
{
    m_next.italic = italic;
    dirty = 1;
}

void FontEngine::apply()
{
    if (dirty) {
        if (m_cur.faceId != m_next.faceId) {
            /*TODO*/;
        }
        if (m_cur.points != m_next.points) {
            g_ft->setSize(m_next.points);
        }
        if (m_cur.underline != m_next.underline) {
            /*TODO*/;
        }
        if (m_cur.bold != m_next.bold) {
            /*TODO*/;
        }
        if (m_cur.italic != m_next.italic) {
            /*TODO*/;
        }
        m_cur = m_next;
        m_cur.ascender = g_ft->getAscender();
        m_cur.descender = g_ft->getDescender();
        m_cur.bearingY = m_cur.ascender;  /* TODO */
        m_cur.lineHeight = g_ft->getLineHeight();
        dirty = 0;
    }
}

void FontEngine::plotString(const char* p, unsigned int len, Glyph** glyphs, Rect* bbox)
{
    GlyphDescr d;
    d.faceId = m_cur.faceId;
    d.points = m_cur.points;
    d.underline = m_cur.underline;
    d.bold = m_cur.bold;
    d.italic = m_cur.italic;

    bbox->w = bbox->h = 0;
    unsigned int i = 0;
    for (const char* end = p+len; p < end; ) {
        p += utf8ToUtf32(p, &d.c);

        Glyph* g = m_cache.get(&d);
        if (!g) {
            //clc::Log::trace(LOG_NAME, "%d pt %c is not cached", d.face.points, d.c);
            g = new Glyph;
            if (g_ft->plotGlyph(&d, g) < 0) {
                clc::Log::warn(LOG_NAME, "plotGlyph failed for %x; skipping", d.c);
                delete g;
                continue;
            }

            invert(g->bitmap, g->w * g->h);
            /* TODO underline; may have to enlarge bitmap */
            //if () {
            //}
            m_cache.put(&d, g);
        }

        // TODO:  how to guarantee cache is big enough: refcount chars per line?  or "big" LRU?
        glyphs[i++] = g;
        bbox->w += g->advanceX;
        // TODO: h
    }
    glyphs[i] = 0;
}

unsigned int FontEngine::renderString(const char* str, unsigned int len, Pos* pen, const Rect* r, unsigned int flags)
{
    const char* p = str;
    bool wordWrapped = false;
    Rect bbox;

    if (! (flags & FE_YCLIP) && pen->y >= r->h - m_cur.descender) {
        return 0;
    }

    do {
        // If at start of line, eat spaces
        if (pen->x == 0) {
            while (len && isspace(*p)) {
                ++p;
                --len;
            }
        }

        if (*p != '\n') {
            // Where is the next word break?
            unsigned int w = 0;
            while (w < len && !isspace(*(p+w))) {
                ++w;
            }
            if (w < len)
                ++w;

            Glyph* glyphs[w+1];
            bbox.x = pen->x;
            bbox.y = pen->y;
            plotString(p, w, glyphs, &bbox);
            if (flags & FE_WRAP &&  // want wrap
                    pen->x + bbox.w > r->w &&  // but wouldn't fit on this line
                    bbox.w <= r->w) {  // but would fit on next
                bbox.x = pen->x = 0;
                pen->y += m_cur.lineHeight;
                bbox.y = pen->y;
            }
            if (pen->y >= r->h - m_cur.descender)
                return p - str;
            bbox.y -= m_cur.ascender;
            bbox.h = m_cur.lineHeight;
            /* TODO save bounding box + glyphs for selection */

            // Fits; render it and advance
            if (flags & FE_NOBLIT) {
                for (unsigned int i = 0; glyphs[i]; ++i) {
                    pen->x += glyphs[i]->advanceX;
                    pen->y += glyphs[i]->advanceY;
                }
            } else {
                Pos dst;
                dst.x = pen->x + r->x;
                dst.y = pen->y + r->y;
                g_fb->blitGlyphs(glyphs, &dst, r);
                pen->x = dst.x - r->x;
                pen->y = dst.y - r->y;
            }
            p += w;
            len -= w;
        }

        // Word-wrap or hard linefeed, but avoid the two back-to-back.
        if ((*p == '\n' && !wordWrapped) || pen->x >= r->w) {
            pen->x = 0;
            pen->y += m_cur.lineHeight;
            if (*p == '\n' && len) {
                p++;
                len--;
            } else {
                wordWrapped = true;
            }
            if (pen->y >= r->h - m_cur.descender)
                return p - str;
        }
    } while (len > 0);
    return -1;  // think of this as "failed to cross page boundary"
}
