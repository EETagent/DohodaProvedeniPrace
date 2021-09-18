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
    fprintf(stderr, "-h Vypsání této nápovědy\n-n Seřazení položek od nejnovější\n-s Seřazení položek od nejstarší\n-t Vypsat počet odpracovaných hodin\n-f Cesta k souboru\n-- Vypsat PDF do stdout (Musí být na konci příkazu)\n");
    fprintf(stderr, "\nPŘÍKLADY:\n");
    fprintf(stderr, "\t%s < vykaz.toml\n", program);
    fprintf(stderr, "\t%s -s < vykaz.toml\n", program);
    fprintf(stderr, "\t%s -- < vykaz.toml > dohoda.pdf\n", program);
    fprintf(stderr, "\t%s -n -f /home/thinkpad/vykaz.toml -- > dohoda.pdf\n", program);
}

int main(int argc, char **argv) {
    int argumenty;
    unsigned short argumenty_pocet = 0;

    bool pocet_hodin_argument = false;
    bool soubor_argument = false;
    bool pdf_do_stdout = false;


    char *soubor;

    SSPS_DOHODA_PDF pdf;
    SSPS_DOHODA_Konfigurace toml_konfigurace;
    SSPS_DOHODA_RAZENI_POLOZEK razeni = NERADIT;

    while ((argumenty = getopt(argc, argv, "hnstf:")) != -1) {
        switch (argumenty) {
            // Vypsání nápovědy
            case 'h':
                napoveda(argv[0]);
                return 0;
            // Řazení od nejnovějšího
            case 'n':
                argumenty_pocet++;
                razeni = OD_NEJNOVEJSIHO;
                break;
            // Řazení od nejstaršího
            case 's':
                argumenty_pocet++;
                razeni = OD_NEJSTARSIHO;
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
                return 1;
        }
    }

    // Pokud byl zadán soubor přes argument programu -f
    if (soubor_argument == true) {
        FILE *fp = fopen(soubor, "r");
        if (!fp) {
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
    if (argc - 1 > argumenty_pocet) {
        if (strcmp(argv[++argumenty_pocet], "--") == 0) {
            pdf_do_stdout = true;
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
        }


    }

    // Jinak uložit do souboru
    if (pdf_do_stdout == false) {
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
    SSPS_DOHODA_Konfigurace_Free(&toml_konfigurace);

    return 0;
}
