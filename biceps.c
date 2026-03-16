/* etape 3.2 : gestion des commandes externes avec fork et execvp */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h> /* ajout pour la fonction waitpid */
#include <readline/readline.h>
#include <readline/history.h>

#define MAXPAR 10
#define NBMAXC 10

static char *Mots[MAXPAR];
static int NMots;

struct commande_interne {
    char *nom;
    int (*fonction)(int, char **);
};

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
            /* on limite a maxpar - 1 pour garantir la place du null final */
            if (NMots < MAXPAR - 1) {
                Mots[NMots] = copyString(token);
                NMots++;
            } else {
                fprintf(stderr, "erreur : nombre maximum de parametres atteint\n");
                break;
            }
        }
    }
    /* ajout indispensable pour execvp : le tableau doit se terminer par null */
    Mots[NMots] = NULL;
    return NMots;
}

int Sortie(int n, char *p[]) {
    exit(0);
    return 0;
}

void ajouteCom(char *nom, int (*fonc)(int, char **)) {
    if (nb_com_int >= NBMAXC) {
        fprintf(stderr, "erreur fatale : tableau des commandes internes plein\n");
        exit(1);
    }
    tab_com_int[nb_com_int].nom = nom;
    tab_com_int[nb_com_int].fonction = fonc;
    nb_com_int++;
}

void majComInt(void) {
    ajouteCom("exit", Sortie);
}

void listeComInt(void) {
    int i;
    printf("commandes internes disponibles :\n");
    for (i = 0; i < nb_com_int; i++) {
        printf("- %s\n", tab_com_int[i].nom);
    }
}

int execComInt(int n, char **p) {
    int i;
    if (n == 0) return 0;
    
    for (i = 0; i < nb_com_int; i++) {
        if (strcmp(p[0], tab_com_int[i].nom) == 0) {
            tab_com_int[i].fonction(n, p);
            return 1;
        }
    }
    return 0;
}

/* nouvelle fonction de gestion des processus externes */
int execComExt(char **p) {
    pid_t pid;
    int status;

    /* duplication du processus courant */
    pid = fork();
    
    if (pid < 0) {
        perror("erreur fork");
        return -1;
    }

    if (pid == 0) {
        /* espace memoire du processus fils */
#ifdef TRACE
        printf("[trace] execution du processus fils (pid=%d) pour %s\n", getpid(), p[0]);
#endif
        /* recouvrement de l'espace memoire par le binaire appele */
        execvp(p[0], p);
        
        /* si execvp echoue (commande introuvable), les instructions suivantes s'executent */
        perror(p[0]);
        exit(1);
    } else {
        /* espace memoire du processus pere */
#ifdef TRACE
        printf("[trace] attente de la fin du processus fils (pid=%d)\n", pid);
#endif
        /* le shell pere est suspendu tant que le fils n'a pas termine */
        waitpid(pid, &status, 0);
    }
    
    return 0;
}

int main(void) {
    char* ligne = NULL;
    char* prompt = NULL;
    int i;

    majComInt();
    
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
                /* l'ordre d'execution est strict : interne puis externe */
                if (execComInt(NMots, Mots) == 0) {
                    execComExt(Mots);
                }
                
                for (i = 0; i < NMots; i++) {
                    if (Mots[i] != NULL) {
                        free(Mots[i]);
                        Mots[i] = NULL;
                    }
                }
            }
        }
        free(ligne);
    }

    return 0;
}