/* etape 4.4 : integration de la librairie gescom */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "gescom.h" /* inclusion de notre nouvelle librairie */

#define HIST_FILE ".biceps_history" /* fichier de stockage de l'historique */

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

/* fonctions des commandes internes */

int Sortie(int n, char *p[]) {
    /* sauvegarde de l'historique avant de quitter */
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
    printf("version 1.0\n");
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

    /* ignore le signal d'interruption ctrl-c pour le shell pere */
    signal(SIGINT, SIG_IGN);

    /* chargement de l'historique de la session precedente */
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

        /* gestion du eof (ctrl-d) */
        if (ligne == NULL) {
            printf("\n");
            /* appel de la fonction de sortie pour sauvegarder et quitter proprement */
            Sortie(0, NULL);
        }

        if (strlen(ligne) > 0) {
            /* ajout a l'historique uniquement si la ligne est utile */
            add_history(ligne);
            
            char* ptr_ligne = ligne;
            while ((commande_isolee = strsep(&ptr_ligne, ";")) != NULL) {
                if (*commande_isolee != '\0') {
                    analyseCom(commande_isolee);
                    
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
                }
            }
        }
        free(ligne);
    }

    return 0;
}