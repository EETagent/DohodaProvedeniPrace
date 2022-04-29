#include <stdio.h> // Standardní vstup a výstup
#include <string.h> // Práce s textovými řetězci
#include <stdbool.h> // True a false
#include <setjmp.h> // Jump

#include <unistd.h> // Getopt, parsování argumentů

#include <hpdf.h> // libharu, tvorba PDF

#if !defined(WIN32) && !defined(__APPLE__)
#include <sys/ioctl.h> // Pro ioctl()
#include <sys/inotify.h> // Pro monitorování změn v souborech
#endif

#include <ssps_dohoda.h>

// Nápověda k programu
void napoveda(char *program);
// Uložení PDF do stdout
int ulozit_do_stdout (SSPS_DOHODA_PDF *pdf);
// Uložení PDF do souboru
int ulozit_do_souboru (SSPS_DOHODA_PDF *pdf, SSPS_DOHODA_Konfigurace *toml_konfigurace);
// Chyby
void cli_error_handler(char *error);

jmp_buf jmp;

int main(int argc, char **argv) {
    int argumenty;

    bool pocet_hodin_argument = false;
    bool pocet_penez_argument = false;
    bool soubor_argument = false;
    bool watch_argument = false;
    bool pdf_do_stdout = false;

    char *soubor;

    SSPS_DOHODA_PDF pdf;
    SSPS_DOHODA_Konfigurace toml_konfigurace;
    SSPS_DOHODA_RAZENI_POLOZEK razeni = NERADIT;

    while ((argumenty = getopt(argc, argv, "hnstpwf:")) != -1) {
        switch (argumenty) {
            // Vypsání nápovědy
            case 'h':
                napoveda(argv[0]);
                return 0;
            // Řazení od nejnovějšího
            case 'n':
                razeni = OD_NEJNOVEJSIHO;
                break;
            // Řazení od nejstaršího
            case 's':
                razeni = OD_NEJSTARSIHO;
                break;
            // Vypsání odpracovaných hodin
            case 't':
                pocet_hodin_argument = true;
                break;
            // Vypsání celkové částky za odpracované hodiny
            case 'p':
                pocet_penez_argument = true;
                break;
            // Sledování změn v souboru
            case 'w':
                watch_argument = true;
                break;
            // Načtení konfigurace přes cestu k souboru jako argument
            case 'f':
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
            fprintf(stderr, u8"Soubor nelze otevřít: %s", soubor);
            free(soubor);
            return 1;
        }
        // Načtení TOML konfigurace z přiloženého souboru
        if (SSPS_DOHODA_Konfigurace_TOML(fp, &toml_konfigurace, SOUBOR, razeni) == 1)
            return 1;
    } else {
        // Načtení TOML konfigurace ze standardního vstupu
        if (SSPS_DOHODA_Konfigurace_TOML(stdin, &toml_konfigurace, SOUBOR, razeni) == 1)
            return 1;
    }

    // Vypsání celkové částky za odpracované hodiny
    if (pocet_penez_argument == true) {
        float pocet_penez;
        SSPS_DOHODA_PocetPenez(toml_konfigurace, &pocet_penez);
        printf(u8"%0.2f Kč", pocet_penez);
        return 0;
    }

    // Vypsání celkového počtu hodin
    if (pocet_hodin_argument == true) {
        float pocet_hodin;
        SSPS_DOHODA_PocetHodin(toml_konfigurace, &pocet_hodin);
        printf(u8"%0.2f", pocet_hodin);
        return 0;
    }

    // Vytvoření PDF
    if (SSPS_DOHODA_SepsatDohodu(toml_konfigurace, &pdf) == 1)
        cli_error_handler(u8"Nepovedlo se vytvořit PDF");

    // Vypsat do stdout při argumentu --
    // ./dohoda_ssps -- < data.toml > moje.pdf
    if (strcmp(argv[--argc], "--") == 0) {
        pdf_do_stdout = true;
        if (ulozit_do_stdout(&pdf) != 0)
            cli_error_handler("Nelze vypsat do stdout");
    }

    // Jinak uložit do souboru
    if (pdf_do_stdout == false)
        ulozit_do_souboru(&pdf, &toml_konfigurace);

    if (watch_argument == true) {
        #if defined(WIN32) || defined(__APPLE__)
        cli_error_handler(u8"Neimplementováno pro WIN32 a macOS");
        #endif
        #if !defined(WIN32) && !defined(__APPLE__)
        bool while_smycka_ukoncena = false;
        int fd = inotify_init();
        // Nelze vytvořit inotify
        if (fd < 0)
            cli_error_handler(u8"Vytvoření inotify selhalo");
        int wd = inotify_add_watch(fd, soubor, IN_ALL_EVENTS);
        // Nelze spustit monitorování souboru
        if (wd == -1)
            cli_error_handler(u8"Selhalo monitorování souboru");
        while (true) {
            unsigned int fd_velikost;
            ioctl(fd, FIONREAD, &fd_velikost);
            char buffer[fd_velikost]; unsigned long offset = 0;
            read(fd, buffer, fd_velikost);
            while (offset < fd_velikost) {
                struct inotify_event *event = (struct inotify_event *)(buffer + offset);
                if (event->mask & IN_MODIFY) {
                    // Načtení souboru
                    FILE *fp = fopen(soubor, "r");
                    if (!fp)
                        break;
                    // Ošklivý hack, bude třeba vyřešit
                    // Při rychlém refreshy možností neprobíhá čtení souboru úplně v pořádku
                    usleep(100);
                    // Znovuvytvoření konfigurace a PDF
                    SSPS_DOHODA_Konfigurace_TOML(fp, &toml_konfigurace, SOUBOR, razeni);
                    //fprintf(stderr, "\n%s\n", toml_konfigurace.kde);
                    SSPS_DOHODA_SepsatDohodu(toml_konfigurace, &pdf);
                    if (pdf_do_stdout == true)
                        ulozit_do_stdout(&pdf);
                    else
                        ulozit_do_souboru(&pdf, &toml_konfigurace);
                    fclose(fp);
                }
                // Byl-li soubor smazán, přemístěn, nebo se s ním ztratilo spojení
                else if(event->mask == 4 || event->mask & IN_DELETE_SELF || event->mask & IN_IGNORED || event->mask & IN_MOVED_FROM)
                    while_smycka_ukoncena = true;
                offset = offset + sizeof(struct inotify_event) + event->len;
            }
            // Ukončit smyčku
            if (while_smycka_ukoncena)
                break;
        }
        // Ukončení monitoringu
        inotify_rm_watch(fd, wd);
        close(fd);
        #endif
    }

    if (setjmp(jmp)) {
        if (soubor_argument)
            free(soubor);
        if (pdf)
            HPDF_Free(pdf);
        SSPS_DOHODA_Konfigurace_Free(&toml_konfigurace);
        return 1;
    }

    //Vyčištění paměti zabrané PDF souborem a tomlem
    if (soubor_argument)
        free(soubor);
    HPDF_Free(pdf);
    SSPS_DOHODA_Konfigurace_Free(&toml_konfigurace);

    return 0;
}

void napoveda (char *program) {
    fprintf(stderr, u8"DohodaProvedeniPrace (https://github.com/EETagent/DohodaProvedeniPrace)\n");
    fprintf(stderr, u8"Použití programu: %s [-hnstf] < soubor\n", program);
    fprintf(stderr, u8"-h Vypsání této nápovědy\n-n Seřazení položek od nejnovější\n-s Seřazení položek od nejstarší\n-t Vypsat počet odpracovaných hodin\n-p Vypsat celkovou částku za odpracované hodiny\n-w Živé sledování změn v souboru - watch\n-f Cesta k souboru\n-- Vypsat PDF do stdout (Musí být na konci příkazu)\n");
    fprintf(stderr, u8"\nPŘÍKLADY:\n");
    fprintf(stderr, u8"\t%s < vykaz.toml\n", program);
    fprintf(stderr, u8"\t%s -s < vykaz.toml\n", program);
    fprintf(stderr, u8"\t%s -- < vykaz.toml > dohoda.pdf\n", program);
    fprintf(stderr, u8"\t%s -n -f /home/thinkpad/vykaz.toml -- > dohoda.pdf\n", program);
}

int ulozit_do_souboru (SSPS_DOHODA_PDF *pdf, SSPS_DOHODA_Konfigurace *toml_konfigurace) {
    // Alokování paměti pro řetězec o velikosti předložky, příjmení a rezervy na .pdf
    char *pdf_soubor = malloc(strlen(PDF_SOUBOR_PREDLOZKA) + strlen(toml_konfigurace->jmeno) + 5);
    strcpy(pdf_soubor, PDF_SOUBOR_PREDLOZKA);
    // Rozdělení jména
    strtok(toml_konfigurace->jmeno, " ");
    char *prijmeni = strtok(NULL, " ");
    // V případě že existuje nějaký výstup
    if (prijmeni) {
        // _Jungmann třeba
        strcat(pdf_soubor, u8"_");
        strcat(pdf_soubor, prijmeni);
    }
    // Přidání koncovky
    strcat(pdf_soubor, u8".pdf");
    // Uložení PDF do souboru
    HPDF_SaveToFile(*pdf, pdf_soubor);
    // Vyčištění paměti
    free(pdf_soubor);
    return 0;
}

int ulozit_do_stdout (SSPS_DOHODA_PDF *pdf) {
    // Uložení PDF do paměti
    HPDF_SaveToStream(*pdf);
    // Přetočení PDF streamu na začátek (pro postupné vypsání do stdout)
    HPDF_ResetStream(*pdf);
    // Vypsání do stdout
    for (;;) {
        HPDF_BYTE buffer[4096];
        HPDF_UINT32 buffer_len = sizeof(buffer);
        HPDF_ReadFromStream(*pdf, buffer, &buffer_len);
        if (buffer_len == 0)
            break;
        if (fwrite(buffer, buffer_len, 1, stdout) != 1) {
            return 1;
        }
    }
    return 0;
}

void cli_error_handler(char *error) {
    fprintf(stderr, u8"%s\n", error);
    longjmp(jmp, 1);
}