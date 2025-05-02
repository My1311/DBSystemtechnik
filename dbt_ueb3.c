/*
 *  dbt_ueb3.pc
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*  Definition von Konstanten fuer VARCHAR-Laengen. */
#define     NAME_LEN      35
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
    VARCHAR       del_anweisung[ANWEISUNG_LEN];
/*  Definition von Host-Variablen fuer die Ausgabe. */
    struct
    {
        int  lv_nr ;
        VARCHAR lv_name[NAME_LEN];
        char     fb_nr;
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
    int          in_lv_nr;

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
               "SELECT LV_NR, LV_NAME, FB_NR ");
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
               lehrveranstaltung.lv_name.arr[lehrveranstaltung.lv_name.len] = '\0';
            

/*  Ausgabe  */

                if (lehrveranstaltung_ind.lv_nr_ind == -1) {
                    printf("%-20s\t", "- ANGABE FEHLT -");
                }
                else {
                    printf("%-10d\t%-40s\t", lehrveranstaltung.lv_nr, lehrveranstaltung.lv_name.arr);
                }

               printf("%c\n", lehrveranstaltung.fb_nr);
 /*  Gebrauch der Indikator-Variable  */
            
               total_gefunden++;

/*  Ende der Cursor-Schleife  */
           }

           printf("\nDiese oberen Lehrveranstaltungen wurden gefunden.\n");
/*  Schliessen des Cursors c_aktuelle_anweisung  */
            EXEC SQL close c_aktuelle_anfrage;

            printf("\nWelche Lehrveranstaltung wollen Sie löschen?\nGeben Sie bitte die Lehrveranstaltungsnummer an: (Abbruch mit Nummer 0)\n");
            gets(temp_char);
            in_lv_nr = atoi(temp_char);
            if (strlen((char *) temp_char) == 0)
                break;

            strcpy ((char *) del_anweisung.arr,
                                       "DELETE FROM DBS_TAB_PROF_HAELT_LV ");
            strcat ((char *) del_anweisung.arr,
               "Where LV_NR=:in_lv_nr");
            del_anweisung.len = strlen((char *) del_anweisung.arr);


            EXEC SQL prepare PROF_HAELT_LV_loeschen from :del_anweisung;

            strcpy ((char *) del_anweisung.arr,
               "DELETE FROM DBS_TAB_LV_ORT ");
            strcat ((char *) del_anweisung.arr,
               "Where LV_NR=:in_lv_nr");
            del_anweisung.len = strlen((char *) del_anweisung.arr);


            EXEC SQL prepare lv_ort_loeschen from :del_anweisung;

            strcpy ((char *) del_anweisung.arr,
                           "DELETE FROM DBS_TAB_PRUEFUNG ");
            strcat ((char *) del_anweisung.arr,
               "Where LV_NR=:in_lv_nr");
            del_anweisung.len = strlen((char *) del_anweisung.arr);


            EXEC SQL prepare PRUEFUNG_loeschen from :del_anweisung;

            strcpy ((char *) del_anweisung.arr,
                                       "DELETE FROM DBS_TAB_LEHRVERANSTALTUNG ");
            strcat ((char *) del_anweisung.arr,
               "Where LV_NR=:in_lv_nr");
            del_anweisung.len = strlen((char *) del_anweisung.arr);


            EXEC SQL prepare LEHRVERANSTALTUNG_loeschen from :del_anweisung;

            EXEC SQL EXECUTE PROF_HAELT_LV_loeschen USING :in_lv_nr;
            EXEC SQL EXECUTE lv_ort_loeschen USING :in_lv_nr;
            EXEC SQL EXECUTE PRUEFUNG_loeschen USING :in_lv_nr;
            EXEC SQL EXECUTE LEHRVERANSTALTUNG_loeschen USING :in_lv_nr;

            printf("\n Abschließung der Löschung von der Lehrveranstallung %d\n", in_lv_nr);
            
/*  Ende der inneren Schleife  */
        }

        if (in_lv_nr == 0)
        { 
            printf("\nEnde gewuenscht !\n");
            break;
        }

/*  Ende der aeusseren Schleife  */
    }

    printf("\n\nAnzahl der gefundenen Datensaetze war %d.\n",total_gefunden);
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
