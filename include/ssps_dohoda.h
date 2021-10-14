/*
 * Hodinové ohodnocení
 */
#define SABLOVA_KONSTANTA 200

/*
 * Český alias pro HPDF_Doc
 */
typedef HPDF_Doc SSPS_DOHODA_PDF;

/*
 * Název vytvořeného souboru
 */
#define PDF_SOUBOR_PREDLOZKA u8"DPP"

/*
 * Struktura s konfigurací PDF
 */
typedef struct SSPS_DOHODA_Konfigurace {
    // Název dohody o provedení práce
    char *nazev;
    // V jakém městě byla dohoda podepsána
    char *kde;
    // Jméno zaměstnance
    char *jmeno;
    // Rodné číslo zaměstnance
    char *rodne_cislo;
    // Bankovní účet zaměstnance
    char *banka;
    // Místo narození zaměstnance
    char *misto_narozeni;
    // Adresa zaměstnance
    char *adresa;
    // Pojišťovna zaměstnance
    char *pojistovna;

    // Nemusí být vyplněny všechny
    // 2D pole obsahující datum pracovní činnosti
    char **datum;
    // 2D pole obsahující název pracovní činnosti
    char **cinnost;
    // 2D pole obsahující počet hodin pracovní činnosti
    char **hodiny;
    // 2D pole obsahující poznámky pracovní činnosti
    char **poznamka;

    // Počet položek
    unsigned int len;
} SSPS_DOHODA_Konfigurace;

/*
 * Typ vstupu do funkce SSPS_DOHODA_Konfigurace_TOML
 */
typedef enum {
    SOUBOR, STRING
} SSPS_DOHODA_VSTUP_TYP;

/*
 * Řazení pracovních položek v dokumentu
 */
typedef enum {
    NERADIT,
    OD_NEJSTARSIHO,
    OD_NEJNOVEJSIHO
} SSPS_DOHODA_RAZENI_POLOZEK;

/*
 * Generický wrapper pro SSPS_DOHODA_Konfigurace_TOML
 * Přes C11 funkci _Generic rozezná typ vstupu a zvolí správný SSPS_DOHODA_VSTUP_TYP
 */
#define SSPS_DOHODA_Konfigurace_TOML_Generic(vstup, konfigurace_in, razeni_polozek) SSPS_DOHODA_Konfigurace_TOML(vstup, konfigurace_in, _Generic((vstup), FILE*: SOUBOR, char*: SSPS_DOHODA_VSTUP_TYP, default: 3), razeni_polozek)

/*
 * Wrapper pro SSPS_DOHODA_Konfigurace_TOML
 * Pro vstup typu FILE*
 */
#define SSPS_DOHODA_Konfigurace_TOML_Soubor(vstup, konfigurace_in, razeni_polozek) SSPS_DOHODA_Konfigurace_TOML(vstup, konfigurace_in, SOUBOR, razeni_polozek)

/*
 * Wrapper pro SSPS_DOHODA_Konfigurace_TOML
 * Pro vstup typu char*
 */
#define SSPS_DOHODA_Konfigurace_TOML_String(vstup, konfigurace_in, razeni_polozek) SSPS_DOHODA_Konfigurace_TOML(vstup, konfigurace_in, STRING, razeni_polozek)

/*
 * Konfigurace dokumentu přes TOML, výstup uložen do konfigurace_in
 */
int SSPS_DOHODA_Konfigurace_TOML(void *vstup, SSPS_DOHODA_Konfigurace *konfigurace_in, SSPS_DOHODA_VSTUP_TYP typ, SSPS_DOHODA_RAZENI_POLOZEK razeni_polozek);

/*
 * Vyčištění paměti alokované v rámci struktury SSPS_DOHODA_Konfigurace
 */
int SSPS_DOHODA_Konfigurace_Free(SSPS_DOHODA_Konfigurace *konfigurace);
/*
 * Funkce pro vytvoření dohody ve formě PDF, výstup uložen do pdf_in
 */
int SSPS_DOHODA_SepsatDohodu(SSPS_DOHODA_Konfigurace toml_konfigurace, SSPS_DOHODA_PDF *pdf_in);

/*
 * Funkce pro vypsání celkového počtu odpracovaných hodin
 * Vrací 1 v případě chyby, 0 v případě úspěšného provedení
 */
int SSPS_DOHODA_PocetHodin(SSPS_DOHODA_Konfigurace toml_konfigurace, float *hodiny);

/*
 * Funkce pro vypsání celkové částky za odpracované hodiny
 * Vrací 1 v případě chyby, 0 v případě úspěšného provedení
 */
int SSPS_DOHODA_PocetPenez(SSPS_DOHODA_Konfigurace toml_konfigurace, float *penize);
