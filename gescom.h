/* en-tete de la librairie gescom */
#ifndef GESCOM_H
#define GESCOM_H

#define MAXPAR 10
#define NBMAXC 10

/* variables globales accessibles par biceps.c pour les free() */
extern char *Mots[MAXPAR];
extern int NMots;

struct commande_interne {
    char *nom;
    int (*fonction)(int, char **);
};

/* prototypes des fonctions du moteur */
char* copyString(char* s);
int analyseCom(char* b);
void ajouteCom(char *nom, int (*fonc)(int, char **));
int execComInt(int n, char **p);
int execComExt(char **p);

#endif