#include <hpdf.h>
#include <hpdf_utils.h>
#include <hpdf_encryptdict.h>

// https://github.com/libharu/libharu/pull/44/files
// Založeno na nepřijatém pull request
// Načítání TTF z paměti


static const char *LoadTTFontFromStream(HPDF_Doc pdf,
                                        HPDF_Stream font_data,
                                        HPDF_BOOL embedding,
                                        const char *file_name) {
    HPDF_FontDef def;

    HPDF_PTRACE((" HPDF_LoadTTFontFromStream\n"));
    HPDF_UNUSED(file_name);

    def = HPDF_TTFontDef_Load(pdf->mmgr, font_data, embedding);
    if (def) {
        HPDF_FontDef tmpdef = HPDF_Doc_FindFontDef(pdf, def->base_font);
        if (tmpdef) {
            HPDF_FontDef_Free(def);
            return tmpdef->base_font;
        }

        if (HPDF_List_Add(pdf->fontdef_list, def) != HPDF_OK) {
            HPDF_FontDef_Free(def);
            return NULL;
        }
    } else
        return NULL;

    if (embedding) {
        if (pdf->ttfont_tag[0] == 0) {
            HPDF_MemCpy(pdf->ttfont_tag, (HPDF_BYTE *) "HPDFAA", 6);
        } else {
            HPDF_INT i;

            for (i = 5; i >= 0; i--) {
                pdf->ttfont_tag[i] += 1;
                if (pdf->ttfont_tag[i] > 'Z')
                    pdf->ttfont_tag[i] = 'A';
                else
                    break;
            }
        }

        HPDF_TTFontDef_SetTagName(def, (char *) pdf->ttfont_tag);
    }

    return def->base_font;
}

HPDF_EXPORT(const char *)
HPDF_LoadTTFontFromMemory(HPDF_Doc pdf,
                          const HPDF_BYTE *buffer,
                          HPDF_UINT size,
                          HPDF_BOOL embedding) {
    HPDF_Stream font_data;
    const char *ret;

    HPDF_PTRACE((" HPDF_LoadTTFontFromMemory\n"));

    if (!HPDF_HasDoc(pdf))
        return NULL;

    /* create memory stream */
    font_data = HPDF_MemStream_New(pdf->mmgr, size);
    if (!HPDF_Stream_Validate(font_data)) {
        HPDF_RaiseError(&pdf->error, HPDF_INVALID_STREAM, 0);
        return NULL;
    }

    if (HPDF_Stream_Write(font_data, buffer, size) != HPDF_OK) {
        HPDF_Stream_Free(font_data);
        return NULL;
    }

    return LoadTTFontFromStream(pdf, font_data, embedding, "");
}
