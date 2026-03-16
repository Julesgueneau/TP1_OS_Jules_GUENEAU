/* etape 2 : analyse de la commande avec strsep et allocation dynamique */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAXPAR 10 /* nombre maximum de mots dans la commande */

/* variables globales requises par l'enonce */
static char *Mots[MAXPAR];
static int NMots;

/* fonction de creation dynamique du prompt utilisateur (inchangée) */
char* creer_prompt(void) {
    char* user;
    char hostname[256];
    char suffixe;
    int taille;
    char* prompt;

    user = getenv("USER");
    if (user == NULL) {
        user = "inconnu";
    }

    if (gethostname(hostname, sizeof(hostname)) != 0) {
        strcpy(hostname, "machine");
    }

    if (geteuid() == 0) {
        suffixe = '#';
    } else {
        suffixe = '$';
    }

    taille = snprintf(NULL, 0, "%s@%s%c ", user, hostname, suffixe);
    prompt = malloc(taille + 1);
    
    if (prompt != NULL) {
        snprintf(prompt, taille + 1, "%s@%s%c ", user, hostname, suffixe);
    }

    return prompt;
}

/* fonction de copie dynamique d'une chaine de caracteres */
char* copyString(char* s) {
    char* copie;
    
    if (s == NULL) return NULL;
    
    copie = malloc(strlen(s) + 1);
    if (copie != NULL) {
        strcpy(copie, s);
    }
    
    return copie;
}

/* fonction d'analyse de la commande saisie */
int analyseCom(char* b) {
    char* token;
    char* delimiteurs = " \t\n"; /* espace, tabulation, newline */
    
    NMots = 0;
    
    /* strsep extrait les mots en remplacant les delimiteurs par \0 */
    while ((token = strsep(&b, delimiteurs)) != NULL) {
        /* on ignore les chaines vides dues a des espaces multiples */
        if (*token != '\0') {
            if (NMots < MAXPAR) {
                Mots[NMots] = copyString(token);
                NMots++;
            } else {
                fprintf(stderr, "erreur : nombre maximum de parametres atteint\n");
                break;
            }
        }
    }
    
    return NMots;
}

/* point d'entree du programme */
int main(void) {
    char* ligne = NULL;
    char* prompt = NULL;
    int i;

    while (1) {
        prompt = creer_prompt();
        if (prompt == NULL) {
            fprintf(stderr, "erreur d'allocation memoire pour le prompt\n");
            break;
        }

        ligne = readline(prompt);
        free(prompt);

        if (ligne == NULL) {
            printf("\n");
            break;
        }

        if (strlen(ligne) > 0) {
            add_history(ligne);
            
            /* appel de la fonction d'analyse sur la ligne saisie */
            analyseCom(ligne);
            
            /* affichage du resultat de l'analyse */
            if (NMots > 0) {
                printf("nom de la commande : %s\n", Mots[0]);
                if (NMots > 1) {
                    printf("parametres :\n");
                    for (i = 1; i < NMots; i++) {
                        printf("\t%s\n", Mots[i]);
                    }
                }
                
                /* liberation rigoureuse des allocations de copyString */
                for (i = 0; i < NMots; i++) {
                    free(Mots[i]);
                    Mots[i] = NULL;
                }
            }
        }

        free(ligne);
    }

    return 0;
}