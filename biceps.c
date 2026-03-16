/* etape 4.1 : ajout des commandes internes indispensables (cd, pwd, vers) */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
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

/* commandes internes */

int Sortie(int n, char *p[]) {
    exit(0);
    return 0;
}

/* changement de repertoire courant */
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

/* affichage du repertoire courant */
int CommandePWD(int n, char *p[]) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("pwd");
    }
    return 0;
}

/* affichage de la version du shell */
int CommandeVERS(int n, char *p[]) {
    printf("biceps (bel interpreteur de commandes des eleves de polytech sorbonne)\n");
    printf("version 1.0\n");
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

/* enregistrement des nouvelles commandes dans le tableau au demarrage */
void majComInt(void) {
    ajouteCom("exit", Sortie);
    ajouteCom("cd", CommandeCD);
    ajouteCom("pwd", CommandePWD);
    ajouteCom("vers", CommandeVERS);
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