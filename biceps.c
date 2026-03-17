/* etape 5.1 : gestion des pipes ( | ) */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "gescom.h"

#define HIST_FILE ".biceps_history"

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

int Sortie(int n, char *p[]) {
    write_history(HIST_FILE);
    printf("fermeture de biceps. au revoir.\n");
    exit(0);
    return 0;
}

int CommandeCD(int n, char *p[]) {
    if (n < 2) {
        fprintf(stderr, "cd: argument manquant\n");
    } else {
        if (chdir(p[1]) != 0) {
            perror("cd");
        }
    }
    return 0;
}

int CommandePWD(int n, char *p[]) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("pwd");
    }
    return 0;
}

int CommandeVERS(int n, char *p[]) {
    printf("biceps (bel interpreteur de commandes des eleves de polytech sorbonne)\n");
    printf("version 1.1\n"); /* mise a jour de la version */
    return 0;
}

void majComInt(void) {
    ajouteCom("exit", Sortie);
    ajouteCom("cd", CommandeCD);
    ajouteCom("pwd", CommandePWD);
    ajouteCom("vers", CommandeVERS);
}

int main(void) {
    char* ligne = NULL;
    char* prompt = NULL;
    char* commande_isolee;
    int i;

    signal(SIGINT, SIG_IGN);
    read_history(HIST_FILE);
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
            Sortie(0, NULL);
        }

        if (strlen(ligne) > 0) {
            add_history(ligne);
            
            char* ptr_ligne = ligne;
            while ((commande_isolee = strsep(&ptr_ligne, ";")) != NULL) {
                if (*commande_isolee != '\0') {
                    
                    /* etape 5.1 : decoupage par pipe au sein de la commande sequencee */
                    char *cmds_pipe[MAXPAR];
                    int nb_pipe = 0;
                    char *ptr_pipe = commande_isolee;
                    
                    while ((cmds_pipe[nb_pipe] = strsep(&ptr_pipe, "|")) != NULL) {
                        if (*cmds_pipe[nb_pipe] != '\0') nb_pipe++;
                    }
                    
                    if (nb_pipe == 1) {
                        /* cas classique : pas de pipe */
                        analyseCom(cmds_pipe[0]);
                        if (NMots > 0) {
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
                    } else if (nb_pipe > 1) {
                        /* cas pipeline : plusieurs commandes reliees */
                        execPipeline(cmds_pipe, nb_pipe);
                    }
                }
            }
        }
        free(ligne);
    }

    return 0;
}