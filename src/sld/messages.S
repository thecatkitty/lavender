#include <nls.h>


.section .rodata

                                .global IDS_ENTERDSN
IDS_ENTERDSN:
IF_LANG(CSY,                    .asciz  "Zadejte sériové číslo disku")
IF_LANG(ENU,                    .asciz  "Enter Disk Serial Number")
IF_LANG(PLK,                    .asciz  "Wprowadź numer seryjny dysku")

                                .global IDS_ENTERDSN_DESC
IDS_ENTERDSN_DESC:
IF_LANG(CSY,                    .asciz  "Zadejte prosím sériové číslo\npřipojené k disketě programu.")
IF_LANG(ENU,                    .asciz  "Please type the serial number\nattached to Your program diskette.")
IF_LANG(PLK,                    .asciz  "Proszę podać numer seryjny dołączony\ndo dyskietki z programem.")

                                .global IDS_ENTERPASS
IDS_ENTERPASS:
IF_LANG(CSY,                    .asciz  "Vložte heslo")
IF_LANG(ENU,                    .asciz  "Enter Password")
IF_LANG(PLK,                    .asciz  "Wprowadź hasło")

                                .global IDS_ENTERPASS_DESC
IDS_ENTERPASS_DESC:
IF_LANG(CSY,                    .asciz  "Tento obsah je zašifrován.\n\nZadejte přístupový klíč.")
IF_LANG(ENU,                    .asciz  "This content is encrypted.\n\nPlease enter the access key.")
IF_LANG(PLK,                    .asciz  "Ta zawartość jest zaszyfrowana.\n\nProszę podać klucz dostępu.")

                                .global IDS_INVALIDKEY
IDS_INVALIDKEY:
IF_LANG(CSY,                    .asciz  "Zadaný klíč je neplatný!")
IF_LANG(ENU,                    .asciz  "Provided key is invalid!")
IF_LANG(PLK,                    .asciz  "Wprowadzony klucz jest niepoprawny!")

                                .global IDS_LOADERROR
IDS_LOADERROR:
IF_LANG(CSY,                    .asciz  "Chyba nacitani scenare!")
IF_LANG(ENU,                    .asciz  "Script loading error!")
IF_LANG(PLK,                    .asciz  "Blad ladowania scenariusza!")

                                .global IDS_EXECERROR
IDS_EXECERROR:
IF_LANG(CSY,                    .asciz  "Chyba pri provadeni scenare!")
IF_LANG(ENU,                    .asciz  "Script execution error!")
IF_LANG(PLK,                    .asciz  "Blad wykonania scenariusza!")

                                .global IDS_NOEXECCTX
IDS_NOEXECCTX:
IF_LANG(CSY,                    .asciz  "Nelze vytvorit kontext provadeni")
IF_LANG(ENU,                    .asciz  "Cannot create execution context")
IF_LANG(PLK,                    .asciz  "Nie mozna utworzyc kontekstu wykonania")

                                .global IDS_INVALIDDELAY
IDS_INVALIDDELAY:
IF_LANG(CSY,                    .asciz  "Neplatne zpozdeni")
IF_LANG(ENU,                    .asciz  "Invalid delay")
IF_LANG(PLK,                    .asciz  "Niepoprawne opoznienie")

                                .global IDS_UNKNOWNTYPE
IDS_UNKNOWNTYPE:
IF_LANG(CSY,                    .asciz  "Neznamy typ")
IF_LANG(ENU,                    .asciz  "Unknown type")
IF_LANG(PLK,                    .asciz  "Nieznany typ")

                                .global IDS_INVALIDVPOS
IDS_INVALIDVPOS:
IF_LANG(CSY,                    .asciz  "Neplatna vertikalni poloha")
IF_LANG(ENU,                    .asciz  "Invalid vertical position")
IF_LANG(PLK,                    .asciz  "Niepoprawne polozenie pionowe")

                                .global IDS_INVALIDHPOS
IDS_INVALIDHPOS:
IF_LANG(CSY,                    .asciz  "Neplatna horizontalni poloha")
IF_LANG(ENU,                    .asciz  "Invalid horizontal position")
IF_LANG(PLK,                    .asciz  "Niepoprawne polozenie poziome")

                                .global IDS_LONGNAME
IDS_LONGNAME:
IF_LANG(CSY,                    .asciz  "Nazev souboru prilis dlouhy")
IF_LANG(ENU,                    .asciz  "File name too long")
IF_LANG(PLK,                    .asciz  "Nazwa pliku za dluga")

                                .global IDS_LONGCONTENT
IDS_LONGCONTENT:
IF_LANG(CSY,                    .asciz  "Obsah prilis dlouhy")
IF_LANG(ENU,                    .asciz  "Content too long")
IF_LANG(PLK,                    .asciz  "Zawartosc za dluga")

                                .global IDS_NOLABEL
IDS_NOLABEL:
IF_LANG(CSY,                    .asciz  "Stitek nenalezen")
IF_LANG(ENU,                    .asciz  "Label not found")
IF_LANG(PLK,                    .asciz  "Nie znaleziono etykiety")

                                .global IDS_INVALIDCMPVAL
IDS_INVALIDCMPVAL:
IF_LANG(CSY,                    .asciz  "Neplatna hodnota v porovnani")
IF_LANG(ENU,                    .asciz  "Invalid value in comparison")
IF_LANG(PLK,                    .asciz  "Niepoprawna wartosc w porownaniu")

                                .global IDS_NOASSET
IDS_NOASSET:
IF_LANG(CSY,                    .asciz  "Dilo nelze nacist")
IF_LANG(ENU,                    .asciz  "Cannot load asset")
IF_LANG(PLK,                    .asciz  "Nie mozna zaladowac zasobu")

                                .global IDS_BADENCODING
IDS_BADENCODING:
IF_LANG(CSY,                    .asciz  "Neplatne nebo nepodporovane kodovani")
IF_LANG(ENU,                    .asciz  "Invalid or unsupported encoding")
IF_LANG(PLK,                    .asciz  "Niepoprawne lub niewspierane kodowanie")

                                .global IDS_UNSUPPORTED
IDS_UNSUPPORTED:
IF_LANG(CSY,                    .ascii  "Byla pozadovana nepodporovana funkce")
IF_LANG(ENU,                    .ascii  "Unsupported feature requested")
IF_LANG(PLK,                    .ascii  "Zazadano niewspieranej funkcjonalnosci")
