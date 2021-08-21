#include <stdio.h> // Standardní vstup a výstup
#include <string.h> // Práce s textovými řetězci
#include <setjmp.h> // Zpracovávání chyb přes jump

#include <hpdf.h> // libharu, tvorba PDF
#include <toml.h> // tomlc99, parsování TOML

#include <hpdf_ttf_mem.h> // Funkce pro načítání TTF souborů z paměti
#include <liberationsans.h> // Liberation Sans Regular a Bold jako byty v C hlavičce, xxd -i font.ttf > header.h

#include <ssps_dohoda.h>

// Maximální počet položek na hlavní stránce A4
#define MAX_POLOZEK 14

// Písmo
#define FONT_VELKY 25
#define FONT_NORMALNI 12
#define FONT_MALY 10

// Nadpis dokumentu
#define NADPIS "VÝKAZ PRÁCE"
// Podnadpis dokumentu
#define PODNADPIS "DOHODA O PROVEDENÍ PRÁCE | DOHODA O PRACOVNÍ ČINNOSTI"

// Popisky
#define NAZEV_TEXT "Název dohody: "
#define JMENO_TEXT "Jméno, příjmení zaměstnance: "
#define RODNECISLO_TEXT "Rodné číslo: "
#define BANKA_TEXT "Č. účtu/kód banky: "
#define MISTO_TEXT "Místo narození: "
#define ADRESA_TEXT "Adresa zaměstnance: "
#define POJISTOVNA_TEXT "Zdravotní pojištovna - název a číslo: "

#define POPISEK_ODSAZENI 160
#define POPISEK_ROZESTUPY 30

// Výpočet odsazení pro umístění do levé poloviny dokumentu
#define LEVA_POLOVINA_DOKUMENTU(stranka) HPDF_Page_GetWidth(stranka) / 8
// Výpočet odsazení pro umístění do prostřed dokumentu
#define PROSTREDEK_DOKUMENTU(stranka, text) (HPDF_Page_GetWidth(stranka) - HPDF_Page_TextWidth(stranka, text)) / 2
// Výpočet odsazení pro umístění do prostřed dokumentu
#define PRAVA_POLOVINA_DOKUMENTU(stranka) HPDF_Page_GetWidth(stranka) - 300

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
    fprintf(stderr, "CHYBA: %04X\n" \
    "http://libharu.sourceforge.net/error_handling.html#The_list_of_error_code_\n", (HPDF_UINT) error);
    longjmp(jmp_pdf, 1);
}

// Chyby při práci s TOML
void toml_error_handler(char *error) {
    fprintf(stderr, "CHYBA: %s\n", error);
    longjmp(jmp_toml, 1);
}

// Konfigurace dokumentu přes TOML, výstup uložen do konfigurace_in
int SSPS_DOHODA_Konfigurace_TOML(void *vstup, SSPS_DOHODA_Konfigurace *konfigurace_in, SSPS_DOHODA_VSTUP_TYP typ) {
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
        fprintf(stderr, "CHYBA: %s\n", errbuf);
        return 1;
    }
    // V případě chyby kdekoliv během zpracovávání tomlu
    if (setjmp(jmp_toml)) {
        toml_free(toml);
        return 1;
    }
    // Část [dohoda]
    toml_table_t *dohoda_tabulka = toml_table_in(toml, "dohoda");
    if (!dohoda_tabulka)
        toml_error_handler("Pole [dohoda] nenalezeno");
    // Část [zamestnanec]
    toml_table_t *zamestnanec_tabulka = toml_table_in(toml, "zamestnanec");
    if (!zamestnanec_tabulka)
        toml_error_handler("Pole [zamestnanec] nenalezeno");

    // Hodnota název v části [dohoda]
    toml_datum_t nazev_hodnota = toml_string_in(dohoda_tabulka, "nazev");
    if (!nazev_hodnota.ok)
        toml_error_handler("Položká nazev = nenalezena");
    // Hodnota kde v části [dohoda]
    toml_datum_t kde_hodnota = toml_string_in(dohoda_tabulka, "kde");
    if (!kde_hodnota.ok)
        toml_error_handler("Položká kde = nenalezena");

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
            toml_error_handler("V tabulce [zamestnanec] nebyly nalezeny všechny položky");
        zamestnanec_polozky[i] = toml_string_in(zamestnanec_tabulka, key);
    }

    // List částí [[prace]], není znám jejich přesný počet
    toml_array_t *prace_tabulky = toml_array_in(toml, "prace");
    if (!prace_tabulky)
        toml_error_handler("Tabulka [[prace]] nenalezena");
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
                toml_error_handler("V tabulce [[prace]] nebyly nalezeny všechny položky");
            prace_polozky[i][o] = toml_string_in(tabulka, key);
        }
    }

    // Konfigurace
    SSPS_DOHODA_Konfigurace konf;
    // Počet všech vytvořených položek
    konf.len = prace_velikost;
    // Údaje o dohodě
    strncpy(konf.nazev, nazev_hodnota.u.s, sizeof(konf.nazev));
    strncpy(konf.kde, kde_hodnota.u.s, sizeof(konf.kde));
    // Údaje o zaměstnanci
    strncpy(konf.jmeno, zamestnanec_polozky[0].u.s, sizeof(konf.jmeno));
    strncpy(konf.rodne_cislo, zamestnanec_polozky[1].u.s, sizeof(konf.rodne_cislo));
    strncpy(konf.banka, zamestnanec_polozky[2].u.s, sizeof(konf.banka));
    strncpy(konf.misto_narozeni, zamestnanec_polozky[3].u.s, sizeof(konf.misto_narozeni));
    strncpy(konf.adresa, zamestnanec_polozky[4].u.s, sizeof(konf.adresa));
    strncpy(konf.pojistovna, zamestnanec_polozky[5].u.s, sizeof(konf.pojistovna));
    // Údaje o jednotlivých činnostech
    for (int i = 0; i < prace_velikost && i < MAX_POLE; i++) {
        strncpy(konf.datum[i], prace_polozky[i][0].u.s, sizeof(konf.datum[i]));
        strncpy(konf.cinnost[i], prace_polozky[i][1].u.s, sizeof(konf.cinnost[i]));
        strncpy(konf.hodiny[i], prace_polozky[i][2].u.s, sizeof(konf.hodiny[i]));
        strncpy(konf.poznamka[i], prace_polozky[i][3].u.s, sizeof(konf.hodiny[i]));
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

// Funkce pro vytvoření dohody ve formě PDF, výstup uložen do pdf_in
int SSPS_DOHODA_SepsatDohodu(SSPS_DOHODA_Konfigurace toml_konfigurace, HPDF_Doc *pdf_in) {

    // PDF dokument
    HPDF_Doc pdf;

    // Hlavní stránka PDF
    HPDF_Page pdf_strana;
    // Dva druhy písma, normální a tlusté
    HPDF_Font pismo_regular, pismo_bold;

    // Vytvoření PDF
    pdf = HPDF_New((HPDF_Error_Handler) pdf_error_handler, NULL);
    if (!pdf) {
        fprintf(stderr, "Chyba: Nelze vytvořit PDF\n");
        return 1;
    }
    // V případě chyby kdekoliv při zpracovávání PDF nás longjmp hodí zase sem
    if (setjmp(jmp_pdf)) {
        HPDF_Free(pdf);
        return 1;
    }

    // UTF8 - Důležité pro podporu českého jazyka
    HPDF_UseUTFEncodings(pdf);
    HPDF_SetCurrentEncoder(pdf, "UTF-8");

    // Vlastnosti PDF
    HPDF_SetInfoAttr(pdf, HPDF_INFO_TITLE, "Dohoda o provedení práce");
    HPDF_SetInfoAttr(pdf, HPDF_INFO_SUBJECT, "Dohoda o provedení práce");
    HPDF_SetInfoAttr(pdf, HPDF_INFO_CREATOR, "Smíchovská střední průmyslová škola");

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
                         "Datum",
                         "Činnost,", "Hodiny", "Pozn.");

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
    SSPS_Page_AddPopisek(pdf_strana, " dne", toml_konfigurace.kde, LEVA_POLOVINA_DOKUMENTU(pdf_strana),
                         HPDF_Page_GetHeight(pdf_strana) - 680);
    HPDF_Page_AddText(pdf_strana, PRAVA_POLOVINA_DOKUMENTU(pdf_strana), HPDF_Page_GetHeight(pdf_strana) - 680,
                      "Podpis zaměstnance: ...............................");
    // Podpis ředitele školy, neměnný
    HPDF_Page_AddText(pdf_strana, LEVA_POLOVINA_DOKUMENTU(pdf_strana), HPDF_Page_GetHeight(pdf_strana) - 730,
                      "Schválení ředitelem školy");
    HPDF_Page_AddText(pdf_strana, PRAVA_POLOVINA_DOKUMENTU(pdf_strana), HPDF_Page_GetHeight(pdf_strana) - 730,
                      "Podpis ředitele školy: ...............................");
    // Informace pro odevzdání dokumentu, neměnné
    HPDF_Page_SetFontAndSize(pdf_strana, pismo_regular, FONT_NORMALNI - 1);
    HPDF_Page_AddText(pdf_strana, LEVA_POLOVINA_DOKUMENTU(pdf_strana), HPDF_Page_GetHeight(pdf_strana) - 760,
                      "Kompletně vyplněný a podepsaný výkaz je nutné odevzdat vždy do 26. v měsíci");
    HPDF_Page_AddText(pdf_strana, LEVA_POLOVINA_DOKUMENTU(pdf_strana), HPDF_Page_GetHeight(pdf_strana) - 780,
                      "Výkaz za prosinec - do 20. prosince");
    HPDF_Page_AddText(pdf_strana, LEVA_POLOVINA_DOKUMENTU(pdf_strana), HPDF_Page_GetHeight(pdf_strana) - 800,
                      "Vždy lze (po řádném vyplnění a podepsání) zaslat jako sken na: dita.binderova@ssps.cz");

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
