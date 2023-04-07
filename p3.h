 /* TITLE: Sistemas Operativos Práctica 3
 AUTHOR 1: Fernando Iglesias Iglesias         LOGIN 1: f.iglesias2
 GROUP: 1.1
 DATE: 07/12/2022
*/

#include "list.h"

//--------Comandos P0:-------------

int autores(char * trozos[], int ntrozos, struct info *inf);
int carpeta(char * trozos[], int ntrozos, struct info *inf);
int fecha(char * trozos[], int ntrozos, struct info *inf);
int hist(char * trozos[], int ntrozos, struct info *inf);
int comando(char * trozos[], int ntrozos, struct info *inf);
int pid(char * trozos[], int ntrozos, struct info *inf);
int infosis(char * trozos[], int ntrozos, struct info *inf);
int ayuda(char * trozos[], int ntrozos, struct info *inf);
int fin(char * trozos[], int ntrozos, struct info *inf);
int salir (char * trozos[], int ntrozos, struct info *inf);
int bye(char * trozos[], int ntrozos, struct info *inf);

//-------Comandos P1:-------------

int create(char * trozos[], int ntrozos, struct info *inf);
int deletee(char * trozos[], int ntrozos, struct info *inf);
int deltree(char * trozos[], int ntrozos, struct info *inf);
int stats(char * trozos[], int ntrozos, struct info *inf);
int listar(char * trozos[], int ntrozos, struct info *inf);

//-------Comandos P2:-------------

int allocate(char * trozos[], int ntrozos, struct info *inf);
int deallocate(char * trozos[], int ntrozos, struct info *inf);
int i_o(char * trozos[], int ntrozos, struct info *inf);
int memdump(char * trozos[], int ntrozos, struct info *inf);
int memfill(char * trozos[], int ntrozos, struct info *inf);
int memory(char * trozos[], int ntrozos, struct info *inf);
int recurse(char * trozos[], int ntrozos, struct info *inf);

//-------Comandos P3:-------------

int priority(char * trozos[], int ntrozos, struct info *inf);
int showvar(char * trozos[], int ntrozos, struct info *inf);
int changevar(char * trozos[], int ntrozos, struct info *inf);
int showenv(char * trozos[], int ntrozos, struct info *inf);
int dofork(char * trozos[], int ntrozos, struct info *inf);
int execute(char * trozos[], int ntrozos, struct info *inf);
int listjobs(char * trozos[], int ntrozos, struct info *inf);
int deljobs(char * trozos[], int ntrozos, struct info *inf);
int jobs(char * trozos[], int ntrozos, struct info *inf);
int doAsterisco(char * trozos[], int ntrozos, struct info *inf);

//-------Struct de comandos:-------------

struct command {
    char* cmd_name;
    int (* cmd_function) (char * trozos[], int ntrozos, struct info *inf);
} cmds [] = {
        {"autores",autores}, //0
        {"pid",pid}, //1
        {"carpeta",carpeta}, //2
        {"fecha",fecha}, //3
        {"ayuda",ayuda}, //4
        {"infosis",infosis}, //5
        {"hist",hist},
        {"comando",comando},
        {"fin",fin}, //8
        {"salir",salir}, //9
        {"bye",bye}, //10
        {"create",create}, //11
        {"delete",deletee}, //12
        {"deltree",deltree}, //13
        {"stat",stats}, //14
        {"list",listar}, //15
        {"allocate",allocate}, //16
        {"deallocate",deallocate}, //17
        {"i-o",i_o}, //18
        {"memdump",memdump}, //19
        {"memfill",memfill}, //20
        {"memory",memory}, //21
        {"recurse",recurse}, //22
        {"priority",priority}, //23
        {"showvar",showvar}, //24
        {"showenv",showenv}, //25
        {"changevar",changevar}, //26
        {"fork",dofork}, //27
        {"execute",execute}, //28
        {"listjobs",listjobs}, //29
        {"deljobs",deljobs}, //30
        {"jobs",jobs}, //31
        {"help",ayuda}

};
