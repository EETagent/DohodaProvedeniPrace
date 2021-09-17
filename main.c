#include <stdio.h> // Standardní vstup a výstup
#include <string.h> // Práce s textovými řetězci
#include <stdbool.h> // True a false

#include <unistd.h> // Getopt, parsování argumentů

#include <hpdf.h> // libharu, tvorba PDF

#include <ssps_dohoda.h>

// Nápověda k programu
void napoveda(char *program) {
    fprintf(stderr, "DohodaProvedeniPrace (https://github.com/EETagent/DohodaProvedeniPrace)\n");
    fprintf(stderr, "Použití programu: %s [-hnstf] < soubor\n", program);
    fprintf(stderr, "-h Vypsání této nápovědy\n-n Seřazení položek od nejnovější\n-s Seřazení položek od nejstarší\n-t Vypsat počet odpracovaných hodin\n-f Cesta k souboru\n-- Vypsat PDF do stdout\n");
    fprintf(stderr, "\nPŘÍKLADY:\n");
    fprintf(stderr, "\t%s < vykaz.toml\n", program);
    fprintf(stderr, "\t%s -s < vykaz.toml\n", program);
    fprintf(stderr, "\t%s -- < vykaz.toml > dohoda.pdf\n", program);
    fprintf(stderr, "\t%s -n -f /home/thinkpad/vykaz.toml -- > dohoda.pdf\n", program);

    exit(1);
}

int main(int argc, char **argv) {
    int argumenty;
    unsigned short argumenty_pocet = 1;

    bool pocet_hodin_argument;
    bool soubor_argument;

    char *soubor;

    SSPS_DOHODA_PDF pdf;
    SSPS_DOHODA_Konfigurace toml_konfigurace;
    SSPS_DOHODA_RAZENI_POLOZEK razeni = NERADIT;

    while (( argumenty = getopt(argc, argv, "hnstf:")) != -1) {
        switch (argumenty) {
            // Vypsání nápovědy
            case 'h':
                napoveda(argv[0]);
            // Řazení od nejnovějšího
            case 'n':
                razeni = OD_NEJNOVEJSIHO;
                argumenty_pocet++;
                break;
            // Řazení od nejstaršího
            case 's':
                razeni = OD_NEJSTARSIHO;
                argumenty_pocet++;
                break;
            // Vypsání odpracovaných hodin
            case 't':
                argumenty_pocet++;
                pocet_hodin_argument = true;
                break;
            // Načtení konfigurace přes cestu k souboru jako argument
            case 'f':
                argumenty_pocet++;
                soubor_argument = true;
                soubor = strdup(optarg);
                break;
            default:
                napoveda(argv[0]);
        }
    }

    // Pokud byl zadán soubor přes argument programu -f
    if (soubor_argument == true) {
        FILE *fp = fopen(soubor,"r");
        if ( !fp ) {
            fprintf(stderr, "Soubor nelze otevřít: %s", soubor);
            free(soubor);
            return 1;
        }
        free(soubor);
        // Načtení TOML konfigurace z přiloženého souboru
        if (SSPS_DOHODA_Konfigurace_TOML(fp, &toml_konfigurace, SOUBOR, razeni) == 1)
            return 1;
    } else {
        // Načtení TOML konfigurace ze standardního vstupu
        if (SSPS_DOHODA_Konfigurace_TOML(stdin, &toml_konfigurace, SOUBOR, razeni) == 1)
            return 1;
    }

    // Vypsání celkového počtu hodin
    if (pocet_hodin_argument == true) {
        float pocet_hodin;
        SSPS_DOHODA_PocetHodin(toml_konfigurace, &pocet_hodin);
        printf("%0.2f", pocet_hodin);
        return 0;
    }

    // Vytvoření PDF
    if (SSPS_DOHODA_SepsatDohodu(toml_konfigurace, &pdf) == 1)
        return 1;

    // Vypsat do stdout při argumentu --
    // ./dohoda_ssps -- < data.toml > moje.pdf
    if (argc > 1 && strcmp(argv[argumenty_pocet], "--") == 0) {
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
