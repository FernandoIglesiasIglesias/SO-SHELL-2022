 /* TITLE: Sistemas Operativos Práctica 3
 AUTHOR 1: Fernando Iglesias Iglesias         LOGIN 1: f.iglesias2
 GROUP: 1.1
 DATE: 07/12/2022
*/
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>
 #include <unistd.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <sys/ipc.h>
 #include <sys/shm.h>
 #include <errno.h>
 #include <dirent.h>
 #include <sys/utsname.h>
 #include <grp.h>
 #include <ctype.h>
 #include <sys/wait.h>
 #include <fcntl.h>
 #include <sys/mman.h>
 #include <sys/resource.h>
 #include <signal.h>
 
 #define MAXL 255
 #define TAMANO 2048


typedef struct node * lista;
typedef struct node ** iterator;

struct info{
    lista historial;
    lista memoria;
    lista procesos;
    char **arg3main;
};
  
lista CreateList();
  
void InsertElement(lista *last, void *element);

int RemoveElementAt(lista *plist, iterator position, int (*freeE)(void *));

int RemoveElement(lista *plist, void * elem, int (*freeE)(void *));

int disposeAll(lista * ptolist, int (*freeE)(void *));

int isEmptyList(lista l);

iterator first(lista * list);

iterator next(iterator p);

int isLast(iterator p);

void * getElement(iterator p);

int freeHist(void *e);