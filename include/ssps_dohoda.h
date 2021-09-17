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
    char nazev[80];
    // V jakém městě byla dohoda podepsána
    char kde[20];
    // Jméno zaměstnance
    char jmeno[50];
    // Rodné číslo zaměstnance
    char rodne_cislo[12];
    // Bankovní účet zaměstnance
    char banka[30];
    // Místo narození zaměstnance
    char misto_narozeni[50];
    // Adresa zaměstnance
    char adresa[80];
    // Pojišťovna zaměstnance
    char pojistovna[30];

    // Nemusí být vyplněny všechny
    // MAX_POLE dat provedení práce
    char datum[MAX_POLE][6];
    // MAX_POLE činností provedení práce
    char cinnost[MAX_POLE][80];
    // MAX_POLE hodin provedení práce
    char hodiny[MAX_POLE][6];
    // MAX_POLE poznámek provedení práce
    char poznamka[MAX_POLE][10];

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
 * Funkce pro vytvoření dohody ve formě PDF, výstup uložen do pdf_in
 */
int SSPS_DOHODA_SepsatDohodu(SSPS_DOHODA_Konfigurace toml_konfigurace, SSPS_DOHODA_PDF *pdf_in);

/*
 * Funkce pro vypsání celkového počtu odpracovaných hodin
 * Vrací 1 v případě chyby, 0 v případě úspěšného provedení
 * Počítají se jen hodiny do MAX_POLE!
 */
int SSPS_DOHODA_PocetHodin(SSPS_DOHODA_Konfigurace toml_konfigurace, float *hodiny);
