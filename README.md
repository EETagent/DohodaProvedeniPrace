<h1 class="rich-diff-level-zero">Dohoda o provedení práce - Okamžité vytvoření DPP pro SSPŠ</h1>

Jednoduchý terminálový program a knihovna pro tvorbu výkazů práce.
Vytvořená dohoda je totožná a kompatibilní s DPP užívanými na Smíchovské střední průmyslové škole.

[![C](https://img.shields.io/badge/Jazyk-C-yellow.svg)]()
[![C](https://img.shields.io/badge/DPP-SSPŠ-blue.svg)]()
[![C](https://img.shields.io/badge/PDF-red.svg)]()


![Otevřený PDF dokument](.github/img/dohoda.png?raw=true)

## Použití

Program vytvoří PDF na základě poskytnuté [TOML](https://toml.io/en/) konfigurace z `stdin` (Možno přesměrovat obsah souboru viz. níže) a uloží ho do aktuální složky.
Název výsledného souboru je kombinace prefixu DPP_ a přijmení zaměstnance (Pokud je uvedeno).

```bash
dohoda_ssps < konfigurace.toml
```

S přepínačem `--` podporuje program vypsání PDF souboru do `stdout`, tento výstup je tedy možné přesměrovat do libovolného souboru.

```bash
dohoda_ssps -- < konfigurace.toml > ../mojedohoda.pdf
```

Vypsání nápovědy

```bash
dohoda_ssps -h
```
```
DohodaProvedeniPrace (https://github.com/EETagent/DohodaProvedeniPrace)
Použití programu: ./dohoda_ssps [-hnstf] < soubor
-h Vypsání této nápovědy
-n Seřazení položek od nejnovější
-s Seřazení položek od nejstarší
-t Vypsat počet odpracovaných hodin
-f Cesta k souboru
-- Vypsat PDF do stdout (Musí být na konci příkazu)

PŘÍKLADY:
	./dohoda_ssps < vykaz.toml
	./dohoda_ssps -s < vykaz.toml
	./dohoda_ssps -- < vykaz.toml > dohoda.pdf
	./dohoda_ssps -n -f /home/thinkpad/vykaz.toml -- > dohoda.pdf
```

## Ukázkový TOML

```toml
[dohoda]
nazev = "Program na tovrbu výkazů pro dohody o provedení práce"
kde = "V Liberci"

[zamestnanec]
jmeno = "Franta Novák"
rodne_cislo = ""
banka = ""
misto_narozeni = "Praha"
adresa = "Novákových 123/2, Praha"
pojistovna = "VZP - xxxxx"

[[prace]]
datum = "14.8"
cinnost = "Hodně jsem pracoval"
hodiny = "4"
poznamka = ""

[[prace]]
datum = "15.8"
cinnost = "Další práce"
hodiny = "2"
poznamka = ""

[[prace]]
datum = "16.8"
cinnost = "Třetí položka už"
hodiny = "9"
poznamka = ""

[[prace]]
datum = "17.8"
cinnost = "Poslední testovací blok"
hodiny = "2"
poznamka = ""
```
Další příklady v toml/

## Kompilace

K sestavení projektu jsou potřeba následující závislosti:

* C11 kompilátor (gcc / clang)
* cmake
* ninja nebo make

Stažení projektu:

```bash
git clone https://github.com/EETagent/DohodaProvedeniPrace.git
```

Sestavení aplikace:  

```bash
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release  ..
ninja
```

## Testy

Součástí projektu jsou i zabudované testy:

- `PDF_TEST` -> Zda je výstup PDF totožný k jeho TOML konfiguraci
- `PDF_VELKE_TEST` -> Zda je výstup PDF totožný k jeho TOML konfiguraci větší než 15 položek
- `PDF_RAZENI_TEST` -> Zda jsou položky v PDF správě seřazeny, jinak stejný jako PDF_SAME_HASH_TEST 
- `PDF_VELIKOST_TEST` -> Zda je výsledné PDF menší než 1 MB

```bash
ninja test
```

## TODO

- [x] Řazení podle data
- [x] Optimalizace kódu a dokumentace
- [x] Výpis počtu odpracovaných hodin
- [x] Dynamický počet maximálních možných položek
- [ ] UTF-8 ready omezení délky textu
- [ ] **Vícestránkový dokument**
- [ ] **CGI interface a webový frontend**
