/*
 *  dbt_ueb3.pc
 *
 *
 */

#include <stdio.h>
#include <string.h>


/*  Definition von Konstanten fuer VARCHAR-Laengen. */
#define     NAME_LEN      35
#define     BERUF_LEN     30
#define     LV_NR_LEN      11
#define     PWD_LEN       10
#define     ANWEISUNG_LEN 300
#define     PRAEDIKAT_LEN 100


/*  Definition von Variablen */
int         total_gefunden;

/* Definition von Host-Variablen */
EXEC SQL BEGIN DECLARE SECTION;

/*  Definition von Variablen fuer den Verbindungsaufbau,
 *  Achtung: VARCHAR ist in C nicht bekannt !!!
 */
    VARCHAR       username[NAME_LEN];  
    VARCHAR       password[PWD_LEN]; 

/*  Definition von Variablen fuer die Selektionsanweisung */
    VARCHAR       sel_anweisung[ANWEISUNG_LEN];

/*  Definition von Host-Variablen fuer die Ausgabe. */
    struct
    {
        int   lv_nr_ind;
        VARCHAR lv_name_ind;
        int     fb_nr_ind;
    } lehrveranstaltung;

/*  Definition einer Indikator-Struktur entsprechend 
 *  den Ausgabe-Variablen. 
 */
    struct
    {
        short     lv_nr_ind;
        short     lv_name_ind;
        short     fb_nr_ind;
    } lehrveranstaltung_ind;

/*  Definition von Host-Variablen fuer die Eingabe. */
    char          in_lv_nr[LV_NR_LEN];

EXEC SQL END DECLARE SECTION;

/*  SQL Communications Area. */
EXEC SQL INCLUDE SQLCA;

/*  Definition der Fehlerbehandlung. */
void sql_error();

/*  Hauptprogramm */
main()
{
    char temp_char[PRAEDIKAT_LEN];

/*  Eingabe von Benutzername und Passwort,
 *  Setzen der Laengen-Komponente der VARCHAR-Variablen
 */
    printf("\nEingabe User-ID (leer = Ende): ");
    gets(temp_char);
    strncpy((char *) username.arr, temp_char, NAME_LEN);
    username.len = strlen((char *) username.arr);
    if (username.len == 0)
        exit(-1);

    printf("\nEingabe Passwort (leer = Ende): ");
    gets(temp_char);
    strncpy((char *) password.arr, temp_char, PWD_LEN);
    password.len = strlen((char *) password.arr);
    if (password.len == 0)
        exit(-1);

/*  Fehlerbehandlung sql_error() als the error handler. */
    EXEC SQL WHENEVER SQLERROR DO sql_error("ORACLE Fehler --\t");

/*  Verbindungsaufbau (Oracle-Syntax !!! ). Das Programm ruft
 *  sql_error() wenn ein Fehler waehrend des Verbindungsaufbaus auftritt.
 */
    EXEC SQL CONNECT :username IDENTIFIED BY :password;

    printf("\nVerbunden mit ORACLE als Benutzer: %s\n", username.arr);

    total_gefunden = 0;

/*  Endlosschleife, Selektion individueller Lehrveranstalltung */
    for (;;)
    {

/*  Verlassen der Schleifen, wenn ein 
 *  1403-Fehler ("No data found") auftritt.
 */
        EXEC SQL WHENEVER NOT FOUND DO break;
        for (;;)
        {

 /*  Zusammenbau einer SQL-Anweisung mit Host-Variable */
            strcpy ((char *) sel_anweisung.arr, 
               "SELECT LV_NR, LV_NAME");
            strcat ((char *) sel_anweisung.arr, 
               "FROM DBS_TAB_LEHRVERANSTALTUNG");
            sel_anweisung.len = strlen((char *) sel_anweisung.arr); 

/*  Vorbereitung der Anfrage "aktuelle_anweisung" (Methode 3)  */
            EXEC SQL prepare aktuelle_anweisung from :sel_anweisung;

/*  Definition eines Cursors "c_aktuelle_anfrage" fuer die Suchanfrage  */
            EXEC SQL declare c_aktuelle_anfrage cursor for aktuelle_anweisung;

/*  Oeffnen des Cursors c_aktuelle_anweisung 
 *  unter Verwendung der Host-Variablen "in_fb_nr
 */
            EXEC SQL open c_aktuelle_anfrage;
            

 /*  Ausgabe der Daten  */
            printf("\n\n");
            printf("LV_NR \tName                \tFachbereichsnummer\n");
            printf("----------\t--------------------\t-----------------------\n");

/*  Cursor-Schleife  */
            for (;;)
            {

/*  Holen eines Datensatzes des Cursors "c_aktuelle_anweisung"
 *  in die Struktur "person" 
 *  unter Verwendung der Indikator-Variablen "person_ind" 
 */
               EXEC SQL fetch c_aktuelle_anfrage into :lehrveranstaltung :lehrveranstaltung_ind;

/*  Null-Abschluss der Ausgabe-String-Variablen  */
               lehrveranstaltung.personalnummer.arr[lehrveranstaltung.personalnummer.len] = '\0';
               lehrveranstaltung.name.arr[person.name.len] = '\0';
               lehrveranstaltung.beruf.arr[person.beruf.len] = '\0';
            

/*  Ausgabe  */
               printf("%-10s\t%-20s\t", person.personalnummer.arr, person.name.arr);

/*  Gebrauch der Indikator-Variable  */
               if (person_ind.beruf_ind == -1)
                   printf("%-20s\t", "- ANGABE FEHLT -");
               else
                   printf("%-20s\t", person.beruf.arr);

               printf("%6.2f\n", person.gehalt);
            
               total_gefunden++;

/*  Ende der Cursor-Schleife  */
           }

           printf("\nIm Fachbereich %d wurde%s %d Zeile%s gefunden:\n\n", 
                  in_fb_nr, 
                 (sqlca.sqlerrd[2] == 1) ? "" : "n", 
                  sqlca.sqlerrd[2], 
                 (sqlca.sqlerrd[2] == 1) ? "" : "n");

/*  Schliessen des Cursors c_aktuelle_anweisung  */
           EXEC SQL close c_aktuelle_anfrage;
 
            
/*  Ende der inneren Schleife  */
        }

        if (in_fb_nr == 0)
        { 
            printf("\nEnde gewuenscht !\n");
            break;
        }
        printf("\nKeine Daten gefunden - Neuer Versuch.\n");

/*  Ende der aeusseren Schleife  */
    }

    printf("\n\nAnzahl der gefundenen Datensaetze war %d.\n", total_gefunden); 
    printf("\nHave a Nice Day - and Don't Forget to Lern for DBT ...\n\n\n");

/* Bestaetigung  und Disconnect von der Datenbank,  
 * ( ORACLE-Syntax !!! ) 
 */
    EXEC SQL COMMIT WORK RELEASE;
    exit(0);
}

void
sql_error(meldung)
char *meldung;
{
    char fehler_meldung[128];
    size_t buffer_len, meldung_len;

    EXEC SQL WHENEVER SQLERROR CONTINUE;

    printf("\n\n%s", meldung);
    buffer_len = sizeof (fehler_meldung);

/*  Ermittlung des Fehlertextes mittels sqlglm() */
    sqlglm(fehler_meldung, &buffer_len, &meldung_len);
    printf("%.*s\n", meldung_len, fehler_meldung);

/* Ruecksetzen und Disconnect von der Datenbank, 
 * ( ORACLE-Syntax !!! ) 
 */
    EXEC SQL ROLLBACK RELEASE;
    exit(1);
}
