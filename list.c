 /* TITLE: Sistemas Operativos Práctica 3
 AUTHOR 1: Fernando Iglesias Iglesias         LOGIN 1: f.iglesias2
 GROUP: 1.1
 DATE: 07/12/2022
*/

#include "list.h"

struct node {
  void * elem;
  struct node *next;
};

struct node * CreateList(){
    return NULL;
}

void InsertElement(struct node ** last, void *element){
  while (*last != NULL){ 
    last = &((*last)->next); 
  }
  *last = (struct node *) malloc(sizeof(struct node));
  (*last)->next = NULL; 
  (*last)->elem = element;
}

int RemoveElementAt(struct node **plist, iterator position, int (*freeE)(void *)){
  if (*plist == NULL) return -1;

  struct node **pointertopointer = plist; 
  for (iterator i = first(pointertopointer);i != position; i++) {
    if (*pointertopointer == NULL) return -1; 
    pointertopointer = &(*pointertopointer)->next;
  }
  struct node *auxiliar = *pointertopointer;  
  *pointertopointer = (*pointertopointer)->next; 
  int ret = freeE(auxiliar->elem);
  free(auxiliar);
  return ret;
}

int RemoveElement(struct node **plist, void * elem,  int (*freeE)(void *)){
  struct node **pointertopointer = plist;
  for (iterator i = first(pointertopointer);!isLast(i);i = next(i)) {
    if (elem == getElement(i)) {
      return RemoveElementAt(i,first(i),freeE);
    }
  }
  return 1;
}

int disposeAll(struct node ** ptolist, int (*freeE)(void *)) {
  int ret = 0;
  while ((ret = RemoveElementAt(ptolist,first(ptolist),freeE)) == 0);
  return ret <= 0 ? ret : 0;
}

int isEmptyList(lista l) {
  return (l == NULL);
}

iterator first(iterator it) {
  return it;
}

iterator next(iterator it) {
  return &(*it)->next;
}

int isLast(iterator it) {
  if (*it == NULL) {
    return 1;
  }
  else return 0;
}

void * getElement(iterator p) {
  return (*p)->elem;
}

int freeHist(void *e) {
  free(e);
  return 0;
}