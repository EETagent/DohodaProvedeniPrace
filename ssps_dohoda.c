#include <stdio.h> // Standardní vstup a výstup
#include <stdbool.h> // Boolean datové typy
#include <string.h> // Práce s textovými řetězci
#include <setjmp.h> // Zpracovávání chyb přes jump

#include <hpdf.h> // libharu, tvorba PDF
#include <toml.h> // tomlc99, parsování TOML

#include <hpdf_ttf_mem.h> // Funkce pro načítání TTF souborů z paměti
#include <liberationsans.h> // Liberation Sans Regular a Bold jako byty v C hlavičce, xxd -i font.ttf > header.h

#include <ssps_dohoda.h>

// Maximální počet položek na hlavní stránce A4
#define MAX_POLOZEK 14

// Maximální počet znaků, který se vejde do políček tabulky
#define MALE_POLICKO_VELIKOST 8
#define VELKE_POLICKO_VELIKOST 70

// Písmo
#define FONT_VELKY 25
#define FONT_NORMALNI 12
#define FONT_MALY 10

// Nadpis dokumentu
#define NADPIS u8"VÝKAZ PRÁCE"
// Podnadpis dokumentu
#define PODNADPIS u8"DOHODA O PROVEDENÍ PRÁCE | DOHODA O PRACOVNÍ ČINNOSTI"

// Popisky
#define NAZEV_TEXT u8"Název dohody: "
#define JMENO_TEXT u8"Jméno, příjmení zaměstnance: "
#define RODNECISLO_TEXT u8"Rodné číslo: "
#define BANKA_TEXT u8"Č. účtu/kód banky: "
#define MISTO_TEXT u8"Místo narození: "
#define ADRESA_TEXT u8"Adresa zaměstnance: "
#define POJISTOVNA_TEXT u8"Zdravotní pojištovna - název a číslo: "

#define POPISEK_ODSAZENI 160
#define POPISEK_ROZESTUPY 30

// Výpočet odsazení pro umístění do levé poloviny dokumentu
#define LEVA_POLOVINA_DOKUMENTU(stranka) HPDF_Page_GetWidth(stranka) / 8
// Výpočet odsazení pro umístění do prostřed dokumentu
#define PROSTREDEK_DOKUMENTU(stranka, text) (HPDF_Page_GetWidth(stranka) - HPDF_Page_TextWidth(stranka, text)) / 2
// Výpočet odsazení pro umístění do prostřed dokumentu
#define PRAVA_POLOVINA_DOKUMENTU(stranka) HPDF_Page_GetWidth(stranka) - 300

#if defined(_WIN32) || defined(_WIN64)
char *strndup( const char *s1, size_t n)
{
    char *copy= (char*)malloc( n+1 );
    memcpy( copy, s1, n );
    copy[n] = 0;
    return copy;
};
#endif

// Seřadí prvky v toml_datum_t prace_polozky[][4]
int toml_datum_t_bubblesort(toml_datum_t prace_polozky[][4], unsigned int len, SSPS_DOHODA_RAZENI_POLOZEK razeni);

// Porovná dvě data ve formátu dd.mm.
int porovnani_mesicu_dnu(char *datum_1, char *datum_2);

int utf8_bytelen_podle_poctu_znaku(const char * s, int pocet_znaku);

// Přidání textu do dokumentu
void HPDF_Page_AddText(HPDF_Page page, HPDF_REAL x, HPDF_REAL y, char *text);

// Přidání popisku pro SSPŠ dohodu
void SSPS_Page_AddPopisek(HPDF_Page stranka, char *konf, char *text, HPDF_REAL x, HPDF_REAL y);

// Přidání položky pro SSPŠ dohodu
void SSPS_Page_AddPolozka(HPDF_Page stranka, HPDF_REAL x, HPDF_REAL y, HPDF_REAL vyska, char *datum, char *cinnost,
                          char *hodiny, char *poznamka);


// Jumpy
jmp_buf jmp_pdf, jmp_toml;

// Chyby při práci s PDF, zajímavá věc ten longjmp a setjmp
void pdf_error_handler(HPDF_STATUS error, __attribute__((unused)) void *detail, __attribute__((unused)) void *data) {
    fprintf(stderr, u8"CHYBA: %04X\n" \
    "http://libharu.sourceforge.net/error_handling.html#The_list_of_error_code_\n", (HPDF_UINT) error);
    longjmp(jmp_pdf, 1);
}

// Chyby při práci s TOML
void toml_error_handler(char *error) {
    fprintf(stderr, u8"CHYBA: %s\n", error);
    longjmp(jmp_toml, 1);
}

// Konfigurace dokumentu přes TOML, výstup uložen do konfigurace_in
int SSPS_DOHODA_Konfigurace_TOML(void *vstup, SSPS_DOHODA_Konfigurace *konfigurace_in, SSPS_DOHODA_VSTUP_TYP typ, SSPS_DOHODA_RAZENI_POLOZEK razeni_polozek) {
    toml_table_t *toml;
    char errbuf[50];
    // Zpracování TOMLU podle datového typu
    switch (typ) {
        case SOUBOR:
            // Parsování TOML ze souboru
            toml = toml_parse_file((FILE *) vstup, errbuf, sizeof(errbuf));
            break;
        case STRING:
            // Parsování TOML z textového řetězce
            toml = toml_parse((char *) vstup, errbuf, sizeof(errbuf));
            break;
        default:
            return 1;
    }

    if (!toml) {
        fprintf(stderr, u8"CHYBA: %s\n", errbuf);
        return 1;
    }
    // V případě chyby kdekoliv během zpracovávání tomlu
    if (setjmp(jmp_toml)) {
        toml_free(toml);
        return 1;
    }
    // Část [dohoda]
    toml_table_t *dohoda_tabulka = toml_table_in(toml, u8"dohoda");
    if (!dohoda_tabulka)
        toml_error_handler(u8"Pole [dohoda] nenalezeno");
    // Část [zamestnanec]
    toml_table_t *zamestnanec_tabulka = toml_table_in(toml, u8"zamestnanec");
    if (!zamestnanec_tabulka)
        toml_error_handler(u8"Pole [zamestnanec] nenalezeno");

    // Hodnota název v části [dohoda]
    toml_datum_t nazev_hodnota = toml_string_in(dohoda_tabulka, u8"nazev");
    if (!nazev_hodnota.ok)
        toml_error_handler(u8"Položká nazev = nenalezena");
    // Hodnota kde v části [dohoda]
    toml_datum_t kde_hodnota = toml_string_in(dohoda_tabulka, u8"kde");
    if (!kde_hodnota.ok)
        toml_error_handler(u8"Položká kde = nenalezena");

    // Hodnoty v části [zamestnanec]
    // zamestnanec_polozky[0] = jmeno
    // zamestnanec_polozky[1] = rodne_cislo
    // zamestnanec_polozky[2] = banka
    // zamestnanec_polozky[3] = misto_narozeni
    // zamestnanec_polozky[4] = adresa
    // zamestnanec_polozky[5] = pojistovna
    toml_datum_t zamestnanec_polozky[6];
    for (int i = 0; i < 6; i++) {
        const char *key = toml_key_in(zamestnanec_tabulka, i);
        if (!key)
            toml_error_handler(u8"V tabulce [zamestnanec] nebyly nalezeny všechny položky");
        zamestnanec_polozky[i] = toml_string_in(zamestnanec_tabulka, key);
    }

    // List částí [[prace]], není znám jejich přesný počet
    toml_array_t *prace_tabulky = toml_array_in(toml, u8"prace");
    if (!prace_tabulky)
        toml_error_handler(u8"Tabulka [[prace]] nenalezena");
    // Počet instanci [[prace]]
    unsigned int prace_velikost = toml_array_nelem(prace_tabulky);

    // Hodnoty v části [[prace]]
    // prace_polozky[0][0] = datum
    // prace_polozky[0][1] = cinnost
    // prace_polozky[0][2] = hodiny
    // prace_polozky[0][3] = poznamka
    // prace_polozky[1][0] = datum v druhé části [[prace]]
    // atd ...
    toml_datum_t prace_polozky[prace_velikost][4];
    for (int i = 0; i < prace_velikost; i++) {
        toml_table_t *tabulka = toml_table_at(prace_tabulky, i);
        for (int o = 0; o < 4; o++) {
            const char *key = toml_key_in(tabulka, o);
            if (!key)
                toml_error_handler(u8"V tabulce [[prace]] nebyly nalezeny všechny položky");
            prace_polozky[i][o] = toml_string_in(tabulka, key);
        }
    }

    // Není li nastaveno NEŘADIT
    if (razeni_polozek != NERADIT) {
        // Seřazení položek algoritmem bubblesort
        if (toml_datum_t_bubblesort(prace_polozky, prace_velikost, razeni_polozek) == 1)
            toml_error_handler(u8"Nepovedlo se seřadit položky v dokumentu");
    }

    // Konfigurace
    SSPS_DOHODA_Konfigurace konf;
    // Počet všech vytvořených položek
    konf.len = prace_velikost;

    // Inicializace všech 4 položek podle jejich počtu v TOML konfiguraci
    // Nejprve hodnota[x] az hodnota[y]
    konf.datum = malloc(konf.len * sizeof (char *));
    konf.cinnost = malloc(konf.len * sizeof (char *));
    konf.hodiny = malloc(konf.len * sizeof (char *));
    konf.poznamka = malloc(konf.len * sizeof (char *));

    // Údaje o dohodě
    konf.nazev = strndup(nazev_hodnota.u.s, utf8_bytelen_podle_poctu_znaku(nazev_hodnota.u.s, 55));
    konf.kde = strndup(kde_hodnota.u.s, utf8_bytelen_podle_poctu_znaku(kde_hodnota.u.s, 30));
    // Údaje o zaměstnanci
    konf.jmeno = strndup(zamestnanec_polozky[0].u.s, utf8_bytelen_podle_poctu_znaku(zamestnanec_polozky[0].u.s, 50));
    konf.rodne_cislo = strndup(zamestnanec_polozky[1].u.s, utf8_bytelen_podle_poctu_znaku(zamestnanec_polozky[1].u.s, 25));
    konf.banka = strndup(zamestnanec_polozky[2].u.s, utf8_bytelen_podle_poctu_znaku(zamestnanec_polozky[2].u.s, 25));
    konf.misto_narozeni = strndup(zamestnanec_polozky[3].u.s, utf8_bytelen_podle_poctu_znaku(zamestnanec_polozky[3].u.s, 55));
    konf.adresa = strndup(zamestnanec_polozky[4].u.s, utf8_bytelen_podle_poctu_znaku(zamestnanec_polozky[4].u.s, 55));
    konf.pojistovna = strndup(zamestnanec_polozky[5].u.s, utf8_bytelen_podle_poctu_znaku(zamestnanec_polozky[5].u.s, 45));

    // Údaje o jednotlivých činnostech
    for (int i = 0; i < konf.len; i++) {
        // Inicializace všech vnořených polí
        // hodnota[x][x] az hodnota[y][y]
        konf.datum[i] = strndup(prace_polozky[i][0].u.s, utf8_bytelen_podle_poctu_znaku(prace_polozky[i][0].u.s, MALE_POLICKO_VELIKOST));
        konf.cinnost[i] = strndup(prace_polozky[i][1].u.s, utf8_bytelen_podle_poctu_znaku(prace_polozky[i][1].u.s, VELKE_POLICKO_VELIKOST));
        konf.hodiny[i] = strndup(prace_polozky[i][2].u.s, utf8_bytelen_podle_poctu_znaku(prace_polozky[i][2].u.s, MALE_POLICKO_VELIKOST));
        konf.poznamka[i] = strndup(prace_polozky[i][3].u.s, utf8_bytelen_podle_poctu_znaku(prace_polozky[i][3].u.s, MALE_POLICKO_VELIKOST));
    }

    // Vyčištění paměti zabrané stringy ve struktuře
    free(nazev_hodnota.u.s);
    free(kde_hodnota.u.s);
    for (int i = 0; i < 6; i++) {
        free(zamestnanec_polozky[i].u.s);
    }
    for (int i = 0; i < prace_velikost; i++) {
        for (int o = 0; o < 4; o++) {
            free(prace_polozky[i][o].u.s);
        }
    }

    toml_free(toml);

    *konfigurace_in = konf;

    return 0;
}

// Vyčištění paměti alokované v rámci struktury SSPS_DOHODA_Konfigurace
int SSPS_DOHODA_Konfigurace_Free(SSPS_DOHODA_Konfigurace *konfigurace) {
    free(konfigurace->datum); free(konfigurace->cinnost); free(konfigurace->poznamka); free(konfigurace->hodiny);
    free(konfigurace->nazev); free(konfigurace->kde);
    free(konfigurace->jmeno); free(konfigurace->adresa); free(konfigurace->rodne_cislo); free(konfigurace->misto_narozeni);
    free(konfigurace->banka); free(konfigurace->pojistovna);
    return 0;
}

// Funkce pro vypsání celkového počtu odpracovaných hodin
int SSPS_DOHODA_PocetHodin(SSPS_DOHODA_Konfigurace toml_konfigurace, float *hodiny) {
    char *endptr;
    for (int i = 0; i < toml_konfigurace.len; ++i) {
        // Odstranění mezer z textového řetězce
        char trim[strlen(toml_konfigurace.hodiny[i]) + 1];
        int j=0;
        for(int o = 0; toml_konfigurace.hodiny[i][o]!='\0'; o++)
        {
            if (toml_konfigurace.hodiny[i][o] != ' ' && toml_konfigurace.hodiny[i][o] != '\t')
                trim[j++]=toml_konfigurace.hodiny[i][o];
        }
        trim[j]='\0';
        toml_konfigurace.hodiny[i] = trim;
        // Převod do typu float
        float prace_cas = strtof(toml_konfigurace.hodiny[i], &endptr);
        if (*endptr != '\0' && strcmp(endptr, "h") != 0) {
            return 1;
        }
        *hodiny += prace_cas;
    }
    return 0;
}

// Funkce pro vypsání celkové částky za odpracované hodiny
int SSPS_DOHODA_PocetPenez(SSPS_DOHODA_Konfigurace toml_konfigurace, float *penize) {
    float hodiny;
    if (SSPS_DOHODA_PocetHodin(toml_konfigurace, &hodiny) != 0)
        return 1;
    *penize = hodiny * SABLOVA_KONSTANTA;
    return 0;
}

// Funkce pro vytvoření dohody ve formě PDF, výstup uložen do pdf_in
int SSPS_DOHODA_SepsatDohodu(SSPS_DOHODA_Konfigurace toml_konfigurace, HPDF_Doc *pdf_in, bool zastupkyne_reditele_legacy) {

    // PDF dokument
    HPDF_Doc pdf;

    // Hlavní stránka PDF
    HPDF_Page pdf_strana;
    // Dva druhy písma, normální a tlusté
    HPDF_Font pismo_regular, pismo_bold;

    // Vytvoření PDF
    pdf = HPDF_New((HPDF_Error_Handler) pdf_error_handler, NULL);
    if (!pdf) {
        fprintf(stderr, u8"Chyba: Nelze vytvořit PDF\n");
        return 1;
    }
    // V případě chyby kdekoliv při zpracovávání PDF nás longjmp hodí zase sem
    if (setjmp(jmp_pdf)) {
        HPDF_Free(pdf);
        return 1;
    }

    // UTF8 - Důležité pro podporu českého jazyka
    HPDF_UseUTFEncodings(pdf);
    HPDF_SetCurrentEncoder(pdf, u8"UTF-8");

    // Vlastnosti PDF
    HPDF_SetInfoAttr(pdf, HPDF_INFO_TITLE, u8"Dohoda o provedení práce");
    HPDF_SetInfoAttr(pdf, HPDF_INFO_SUBJECT, u8"Dohoda o provedení práce");
    HPDF_SetInfoAttr(pdf, HPDF_INFO_CREATOR, u8"Smíchovská střední průmyslová škola");

    // První stránka
    pdf_strana = HPDF_AddPage(pdf);
    // Rozměry stránky A4 na výšku
    HPDF_Page_SetSize(pdf_strana, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
    // Font Liberation Sans, kompatibilní s Arial, normální i tlustý, načítání z paměti
    pismo_regular = HPDF_GetFont(pdf, HPDF_LoadTTFontFromMemory(pdf, LiberationSans_Regular_ttf,
                                                                LiberationSans_Regular_ttf_len, HPDF_TRUE), "UTF-8");
    pismo_bold = HPDF_GetFont(pdf, HPDF_LoadTTFontFromMemory(pdf, LiberationSans_Bold_ttf, LiberationSans_Bold_ttf_len,
                                                             HPDF_TRUE), "UTF-8");

    HPDF_Page_SetFontAndSize(pdf_strana, pismo_bold, FONT_VELKY);
    HPDF_Page_AddText(pdf_strana, PROSTREDEK_DOKUMENTU(pdf_strana, NADPIS), HPDF_Page_GetHeight(pdf_strana) - 100,
                      NADPIS);

    HPDF_Page_SetFontAndSize(pdf_strana, pismo_bold, FONT_NORMALNI);
    HPDF_Page_AddText(pdf_strana, PROSTREDEK_DOKUMENTU(pdf_strana, PODNADPIS), HPDF_Page_GetHeight(pdf_strana) - 130,
                      PODNADPIS);

    // Údaje o zaměstnanci
    HPDF_Page_SetFontAndSize(pdf_strana, pismo_regular, FONT_NORMALNI);

    // DRY
    // Popisky přes for loop
    char *ukazatele_hodnoty[6] = {toml_konfigurace.nazev, toml_konfigurace.jmeno, toml_konfigurace.rodne_cislo,
                                  toml_konfigurace.misto_narozeni, toml_konfigurace.adresa,
                                  toml_konfigurace.pojistovna};
    char *ukazatele_popisky[6] = {NAZEV_TEXT, JMENO_TEXT, RODNECISLO_TEXT, MISTO_TEXT, ADRESA_TEXT, POJISTOVNA_TEXT};
    for (int i = 0; i < 6; ++i) {
        SSPS_Page_AddPopisek(pdf_strana, ukazatele_hodnoty[i], ukazatele_popisky[i],
                             LEVA_POLOVINA_DOKUMENTU(pdf_strana),
                             HPDF_Page_GetHeight(pdf_strana) - POPISEK_ODSAZENI - POPISEK_ROZESTUPY * i);
    }
    // Odsazený poposek nekompatibilní se smyčkou
    SSPS_Page_AddPopisek(pdf_strana, toml_konfigurace.banka, BANKA_TEXT, PRAVA_POLOVINA_DOKUMENTU(pdf_strana),
                         HPDF_Page_GetHeight(pdf_strana) - POPISEK_ODSAZENI - POPISEK_ROZESTUPY * 2);

    // Položky
    HPDF_Page_SetFontAndSize(pdf_strana, pismo_regular, FONT_MALY);
    HPDF_Page_SetLineWidth(pdf_strana, 0.5f);
    // Referenční položka
    SSPS_Page_AddPolozka(pdf_strana, LEVA_POLOVINA_DOKUMENTU(pdf_strana), HPDF_Page_GetHeight(pdf_strana) - 330, 35,
                         u8"Datum",
                         u8"Činnost,", u8"Hodiny", u8"Pozn.");

    // Odsazení pro prázdné stránky v druhém for
    int odsazeni = 0;
    // Maximálně 14 zobrazených položek na hlavní stránce
    for (int i = 0; i < MAX_POLOZEK; ++i) {
        // Existuje-li obsah
        if (i < toml_konfigurace.len)
            SSPS_Page_AddPolozka(pdf_strana, LEVA_POLOVINA_DOKUMENTU(pdf_strana),
                                 HPDF_Page_GetHeight(pdf_strana) - 365 - (float) i * 20, 20, toml_konfigurace.datum[i],
                                 toml_konfigurace.cinnost[i],
                                 toml_konfigurace.hodiny[i], toml_konfigurace.poznamka[i]);
            // Prázdné řádky
        else
            SSPS_Page_AddPolozka(pdf_strana, LEVA_POLOVINA_DOKUMENTU(pdf_strana),
                                 HPDF_Page_GetHeight(pdf_strana) - 365 - (float) odsazeni - (float) i * 20, 20, "", "",
                                 "", "");
    }

    // Podpis zaměstnance, neměnný
    HPDF_Page_SetFontAndSize(pdf_strana, pismo_regular, FONT_NORMALNI);
    // V jakém městě byla dohoda podepsána
    SSPS_Page_AddPopisek(pdf_strana, u8" dne", toml_konfigurace.kde, LEVA_POLOVINA_DOKUMENTU(pdf_strana),
                         HPDF_Page_GetHeight(pdf_strana) - 680);
    HPDF_Page_AddText(pdf_strana, PRAVA_POLOVINA_DOKUMENTU(pdf_strana), HPDF_Page_GetHeight(pdf_strana) - 680,
                      u8"Podpis zaměstnance: ...............................");
    // Podpis ředitele školy, neměnný
    HPDF_Page_AddText(pdf_strana, LEVA_POLOVINA_DOKUMENTU(pdf_strana), HPDF_Page_GetHeight(pdf_strana) - 730,
                      u8"Schválení ředitelem školy");
    HPDF_Page_AddText(pdf_strana, PRAVA_POLOVINA_DOKUMENTU(pdf_strana), HPDF_Page_GetHeight(pdf_strana) - 730,
                      u8"Podpis ředitele školy: ...............................");
    // Informace pro odevzdání dokumentu, neměnné
    HPDF_Page_SetFontAndSize(pdf_strana, pismo_regular, FONT_NORMALNI - 1);
    HPDF_Page_AddText(pdf_strana, LEVA_POLOVINA_DOKUMENTU(pdf_strana), HPDF_Page_GetHeight(pdf_strana) - 760,
                      u8"Kompletně vyplněný a podepsaný výkaz je nutné odevzdat vždy do 26. v měsíci");
    HPDF_Page_AddText(pdf_strana, LEVA_POLOVINA_DOKUMENTU(pdf_strana), HPDF_Page_GetHeight(pdf_strana) - 780,
                      u8"Výkaz za prosinec - do 20. prosince");
    
    if (zastupkyne_reditele_legacy) {
        HPDF_Page_AddText(pdf_strana, LEVA_POLOVINA_DOKUMENTU(pdf_strana), HPDF_Page_GetHeight(pdf_strana) - 800,
                          u8"Vždy lze (po řádném vyplnění a podepsání) zaslat jako sken na: dita.binderova@ssps.cz");
    }
    else {
        HPDF_Page_AddText(pdf_strana, LEVA_POLOVINA_DOKUMENTU(pdf_strana), HPDF_Page_GetHeight(pdf_strana) - 800,
                          u8"Vždy lze (po řádném vyplnění a podepsání) zaslat jako sken na: aramis.tochjan@ssps.cz");
    }

    *pdf_in = pdf;

    return 0;
}

/*
 * DRY
 * Přidání textu do dokumentu
 * Stejné značení jako funkce z knihovny libharu pro lepší čitelnost kódu a jasnou odlišitelnost od kompletně custom funkcí níže
 */
inline void HPDF_Page_AddText(HPDF_Page page, HPDF_REAL x, HPDF_REAL y, char *text) {
    HPDF_REAL font_size = HPDF_Page_GetCurrentFontSize(page);
    HPDF_Font font = HPDF_Page_GetCurrentFont(page);
    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, font, font_size);
    HPDF_Page_MoveTextPos(page, x, y);
    HPDF_Page_ShowText(page, text);
    HPDF_Page_EndText(page);
}

/*
 * DRY
 * Přidání popisku (Například pojišťovna zaměstnance) do SSPŠ dohody
 */
inline void SSPS_Page_AddPopisek(HPDF_Page stranka, char *konf, char *text, HPDF_REAL x, HPDF_REAL y) {
    char *popisek = malloc(strlen(text) + strlen(konf) + 1);
    strcpy(popisek, text);
    strcat(popisek, konf);
    HPDF_Page_AddText(stranka, x, y, popisek);
    free(popisek);
}

/*
 * DRY
 * Přidání ohraničené pracovní položky do SSPŠ dohody
 */
inline void
SSPS_Page_AddPolozka(HPDF_Page stranka, HPDF_REAL x, HPDF_REAL y, HPDF_REAL vyska, char *datum, char *cinnost,
                     char *hodiny, char *poznamka) {
    HPDF_Page_Rectangle(stranka, x, y, 450, -vyska);
    HPDF_Page_Rectangle(stranka, x, y, 50, -vyska);
    HPDF_Page_Rectangle(stranka, x + 50, y, 300, -vyska);
    HPDF_Page_Rectangle(stranka, x + 350, y, 50, -vyska);
    HPDF_Page_Rectangle(stranka, x + 400, y, 50, -vyska);

    HPDF_Page_Stroke(stranka);

    HPDF_Page_AddText(stranka, x + 5, y - 15, datum);
    HPDF_Page_AddText(stranka, x + 55, y - 15, cinnost);
    HPDF_Page_AddText(stranka, x + 355, y - 15, hodiny);
    HPDF_Page_AddText(stranka, x + 405, y - 15, poznamka);
}

/*
 * Seřadí prvky v toml_datum_t prace_polozky[][4]
 * Algoritmus Bubblesort
 * Vrací 0 v případě úspěšného provedení, 1 když nastane chyba
 */
int toml_datum_t_bubblesort(toml_datum_t prace_polozky[][4], unsigned int len, SSPS_DOHODA_RAZENI_POLOZEK razeni) {
    int je_serazeno = 0;
    int zpusob_razeni;

    // Opakování smyšky dokud nebudou veškeré položky seřazeny
    while (je_serazeno != 1) {
        je_serazeno = 1;
        for (int i = 0; i < len - 1; i++) {
            // Které datum je větší?
            int t = porovnani_mesicu_dnu(prace_polozky[i][0].u.s, prace_polozky[i + 1][0].u.s);
            // Chyba během porovávání
            if (t == 2)
                return 1;
            switch (razeni) {
                // Řazení položek od nejstarší
                case OD_NEJSTARSIHO:
                    zpusob_razeni = 0;
                    break;
                // Řazení položek od nejnovější
                case OD_NEJNOVEJSIHO:
                    zpusob_razeni = 1;
                    break;
                default:
                    break;
            }
            // provnani_dat vrací 0 při >, 1 při <
            // Přes zpusob_razeni tak realizujeme SSPS_DOHODA_RAZENI_POLOZEK
            if (t == zpusob_razeni) {
                // Prohození všech čtyř údaju položky práce
                char *temp[4] = {prace_polozky[i][0].u.s, prace_polozky[i][1].u.s, prace_polozky[i][2].u.s,
                                 prace_polozky[i][3].u.s};
                for (int o = 0; o < 4; o++) {
                    prace_polozky[i][o].u.s = prace_polozky[i + 1][o].u.s;
                    prace_polozky[i + 1][o].u.s = temp[o];
                }
                je_serazeno = 0;
            }
        }
    }
    return 0;
}

/*
 * Porovná dvě data ve formátu dd.mm.
 * Vrátí nulu v případě, že je datum_1 větší, jedna je-li větší datum_2
 * Při chybě během porovnání vrací funkce číslo 2
 */
int porovnani_mesicu_dnu(char *datum_1, char *datum_2) {
    char *endptr;

    // Budeme modifikovat lokální textové řetězce
    char prvnidatum[6], druhedatum[6];
    strncpy(prvnidatum, datum_1, 6);
    strncpy(druhedatum, datum_2, 6);

    jmp_buf jump;
    // V případě chyby
    if (setjmp(jump) == 1) {
        return 2;
    }

    int prvnidatum_den = (int) strtol(strtok(prvnidatum, "."), &endptr, 10);
    // Pokud nebyl převeden celý vstup
    if (*endptr != '\0') {
        longjmp(jump, 1);
    }
    int prvnidatum_mesic = (int) strtol(strtok(NULL, "."), &endptr, 10);
    if (*endptr != '\0') {
        longjmp(jump, 1);
    }
    int druhedatum_den = (int) strtol(strtok(druhedatum, "."), &endptr, 10);
    if (*endptr != '\0') {
        longjmp(jump, 1);
    }
    int druhedatum_mesic = (int) strtol(strtok(NULL, "."), &endptr, 10);
    if (*endptr != '\0') {
        longjmp(jump, 1);
    }

    // První měsíc je větší než druhý měsíc
    if (prvnidatum_mesic > druhedatum_mesic)
        return 0;
    else if (prvnidatum_mesic == druhedatum_mesic) {
        // V případě, že jsou měsíce stejné a den u prvního data je větší
        if (prvnidatum_den > druhedatum_den)
            return 0;
    }
    // Jinak je větší druhé datum
    return 1;
}

int utf8_bytelen_podle_poctu_znaku(const char * s, int pocet_znaku) {
    int i = 0, len = 0, c = 0;
    while(s[i]) {
        ++c; ++i;
        // s[i] není část multibyte znaku
        if ((s[i] & 0xc0) != 0x80) {
            ++len;
            if(len == pocet_znaku) {
                break;
            }
        }
    }
    return c;
}