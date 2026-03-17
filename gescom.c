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

/* execution d'une sequence de commandes reliees par des tubes */
void execPipeline(char **cmds, int nb) {
    int i;
    int fd[2];
    int fd_in = 0;
    pid_t pids[MAXPAR];

    for (i = 0; i < nb; i++) {
        pipe(fd);
        pids[i] = fork();
        
        if (pids[i] < 0) {
            perror("erreur fork pipe");
            return;
        }
        
        if (pids[i] == 0) {
            /* espace du processus fils */
            signal(SIGINT, SIG_DFL);
            dup2(fd_in, 0); /* l'entree standard lit le tube precedent */
            if (i < nb - 1) {
                dup2(fd[1], 1); /* la sortie standard ecrit dans le nouveau tube */
            }
            close(fd[0]); /* on ferme la lecture non utilisee par ce fils */
            
            /* on analyse et on execute la commande courante */
            analyseCom(cmds[i]);
            execvp(Mots[0], Mots);
            perror(Mots[0]);
            exit(1);
        } else {
            /* espace du processus pere */
            close(fd[1]); /* le pere n'ecrit pas dans le tube */
            if (fd_in != 0) close(fd_in); /* on ferme l'ancienne extremite de lecture */
            fd_in = fd[0]; /* on sauvegarde la lecture pour le prochain fils */
        }
    }
    
    /* le pere attend la fin de tous les processus du pipeline */
    for (i = 0; i < nb; i++) {
        waitpid(pids[i], NULL, 0);
    }
}