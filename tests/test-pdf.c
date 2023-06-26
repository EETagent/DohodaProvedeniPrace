#include <stdio.h> // Standardní vstup a výstup
#include <string.h> // Práce s textovými řetězci
#include <stdint.h> // Různé číselné typy

#include <hpdf.h> // libharu, tvorba PDF

#include <assert.h> // assert makro

#include <md5.h> // MD5 hash souboru
#include <toml_stdin.h> // Testovací TOML stdin vstup

#include <ssps_dohoda.h>

int main(void) {
    // Hash nepoškozeného PDF
    const char pdf_hash_reference[33] = "17111b8b1ead97b81e255ec626c87d3a";
    char pdf_hash_opravodvy[33] = {'\0'};
    char temp[4];

    HPDF_BYTE buffer[4096];
    HPDF_UINT32 buffer_len = sizeof(buffer);

    FILE *toml_soubor, *pdf_soubor;

    // Otevření toml dat jako FILE* v paměti
    toml_soubor = fmemopen(toml_stdin, toml_stdin_len, "r");

    char *pdf_soubor_buffer;
    size_t pdf_soubor_buffer_len;

    // Vytvoření FILE* v paměti, open_memstream podporuje jenom zápis, buffer je narozdíl od fmemopen dynamicky alokovaný
    pdf_soubor = open_memstream(&pdf_soubor_buffer, &pdf_soubor_buffer_len);

    SSPS_DOHODA_Konfigurace toml_konfigurace;
    SSPS_DOHODA_PDF pdf;

    if (SSPS_DOHODA_Konfigurace_TOML(toml_soubor, &toml_konfigurace, SOUBOR, NERADIT) == 1)
        return 1;
    fclose(toml_soubor);

    if (SSPS_DOHODA_SepsatDohodu(toml_konfigurace, &pdf) == 1)
        return 1;

    HPDF_SaveToStream(pdf);
    HPDF_ResetStream(pdf);

    for (;;) {
        HPDF_ReadFromStream(pdf, buffer, &buffer_len);
        if (buffer_len == 0)
            break;
        if (fwrite(buffer, buffer_len, 1, pdf_soubor) != 1) {
            fprintf(stderr, "Nelze zapsat do open_memstram");
            break;
        }
    }

    // Ukončení a zavření streamu
    fflush(pdf_soubor);
    fclose(pdf_soubor);

    // Otevření PDF souboru pro čtení přes fmemopen
    pdf_soubor = fmemopen(pdf_soubor_buffer, pdf_soubor_buffer_len, "r");

    // MD5 hash virtuálního souboru
    uint8_t *hash_buffer = md5File(pdf_soubor);

    // Uzavření souboru
    fclose(pdf_soubor);
    // Vyčištění paměti
    free(pdf_soubor_buffer);

    // Generování MD5 hashe
    for (unsigned int i = 0; i < 16; ++i) {
        snprintf(temp, 3, "%02x", hash_buffer[i]);
        strncat(pdf_hash_opravodvy, temp, 3);
    }

    fprintf(stderr, "Hash 1: %s\nHash 2: %s", pdf_hash_reference, pdf_hash_opravodvy);

    HPDF_Free(pdf);
    SSPS_DOHODA_Konfigurace_Free(&toml_konfigurace);

    // Jsou oba výstupy hašovací funkce rovny?
    assert(strcmp(pdf_hash_reference, pdf_hash_opravodvy) == 0);

    return 0;

}