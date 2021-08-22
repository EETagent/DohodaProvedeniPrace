#include <stdio.h> // Standardní vstup a výstup

#include <hpdf.h> // libharu, tvorba PDF

#include <assert.h> // assert makro

#include "toml_stdin.h" // Testovací TOML stdin vstup

#include <ssps_dohoda.h>

int main(void) {
    FILE *toml_soubor;

    // Otevření toml dat jako FILE* v paměti
    toml_soubor = fmemopen(toml_stdin, toml_stdin_len, "r");

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