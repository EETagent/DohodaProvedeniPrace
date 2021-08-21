#include <stdio.h> // Standardní vstup a výstup
#include <string.h> // Práce s textovými řetězci

#include <hpdf.h> // libharu, tvorba PDF

#include <ssps_dohoda.h>

int main(int argc, char **argv) {

    SSPS_DOHODA_PDF pdf;
    SSPS_DOHODA_Konfigurace toml_konfigurace;

    // Načtení TOML konfigurace ze standardního vstupu
    if (SSPS_DOHODA_Konfigurace_TOML(stdin, &toml_konfigurace, SOUBOR) == 1)
        return 1;

    // Vytvoření PDF
    if (SSPS_DOHODA_SepsatDohodu(toml_konfigurace, &pdf) == 1)
        return 1;

    // Vypsat do stdout při argumentu --
    // ./dohoda_ssps -- < data.toml > moje.pdf
    if (argc > 1 && strcmp(argv[1], "--") == 0) {
        // Uložení PDF do paměti
        HPDF_SaveToStream(pdf);
        // Přetočení PDF streamu na začátek (pro postupné vypsání do stdout)
        HPDF_ResetStream(pdf);
        // Vypsání do stdout
        for (;;) {
            HPDF_BYTE buffer[4096];
            HPDF_UINT32 buffer_len = sizeof(buffer);
            HPDF_ReadFromStream(pdf, buffer, &buffer_len);
            if (buffer_len == 0)
                break;
            if (fwrite(buffer, buffer_len, 1, stdout) != 1) {
                fprintf(stderr, "Nelze vypsat do stdout");
                break;
            }
        }

    // Jinak uložit do souboru
    } else {
        // Alokování paměti pro řetězec o velikosti předložky, příjmení a rezervy na .pdf
        char *pdf_soubor = malloc(strlen(PDF_SOUBOR_PREDLOZKA) + strlen(toml_konfigurace.jmeno) + 5);
        strcpy(pdf_soubor, PDF_SOUBOR_PREDLOZKA);
        // Rozdělení jména
        strtok(toml_konfigurace.jmeno, " ");
        char *prijmeni = strtok(NULL, " ");
        // V případě že existuje nějaký výstup
        if (prijmeni) {
            // _Jungmann třeba
            strcat(pdf_soubor, "_");
            strcat(pdf_soubor, prijmeni);
        }
        // Přidání koncovky
        strcat(pdf_soubor, ".pdf");
        // Uložení PDF do souboru
        HPDF_SaveToFile(pdf, pdf_soubor);
        // Vyčištění paměti
        free(pdf_soubor);
    }

    //Vyčištění paměti zabrané PDF souborem a tomlem
    HPDF_Free(pdf);

    return 0;
}
