/*
 *  dbt_ueb2.pc
 *
 *  Eingabeaufforderung des Benutzers nach Benutzerkennung und Passwort,
 *  Eingabeaufforderung des Benutzers nach der Nummer eines Fachbereichs,
 *  Eingabeaufforderung nach weiteren Suchpraedikaten,
 *  Suche der Tabellen "mitarbeiter" und "hochschulangehoerige"
 *  nach entsprechenden Mitarbeitern,
 *  dynamischer Zusammenbau einer Datenbankanfrage (Methode 3),
 *  Einsatz eines Cursors,
 *  Ausgabe von Name, Personalnummer, Beruf und Gehalt,
 *  Einsatz von Indikator-Variablen in einer Struktur (struct) 
 *  zur Ueberpruefung, ob eine Ausgabe den Wert NULL hat.
 *
 */

#include <stdio.h>
#include <string.h>


/*  Definition von Konstanten fuer VARCHAR-Laengen. */
#define     NAME_LEN      35
#define     BERUF_LEN     30
#define     PERS_LEN      11
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
    char          sel_praedikat[PRAEDIKAT_LEN]; 

/*  Definition von Host-Variablen fuer die Ausgabe. */
    struct
    {
        VARCHAR   personalnummer[PERS_LEN];
        VARCHAR   name[NAME_LEN];
        VARCHAR   beruf[BERUF_LEN];
        float     gehalt;
    } person;

/*  Definition einer Indikator-Struktur entsprechend 
 *  den Ausgabe-Variablen. 
 */
    struct
    {
        short     personalnummer_ind;
        short     name_ind;
        short     beruf_ind;
        short     gehalt_ind;
    } person_ind;

/*  Definition von Host-Variablen fuer die Eingabe. */
    char          in_personalnummer[PERS_LEN];
    int           in_laenge;
    int           in_fb_nr;

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

/*  Endlosschleife, Selektion individueller Mitarbeiter */
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
               "SELECT pers_nr, ho_name, beruf, gehalt ");
            strcat ((char *) sel_anweisung.arr, 
               "FROM dbs_tab_mitarbeiter, dbs_tab_hochschulangehoeriger ");
            strcat ((char *) sel_anweisung.arr, 
               "WHERE fb_nr = :in_fb_nr ");
            strcat ((char *) sel_anweisung.arr, 
               "AND dbs_tab_mitarbeiter.ho_nr = dbs_tab_hochschulangehoeriger.ho_nr ");
            sel_anweisung.len = strlen((char *) sel_anweisung.arr); 

/*  Eingabe weiterer Suchpraedikate  */
            printf ("\nEingabe der Suchpraedikate (leer = keine Praedikate): ");
            gets(temp_char);
            strncpy((char *) sel_praedikat, temp_char, PRAEDIKAT_LEN);
            strncat ((char *) sel_anweisung.arr, sel_praedikat, PRAEDIKAT_LEN);
            sel_anweisung.len = strlen((char *) sel_anweisung.arr);

/*  Kontrolle der Eingabe  */
            printf ("Gewuenschte Anfrage:\n\n%s ", sel_anweisung.arr);

/*  Vorbereitung der Anfrage "aktuelle_anweisung" (Methode 3)  */
            EXEC SQL prepare aktuelle_anweisung from :sel_anweisung;

/*  Definition eines Cursors "c_aktuelle_anfrage" fuer die Suchanfrage  */
            EXEC SQL declare c_aktuelle_anfrage cursor for aktuelle_anweisung;

/*  Eingabe der Fachbereichsnummer der zu suchenden Personen  */
            printf("\nEingabe der Fachbereichsnummer (leer = Ende): ");
            gets (temp_char);
            in_fb_nr = atoi(temp_char); 
            if (strlen((char *) temp_char) == 0)
               break;
               
/*  Kontrolle der Eingabe  */
            printf ("\nGewuenschter Fachbereich: %d\n\n", in_fb_nr);

/*  Oeffnen des Cursors c_aktuelle_anweisung 
 *  unter Verwendung der Host-Variablen "in_fb_nr
 */
            EXEC SQL open c_aktuelle_anfrage using :in_fb_nr;
            

 /*  Ausgabe der Daten  */
            printf("\n\n");
            printf("Pers.-Nr. \tName                \tBeruf                  \tGehalt\n");
            printf("----------\t--------------------\t-----------------------\t----------\n");

/*  Cursor-Schleife  */
            for (;;)
            {

/*  Holen eines Datensatzes des Cursors "c_aktuelle_anweisung"
 *  in die Struktur "person" 
 *  unter Verwendung der Indikator-Variablen "person_ind" 
 */
               EXEC SQL fetch c_aktuelle_anfrage into :person :person_ind;

/*  Null-Abschluss der Ausgabe-String-Variablen  */
               person.personalnummer.arr[person.personalnummer.len] = '\0';
               person.name.arr[person.name.len] = '\0';
               person.beruf.arr[person.beruf.len] = '\0';
            

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
