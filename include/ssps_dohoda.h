/*
 * Český alias pro HPDF_Doc
 */
typedef HPDF_Doc SSPS_DOHODA_PDF;

/*
 * Název vytvořeného souboru
 */
#define PDF_SOUBOR_PREDLOZKA "DPP"

/*
 * Počet maximálně možných položek
 */
#define MAX_POLE 15

/*
 * Struktura s konfigurací PDF
 */
typedef struct SSPS_DOHODA_Konfigurace {
    // Název dohody o provedení práce
    char nazev[100];
    // V jakém městě byla dohoda podepsána
    char kde[50];
    // Jméno zaměstnance
    char jmeno[80];
    // Rodné číslo zaměstnance
    char rodne_cislo[80];
    // Bankovní účet zaměstnance
    char banka[80];
    // Místo narození zaměstnance
    char misto_narozeni[100];
    // Adresa zaměstnance
    char adresa[150];
    // Pojišťovna zaměstnance
    char pojistovna[80];

    // Nemusí být vyplněny všechny
    // MAX_POLE dat provedení práce
    char datum[MAX_POLE][50];
    // MAX_POLE činností provedení práce
    char cinnost[MAX_POLE][150];
    // MAX_POLE hodin provedení práce
    char hodiny[MAX_POLE][50];
    // MAX_POLE poznámek provedení práce
    char poznamka[MAX_POLE][50];

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
 * Generický wrapper pro SSPS_DOHODA_Konfigurace_TOML
 * Přes C11 funkci _Generic rozezná typ vstupu a zvolí správný SSPS_DOHODA_VSTUP_TYP
 */
#define SSPS_DOHODA_Konfigurace_TOML_Generic(vstup, konfigurace_in) SSPS_DOHODA_Konfigurace_TOML(vstup, konfigurace_in, _Generic((vstup), FILE*: SOUBOR, char*: SSPS_DOHODA_VSTUP_TYP, default: 3))

/*
 * Wrapper pro SSPS_DOHODA_Konfigurace_TOML
 * Pro vstup typu FILE*
 */
#define SSPS_DOHODA_Konfigurace_TOML_Soubor(vstup, konfigurace_in) SSPS_DOHODA_Konfigurace_TOML(vstup, konfigurace_in, SOUBOR)

/*
 * Wrapper pro SSPS_DOHODA_Konfigurace_TOML
 * Pro vstup typu char*
 */
#define SSPS_DOHODA_Konfigurace_TOML_String(vstup, konfigurace_in) SSPS_DOHODA_Konfigurace_TOML(vstup, konfigurace_in, STRING)

/*
 * Konfigurace dokumentu přes TOML, výstup uložen do konfigurace_in
 */
int SSPS_DOHODA_Konfigurace_TOML(void *vstup, SSPS_DOHODA_Konfigurace *konfigurace_in, SSPS_DOHODA_VSTUP_TYP typ);

/*
 * Funkce pro vytvoření dohody ve formě PDF, výstup uložen do pdf_in
 */
int SSPS_DOHODA_SepsatDohodu(SSPS_DOHODA_Konfigurace toml_konfigurace, SSPS_DOHODA_PDF *pdf_in);

