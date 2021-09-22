#include <gtk/gtk.h> // GTK4
#include <stdio.h> // Standardní vstup a výstup
#include <string.h> // Práce s textovými řetězci
#include "dohodaprovedeniprace.ui.h" // XML s grafickým rozhraním

// Pole ukazatelů pro uložení jednotlivých vstupů
GObject *dohoda_vlastnosti_entry[9];
GObject *dohoda_datum_entry[15];
GObject *dohoda_prace_entry[15];
GObject *dohoda_hodiny_entry[15];
GObject *dohoda_poznamky_entry[15];

// Define jako náhrada odstraněné funkce
#define gtk_entry_get_text(entry) gtk_entry_buffer_get_text(gtk_entry_get_buffer((GTK_ENTRY(entry))))

// Ukončení aplikace
void ukoncit_aplikaci(GtkWidget *widget, gpointer data) {
    g_application_quit(G_APPLICATION(data));
}

// Vytvoření dohody
void vyplnit_dohodu(GtkWidget *widget, gpointer data) {
    // Celková velikost vytvořeného TOML
    int dohoda_toml_velikost;

    // Šablona pro vlastnosti vákazu
    const char *dohoda_vlastnosti_toml_template = "[dohoda]\n"
                                                  "nazev = \"%s\"\n"
                                                  "kde = \"%s\"\n"
                                                  "\n"
                                                  "[zamestnanec]\n"
                                                  "jmeno = \"%s %s\"\n"
                                                  "rodne_cislo = \"%s\"\n"
                                                  "banka = \"%s\"\n"
                                                  "misto_narozeni = \"%s\"\n"
                                                  "adresa = \"%s\"\n"
                                                  "pojistovna = \"%s\"";

    // Šablona pro položku práce
    const char *dohoda_prace_toml_template = "\n\n[[prace]]\n"
                                             "datum = \"%s\"\n"
                                             "cinnost = \"%s\"\n"
                                             "hodiny = \"%s\"\n"
                                             "poznamka = \"%s\"";


    // Výpočet potřebného místa v paměti
    dohoda_toml_velikost =
            snprintf(NULL, 0, dohoda_vlastnosti_toml_template, gtk_entry_get_text(dohoda_vlastnosti_entry[0]),
                     gtk_entry_get_text(dohoda_vlastnosti_entry[1]), gtk_entry_get_text(dohoda_vlastnosti_entry[2]),
                     gtk_entry_get_text(dohoda_vlastnosti_entry[3]), gtk_entry_get_text(dohoda_vlastnosti_entry[4]),
                     gtk_entry_get_text(dohoda_vlastnosti_entry[5]), gtk_entry_get_text(dohoda_vlastnosti_entry[6]),
                     gtk_entry_get_text(dohoda_vlastnosti_entry[7]), gtk_entry_get_text(dohoda_vlastnosti_entry[8]),
                     NULL, NULL) + 1;

    // Vytvoření první části výkazu
    char *dohoda_toml = malloc(dohoda_toml_velikost);
    sprintf(dohoda_toml, dohoda_vlastnosti_toml_template, gtk_entry_get_text(dohoda_vlastnosti_entry[0]),
            gtk_entry_get_text(dohoda_vlastnosti_entry[1]), gtk_entry_get_text(dohoda_vlastnosti_entry[2]),
            gtk_entry_get_text(dohoda_vlastnosti_entry[3]), gtk_entry_get_text(dohoda_vlastnosti_entry[4]),
            gtk_entry_get_text(dohoda_vlastnosti_entry[5]), gtk_entry_get_text(dohoda_vlastnosti_entry[6]),
            gtk_entry_get_text(dohoda_vlastnosti_entry[7]), gtk_entry_get_text(dohoda_vlastnosti_entry[8]));

    // Postupné přidání jednotlivých prací
    for (int i = 0; i < 15; i++) {
        // Výpočet velikosti
        int dohoda_toml_prace_velikost =
                snprintf(NULL, 0, dohoda_prace_toml_template, gtk_entry_get_text(dohoda_datum_entry[i]),
                         gtk_entry_get_text(dohoda_prace_entry[i]), gtk_entry_get_text(dohoda_hodiny_entry[i]),
                         gtk_entry_get_text(dohoda_poznamky_entry[i]), NULL, NULL) + 1;
        dohoda_toml_velikost += dohoda_toml_prace_velikost;
        // Dočasný char pro uložení hodnoty
        char *temp = malloc(dohoda_toml_prace_velikost);
        // Uložení vyplněného templatu do temp
        sprintf(temp, dohoda_prace_toml_template, gtk_entry_get_text(dohoda_datum_entry[i]),
                gtk_entry_get_text(dohoda_prace_entry[i]), gtk_entry_get_text(dohoda_hodiny_entry[i]),
                gtk_entry_get_text(dohoda_poznamky_entry[i]));
        // Požádání o další pamět
        dohoda_toml = realloc(dohoda_toml, dohoda_toml_velikost);
        // Zapsání temp do dohoda_toml
        strcat(dohoda_toml, temp);
        // Vyčištění temp
        free(temp);
    }
    fprintf(stdout, "%s\n", dohoda_toml);
    free(dohoda_toml);
}

static void spustit_aplikaci(GtkApplication *app, gpointer data) {
    GtkBuilder *gtk_builder = gtk_builder_new_from_string(gtk_builder_ui, strlen(gtk_builder_ui));

    // Hlavní okno aplikace
    GObject *dohoda_okno = gtk_builder_get_object(gtk_builder, "DohodaOkno");
    gtk_window_set_resizable(GTK_WINDOW(dohoda_okno), FALSE);
    gtk_window_set_application(GTK_WINDOW(dohoda_okno), app);

    // Tlačítka
    GObject *dohoda_tlacitko_ukoncit = gtk_builder_get_object(gtk_builder, "TlacitkoUkoncit");
    GObject *dohoda_tlacitko_vyplnit = gtk_builder_get_object(gtk_builder, "TlacitkoVyplnit");

    // Vlastnoti výkazu práce
    GObject *dohoda_nazev_entry = gtk_builder_get_object(gtk_builder, "NazevEntry");
    GObject *dohoda_jmeno_entry = gtk_builder_get_object(gtk_builder, "JmenoEntry");
    GObject *dohoda_prijmeni_entry = gtk_builder_get_object(gtk_builder, "PrijmeniEntry");
    GObject *dohoda_kde_entry = gtk_builder_get_object(gtk_builder, "KdeEntry");
    GObject *dohoda_rodnecislo_entry = gtk_builder_get_object(gtk_builder, "RodneEntry");
    GObject *dohoda_banka_entry = gtk_builder_get_object(gtk_builder, "BankaEntry");
    GObject *dohoda_mistonarozeni_entry = gtk_builder_get_object(gtk_builder, "MistoEntry");
    GObject *dohoda_adresa_entry = gtk_builder_get_object(gtk_builder, "AdresaEntry");
    GObject *dohoda_pojistovna_entry = gtk_builder_get_object(gtk_builder, "PojistovnaEntry");

    dohoda_vlastnosti_entry[0] = dohoda_nazev_entry;
    dohoda_vlastnosti_entry[1] = dohoda_kde_entry;
    dohoda_vlastnosti_entry[2] = dohoda_jmeno_entry;
    dohoda_vlastnosti_entry[3] = dohoda_prijmeni_entry;
    dohoda_vlastnosti_entry[4] = dohoda_rodnecislo_entry;
    dohoda_vlastnosti_entry[5] = dohoda_banka_entry;
    dohoda_vlastnosti_entry[6] = dohoda_mistonarozeni_entry;
    dohoda_vlastnosti_entry[7] = dohoda_adresa_entry;
    dohoda_vlastnosti_entry[8] = dohoda_pojistovna_entry;

    // Smyčka pro kratší vyplnění všech 4 položek
    for (int i = 1; i < 16; i++) {
        char datum[8];
        char prace[8];
        char hodiny[9];
        char poznamka[11];

        snprintf(datum, 8, "Datum%d", i);
        snprintf(prace, 8, "Prace%d", i);
        snprintf(hodiny, 9, "Hodiny%d", i);
        snprintf(poznamka, 11, "Poznamka%d", i);

        // Datum provedení pracovní činnosti
        dohoda_datum_entry[i - 1] = gtk_builder_get_object(gtk_builder, datum);
        // Název pracovní činnosti
        dohoda_prace_entry[i - 1] = gtk_builder_get_object(gtk_builder, prace);
        // Doba provedení pracovní činnosti v hodinách
        dohoda_hodiny_entry[i - 1] = gtk_builder_get_object(gtk_builder, hodiny);
        // Poznámka k pracovní činnosti
        dohoda_poznamky_entry[i - 1] = gtk_builder_get_object(gtk_builder, poznamka);
    }

    // Ukončit aplikaci
    g_signal_connect(GTK_BUTTON(dohoda_tlacitko_ukoncit), "clicked", G_CALLBACK(ukoncit_aplikaci), app);
    // Vyplnit dohodu
    g_signal_connect(GTK_BUTTON(dohoda_tlacitko_vyplnit), "clicked", G_CALLBACK(vyplnit_dohodu), NULL);

    // Zobrazení hlavního okna
    gtk_widget_show(GTK_WIDGET(dohoda_okno));

    g_object_unref(gtk_builder);
}

int main(void) {
    // Vytvoření hlavního objektu aplikace
    // Aktivace uživatelského prostředí
    GtkApplication *app = gtk_application_new("com.github.eetagent.DohodaProvedeniPrace", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(spustit_aplikaci), NULL);

    // Spuštění aplikace a čekání na ukočení loopu
    int status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);

    return status;
}
