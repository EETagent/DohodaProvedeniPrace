#include <stdio.h> // Standardní vstup a výstup

#include <hpdf.h> // libharu, tvorba PDF

#include <assert.h> // assert makro

#include <ssps_dohoda.h>

int main(void) {
    // Vstup ze toml/sample_dohoda.toml
    unsigned char toml_stdin[580] = {
            0x5b, 0x64, 0x6f, 0x68, 0x6f, 0x64, 0x61, 0x5d, 0x0a, 0x6e, 0x61, 0x7a,
            0x65, 0x76, 0x20, 0x3d, 0x20, 0x22, 0x50, 0x72, 0x6f, 0x67, 0x72, 0x61,
            0x6d, 0x20, 0x6e, 0x61, 0x20, 0x74, 0x6f, 0x76, 0x72, 0x62, 0x75, 0x20,
            0x64, 0x6f, 0x68, 0x6f, 0x64, 0x20, 0x6f, 0x20, 0x70, 0x72, 0x6f, 0x76,
            0x65, 0x64, 0x65, 0x6e, 0xc3, 0xad, 0x20, 0x70, 0x72, 0xc3, 0xa1, 0x63,
            0x65, 0x22, 0x0a, 0x6b, 0x64, 0x65, 0x20, 0x3d, 0x20, 0x22, 0x56, 0x20,
            0x4c, 0x69, 0x62, 0x65, 0x72, 0x63, 0x69, 0x22, 0x0a, 0x0a, 0x5b, 0x7a,
            0x61, 0x6d, 0x65, 0x73, 0x74, 0x6e, 0x61, 0x6e, 0x65, 0x63, 0x5d, 0x0a,
            0x6a, 0x6d, 0x65, 0x6e, 0x6f, 0x20, 0x3d, 0x20, 0x22, 0x46, 0x72, 0x61,
            0x6e, 0x74, 0x61, 0x20, 0x4e, 0x6f, 0x76, 0xc3, 0xa1, 0x6b, 0x22, 0x0a,
            0x72, 0x6f, 0x64, 0x6e, 0x65, 0x5f, 0x63, 0x69, 0x73, 0x6c, 0x6f, 0x20,
            0x3d, 0x20, 0x22, 0x22, 0x0a, 0x62, 0x61, 0x6e, 0x6b, 0x61, 0x20, 0x3d,
            0x20, 0x22, 0x22, 0x0a, 0x6d, 0x69, 0x73, 0x74, 0x6f, 0x5f, 0x6e, 0x61,
            0x72, 0x6f, 0x7a, 0x65, 0x6e, 0x69, 0x20, 0x3d, 0x20, 0x22, 0x50, 0x72,
            0x61, 0x68, 0x61, 0x22, 0x0a, 0x61, 0x64, 0x72, 0x65, 0x73, 0x61, 0x20,
            0x3d, 0x20, 0x22, 0x4e, 0x6f, 0x76, 0xc3, 0xa1, 0x6b, 0x6f, 0x76, 0xc3,
            0xbd, 0x63, 0x68, 0x20, 0x31, 0x32, 0x33, 0x2f, 0x32, 0x2c, 0x20, 0x50,
            0x72, 0x61, 0x68, 0x61, 0x22, 0x0a, 0x70, 0x6f, 0x6a, 0x69, 0x73, 0x74,
            0x6f, 0x76, 0x6e, 0x61, 0x20, 0x3d, 0x20, 0x22, 0x56, 0x5a, 0x50, 0x20,
            0x2d, 0x20, 0x78, 0x78, 0x78, 0x78, 0x78, 0x22, 0x0a, 0x0a, 0x5b, 0x5b,
            0x70, 0x72, 0x61, 0x63, 0x65, 0x5d, 0x5d, 0x0a, 0x64, 0x61, 0x74, 0x75,
            0x6d, 0x20, 0x3d, 0x20, 0x22, 0x31, 0x34, 0x2e, 0x38, 0x22, 0x0a, 0x63,
            0x69, 0x6e, 0x6e, 0x6f, 0x73, 0x74, 0x20, 0x3d, 0x20, 0x22, 0x48, 0x6f,
            0x64, 0x6e, 0xc4, 0x9b, 0x20, 0x6a, 0x73, 0x65, 0x6d, 0x20, 0x70, 0x72,
            0x61, 0x63, 0x6f, 0x76, 0x61, 0x6c, 0x22, 0x0a, 0x68, 0x6f, 0x64, 0x69,
            0x6e, 0x79, 0x20, 0x3d, 0x20, 0x22, 0x34, 0x22, 0x0a, 0x70, 0x6f, 0x7a,
            0x6e, 0x61, 0x6d, 0x6b, 0x61, 0x20, 0x3d, 0x20, 0x22, 0x22, 0x0a, 0x0a,
            0x5b, 0x5b, 0x70, 0x72, 0x61, 0x63, 0x65, 0x5d, 0x5d, 0x0a, 0x64, 0x61,
            0x74, 0x75, 0x6d, 0x20, 0x3d, 0x20, 0x22, 0x31, 0x35, 0x2e, 0x38, 0x22,
            0x0a, 0x63, 0x69, 0x6e, 0x6e, 0x6f, 0x73, 0x74, 0x20, 0x3d, 0x20, 0x22,
            0x44, 0x61, 0x6c, 0xc5, 0xa1, 0xc3, 0xad, 0x20, 0x70, 0x72, 0xc3, 0xa1,
            0x63, 0x65, 0x22, 0x0a, 0x68, 0x6f, 0x64, 0x69, 0x6e, 0x79, 0x20, 0x3d,
            0x20, 0x22, 0x32, 0x22, 0x0a, 0x70, 0x6f, 0x7a, 0x6e, 0x61, 0x6d, 0x6b,
            0x61, 0x20, 0x3d, 0x20, 0x22, 0x22, 0x0a, 0x0a, 0x5b, 0x5b, 0x70, 0x72,
            0x61, 0x63, 0x65, 0x5d, 0x5d, 0x0a, 0x64, 0x61, 0x74, 0x75, 0x6d, 0x20,
            0x3d, 0x20, 0x22, 0x31, 0x36, 0x2e, 0x38, 0x22, 0x0a, 0x63, 0x69, 0x6e,
            0x6e, 0x6f, 0x73, 0x74, 0x20, 0x3d, 0x20, 0x22, 0x54, 0xc5, 0x99, 0x65,
            0x74, 0xc3, 0xad, 0x20, 0x70, 0x6f, 0x6c, 0x6f, 0xc5, 0xbe, 0x6b, 0x61,
            0x20, 0x75, 0xc5, 0xbe, 0x22, 0x0a, 0x68, 0x6f, 0x64, 0x69, 0x6e, 0x79,
            0x20, 0x3d, 0x20, 0x22, 0x39, 0x22, 0x0a, 0x70, 0x6f, 0x7a, 0x6e, 0x61,
            0x6d, 0x6b, 0x61, 0x20, 0x3d, 0x20, 0x22, 0x22, 0x0a, 0x0a, 0x5b, 0x5b,
            0x70, 0x72, 0x61, 0x63, 0x65, 0x5d, 0x5d, 0x0a, 0x64, 0x61, 0x74, 0x75,
            0x6d, 0x20, 0x3d, 0x20, 0x22, 0x31, 0x37, 0x2e, 0x38, 0x22, 0x0a, 0x63,
            0x69, 0x6e, 0x6e, 0x6f, 0x73, 0x74, 0x20, 0x3d, 0x20, 0x22, 0x50, 0x6f,
            0x73, 0x6c, 0x65, 0x64, 0x6e, 0xc3, 0xad, 0x20, 0x74, 0x65, 0x73, 0x74,
            0x6f, 0x76, 0x61, 0x63, 0xc3, 0xad, 0x20, 0x62, 0x6c, 0x6f, 0x6b, 0x22,
            0x0a, 0x68, 0x6f, 0x64, 0x69, 0x6e, 0x79, 0x20, 0x3d, 0x20, 0x22, 0x32,
            0x22, 0x0a, 0x70, 0x6f, 0x7a, 0x6e, 0x61, 0x6d, 0x6b, 0x61, 0x20, 0x3d,
            0x20, 0x22, 0x22, 0x0a
    };

    unsigned int toml_len = sizeof(toml_stdin);

    FILE *toml_soubor;

    // Otevření toml dat jako FILE* v paměti
    toml_soubor = fmemopen(toml_stdin, toml_len, "r");

    SSPS_DOHODA_Konfigurace toml_konfigurace;
    SSPS_DOHODA_PDF pdf;
    if (SSPS_DOHODA_Konfigurace_TOML(toml_soubor, &toml_konfigurace, SOUBOR) == 1)
        return 1;
    fclose(toml_soubor);

    if (SSPS_DOHODA_SepsatDohodu(toml_konfigurace, &pdf) == 1)
        return 1;


    HPDF_SaveToStream(pdf);
    HPDF_ResetStream(pdf);

    // Velikost PDF v bytech
    HPDF_UINT32 pdf_velikost = HPDF_GetStreamSize(pdf);

    // Je PDF menší než nebo rovno 1MB?
    assert(pdf_velikost <= 1000000);


    return 0;

}