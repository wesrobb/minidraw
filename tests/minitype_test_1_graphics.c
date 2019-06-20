#define MINITYPE_IMPLEMENTATION
#include "../minitype.h"
#include "../tests/minitype_testcommon.c"

#include <stdio.h>

mt_font g_font;
mt_color g_textBGColor;
mt_color g_textFGColor;
mt_color g_midLineColor;
const char* g_text = "MINITYPE"; //"minitype";

mt_result on_init(mt_testapp* pApp)
{
    mt_result result;
    mt_font_config fontConfig;

    MT_ZERO_OBJECT(&fontConfig);
    fontConfig.family = "Liberation";
    fontConfig.sizeInPixels = 160;
    fontConfig.weight = mt_font_weight_bold;
    fontConfig.slant = mt_font_slant_none;
    fontConfig.noClearType = MT_FALSE;
    result = mt_font_init(&pApp->mt, &fontConfig, &g_font);
    if (result != MT_SUCCESS) {
        return result;
    }

    g_textBGColor  = mt_rgba(255, 255, 255, 255);
    g_textFGColor  = mt_rgba(0,   0,   0,   255);
    g_midLineColor = mt_rgba(192, 192, 192, 255);

    return MT_SUCCESS;
}

void on_uninit(mt_testapp* pApp)
{
    mt_font_uninit(&g_font);
}

void on_size(mt_testapp* pApp, mt_uint32 sizeX, mt_uint32 sizeY)
{
    mt_testapp_scheduled_redraw(pApp, 0, 0, sizeX, sizeY);
}

void on_paint(mt_testapp* pApp, mt_gc* pGC)
{
    mt_result result;
    mt_uint32 gcSizeX;
    mt_uint32 gcSizeY;
    mt_uint32 midX;
    mt_uint32 midY;
    float dashes[2] = {4, 4};

    /* We position everything relative to the size of the graphics context. */
    mt_gc_get_size(pGC, &gcSizeX, &gcSizeY);
    midX = (mt_int32)(gcSizeX / 2);
    midY = (mt_int32)(gcSizeY / 2);

    /* Clear the whole surface before drawing anything just to be keep it clean. */
    mt_gc_clear(pGC, g_textBGColor);

    /* Origin lines. */
    {
#if 0
        mt_gc_set_line_brush_solid(pGC, g_midLineColor);
        mt_gc_set_line_width(pGC, 2);

        /* Vertical. */
        mt_gc_move_to(pGC, midX, 0);
        mt_gc_line_to(pGC, midX, gcSizeY);

        /* Horizontal. */
        mt_gc_move_to(pGC, 0,       midY);
        mt_gc_line_to(pGC, gcSizeX, midY);

        /* Draw the lines. */
        mt_gc_stroke(pGC);
#endif
    }

    /* Text output. This is centered on the screen. */
    {
        mt_gc_set_line_width(pGC, 1);
        mt_gc_set_text_bg_color(pGC, mt_rgba(0, 0, 0, 0));  /* Foreground color is controlled by the clear color. */
        mt_gc_set_text_fg_color(pGC, g_textFGColor);
        mt_gc_set_font(pGC, &g_font);

        /* TODO: This needs to be implemented as part of a helper API. */
        {
            mt_item items[128];
            mt_uint32 itemCount = MT_COUNTOF(items);
            result = mt_itemize_utf8(pGC->pAPI, g_text, strlen(g_text), items, &itemCount);
            if (result == MT_SUCCESS) {
                mt_uint32 iItem;
                for (iItem = 0; iItem < itemCount; ++iItem) {
                    mt_item* pItem = &items[iItem];
                    mt_glyph glyphs[128];
                    size_t glyphCount = MT_COUNTOF(glyphs);
                    size_t clusters[128];
                    result = mt_shape_utf8(&g_font, pItem, g_text + pItem->offset, pItem->length, glyphs, &glyphCount, clusters);
                    if (result == MT_SUCCESS) {
                        mt_text_metrics runMetrics;
                        result = mt_place(&g_font, pItem, glyphs, glyphCount, &runMetrics);
                        if (result == MT_SUCCESS) {
                            mt_int32 cornerRadius = 4;
                            mt_int32 textPosX = midX - (runMetrics.sizeX/2);
                            mt_int32 textPosY = midY - (runMetrics.sizeY/2);
                            mt_gc_draw_glyphs(pGC, pItem, glyphs, glyphCount, textPosX, textPosY);

                            /* Left and right boundaries. */
                            mt_gc_set_line_brush_solid(pGC, mt_rgba(160, 64, 32, 255));
                            mt_gc_move_to(pGC, textPosX, 0);
                            mt_gc_line_to(pGC, textPosX, gcSizeY);
                            mt_gc_move_to(pGC, textPosX + runMetrics.sizeX, 0);
                            mt_gc_line_to(pGC, textPosX + runMetrics.sizeX, gcSizeY);
                            mt_gc_stroke(pGC);

                            /* Base line. */
                            mt_gc_set_line_brush_solid(pGC, mt_rgba(0, 0, 0, 255));
                            mt_gc_move_to(pGC, 0,       textPosY + g_font.metrics.ascent);
                            mt_gc_line_to(pGC, gcSizeX, textPosY + g_font.metrics.ascent);
                            mt_gc_stroke(pGC);

                            /* Ascender line. */
                            mt_gc_set_line_dash(pGC, MT_COUNTOF(dashes), dashes);
                            mt_gc_move_to(pGC, 0,       textPosY);
                            mt_gc_line_to(pGC, gcSizeX, textPosY);
                            mt_gc_stroke(pGC);

                            /* Decender line. */
                            mt_gc_set_line_dash(pGC, MT_COUNTOF(dashes), dashes);
                            mt_gc_move_to(pGC, 0,       textPosY + g_font.metrics.lineHeight);
                            mt_gc_line_to(pGC, gcSizeX, textPosY + g_font.metrics.lineHeight);
                            mt_gc_move_to(pGC, 0,       textPosY + g_font.metrics.ascent + g_font.metrics.descent);
                            mt_gc_line_to(pGC, gcSizeX, textPosY + g_font.metrics.ascent + g_font.metrics.descent);
                            mt_gc_stroke(pGC);

                            /* Corder dots. */
                            mt_gc_set_fill_brush_solid(pGC, mt_rgba(92, 92, 92, 255));
                            mt_gc_move_to(pGC, textPosX + cornerRadius,                    textPosY + g_font.metrics.lineHeight);
                            mt_gc_arc(    pGC, textPosX,                                   textPosY + g_font.metrics.lineHeight, cornerRadius, 0, MT_RADIANSF(360));
                            mt_gc_move_to(pGC, textPosX + runMetrics.sizeX + cornerRadius, textPosY + g_font.metrics.lineHeight);
                            mt_gc_arc(    pGC, textPosX + runMetrics.sizeX,                textPosY + g_font.metrics.lineHeight, cornerRadius, 0, MT_RADIANSF(360));

                            mt_gc_move_to(pGC, textPosX + cornerRadius,                    textPosY                            );
                            mt_gc_arc(    pGC, textPosX,                                   textPosY                            , cornerRadius, 0, MT_RADIANSF(360));
                            mt_gc_move_to(pGC, textPosX + runMetrics.sizeX + cornerRadius, textPosY                            );
                            mt_gc_arc(    pGC, textPosX + runMetrics.sizeX,                textPosY                            , cornerRadius, 0, MT_RADIANSF(360));

                            mt_gc_move_to(pGC, textPosX + cornerRadius,                    textPosY + g_font.metrics.ascent    );
                            mt_gc_arc(    pGC, textPosX,                                   textPosY + g_font.metrics.ascent    , cornerRadius, 0, MT_RADIANSF(360));
                            mt_gc_move_to(pGC, textPosX + runMetrics.sizeX + cornerRadius, textPosY + g_font.metrics.ascent    );
                            mt_gc_arc(    pGC, textPosX + runMetrics.sizeX,                textPosY + g_font.metrics.ascent    , cornerRadius, 0, MT_RADIANSF(360));

                            mt_gc_fill(pGC);
                        }
                    }
                }
            }
        }


    }
}

int main(int argc, char** argv)
{
    int exitCode;
    mt_result result;
    mt_testapp_config appConfig;
    mt_testapp app;

    MT_ZERO_OBJECT(&appConfig);
    appConfig.pWindowTitle = "Graphics Test";
    appConfig.windowWidth  = 1280;
    appConfig.windowHeight = 720;
    appConfig.onInit       = on_init;
    appConfig.onUninit     = on_uninit;
    appConfig.onSize       = on_size;
    appConfig.onPaint      = on_paint;
    appConfig.pUserData    = NULL;
    
    result = mt_testapp_init(&appConfig, &app);
    if (result != MT_SUCCESS) {
        return (int)result;
    }

    exitCode = mt_testapp_run(&app);

    mt_testapp_uninit(&app);
    return exitCode;
}