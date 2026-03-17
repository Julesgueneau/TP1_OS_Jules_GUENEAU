/* code source de la librairie gescom */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include "gescom.h"

char *Mots[MAXPAR];
int NMots;

static struct commande_interne tab_com_int[NBMAXC];
static int nb_com_int = 0;

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
            if (NMots < MAXPAR - 1) {
                Mots[NMots] = copyString(token);
                NMots++;
            } else {
                fprintf(stderr, "erreur : nombre maximum de parametres atteint\n");
                break;
            }
        }
    }
    Mots[NMots] = NULL;
    return NMots;
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

int execComExt(char **p) {
    pid_t pid;
    int status;
    pid = fork();
    if (pid < 0) {
        perror("erreur fork");
        return -1;
    }
    if (pid == 0) {
#ifdef TRACE
        printf("[trace] execution du processus fils (pid=%d) pour %s\n", getpid(), p[0]);
#endif
        /* le fils doit pouvoir etre interrompu par ctrl-c */
        signal(SIGINT, SIG_DFL);
        execvp(p[0], p);
        perror(p[0]);
        exit(1);
    } else {
#ifdef TRACE
        printf("[trace] attente de la fin du processus fils (pid=%d)\n", pid);
#endif
        waitpid(pid, &status, 0);
    }
    return 0;
}