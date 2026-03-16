/* etape 1 : interface de commande interactive avec readline */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

/* fonction de creation dynamique du prompt utilisateur */
char* creer_prompt(void) {
    char* user;
    char hostname[256];
    char suffixe;
    int taille;
    char* prompt;

    /* recuperation des informations systeme */
    user = getenv("USER");
    if (user == NULL) {
        user = "inconnu";
    }

    if (gethostname(hostname, sizeof(hostname)) != 0) {
        strcpy(hostname, "machine");
    }

    /* determination des privileges */
    if (geteuid() == 0) {
        suffixe = '#';
    } else {
        suffixe = '$';
    }

    /* calcul de la taille et allocation dynamique */
    taille = snprintf(NULL, 0, "%s@%s%c ", user, hostname, suffixe);
    prompt = malloc(taille + 1);
    
    /* formatage de la chaine finale */
    if (prompt != NULL) {
        snprintf(prompt, taille + 1, "%s@%s%c ", user, hostname, suffixe);
    }

    return prompt;
}

/* point d'entree du programme */
int main(void) {
    char* ligne = NULL;
    char* prompt = NULL;

    /* boucle principale de saisie */
    while (1) {
        prompt = creer_prompt();
        if (prompt == NULL) {
            fprintf(stderr, "erreur d'allocation memoire pour le prompt\n");
            break;
        }

        /* lecture de la commande via readline */
        ligne = readline(prompt);
        free(prompt);

        /* gestion de la fin de fichier (ctrl-d) */
        if (ligne == NULL) {
            printf("\n");
            break;
        }

        /* affichage de la ligne saisie si elle n'est pas vide */
        if (strlen(ligne) > 0) {
            add_history(ligne);
            printf("%s\n", ligne);
        }

        free(ligne);
    }

    return 0;
}