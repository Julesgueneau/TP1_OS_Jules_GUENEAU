/* etape 3.1 : mise en place des commandes internes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAXPAR 10
#define NBMAXC 10 /* nb maxi de commandes internes */

static char *Mots[MAXPAR];
static int NMots;

/* definition de la structure d'une commande interne */
struct commande_interne {
    char *nom;
    int (*fonction)(int, char **);
};

/* tableau global des commandes internes et compteur */
static struct commande_interne tab_com_int[NBMAXC];
static int nb_com_int = 0;

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

char* copyString(char* s) {
    char* copie;
    if (s == NULL) return NULL;
    copie = malloc(strlen(s) + 1);
    if (copie != NULL) {
        strcpy(copie, s);
    }
    return copie;
}

int analyseCom(char* b) {
    char* token;
    char* delimiteurs = " \t\n";
    NMots = 0;
    while ((token = strsep(&b, delimiteurs)) != NULL) {
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

/* fonction d'arret du programme */
int Sortie(int n, char *p[]) {
    exit(0);
    return 0;
}

/* ajout d'une commande dans le tableau */
void ajouteCom(char *nom, int (*fonc)(int, char **)) {
    if (nb_com_int >= NBMAXC) {
        fprintf(stderr, "erreur fatale : tableau des commandes internes plein\n");
        exit(1);
    }
    tab_com_int[nb_com_int].nom = nom;
    tab_com_int[nb_com_int].fonction = fonc;
    nb_com_int++;
}

/* initialisation du tableau au demarrage */
void majComInt(void) {
    ajouteCom("exit", Sortie);
}

/* affichage des commandes internes disponibles */
void listeComInt(void) {
    int i;
    printf("commandes internes disponibles :\n");
    for (i = 0; i < nb_com_int; i++) {
        printf("- %s\n", tab_com_int[i].nom);
    }
}

/* execution d'une commande si elle est reconnue comme interne */
int execComInt(int n, char **p) {
    int i;
    if (n == 0) return 0;
    
    for (i = 0; i < nb_com_int; i++) {
        if (strcmp(p[0], tab_com_int[i].nom) == 0) {
            tab_com_int[i].fonction(n, p);
            return 1; /* vrai */
        }
    }
    return 0; /* faux */
}

int main(void) {
    char* ligne = NULL;
    char* prompt = NULL;
    int i;

    /* chargement des commandes internes */
    majComInt();
    
    /* affichage optionnel pour valider la structure */
    listeComInt();

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
            analyseCom(ligne);
            
            if (NMots > 0) {
                /* tentative d'execution comme commande interne */
                if (execComInt(NMots, Mots) == 0) {
                    printf("commande externe (non traitee) : %s\n", Mots[0]);
                }
                
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