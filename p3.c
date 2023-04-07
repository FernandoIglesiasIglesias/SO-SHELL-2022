 /* TITLE: Sistemas Operativos Práctica 3
 AUTHOR 1: Fernando Iglesias Iglesias         LOGIN 1: f.iglesias2
 GROUP: 1.1
 DATE: 07/12/2022
*/

#include "p3.h"

#define ESMALLOC 1
#define ESSHARED 2
#define ESMMAP 3
#define FINISHED 1
#define STOPPED 2
#define SIGNALED 3
#define ACTIVE 4

#define MAXNAME 4096 


struct memelem {
    int tpmem;
    void * dir;
    int size;
    char date[MAXL];
    key_t key;
    char namefilemmap[MAXL];
    int df;
};

int freeMemoria(void * elem) {
    struct memelem * aux = elem;
    if (aux->tpmem == ESMALLOC) {
        free(aux->dir);
    } else if (aux->tpmem == ESMMAP) {
        munmap(aux->dir, aux->size);
    } else if (aux->tpmem == ESSHARED) {
        shmdt(aux->dir);
    }
    free(elem);
    return 0;
}

struct procelem {
    int pid;
    char date[MAXL];
    int status;
    char cmdline[MAXL];
    int priority;
    int out;
};

int freeProcElem(void * elem) {
    free(elem);
    return 0;
}

//---------FUNCIONES SHELL: -----------

void imprimirPrompt(){
    printf("*) ");
}

int TrocearCadena(char * cadena, char * trozos[]){
    int i=1;
    if ((trozos[0]=strtok(cadena," \n\t"))==NULL)
        return 0;
    while ((trozos[i]=strtok(NULL," \n\t"))!=NULL)
        i++;
    return i;
}

void leerEntrada(char comando[], struct info *inf){
    fgets(comando,MAXL,stdin);
    char * comd = malloc(256*sizeof(char));
    sprintf(comd,"%s\b",comando);
    InsertElement(&(inf->historial),comd);
}

int procesarEntrada(char comando[], struct info *inf){
    char const * trozos[MAXL]; int correct=0; int end=0;
    int ntrozos = TrocearCadena(comando, (char **) trozos);
    if (ntrozos>0) {
        for (int i = 0; cmds[i].cmd_name!=NULL; ++i) {
            if (!(strcmp(trozos[0],cmds[i].cmd_name))) {
                end = cmds[i].cmd_function((char **) trozos, ntrozos, inf);
                correct=1;
                break;
            }
        }
        if (correct==0) {
            doAsterisco((char **)trozos,ntrozos,inf);
        }
    } else {
        printf("Error: you have not typed anything\n");
    }
    return end;
}

int main(int argc, char *argv[], char *envp[]) {
    char comando[MAXL]; int end=0;
    struct info * inf;
    inf = (struct info *) malloc(sizeof(struct info));
    inf->historial = CreateList();
    inf->memoria = CreateList();
    inf->procesos = CreateList();
    inf->arg3main = envp;
    while(end!=1){
        imprimirPrompt();
        leerEntrada(comando, inf);
        end=procesarEntrada(comando, inf);
    }
    disposeAll(&inf->historial, freeHist);
    disposeAll(&inf->memoria, freeMemoria);
    disposeAll(&inf->procesos,freeProcElem);
    free(inf);
    
    return 0;
}

//--------------FUNCIONES P3: --------------------

//--------------funciones auxiliares P3:----------

int entorno(char **entorno, char * entornoname) {
    for(int i = 0; entorno[i] != NULL; i++) {
        printf("%p->%s[%d]=(%p) %s \n",&entorno[i],entornoname,i,entorno[i],entorno[i]);
    }
    return 0;
}

int BuscarVariable (char * var, char *e[])  /*busca una variable en el entorno que se le pasa como parámetro*/
{
  int pos=0;
  char aux[MAXL];
  
  strcpy (aux,var);
  strcat (aux,"=");
  
  while (e[pos]!=NULL)
    if (!strncmp(e[pos],aux,strlen(aux)))
      return (pos);
    else 
      pos++;
  errno=ENOENT;   /*no hay tal variable*/
  return(-1);
}

int CambiarVariable(char * var, char * valor, char *e[]) /*cambia una variable en el entorno que se le pasa como parámetro*/
{                                                        /*lo hace directamente, no usa putenv*/
  int pos;
  char *aux;
   
  if ((pos=BuscarVariable(var,e))==-1)
    return(-1);
 
  if ((aux=(char *)malloc(strlen(var)+strlen(valor)+2))==NULL)
	return -1;
  strcpy(aux,var);
  strcat(aux,"=");
  strcat(aux,valor);
  e[pos]=aux;
  return (pos);
}

char * Ejecutable (char *s)
{
	char path[MAXNAME];
	static char aux2[MAXNAME];
	struct stat st;
	char *p;
	if (s==NULL || (p=getenv("PATH"))==NULL)
		return s;
	if (s[0]=='/' || !strncmp (s,"./",2) || !strncmp (s,"../",3))
        return s;       /*is an absolute pathname*/
	strncpy (path, p, MAXNAME);
	for (p=strtok(path,":"); p!=NULL; p=strtok(NULL,":")){
       sprintf (aux2,"%s/%s",p,s);
	   if (lstat(aux2,&st)!=-1)
		return aux2;
	}
	return s;
}

int OurExecvpe( char *file, char *const argv[], char *const envp[])
{
   return (execve(Ejecutable(file),argv, envp));
}

struct SEN {
    char * nombre;
    int senal;
};

/*las siguientes funciones nos permiten obtener el nombre de una senal a partir
del número y viceversa */
static struct SEN sigstrnum[]={   
	{"HUP", SIGHUP},
	{"INT", SIGINT},
	{"QUIT", SIGQUIT},
	{"ILL", SIGILL}, 
	{"TRAP", SIGTRAP},
	{"ABRT", SIGABRT},
	{"IOT", SIGIOT},
	{"BUS", SIGBUS},
	{"FPE", SIGFPE},
	{"KILL", SIGKILL},
	{"USR1", SIGUSR1},
	{"SEGV", SIGSEGV},
	{"USR2", SIGUSR2}, 
	{"PIPE", SIGPIPE},
	{"ALRM", SIGALRM},
	{"TERM", SIGTERM},
	{"CHLD", SIGCHLD},
	{"CONT", SIGCONT},
	{"STOP", SIGSTOP},
	{"TSTP", SIGTSTP}, 
	{"TTIN", SIGTTIN},
	{"TTOU", SIGTTOU},
	{"URG", SIGURG},
	{"XCPU", SIGXCPU},
	{"XFSZ", SIGXFSZ},
	{"VTALRM", SIGVTALRM},
	{"PROF", SIGPROF},
	{"WINCH", SIGWINCH}, 
	{"IO", SIGIO},
	{"SYS", SIGSYS},
/*senales que no hay en todas partes*/
#ifdef SIGPOLL
	{"POLL", SIGPOLL},
#endif
#ifdef SIGPWR
	{"PWR", SIGPWR},
#endif
#ifdef SIGEMT
	{"EMT", SIGEMT},
#endif
#ifdef SIGINFO
	{"INFO", SIGINFO},
#endif
#ifdef SIGSTKFLT
	{"STKFLT", SIGSTKFLT},
#endif
#ifdef SIGCLD
	{"CLD", SIGCLD},
#endif
#ifdef SIGLOST
	{"LOST", SIGLOST},
#endif
#ifdef SIGCANCEL
	{"CANCEL", SIGCANCEL},
#endif
#ifdef SIGTHAW
	{"THAW", SIGTHAW},
#endif
#ifdef SIGFREEZE
	{"FREEZE", SIGFREEZE},
#endif
#ifdef SIGLWP
	{"LWP", SIGLWP},
#endif
#ifdef SIGWAITING
	{"WAITING", SIGWAITING},
#endif
 	{NULL,-1},
	};    /*fin array sigstrnum */

/*devuelve el numero de senial a partir del nombre*/
int ValorSenal(char * sen)
{ 
  int i;
  for (i=0; sigstrnum[i].nombre!=NULL; i++)
  	if (!strcmp(sen, sigstrnum[i].nombre))
		return sigstrnum[i].senal;
  return -1;
}

char *NombreSenal(int sen)  /*devuelve el nombre senal a partir de la senal*/ 
{			/* para sitios donde no hay sig2str*/
 int i;
  for (i=0; sigstrnum[i].nombre!=NULL; i++)
  	if (sen==sigstrnum[i].senal)
		return sigstrnum[i].nombre;
 return ("SIGUNKNOWN");
}

int insertProcessToPList(iterator list, char * trozos[], int ntrozos, int pid, int priority) {
    struct procelem * pelem = malloc(sizeof(struct procelem));

    if (pelem == NULL) {
        perror("Error: ");
        return -1;
    }

    pelem->pid = pid;
    pelem->out = 0;

    time_t hora;
    if (time(&hora)==-1) {
        perror("Error: ");
        return -1;
    }
    char date[MAXL]; int len;
    strcpy(date,ctime(&hora));
    len = strlen(date);
    date[len-1] = '\0';
    strcpy(pelem->date,date);

    pelem->status = ACTIVE;
    pelem->priority = priority;

    strcpy(pelem->cmdline,trozos[0]);
    for(int i = 1; i < ntrozos; i++) {
        strcat(pelem->cmdline, " ");
        strcat(pelem->cmdline,trozos[i]); 
    }

    InsertElement(list,pelem);
    return 0;
}

int mostarProceso(struct procelem * proce) {
    if (proce->status == FINISHED) {
        printf("%d %11s p=%d %s TERMINADO (%03d) %s\n",proce->pid, getlogin(), proce->priority,proce->date,proce->out,proce->cmdline);
    } else if (proce->status == STOPPED) {
        printf("%d %11s p=%d %s PARADO (%03d) %s\n",proce->pid, getlogin(), proce->priority,proce->date,(proce->out),proce->cmdline);
    } else if (proce->status == SIGNALED) {
        printf("%d %11s p=%d %s SENALADO (%s) %s\n",proce->pid, getlogin(), proce->priority,proce->date,NombreSenal(proce->out),proce->cmdline);
    } else {
        printf("%d %11s p=%d %s ACTIVO (%03d) %s\n",proce->pid, getlogin(), proce->priority,proce->date,proce->out,proce->cmdline);
    }
    return 0;
}

struct procelem * buscarPidEnLista(int pidToSearch, lista l) { //devuelve el elemento si se encuentra el pid en la lista, si no NULL
    struct procelem *pelem;
    for (iterator i = first(&l); !isLast(i); i = next(i)) {
        pelem = getElement(i);
        if(pelem->pid == pidToSearch) return pelem; //se encuentra el pid en la lista
    }
    return NULL; //no se encuentra el pid en la lista
}

//--------------FUNCIONES P3: --------------------

int priority(char * trozos[], int ntrozos, struct info *inf) {
    int pid, priority;
    
    if (trozos[1] == NULL) { //only priority
        pid = getpid();
        priority = getpriority(PRIO_PROCESS, pid);
        printf("Prioridad del proceso %d es %d\n", pid, priority);
    } else {
        pid = atoi(trozos[1]);
        if (trozos[2] == NULL) { //priority & pid no valor
            priority = getpriority(PRIO_PROCESS,pid);
            printf("Prioridad del proceso %d es %d\n", pid, priority);
        } else { 
            pid = atoi(trozos[1]);
            priority = atoi(trozos[2]);
            if(setpriority(PRIO_PROCESS, pid, priority)==-1)
            perror("Error ");
        }

    }
    return 0;
}

int showvar(char * trozos[], int ntrozos, struct info *inf) {
    if (trozos[1] == NULL) {
        showenv(trozos,ntrozos,inf);
    } else {
        if (BuscarVariable(trozos[1], __environ) != -1) { //si devuelve -1, no se encuentra la var
            
            printf("Con arg3 main %s(%p) @%p\n",__environ[BuscarVariable(trozos[1],__environ)],&__environ[BuscarVariable(trozos[1],__environ)],&inf->arg3main[BuscarVariable(trozos[1],__environ)]);
            printf("Con environ %s(%p) @%p\n",__environ[BuscarVariable(trozos[1],__environ)],&__environ[BuscarVariable(trozos[1],__environ)],&__environ[BuscarVariable(trozos[1],__environ)]);
            printf("Con getenv %s(%p)\n",getenv(trozos[1]),&__environ[BuscarVariable(trozos[1],__environ)]);

        } else {
            perror("Error: ");
        }
    }
    return 0;
}

int changevar(char * trozos[], int ntrozos, struct info *inf) {
    if ( (trozos[1] != NULL) && (trozos[2] != NULL) ) {
        if ( (trozos[3] != NULL) ) {

            if (!strcmp(trozos[1], "-a")) { //tercer argumento del main
                if(CambiarVariable(trozos[2],trozos[3], inf->arg3main) == -1) {
                    perror("Error: ");
                }
            } else if (!strcmp(trozos[1],"-e")) { //environ
                if(CambiarVariable(trozos[2], trozos[3], __environ) == -1) {
                    perror("Error: ");
                }
            } else if (!strcmp(trozos[1],"-p")) { //putenv
                if(putenv(strcat((strcat(trozos[2],"=")),trozos[3])) != 0) {
                    perror("Error: ");
                }
            } else printf("Uso: changevar [-a|-e|-p] var valor\n");

        } else {
            printf("Uso: changevar [-a|-e|-p] var valor\n");
        }
    } else {
        printf("Uso: changevar [-a|-e|-p] var valor\n");
    }
    return 0;
}

int showenv(char * trozos[], int ntrozos, struct info *inf) {
    char * entornoname[MAXL];
    
    if (trozos[1] == NULL) { //only showenv
        strcpy((char *)entornoname,"main arg3");
        entorno(__environ,(char *)entornoname);
    } else { 
        if (!strcmp(trozos[1], "-environ")) { //-environ
            strcpy((char *)entornoname,"environ");
            entorno(__environ,(char *)entornoname);
        } else if (!strcmp(trozos[1], "-addr")) { //-addr
            printf("environ: %p (almadenado en %p)\n",&__environ[0], &__environ);
            printf("main arg3: %p (almacenado en %p)\n",&__environ[0],&inf->arg3main);
        } else printf("Uso: environ [-environ|-addr]\n"); //opcion incorrecta
    }
    return 0;
}

int dofork(char * trozos[], int ntrozos, struct info *inf) {
	pid_t pid;

	if ((pid=fork())==0){
        //vaciamos la lista
        for (iterator i = first(&inf->procesos); (!isEmptyList(inf->procesos)); i = next(i)) {
            RemoveElementAt(&inf->procesos,i,freeProcElem);
        }
		printf ("ejecutando proceso %d\n", getpid());
	}
	else if (pid!=-1)
		waitpid (pid,NULL,0);
    else perror("Error: "); //error
    return 0;
}

int execute(char * trozos[], int ntrozos, struct info *inf) {
    
    if (trozos[1] != NULL) {
        
        int x = 1; char arg[MAXL]; char vars[MAXL]; int prio=0; int ntrozosVar;
        char pri[MAXL]; char prioSin[MAXL];
        while(BuscarVariable(trozos[x],inf->arg3main) != -1) {
            x++;
        }

        if (ntrozos == 2) { //caso base, solo cmd
            if(OurExecvpe(trozos[x],NULL,__environ)==-1) {
                perror("Error");
                return -1;
            } else return 0;
        }
        
        char * argT[MAXL];
        strcpy(arg,trozos[x]);
        for(int i = x+1; i < ntrozos; i++) { //guardamos los argumentos
            if (trozos[i][0] == '@') {
                strcpy(pri,trozos[i]);
                int p=0;
                while(pri[p] != '\0') {
                    prioSin[p] = pri[p+1];
                    p++;
                }
                prio = atoi(prioSin);
                setpriority(PRIO_PROCESS,getpid(),prio);
                break; //si encuentra @pri corta el bucle
            }
            
            strcat(arg," ");
            strcat(arg,trozos[i]);
        }

        TrocearCadena(arg,argT);

        if (x != 1) { //si x=1 es que no hay variables
            char * varsT[MAXL];
            strcpy(vars,trozos[1]);
            for(int v = 2; v < x; v++) { //guardamos las variables
                strcat(vars," ");
                strcat(vars, trozos[v]);
            }
            ntrozosVar = TrocearCadena(vars,varsT);
            for(int j = 0; j < ntrozosVar; j++) {
                varsT[j] = __environ[BuscarVariable(varsT[j],__environ)];
            }
            if(OurExecvpe(argT[0],(char * const *)(&argT),(char * const *)(&varsT))==-1) {
                perror("Error");
                return -1;
            } else return 0;
            
        }
        if(OurExecvpe(argT[0],(char * const*)(&argT),__environ)==-1) {
            perror("Error");
            return -1;
        } else return 0;
        

    } else {
        printf("Imposible ejecutar: Bad address\n");
        return 0;
    }

}

int listjobs(char * trozos[], int ntrozos, struct info *inf) {
    if (isEmptyList(inf->procesos)) return 0;

    for(iterator i = first(&inf->procesos); !isLast(i); i = next(i)) {
        struct procelem *pelem = getElement(i);
        if(waitpid(pelem->pid,&pelem->out, WNOHANG |WUNTRACED |WCONTINUED) == pelem->pid) { //si se cumple es que hay un cambio de estado
            //actualizamos el estado:
            if(WIFCONTINUED(pelem->out)) {
                pelem->status = ACTIVE;
                //pelem->priority = getpriority(PRIO_PROCESS,getpid());
            } else if(WIFSTOPPED(pelem->out)) {
                pelem->out = WTERMSIG(pelem->out);
                pelem->status = STOPPED;
                //pelem->priority = getpriority(PRIO_PROCESS,getpid());
            } else if(WIFSIGNALED(pelem->out)) {
                pelem->out = WTERMSIG(pelem->out);
                pelem->status = SIGNALED;
                //pelem->priority = getpriority(PRIO_PROCESS,getpid());
            } else if(WIFEXITED(pelem->out)) {
                pelem->out = WEXITSTATUS(pelem->out);
                pelem->status = FINISHED;
                pelem->priority=-1;
            }
        }

        pelem->priority = getpriority(PRIO_PROCESS,pelem->pid);
        time_t hora;
        if (time(&hora)==-1) {
            perror("Error: ");
            return -1;
        }
        char date[MAXL]; int len;
        strcpy(date,ctime(&hora));
        len = strlen(date);
        date[len-1] = '\0';
        strcpy(pelem->date,date);
    
        mostarProceso(pelem);
    }
    return 0;
}

int deljobs(char * trozos[], int ntrozos, struct info *inf) {
    if (trozos[1] == NULL) {
        listjobs(trozos,ntrozos,inf);
        return 0;
    }

    struct procelem *pelem;
    if (!strcmp(trozos[1],"-term")) {
        iterator i = first(&inf->procesos);
        while(!isEmptyList(inf->procesos) && !isLast(i)) {
            pelem = getElement(i);
            if(pelem->status == FINISHED) {
                RemoveElementAt(&inf->procesos,i,freeProcElem);
                i = first(&inf->procesos);
            } else i = next(i);
        }

    } else if (!strcmp(trozos[1],"-sig")) {
        iterator i = first(&inf->procesos);
        while(!isEmptyList(inf->procesos) && !isLast(i)) {
            pelem = getElement(i);
            if(pelem->status == SIGNALED) {
                RemoveElementAt(&inf->procesos,i,freeProcElem);
                i = first(&inf->procesos);
            } else i = next(i);
        }

    }
    return 0;
}

int jobs(char * trozos[], int ntrozos, struct info *inf) {
    int pid;

    if (trozos[1] == NULL) {
        listjobs(trozos,0,inf);
        return 0;
    }

    if (!strcmp(trozos[1],"-fg")) { //cambiar a primer plano
        if (trozos[2] == NULL) return 0; 
        pid = atoi(trozos[2]); int status;
        struct procelem *pelemfg = buscarPidEnLista(pid,inf->procesos);
        if(pelemfg != NULL) {
            waitpid(pelemfg->pid, &status, 0);
                //actualizamos el estado:
            if(WIFCONTINUED(status)) {
                pelemfg->status = ACTIVE;
            } else if(WIFSTOPPED(status)) {
                pelemfg->out = WTERMSIG(status);
                pelemfg->status = STOPPED;
            } else if(WIFSIGNALED(status)) {
                pelemfg->out = WTERMSIG(pelemfg->out);
                pelemfg->status = SIGNALED;
            } else if(WIFEXITED(status)) {
                pelemfg->out = WEXITSTATUS(status);
                pelemfg->status = FINISHED;
                pelemfg->priority=-1;
            }

            pelemfg->priority = getpriority(PRIO_PROCESS,pelemfg->pid);
            time_t hora;
            if (time(&hora)==-1) {
                perror("Error: ");
                return -1;
            }
            char date[MAXL]; int len;
            strcpy(date,ctime(&hora));
            len = strlen(date);
            date[len-1] = '\0';
            strcpy(pelemfg->date,date);
            
            if(pelemfg->status == ACTIVE) printf("Proceso %d terminado con normalidad, valor devuelto %d", pid, pelemfg->out);
            if(pelemfg->status == FINISHED) printf("Proceso %d terminado por senal %s\n", pid, NombreSenal(pelemfg->out));
            else if(pelemfg->status == STOPPED) printf("Proceso %d parado\n", pid);
            else if(pelemfg->status == SIGNALED) printf("Proceso %d signaled con senal %s\n",pid,NombreSenal(pelemfg->out));
            //lo quitamos de la lista:
            RemoveElement(&inf->procesos,pelemfg,freeProcElem);
        } else printf("Error: no existe ningun proceso con ese pid\n");
    } else { //mostar procesos en segundo plano
        pid = atoi(trozos[1]);
        struct procelem *pelem = buscarPidEnLista(pid,inf->procesos);
        if(pelem != NULL) { //si pelem es = NULL quiere decir que no existe proceso con ese pid
            for(iterator i = first(&inf->procesos); !isLast(i); i = next(i)) {
                pelem = getElement(i);
                if (pelem->pid == pid) {
                    mostarProceso(pelem);
                }
            }
        } else printf("Error: no existe ningun proceso con ese pid\n");
    }
    return 0;
}

int doAsterisco(char * trozos[], int ntrozos, struct info *inf) {
    int pid=fork(), prio=0;
    char newtrozos[MAXL]; char * trozosNuevo[MAXL]; char pri[MAXL]; char prioSin[MAXL];
    if(!strcmp(trozos[ntrozos-1],"&")) { //2º plano
    
        strcpy(newtrozos,"execute");
        for(int i = 0; i < ntrozos-1; i++) {
            strcat(newtrozos," ");
            strcat(newtrozos,trozos[i]);
            if(trozos[i][0]=='@') {
                strcpy(pri,trozos[i]);
                int p=0;
                while(pri[p] != '\0') {
                    prioSin[p] = pri[p+1];
                    p++;
                }
                prio = atoi(prioSin);
            }
        }
        TrocearCadena(newtrozos,trozosNuevo);
        if(pid==0) {
            //si no es un comando correcto hacemos exit e imprime el error
            if(execute(trozosNuevo,ntrozos,inf) == -1) exit(-1); 
        } else {
            insertProcessToPList(&inf->procesos,trozos,ntrozos,pid,prio);
        }
    } else { //1er plano
        if(pid==0) {
            strcpy(newtrozos,"execute");
            for(int j = 0; j < ntrozos; j++) {
                strcat(newtrozos, " ");
                strcat(newtrozos,trozos[j]);
            }
            TrocearCadena(newtrozos,trozosNuevo);
            //si no es un comando correcto hacemos exit e imprime el error
            if(execute(trozosNuevo,ntrozos+1,inf) == -1) exit(-1);
        } else {
            waitpid(pid,NULL,0);
        }
    }

    return 0;
}

//--------------FUNCIONES P2: --------------------

//--------------funciones auxiliares P2:----------

int insertMem(int typemem, int tamb, void * adress, iterator list, key_t key, char * filename, int df) {
    struct memelem * newelem = malloc(sizeof(struct memelem));
    if (newelem == NULL) {
        perror("Error: ");
        return -1;
    }
    time_t hora;
    if (time(&hora)==-1) {
        perror("Error: ");
        return -1;
    }
    newelem->tpmem = typemem;
    newelem->dir = adress;
    newelem->size = tamb;

    char date[MAXL]; int len;
    strcpy(date,ctime(&hora));
    len = strlen(date);
    date[len-1] = '\0';

    strcpy(newelem->date,date);
    newelem->key = key;
    
    strcpy(newelem->namefilemmap,"");
    if (filename!=NULL) strcpy(newelem->namefilemmap,filename);
    
    newelem->df = df;

    InsertElement(list,newelem);
    return 0;
}

int imprimirMalloc(lista l) {
    printf("******Lista de bloques asignados malloc para el proceso %d\n",getpid());
    struct memelem * elem;
    for (iterator i = first(&l); !isLast(i); i = next(i)) {
        elem = getElement(i);
        if (elem->tpmem == ESMALLOC) {
            printf("    %p      %s malloc\n",elem->dir,elem->date);
        }
    }
    return 0;
}
void imprimirListaShared(lista l) {
    printf("******Lista de bloques asignados shared para el proceso %d\n",getpid());
    struct memelem * elem;
    for (iterator i = first(&l); !isLast(i); i = next(i)) {
        elem = getElement(i);
        if (elem->tpmem == ESSHARED) {
            printf("    %p      %s shared (key %d)\n",elem->dir, elem->date, elem->key);
        }
    }
}
void imprimirListaMmap(lista l) {
    printf("******Lista de bloques asignados mmap para el proceso %d\n",getpid());
    struct memelem * elem;
    for (iterator i = first(&l); !isLast(i); i = next(i)) {
        elem = getElement(i);
        if (elem->tpmem == ESMMAP) {
            printf("    %p      %s %s  (descriptor %d)\n",elem->dir,elem->date, elem->namefilemmap,elem->df);
        }
    }
}
iterator searchMallocSize(lista l, struct memelem ** e,int tamb) { //el elemento queda a nulo si no existe en la lista
    *e = NULL;
    struct memelem * elem;
    for (iterator i = first(&l); !isLast(i); i = next(i)) {
        elem = getElement(i);
        if (elem->size == tamb) {
            *e = elem;
            break;
        }
    }
    return 0;
}

void searchSharedKey(lista l, struct memelem ** e, key_t keytosearch) {
    *e = NULL;
    struct memelem * elem;
    for (iterator i = first(&l); !isLast(i); i = next(i)) {
        elem = getElement(i);
        if (keytosearch == elem->key) {
            *e = elem;
            break;
        }
    }
}

void searchMmapFile(lista l, struct memelem ** e, char * filename) {
    *e = NULL;
    struct memelem * elem;
    for (iterator i = first(&l); !isLast(i); i = next(i)) {
        elem = getElement(i);
        if (!strcmp(filename,elem->namefilemmap)) {
            *e = elem;
            break;
        }
    }
}

ssize_t LeerFichero (char *f, void *p, size_t cont)
{
   struct stat s;
   ssize_t  n;  
   int df,aux;

   if (stat (f,&s)==-1 || (df=open(f,O_RDONLY))==-1)
	return -1;     
   if (cont==-1)   /* si pasamos -1 como bytes a leer lo leemos entero*/
	cont=s.st_size;
   if ((n=read(df,p,cont))==-1){
	aux=errno;
	close(df);
	errno=aux;
	return -1;
   }
   close (df);
   return n;
}
ssize_t EscribirFichero (char *f, void *p, size_t cont,int overwrite)
{
   ssize_t  n;
   int df,aux, flags=O_CREAT | O_EXCL | O_WRONLY;

   if (overwrite)
	flags=O_CREAT | O_WRONLY | O_TRUNC;

   if ((df=open(f,flags,0777))==-1)
	return -1;

   if ((n=write(df,p,cont))==-1){
	aux=errno;
	close(df);
	errno=aux;
	return -1;
   }
   close (df);
   return n;
}
void do_I_O_read (char *ar[])
{
   void *p;
   size_t cont=-1;
   ssize_t n;
   if (ar[2]==NULL || ar[3]==NULL){
	printf ("faltan parametros\n");
	return;
   }
   //p=cadtop(ar[3]);  /*convertimos de cadena a puntero*/
   p = (void *)strtoul(ar[3],NULL,16);
   if (ar[4]!=NULL)
	cont=(size_t) atoll(ar[4]);

   if ((n=LeerFichero(ar[2],p,cont))==-1)
	perror ("Imposible leer fichero");
   else
	printf ("leidos %lld bytes de %s en %p\n",(long long) n,ar[2],p);
}
void Do_pmap (void) /*sin argumentos*/
 { pid_t pid;       /*hace el pmap (o equivalente) del proceso actual*/
   char elpid[32];
   char *argv[4]={"pmap",elpid,NULL};
   
   sprintf (elpid,"%d", (int) getpid());
   if ((pid=fork())==-1){
      perror ("Imposible crear proceso");
      return;
      }
   if (pid==0){
      if (execvp(argv[0],argv)==-1)
         perror("cannot execute pmap (linux, solaris)");
         
      argv[0]="procstat"; argv[1]="vm"; argv[2]=elpid; argv[3]=NULL;   
      if (execvp(argv[0],argv)==-1)/*No hay pmap, probamos procstat FreeBSD */
         perror("cannot execute procstat (FreeBSD)");
         
      argv[0]="procmap",argv[1]=elpid;argv[2]=NULL;    
            if (execvp(argv[0],argv)==-1)  /*probamos procmap OpenBSD*/
         perror("cannot execute procmap (OpenBSD)");
         
      argv[0]="vmmap"; argv[1]="-interleave"; argv[2]=elpid;argv[3]=NULL;
      if (execvp(argv[0],argv)==-1) /*probamos vmmap Mac-OS*/
         perror("cannot execute vmmap (Mac-OS)");      
      exit(1);
  }
  waitpid (pid,NULL,0);
}

void LlenarMemoria (void *p, size_t cont, unsigned char byte)
{
  unsigned char *arr=(unsigned char *) p;
  size_t i;

  for (i=0; i<cont;i++)
		arr[i]=byte;
}

//--------------funciones P2:----------


int allocate_malloc(char * trozos[], int ntrozos, struct info *inf) {
    if (trozos[2] == NULL) { //mostrar lista de mallocs
        imprimirMalloc(inf->memoria);
    } else {
        int tamb = atoi(trozos[2]);
        void *blockadress;
        blockadress = malloc(tamb);
        printf("Asignados %d bytes en %p\n",tamb,blockadress);
        insertMem(ESMALLOC,tamb,blockadress,&(inf->memoria), 0, NULL, 0);
    }
    return 0;
}

void * ObtenerMemoriaShmget(char *tr[], key_t clave, size_t tam, struct info *inf)
{
    void * p;
    int aux,id,flags=0777;
    struct shmid_ds s;

    if (tam)     /*tam distito de 0 indica crear */
        flags=flags | IPC_CREAT | IPC_EXCL;
    if (clave==IPC_PRIVATE)  /*no nos vale*/
        {errno=EINVAL; return NULL;}
    if ((id=shmget(clave, tam, flags))==-1)
        return (NULL);
    if ((p=shmat(id,NULL,0))==(void*) -1){
        aux=errno;
        if (tam)
             shmctl(id,IPC_RMID,NULL);
        errno=aux;
        return (NULL);
    }
    shmctl (id,IPC_STAT,&s);
 /* Guardar en la lista   InsertarNodoShared (&L, p, s.shm_segsz, clave); */
    insertMem(ESSHARED, s.shm_segsz, p, &inf->memoria, clave, NULL, 0);
    return (p);
}
void do_AllocateCreateshared(char *tr[], struct info *inf)
{
   key_t cl;
   size_t tam;
   void *p;

   if (tr[2]==NULL || tr[3]==NULL) {
		imprimirListaShared(inf->memoria);
		return;
   }
  
   cl=(key_t)  strtoul(tr[2],NULL,10);
   tam=(size_t) strtoul(tr[3],NULL,10);
   if (tam==0) {
	printf ("No se asignan bloques de 0 bytes\n");
	return;
   }
   if ((p=ObtenerMemoriaShmget(tr,cl,tam,inf))!=NULL)
		printf ("Asignados %lu bytes en %p\n",(unsigned long) tam, p);
   else
		printf ("Imposible asignar memoria compartida clave %lu:%s\n",(unsigned long) cl,strerror(errno));
}

int allocate_shared(char *trozos[], struct info *inf) {
    if (trozos[2] == NULL) {
        imprimirListaShared(inf->memoria);
    } else {
        key_t key2 =(key_t) strtoul(trozos[2],NULL,10);
        void *p;

        if ((p=ObtenerMemoriaShmget(trozos,key2,0,inf)))
            printf("Memoria compartida de clave %d en %p\n",key2,p);
        else
            printf("Imposible asignar memoria compartida clave %d:%s\n",key2,strerror(errno));
    }
    return 0;
}

void * MapearFichero (char * fichero, int protection, struct info *inf)
{
    int df, map=MAP_PRIVATE,modo=O_RDONLY;
    struct stat s;
    void *p;

    if (protection&PROT_WRITE)
          modo=O_RDWR;
    if (stat(fichero,&s)==-1 || (df=open(fichero, modo))==-1)
          return NULL;
    if ((p=mmap (NULL,s.st_size, protection,map,df,0))==MAP_FAILED)
           return NULL;
/* Guardar en la lista    InsertarNodoMmap (&L,p, s.st_size,df,fichero); */
    insertMem(ESMMAP, s.st_size, p, &inf->memoria, 0, fichero, df);
    return p;
}
void do_AllocateMmap(char *arg[], struct info *inf)
{ 
     char *perm;
     void *p;
     int protection=0;
     
     if (arg[2]==NULL)
            {imprimirListaMmap(inf->memoria); return;}
     if ((perm=arg[3])!=NULL && strlen(perm)<4) {
            if (strchr(perm,'r')!=NULL) protection|=PROT_READ;
            if (strchr(perm,'w')!=NULL) protection|=PROT_WRITE;
            if (strchr(perm,'x')!=NULL) protection|=PROT_EXEC;
     }
     if ((p=MapearFichero(arg[2],protection, inf))==NULL)
             perror ("Imposible mapear fichero");
     else
             printf ("fichero %s mapeado en %p\n", arg[2], p);
}

int allocate(char * trozos[], int ntrozos, struct info *inf) {
    if (trozos[1] == NULL) { //mostrar todas las listas
        imprimirMalloc(inf->memoria);
        imprimirListaShared(inf->memoria);
        imprimirListaMmap(inf->memoria);
    } else {
        if (!strcmp(trozos[1],"-malloc")) {
            allocate_malloc(trozos, ntrozos, inf);
        } else if (!strcmp(trozos[1],"-createshared")) {
            do_AllocateCreateshared(trozos, inf);
        } else if (!strcmp(trozos[1],"-shared")) {
            allocate_shared(trozos,inf);
        } else if (!strcmp(trozos[1],"-mmap")) {
            do_AllocateMmap(trozos,inf);
        } else {
            printf("uso: allocate [-malloc| -shared| -createshared| -mmap] ...\n");
        }
    }
    return 0;
}

int deallocate_malloc(char * trozos[], int ntrozos, struct info *inf) {
    if (trozos[2] == NULL) {
        imprimirMalloc(inf->memoria);
    } else {
        int tosearch = atoi(trozos[2]);
        struct memelem * elem = NULL;
        searchMallocSize(inf->memoria,&elem,tosearch);
        if (elem != NULL) RemoveElement(&inf->memoria,elem,freeMemoria);
        else printf("No hay bloque de ese tamano asignado con malloc\n");
    }
    return 0;
}
void do_DeallocateDelkey (char *args[])
{
   key_t clave;
   int id;
   char *key=args[2];

   if (key==NULL || (clave=(key_t) strtoul(key,NULL,10))==IPC_PRIVATE){
        printf ("      delkey necesita clave_valida\n");
        return;
   }
   if ((id=shmget(clave,0,0666))==-1){
        perror ("shmget: imposible obtener memoria compartida");
        return;
   }
   if (shmctl(id,IPC_RMID,NULL)==-1)
        perror ("shmctl: imposible eliminar memoria compartida\n");
}
void deallocate_shared(char * trozos[], int ntrozos, struct info *inf) {
    if (trozos[2] == NULL) {
        imprimirListaShared(inf->memoria);
    } else {
        key_t clave = atoi(trozos[2]);
        struct memelem * elem = NULL;
        searchSharedKey(inf->memoria,&elem,clave);
        if (elem==NULL) printf("No hay bloque de esa clave en el proceso\n");
        else RemoveElement(&inf->memoria, elem, freeMemoria);
    }
}
void deallocate_mmap(char * trozos[], int ntrozos, struct info *inf) {
    if (trozos[2] == NULL) {
        imprimirListaMmap(inf->memoria);
    } else {
        char namefiletosearch[MAXL];
        strcpy(namefiletosearch,trozos[2]);
        struct memelem * elem = NULL;
        searchMmapFile(inf->memoria, &elem,namefiletosearch);
        if (elem == NULL) printf("Fichero no mapeado\n");
        else RemoveElement(&inf->memoria, elem, freeMemoria);
    }
}
void deallocate_addr(lista l,char * trozos[], int ntrozos,struct info *inf) {
    void *p = (void *)strtoul(trozos[1],NULL,16);
    struct memelem * elem;
    int check=0;

    for (iterator i = first(&l); !isLast(i); i = next(i)) {
        elem = getElement(i);
        if (elem->dir == p) {
            check = 1;
            if (elem->tpmem == ESMALLOC) {
                int tosearch = elem->size;
                struct memelem * elem1 = NULL;
                searchMallocSize(inf->memoria,&elem1,tosearch);
                if (elem1 != NULL) RemoveElement(&inf->memoria,elem,freeMemoria);
                break;
            }  else if (elem->tpmem == ESSHARED) {
                    key_t clave = elem->key;
                    struct memelem * elem2 = NULL;
                    searchSharedKey(inf->memoria,&elem2,clave);
                    if (elem2 == NULL) printf("No hay bloque de esa clave en el proceso\n");
                    else RemoveElement(&inf->memoria, elem, freeMemoria);
                    break;
            } else if (elem->tpmem == ESMMAP) {
                    char namefiletosearch[MAXL];
                    strcpy(namefiletosearch,elem->namefilemmap);
                    struct memelem * elem3 = NULL;
                    searchMmapFile(inf->memoria, &elem3,namefiletosearch);
                    if (elem3 == NULL) printf("Fichero no mapeado\n");
                    else RemoveElement(&inf->memoria, elem, freeMemoria);
                    break;
            }

        }
    }

    if (!check) printf("Direccion %p no asignada con malloc, shared o mmap\n",p);
    
}
int deallocate(char * trozos[], int ntrozos, struct info *inf) {
    if (trozos[1] == NULL) { //mostrar todas las listas
        imprimirMalloc(inf->memoria);
        imprimirListaShared(inf->memoria);
        imprimirListaMmap(inf->memoria);
    } else {
        if (!strcmp(trozos[1],"-malloc")) {
            deallocate_malloc(trozos, ntrozos, inf);
        } else if (!strcmp(trozos[1],"-delkey")) {
            do_DeallocateDelkey(trozos);
        } else if (!strcmp(trozos[1],"-shared")) {
            deallocate_shared(trozos,ntrozos,inf);
        } else if (!strcmp(trozos[1],"-mmap")) {
            deallocate_mmap(trozos,ntrozos,inf);
        } else {
            deallocate_addr(inf->memoria,trozos,ntrozos,inf);
        }
    }
    return 0;
}

int i_o(char * trozos[], int ntrozos, struct info *inf) {
    if (ntrozos < 5) {
        if (ntrozos == 1) printf("uso: e-s [read|write] ......\n");
        else printf("faltan parametros\n");
    } else {
        if (!strcmp(trozos[1],"read")) { //read
            do_I_O_read(trozos);
        } else if (!strcmp(trozos[1],"write")) { //write
            void *p;
            size_t cont=-1;
            ssize_t n;
            if (!strcmp(trozos[2],"-o")) {
                if (trozos[3] == NULL || trozos[4] == NULL) {
                    printf("faltan parametros\n");
                    return 0;
                }
                p = (void *)strtoul(trozos[4],NULL,16);
                if (trozos[5] != NULL) {
                    cont = (size_t) atoll(trozos[5]);
                }
                if ((n=EscribirFichero(trozos[3],p,cont,1))==-1)
                    perror("Error: imposible escribir fichero");
                else 
                    printf("escritos %lld bytes en %s desde %p\n",(long long) n, trozos[3],p);
            } else {
                if (trozos[2] == NULL || trozos[3] == NULL) {
                    printf("faltan parametros\n");
                    return 0;
                }
                p = (void *)strtoul(trozos[3],NULL,16);
                if (trozos[4] != NULL) {
                    cont = (size_t) atoll(trozos[4]);
                }
                if ((n=EscribirFichero(trozos[2],p,cont,0))==-1)
                    perror("Error: imposible escribir fichero");
                else
                    printf("escritos %lld bytes en %s desde %p\n",(long long) n, trozos[2], p);
            }
        } 
    }
    return 0;
}

int memdump(char * trozos[], int ntrozos, struct info *inf) {
    if (trozos[1] != NULL ) {
        int cont = 25;
        const char *address = (const char *) strtoul(trozos[1],NULL,16);
        if (trozos[2]!=NULL) cont = atoi(trozos[2]);
        unsigned char array[cont];
        for (int i = 0; i < cont; i+=25) {
            for (int j = 0; (j < 25)&&((i+j) < cont); j++) {
                array[j] = (unsigned char) *(address+i+j);
                if (array[j]==10) printf("\n");
                printf("  %c ",(isprint(*(address+i+j)) ? *(address+i+j) : ' '));
            }
            printf("\n");
            for (int j = 0; j < 25 && i+j < cont; j++) {
                printf(" %2.2x ",array[j]);
            }
            printf("\n");
        }
    }
    return 0;
}

int memfill(char * trozos[], int ntrozos, struct info *inf) {
    if (trozos[1] != NULL) {
        void *p = (void *)strtoul(trozos[1],NULL,16);
        size_t cont = 128;
        int byte = 65;
        if (trozos[2] != NULL) cont = (size_t) atof(trozos[2]);
        if (trozos[3] != NULL) byte = atoi(trozos[3]);
        
        LlenarMemoria(p,cont,byte);
        printf("Llenando %ld bytes de memoria con el byte %c(%s) a partir de la direccion %p\n",cont, byte, trozos[3], p);
    }
    return 0;
}


int g1=0, g2=0, g3=0; //Variables globlales
void call_vars() {
    int l1=0, l2=0, l3=0; //variables locales
    static int e1=0, e2=0, e3=0; //variables estaticas
    printf("Variables locales       %p,     %p,     %p\n", &l1, &l2, &l3);
    printf("Variables globales      %p,     %p,     %p\n", &g1, &g2, &g3);
    printf("Variables estaticas     %p,     %p,     %p\n", &e1, &e2, &e3);
}
void call_funcs() {
    printf("Funciones programa      %p,     %p,     %p\n", autores, fecha, ayuda);
    printf("Funciones libreria      %p,     %p,     %p\n", printf, strcpy, atoi);
} 

int memory(char * trozos[], int ntrozos, struct info *inf) {
    if (trozos[1] == NULL) {
        call_vars();
        call_funcs();
        imprimirMalloc(inf->memoria);
    } else {
        if (!strcmp(trozos[1],"-blocks")) {
            imprimirMalloc(inf->memoria);
            imprimirListaShared(inf->memoria);
            imprimirListaMmap(inf->memoria);
        } else if (!strcmp(trozos[1],"-vars")) {
            call_vars();
        } else if (!strcmp(trozos[1],"-funcs")) {
            call_funcs();
        } else if (!strcmp(trozos[1],"-pmap")) {
            Do_pmap();
        } else if (!strcmp(trozos[1],"-all")) {
            call_vars();
            call_funcs();
            imprimirMalloc(inf->memoria);
            imprimirListaShared(inf->memoria);
            imprimirListaMmap(inf->memoria);
        } else printf("Opcion %s no contemplada\n",trozos[1]);
    }
    return 0;
}

void Recursiva (int n)
{
  char automatico[TAMANO];
  static char estatico[TAMANO];

  printf ("parametro:%3d(%p) array %p, arr estatico %p\n",n,&n,automatico, estatico);

  if (n>0)
    Recursiva(n-1);
}
int recurse(char * trozos[], int ntrozos, struct info *inf) {
    if (trozos[1] != NULL) {
        int n = atoi(trozos[1]);
        if ((n<0)||(n==0)) {
            Recursiva(0);
        } else {
            Recursiva(n);
        }
    }
    return 0;
}

//--------------FUNCIONES P0: -------------

int autores(char * trozos[], int ntrozos, struct info *inf){
    if (ntrozos <= 1){  //If there is no option
        printf("Fernando Iglesias Iglesias : f.iglesias2\n");
    } else {
        if (!strcmp(trozos[1],"-n")) //Only names
        {
            printf("Fernando Iglesias Iglesias\n");
        }
        else if (!strcmp(trozos[1], "-l")) //logins
        {
            printf("f.iglesias2\n");
        }
        else {
            printf("%s is a wrong command option\n",trozos[1]);
        }
    }
    return 0;
}
int carpeta(char * trozos[], int ntrozos, struct info *inf){
    char buf[MAXL];
    if (ntrozos==1){
        printf("%s\n",getcwd(buf,MAXL));
    }
    else if (!strcmp(trozos[1],"direct")){
        chdir(trozos[1]);
        printf("%d\n", chdir(trozos[1]));
    }
    else
    {
        if (chdir(trozos[1])){ //if chdir() returns -1 it means an error ocurred
            printf("Error: %s directory does not exist\n",trozos[1]);
        }
    }
    return 0;
}
void CurrentDate () {
    int year, day, month;
    time_t now; //time type variable
    time(&now); //writes current time to NOW variable
    struct tm *local = localtime(&now); //a structure to separate minutes, hours, day, month, year
    year = local->tm_year + 1900;
    month = local->tm_mon + 1;
    day = local->tm_mday;
    printf("Current date: %02d/%02d/%d\n",day,month,year);
}
void CurrentTime () {
    int hours, mins, secs;
    time_t now; //time type variable
    time(&now); //writes current time to NOW variable
    struct tm *local = localtime(&now); //a structure to separate minutes, hours, day, month, year
    hours = local->tm_hour;
    mins = local->tm_min;
    secs = local->tm_sec;
    printf("Current time: %02d:%02d:%02d\n",hours,mins,secs);
}
int fecha(char * trozos[], int ntrozos, struct info *inf) {
    if (trozos[1] == NULL) {
        CurrentDate();
        CurrentTime();
    } else if (!strcmp(trozos[1], "-d")) {
        CurrentDate(); //we call helper function
    } else if (!strcmp(trozos[1], "-h")) {
        CurrentTime(); //we call helper function
    }
    else printf("%s is a wrong command option\n",trozos[1]);

    return 0;
}

int hist(char * trozos[], int ntrozos, struct info *inf) {
    if (trozos[1] != NULL) {
        if (!strcmp(trozos[1],"-c")) {
            disposeAll(first(&inf->historial),freeHist);
            printf("Cleared command history\n");
        } else {
            printf("Incorrect option: \n");
        }
    } else {
        lista pointer = (inf->historial);
        int n = 1;
        printf("\n");
        for (iterator i = first(&pointer); !isLast(i); i = next(i)) {
            printf("%2d -> %s\n",n,(char *)getElement(i));
            n++;
        }
    }
    return 0;
}

int comando(char * trozos[], int ntrozos, struct info *inf) {
    int n = atoi(trozos[1]);
    if (trozos[1] != NULL) {
        if ((n == 0)||(n<0)) {
            printf("Error: enter the correct number of a command\n");
        } else {
            lista pointer = (inf->historial);
            iterator j = first(&pointer);
            for (int i=1; i!=n; i++) {
                j = next(j);
            }
            procesarEntrada((char *)getElement(j),inf);
        }
    }
    return 0;
}

int pid(char * trozos[], int ntrozos, struct info *inf){
    if (trozos[1]==NULL)
    {
        printf("PID of Shell: %i\n", getpid());
    }
    else
    {
        if (!strcmp(trozos[1],"-p"))
        {
            printf("PID of parent process: %i\n", getppid());
        }
        else{
            printf("%s is a wrong command option\n",trozos[1]);
        }
    }
    return 0;
}

int infosis (char * trozos[], int ntrozos, struct info *inf) {
    struct utsname sys;
    if (!uname(&sys)) {
        printf("%s (%s), OS: %s-%s-%s\n",sys.nodename,sys.machine,sys.sysname,sys.release, sys.version);
    }
    return 0;
}
int ayuda (char * trozos[], int ntrozos, struct info *inf) {
    if (trozos[1] == NULL) {
        printf("List of available commands:\n");
        printf(" autores [-l]-n] \n pid [-p] \n carpeta [direct] \n fecha [-d]-h] \n hist[-c]-N] \n comando N \n infosis \n ayuda[cmd] \n fin \n salir \n bye \n"); //p0
        printf(" create \n stat \n list \n delete \n deltree \n "); //p1
        printf(" allocate \n deallocate \n i-o \n memdump \n memfill \n memory \n recurse \n"); //p2
        printf(" priority \n showvar \n changevar \n showenv \n fork \n execute \n listjobs \n deljobs \n jobs \n"); //p3
    } else {
        if (!strcmp(trozos[1],"autores")) {
            printf("Help of: autores [-l]-n]\n");
            printf("Prints the names and logins of the program authors. authors -l prints\n"
                   "only the logins and authors -n prints only the names\n");
        } else if (!strcmp(trozos[1],"pid")) {
            printf("Help of: pid [-p]\n");
            printf("Prints the pid of the process executing the shell. pid -p prints the pid\n"
                   "of the shell’s parent process\n");
        } else if (!strcmp(trozos[1],"carpeta")) {
            printf("Help of: carpeta [direct]\n");
            printf("Changes the current working directory of the shell to direct. When invoked without auguments it prints the current working directory \n");
        } else if (!strcmp(trozos[1],"fecha")) {
            printf("Help of: fecha [-d]-h]\n");
            printf("Without arguments it prints both the current date and the current time. fecha -d prints the current date in the format DD/MM/YYYY. fecha -h prints the current time in the format hh:mm:ss");
        } else if (!strcmp(trozos[1],"hist")) {
            printf("Help of: hist[-c]-N]\n");
            printf("Without arguments prints all the commands that have been input with their order number. hist -c clears the list of historic commands. hist -N prints de first N commands\n");
        } else if (!strcmp(trozos[1], "comando")) {
            printf("Help of: comando N\n");
            printf("Prints de command number N from historical list\n");
        } else if (!strcmp(trozos[1],"infosis")) {
            printf("Help of: infosis\n");
            printf("Prints information on the machine running the shell\n");
        } else if (!strcmp(trozos[1],"ayuda")) {
            printf("Help of: ayuda [cmd]\n");
            printf("ayuda without argument displays a list of all available commands. ayuda cmd gives a brief help on the usage of command cmd\n");
        } else if (!strcmp(trozos[1],"fin")) {
            printf("Help of: fin\n");
            printf("Ends the shell\n");
        } else if (!strcmp(trozos[1],"salir")) {
            printf("Help of: salir\n");
            printf("Ends the shell\n");
        } else if (!strcmp(trozos[1], "bye")) {
            printf("Help of: bye\n");
            printf("Ends the shell\n");
        } else if (!strcmp(trozos[1],"create")) {
            printf("Help of: create [-f][name]:\n");
            printf("Creates a directory or a file (-f)\n");
        } else if (!strcmp(trozos[1],"stat")) {
            printf("Help of: stat\n");
            printf("stat [-long][-link][-acc] name1 name2 ...  list files;\n");
            printf("-long: long listing\n-acc: acesstime\n-link: if it is a symbolic link, the content path\n");
        } else if (!strcmp(trozos[1],"list")) {
            printf("Help of: list\n");
            printf("list [-reca][-recb][-hid][-long][-link][-acc] name1 name 2 ... content list;\n");
            printf("-hid: include hidden files\n-reca: recursive (before)\n-recb: recursive (after)\n rest parameters as stat\n");
        } else if (!strcmp(trozos[1],"delete")) {
            printf("Help of: delete\n");
            printf("delete [name 1 name2 ...] Delete empty files or directories\n");
        } else if (!strcmp(trozos[1],"deltree")) {
            printf("Help of: deltree\n");
            printf("deltree [name1 name2 ...] Delete files or directories recursively\n");
        } else if (!strcmp(trozos[1],"allocate")) {
            printf("allocate [-malloc|-shared|-createshared|-mmap]... Asigna un bloque de memoria\n");
            printf("-malloc tam: asigna un bloque malloc de tamano tam\n-createshared cl tam: asigna (creando) el bloque de memoria compartida de clave cl y tamano tam\n");
            printf("-shared cl: asigna el bloque de memoria compartida (ya existente) de clave cl\n-mmap fich perm: mapea el fichero fich, perm son los permisos\n");
        } else if (!strcmp(trozos[1],"deallocate")) {
            printf("deallocate [-malloc|-shared|-delkey|-mmap|addr]..	Desasigna un bloque de memoria\n");
            printf("-malloc tam: desasigna el bloque malloc de tamano tam\n-shared cl: desasigna (desmapea) el bloque de memoria compartida de clave cl\n");
            printf("-delkey cl: elimina del sistema (sin desmapear) la clave de memoria cl\n-mmap fich: desmapea el fichero mapeado fich\n");
            printf("addr: desasigna el bloque de memoria en la direccion addr\n");
        } else if (!strcmp(trozos[1],"i-o")) {
            printf("i-o [read|write] [-o] fiche addr cont\n");
            printf("read fich addr cont: Lee cont bytes desde fich a addr\nwrite [-o] fich addr cont: Escribe cont bytes desde addr a fich. -o para sobreescribir\n");
            printf("addr es una direccion de memoria\n");
        } else if (!strcmp(trozos[1],"memdump")) {
            printf("memdump addr cont\n");
            printf("Vuelca en pantallas los contenidos (cont bytes) de la posicion de memoria addr\n");
        } else if (!strcmp(trozos[1],"memfill")) {
            printf("memfill addr cont byte 	Llena la memoria a partir de addr con byte\n");
        } else if (!strcmp(trozos[1],"memory")) {
            printf("memory [-blocks|-funcs|-vars|-all|-pmap] ..\n");
            printf("Muestra muestra detalles de la memoria del proceso\n");
            printf("-blocks: los bloques de memoria asignados\n-funcs: las direcciones de las funciones\n-vars: las direcciones de las variables\n");
            printf(":-all: todo\n-pmap: muestra la salida del comando pmap(o similar)\n");
        } else if (!strcmp(trozos[1],"recurse")) {
            printf("recurse [n]	Invoca a la funcion recursiva n veces\n");
        } else if (!strcmp(trozos[1],"priority")) {
            printf("priority [pid] [valor] 	Muestra o cambia la prioridad del proceso pid a valor\n");
        } else if (!strcmp(trozos[1],"showvar")) {
            printf("showvar var	Muestra el valor y las direcciones de la variable de entorno var\n");
        } else if (!strcmp(trozos[1],"changevar")) {
            printf("changevar [-a|-e|-p] var valor	Cambia el valor de una variable de entorno\n");
            printf("-a: accede por el tercer arg de main\n-e: accede mediante environ\n-p: accede mediante putenv\n");
        } else if (!strcmp(trozos[1],"showenv")) {
            printf("showenv [-environ|-addr] 	 Muestra el entorno del proceso\n");
            printf("-environ: accede usando environ (en lugar del tercer arg de main)\n");
            printf("-addr: muestra el valor y donde se almacenan environ y el 3er arg main \n");
        } else if (!strcmp(trozos[1],"fork")) {
            printf("fork 	El shell hace fork y queda en espera a que su hijo termine\n");
        } else if (!strcmp(trozos[1],"execute")) {
            printf("execute VAR1 VAR2 ..prog args....[@pri]	Ejecuta, sin crear proceso,prog con argumentos en un entorno que contiene solo las variables VAR1, VAR2..\n");
        } else if (!strcmp(trozos[1],"listjobs")) {
            printf("listjobs 	Lista los procesos en segundo plano\n");
        } else if (!strcmp(trozos[1],"deljobs")) {
            printf("deljobs [-term][-sig]	Elimina los procesos de la lista procesos en sp\n-term: los terminados\n-sig: los terminados por senal\n");
        } else if (!strcmp(trozos[1],"jobs")) {
            printf("jobs [-fg] pid	Muestra informacion del proceso pid.\n-fg: lo pasa a primer plano\n");
        }
        else printf("%s is a wrong command option\n",trozos[1]);
    }
    return 0;
}

int fin (char * trozos[], int ntrozos, struct info *inf) {
    return 1;
}

int salir (char * trozos[], int ntrozos, struct info *inf) {
    return 1;
}

int bye (char * trozos[], int ntrozos, struct info *inf) {
    return 1;
}

//--------------FUNCIONES P1: -------------

void imprimirActual () {
    char buf[MAXL];
    printf("%s\n",getcwd(buf,MAXL));
}

int createDirectory (const char * name) {
    int error;
    error= mkdir(name,0777);
    if (error!=-1) {
        printf("The directory has been created successfully\n");
        return 0;
    } else {
        perror("Error: ");
        return -1;
    }
}
int createFile (char * name) {
    FILE* error;
    error = fopen(name,"w+");
    if (error==NULL) {
        perror("Error: ");
        return -1;
    } else {
        printf("The file has been created successfully\n");
        return 0;
    }
}
int create (char * trozos[], int ntrozos, struct info *inf) {
    if (trozos[1]==NULL) {
        char buf[MAXL];
        printf("%s\n",getcwd(buf,MAXL));
    }
    if (!strcmp(trozos[1],"-f")) {
        createFile(trozos[2]);
    } else {
        createDirectory(trozos[1]);
    }
    return 0;
}

char LetraTF (mode_t m) {
    switch (m&S_IFMT) { /*and bit a bit con los bits de formato,0170000 */
        case S_IFSOCK: return 's'; /*socket */
        case S_IFLNK: return 'l'; /*symbolic link*/
        case S_IFREG: return '-'; /* fichero normal*/
        case S_IFBLK: return 'b'; /*block device*/
        case S_IFDIR: return 'd'; /*directorio */
        case S_IFCHR: return 'c'; /*char device*/
        case S_IFIFO: return 'p'; /*pipe*/
        default: return '?'; /*desconocido, no deberia aparecer*/
    }
}

int deletee (char * trozos[], int ntrozos, struct info *inf) {
    struct stat statbuf;
    if (trozos[1]==NULL) {
        imprimirActual();
    }
    else {
        for (int i=1; i<ntrozos ; i++) {
            int checkstat = lstat(trozos[i],&statbuf);
            if (checkstat != -1) {
                char isdir = LetraTF(statbuf.st_mode);
                if (isdir == 'd') {
                    if (rmdir(trozos[i])==-1) {
                        perror("Error: ");
                    }
                } else {
                    if (unlink(trozos[i])==-1) {
                        perror("Error: ");
                    }
                }
            } else {
                perror("Error: ");
            }
        }
    }
    return 0;
}
int recdelete (char *name) {
    DIR *dirpointer;
    struct dirent *contains;
    char todelete[MAXL];
    dirpointer = opendir(name);
    if (dirpointer!=NULL) {
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(todelete,name);
            strcat(strcat(todelete,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                struct stat statbuf;
                int checkstat = lstat(todelete,&statbuf);
                if (checkstat!=-1) {
                    char isdir = LetraTF(statbuf.st_mode);
                    if (isdir == 'd') {
                        recdelete(todelete);
                    }
                    if (remove(todelete)!=0) {
                        return -1;
                    }
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int deltree (char * trozos[], int ntrozos, struct info *inf) {
    struct stat statbuf;
    if (trozos[1]!=NULL) {
        for (int i=1; i<ntrozos; i++) {
            int checkstat = lstat(trozos[i],&statbuf);
            if (checkstat!=-1) {
                char isdir = LetraTF(statbuf.st_mode);
                if (isdir == 'd') {
                    if (recdelete(trozos[i])!=0) {
                        perror("Error: ");
                    }
                } else {
                    if(remove(trozos[i])!=0) {
                        perror("Error: ");
                    }
                }
            } else {
                perror("Error: ");
            }
            if (remove(trozos[1])!=0) {
                perror("Error: ");
            }
        }
    } else {
        imprimirActual();
    }
    return 0;
}
char * ConvierteModo2 (mode_t m)
{
    static char permisos[12];
    strcpy (permisos,"---------- ");

    permisos[0]=LetraTF(m);
    if (m&S_IRUSR) permisos[1]='r';    /*propietario*/
    if (m&S_IWUSR) permisos[2]='w';
    if (m&S_IXUSR) permisos[3]='x';
    if (m&S_IRGRP) permisos[4]='r';    /*grupo*/
    if (m&S_IWGRP) permisos[5]='w';
    if (m&S_IXGRP) permisos[6]='x';
    if (m&S_IROTH) permisos[7]='r';    /*resto*/
    if (m&S_IWOTH) permisos[8]='w';
    if (m&S_IXOTH) permisos[9]='x';
    if (m&S_ISUID) permisos[3]='s';    /*setuid, setgid y stickybit*/
    if (m&S_ISGID) permisos[6]='s';
    if (m&S_ISVTX) permisos[9]='t';

    return permisos;
}


int stats (char * trozos[], int ntrozos, struct info *inf) {
    struct stat statbuf; int longg=0, link=0, acc=0, call=0, totalops=0;
    if (ntrozos>1) {
        for (int i = 1; i < ntrozos; i++) { //search options
            if (!(strcmp(trozos[i],"-long"))) ++longg;
            else if (!(strcmp(trozos[i],"-link"))) ++link;
            else if (!(strcmp(trozos[i],"-acc"))) ++acc;
            else {
                ++call;
                break;
            }
        }
        totalops = longg+link+acc;
        if (totalops==0) { //stat name (without options)
            for (int i = 1; i < ntrozos; i++) {
                if (lstat(trozos[i],&statbuf)!=-1) {
                    printf("%lld %s\n",(long long )statbuf.st_size,trozos[i]);
                } else {
                    perror("Error: ");
                }
            }
        } else {
            if (call==0) { //stat without name
                imprimirActual();
            } else {
                if (longg==0) { //no -long
                    for (int i = totalops+1; i < ntrozos; i++) {
                        if (lstat(trozos[i],&statbuf)!=-1) {
                            printf("%lld %s\n",(long long)statbuf.st_size,trozos[i]);
                        } else {
                            perror("Error: ");
                        }
                    }
                } else { //exist -long
                    if ((link==0)&&(acc==0)) { //only -long
                        for (int i = totalops+1; i < ntrozos; i++) {
                            if(lstat(trozos[i],&statbuf)!=-1) {
                                char *permisos="---------- ";
                                permisos = ConvierteModo2(statbuf.st_mode);
                                printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);
                            } else {
                                perror("Error: ");
                            }
                        }
                    } else { //-acc or -link
                        if (acc!=0) { //options: -long -acc
                            if (link==0) { //no -link
                                for (int i = totalops+1; i < ntrozos; i++) {
                                    if(lstat(trozos[i],&statbuf)!=-1) {
                                        char *permisos="---------- ";
                                        permisos = ConvierteModo2(statbuf.st_mode);
                                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);
                                    } else {
                                        perror("Error: ");
                                    }
                                }
                            } else { //options: -long -acc -link
                                for (int i = totalops+1; i < ntrozos; i++) {
                                    if(lstat(trozos[i],&statbuf)!=-1) {
                                        char isSimbolic = LetraTF(statbuf.st_mode);
                                        char *permisos="---------- ";
                                        permisos = ConvierteModo2(statbuf.st_mode);
                                        if (isSimbolic=='l') { //search link
                                            char buf[200];
                                            if (readlink(trozos[i],buf,200)!=-1) {
                                                printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i],buf);
                                            } else {
                                                perror("Error: ");
                                            }
                                        } else {
                                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);
                                        }
                                    } else {
                                        perror("Error: ");
                                    }
                                }
                            }
                        } else { //options: -long -link
                            for (int i = totalops+1; i < ntrozos; i++) {
                                if(lstat(trozos[i],&statbuf)!=-1) {
                                    char isSimbolic = LetraTF(statbuf.st_mode);
                                    char *permisos="---------- ";
                                    permisos = ConvierteModo2(statbuf.st_mode);
                                    if (isSimbolic=='l') { //search link
                                        char buf[200];
                                        if (readlink(trozos[i],buf,200)!=-1) {
                                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i],buf);
                                        } else {
                                            perror("Error: ");
                                        }
                                    } else {
                                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);
                                    }
                                } else {
                                    perror("Error: ");
                                }
                            }
                        }
                    }
                }

            }

        }
    } else {
        imprimirActual();
    }
    return 0;
}
int isDirectory (char *name) {
    struct stat statbuf;
    if (lstat(name,&statbuf) != -1) {
        if (LetraTF(statbuf.st_mode) != 'd') {
            return -1;
        }
    } else {
        return -1;
    }
    return 0;
}
//Functions for list
int reca1 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    lstat(tosearch, &statbuf);
                    printf("%lld %s\n", (long long) statbuf.st_size, contains->d_name);
                }
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    if (isDirectory(tosearch)==0) {
                        reca1(tosearch);
                    }
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int reca2 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                lstat(tosearch, &statbuf);
                printf("%lld %s\n", (long long) statbuf.st_size, contains->d_name);
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (isDirectory(tosearch)==0) {
                    reca2(tosearch);
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int reca3 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    lstat(tosearch, &statbuf);
                    char *permisos="---------- ";
                    permisos = ConvierteModo2(statbuf.st_mode);
                    printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                }
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    if (isDirectory(tosearch)==0) {
                        reca3(tosearch);
                    }
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int reca4 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                lstat(tosearch, &statbuf);
                char *permisos="---------- ";
                permisos = ConvierteModo2(statbuf.st_mode);
                printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    if (isDirectory(tosearch)==0) {
                        reca4(tosearch);
                    }
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int reca5 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    lstat(tosearch, &statbuf);
                    char *permisos="---------- ";
                    permisos = ConvierteModo2(statbuf.st_mode);
                    printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                }
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    if (isDirectory(tosearch)==0) {
                        reca5(tosearch);
                    }
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int reca6 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                lstat(tosearch, &statbuf);
                char *permisos="---------- ";
                permisos = ConvierteModo2(statbuf.st_mode);
                printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    if (isDirectory(tosearch)==0) {
                        reca6(tosearch);
                    }
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int reca7 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    lstat(tosearch, &statbuf);
                    char *permisos="---------- ";
                    permisos = ConvierteModo2(statbuf.st_mode);
                    char isSimbolic = LetraTF(statbuf.st_mode);
                    if (isSimbolic=='l') {
                        char buf[200];
                        if (readlink(tosearch,buf,200)!=-1) {
                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name,buf);
                        } else {
                            perror("Error: ");
                        }
                    } else {
                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                    }
                }
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    if (isDirectory(tosearch)==0) {
                        reca7(tosearch);
                    }
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int reca8 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                lstat(tosearch, &statbuf);
                char *permisos="---------- ";
                permisos = ConvierteModo2(statbuf.st_mode);
                char isSimbolic = LetraTF(statbuf.st_mode);
                if (isSimbolic=='l') {
                    char buf[200];
                    if (readlink(tosearch,buf,200)!=-1) {
                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name,buf);
                    } else {
                        perror("Error: ");
                    }
                } else {
                    printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                }
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    if (isDirectory(tosearch)==0) {
                        reca8(tosearch);
                    }
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int reca9 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    lstat(tosearch, &statbuf);
                    char *permisos="---------- ";
                    permisos = ConvierteModo2(statbuf.st_mode);
                    char isSimbolic = LetraTF(statbuf.st_mode);
                    if (isSimbolic=='l') {
                        char buf[200];
                        if (readlink(tosearch,buf,200)!=-1) {
                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name,buf);
                        } else {
                            perror("Error: ");
                        }
                    } else {
                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                    }
                }
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    if (isDirectory(tosearch)==0) {
                        reca9(tosearch);
                    }
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int reca10 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                lstat(tosearch, &statbuf);
                char *permisos="---------- ";
                permisos = ConvierteModo2(statbuf.st_mode);
                char isSimbolic = LetraTF(statbuf.st_mode);
                if (isSimbolic=='l') {
                    char buf[200];
                    if (readlink(tosearch,buf,200)!=-1) {
                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name,buf);
                    } else {
                        perror("Error: ");
                    }
                } else {
                    printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                }
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    if (isDirectory(tosearch)==0) {
                        reca10(tosearch);
                    }
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int recb1 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    if (isDirectory(tosearch)==0) {
                        recb1(tosearch);
                    }
                }
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    lstat(tosearch, &statbuf);
                    printf("%lld %s\n", (long long) statbuf.st_size, contains->d_name);
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int recb2 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (isDirectory(tosearch)==0) {
                    recb2(tosearch);
                }
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                lstat(tosearch, &statbuf);
                printf("%lld %s\n", (long long) statbuf.st_size, contains->d_name);
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int recb3 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    if (isDirectory(tosearch)==0) {
                        recb3(tosearch);
                    }
                }
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    lstat(tosearch, &statbuf);
                    char *permisos="---------- ";
                    permisos = ConvierteModo2(statbuf.st_mode);
                    printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int recb4 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (isDirectory(tosearch)==0) {
                    recb4(tosearch);
                }
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                lstat(tosearch, &statbuf);
                char *permisos="---------- ";
                permisos = ConvierteModo2(statbuf.st_mode);
                printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int recb5 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    if (isDirectory(tosearch)==0) {
                        recb5(tosearch);
                    }
                }
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    lstat(tosearch, &statbuf);
                    char *permisos="---------- ";
                    permisos = ConvierteModo2(statbuf.st_mode);
                    printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int recb6 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (isDirectory(tosearch)==0) {
                    recb6(tosearch);
                }
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                lstat(tosearch, &statbuf);
                char *permisos="---------- ";
                permisos = ConvierteModo2(statbuf.st_mode);
                printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int recb7 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    if (isDirectory(tosearch)==0) {
                        recb7(tosearch);
                    }
                }
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    lstat(tosearch, &statbuf);
                    char *permisos="---------- ";
                    permisos = ConvierteModo2(statbuf.st_mode);
                    char isSimbolic = LetraTF(statbuf.st_mode);
                    if (isSimbolic=='l') {
                        char buf[200];
                        if (readlink(tosearch,buf,200)!=-1) {
                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name,buf);
                        } else {
                            perror("Error: ");
                        }
                    } else {
                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                    }
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int recb8 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    if (isDirectory(tosearch)==0) {
                        recb8(tosearch);
                    }
                }
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                lstat(tosearch, &statbuf);
                char *permisos="---------- ";
                permisos = ConvierteModo2(statbuf.st_mode);
                char isSimbolic = LetraTF(statbuf.st_mode);
                if (isSimbolic=='l') {
                    char buf[200];
                    if (readlink(tosearch,buf,200)!=-1) {
                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name,buf);
                    } else {
                        perror("Error: ");
                    }
                } else {
                    printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int recb9 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    if (isDirectory(tosearch)==0) {
                        recb9(tosearch);
                    }
                }
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    lstat(tosearch, &statbuf);
                    char *permisos="---------- ";
                    permisos = ConvierteModo2(statbuf.st_mode);
                    char isSimbolic = LetraTF(statbuf.st_mode);
                    if (isSimbolic=='l') {
                        char buf[200];
                        if (readlink(tosearch,buf,200)!=-1) {
                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name,buf);
                        } else {
                            perror("Error: ");
                        }
                    } else {
                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                    }
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int recb10 (char *name) {
    struct stat statbuf;
    DIR *dirpointer;
    struct dirent *contains;
    char tosearch[MAXL];
    dirpointer=opendir(name);
    if (dirpointer!=NULL) {
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                if (contains->d_name[0]!='.') {
                    if (isDirectory(tosearch)==0) {
                        recb10(tosearch);
                    }
                }
            }
        }
        closedir(dirpointer);
        dirpointer=opendir(name);
        printf("************%s\n",name);
        while ((contains=readdir(dirpointer))!=NULL) {
            strcpy(tosearch,name);
            strcat(strcat(tosearch,"/"),contains->d_name);
            if (!((!strcmp(contains->d_name,"."))||(!strcmp(contains->d_name,"..")))) {
                lstat(tosearch, &statbuf);
                char *permisos="---------- ";
                permisos = ConvierteModo2(statbuf.st_mode);
                char isSimbolic = LetraTF(statbuf.st_mode);
                if (isSimbolic=='l') {
                    char buf[200];
                    if (readlink(tosearch,buf,200)!=-1) {
                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name,buf);
                    } else {
                        perror("Error: ");
                    }
                } else {
                    printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                }
            }
        }
        closedir(dirpointer);
    } else {
        return -1;
    }
    return 0;
}
int listar (char * trozos[], int ntrozos, struct info *inf) {
    struct stat statbuf; int longg=0, link=0, acc=0, call=0, reca=0, recb=0, hid=0, totalops=0;
    char tolist[MAXL];
    if (ntrozos>1) {
        for (int i = 1; i < ntrozos; i++) { //search options
            if (!(strcmp(trozos[i],"-long"))) ++longg;
            else if (!(strcmp(trozos[i],"-link"))) ++link;
            else if (!(strcmp(trozos[i],"-acc"))) ++acc;
            else if (!(strcmp(trozos[i],"-reca"))) ++reca;
            else if (!(strcmp(trozos[i],"-recb"))) ++recb;
            else if (!(strcmp(trozos[i],"-hid"))) ++hid;
            else {
                ++call;
                break;
            }
        }
        totalops = longg+link+acc+reca+recb+hid;
        if (call!=0) {
            if (totalops==0) { //no options
                for (int i = 1; i < ntrozos; i++) {
                    if (lstat(trozos[i], &statbuf) != -1) {
                        if (isDirectory(trozos[i]) == 0) {
                            printf("************%s\n",trozos[i]);
                            DIR *dirpointer;
                            struct dirent *contains;
                            dirpointer = opendir(trozos[i]);
                            if (dirpointer != NULL) {
                                while ((contains = readdir(dirpointer)) != NULL) {
                                    strcpy(tolist,trozos[i]);
                                    strcat(strcat(tolist,"/"),contains->d_name);
                                    if (!((!strcmp(contains->d_name, ".")) || (!strcmp(contains->d_name, "..")))) {
                                        if (contains->d_name[0]!='.') {
                                            lstat(tolist, &statbuf);
                                            printf("%lld %s\n", (long long) statbuf.st_size, contains->d_name);
                                        }
                                    }
                                }
                                closedir(dirpointer);
                            } else {
                                perror("Error: ");
                            }
                        } else { //is not a directory
                            printf("%lld %s\n", (long long) statbuf.st_size, trozos[i]);
                        }
                    } else {
                        perror("Error: ");
                    }
                }
            } else { //exist options
                if (longg==0) { //no long
                    if (hid==0) { //no hid
                        if ((recb==0)&&(reca==0)) { //no reca no recb
                            for (int i = totalops+1; i < ntrozos; i++) {
                                if (lstat(trozos[i], &statbuf) != -1) {
                                    if (isDirectory(trozos[i]) == 0) {
                                        printf("************%s\n",trozos[i]);
                                        DIR *dirpointer;
                                        struct dirent *contains;
                                        dirpointer = opendir(trozos[i]);
                                        if (dirpointer != NULL) {
                                            while ((contains = readdir(dirpointer)) != NULL) {
                                                strcpy(tolist,trozos[i]);
                                                strcat(strcat(tolist,"/"),contains->d_name);
                                                if (!((!strcmp(contains->d_name, ".")) || (!strcmp(contains->d_name, "..")))) {
                                                    if (contains->d_name[0]!='.') {
                                                        lstat(tolist, &statbuf);
                                                        printf("%lld %s\n", (long long) statbuf.st_size, contains->d_name);
                                                    }
                                                }
                                            }
                                            closedir(dirpointer);
                                        } else {
                                            perror("Error: ");
                                        }
                                    } else { //is not a directory
                                        printf("%lld %s\n", (long long) statbuf.st_size, trozos[i]);
                                    }
                                } else {
                                    perror("Error: ");
                                }
                            }
                        } else { //exist -recb or -reca
                            if (reca!=0) { //-reca
                                for (int i = totalops+1; i < ntrozos; i++) {
                                    if (lstat(trozos[i], &statbuf) != -1) {
                                        if (isDirectory(trozos[i]) == 0) {
                                            if (reca1(trozos[i])==-1) {
                                                perror("Error: ");
                                            }
                                        } else {
                                            printf("%lld %s\n", (long long) statbuf.st_size, trozos[i]);
                                        }
                                    } else {
                                        perror("Error: ");
                                    }
                                }
                            } else { //-recb
                                for (int i = totalops+1; i < ntrozos; i++) {
                                    if (lstat(trozos[i], &statbuf) != -1) {
                                        if (isDirectory(trozos[i]) == 0) {
                                            if (recb1(trozos[i])==-1) {
                                                perror("Error: ");
                                            }
                                        } else {
                                            printf("%lld %s\n", (long long) statbuf.st_size, trozos[i]);
                                        }
                                    } else {
                                        perror("Error: ");
                                    }
                                }
                            }
                        }
                    } else { //-hid
                        if ((recb==0)&&(reca==0)) {
                            for (int i = totalops+1; i < ntrozos; i++) {
                                if (lstat(trozos[i], &statbuf) != -1) {
                                    if (isDirectory(trozos[i]) == 0) {
                                        printf("************%s\n",trozos[i]);
                                        DIR *dirpointer;
                                        struct dirent *contains;
                                        dirpointer = opendir(trozos[i]);
                                        if (dirpointer != NULL) {
                                            while ((contains = readdir(dirpointer)) != NULL) {
                                                strcpy(tolist,trozos[i]);
                                                strcat(strcat(tolist,"/"),contains->d_name);
                                                if (!((!strcmp(contains->d_name, ".")) || (!strcmp(contains->d_name, "..")))) {
                                                    lstat(tolist, &statbuf);
                                                    printf("%lld %s\n", (long long) statbuf.st_size, contains->d_name);
                                                }
                                            }
                                            closedir(dirpointer);
                                        } else {
                                            perror("Error: ");
                                        }
                                    } else { //is not a directory
                                        printf("%lld %s\n", (long long) statbuf.st_size, trozos[i]);
                                    }
                                } else {
                                    perror("Error: ");
                                }
                            }
                        } else { //exist -reca or -recb
                            if (reca!=0) { //-hid -reca
                                for (int i = totalops+1; i < ntrozos; i++) {
                                    if (lstat(trozos[i], &statbuf) != -1) {
                                        if (isDirectory(trozos[i]) == 0) {
                                            if (reca2(trozos[i])==-1) {
                                                perror("Error: ");
                                            }
                                        } else {
                                            printf("%lld %s\n", (long long) statbuf.st_size, trozos[i]);
                                        }
                                    } else {
                                        perror("Error: ");
                                    }
                                }
                            } else { //-hid -recb
                                for (int i = totalops+1; i < ntrozos; i++) {
                                    if (lstat(trozos[i], &statbuf) != -1) {
                                        if (isDirectory(trozos[i]) == 0) {
                                            if (recb2(trozos[i])==-1) {
                                                perror("Error: ");
                                            }
                                        } else {
                                            printf("%lld %s\n", (long long) statbuf.st_size, trozos[i]);
                                        }
                                    } else {
                                        perror("Error: ");
                                    }
                                }
                            }
                        }
                    }
                } else { //-long
                    if ((recb==0)&&(reca==0)) { //no -recb no -reca
                        if ((acc==0)&&(link==0)) { //no -acc no -link
                            if (hid==0) { //no -hid (only -long)
                                for (int i = totalops+1; i < ntrozos; i++) {
                                    if (lstat(trozos[i], &statbuf) != -1) {
                                        if (isDirectory(trozos[i]) == 0) {
                                            printf("************%s\n",trozos[i]);
                                            DIR *dirpointer;
                                            struct dirent *contains;
                                            dirpointer = opendir(trozos[i]);
                                            if (dirpointer != NULL) {
                                                while ((contains = readdir(dirpointer)) != NULL) {
                                                    strcpy(tolist,trozos[i]);
                                                    strcat(strcat(tolist,"/"),contains->d_name);
                                                    if (!((!strcmp(contains->d_name, ".")) || (!strcmp(contains->d_name, "..")))) {
                                                        if (contains->d_name[0]!='.') {
                                                            lstat(tolist, &statbuf);
                                                            char *permisos="---------- ";
                                                            permisos = ConvierteModo2(statbuf.st_mode);
                                                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                                                        }
                                                    }
                                                }
                                                closedir(dirpointer);
                                            } else {
                                                perror("Error: ");
                                            }
                                        } else { //is not a directory
                                            char *permisos="---------- ";
                                            permisos = ConvierteModo2(statbuf.st_mode);
                                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);
                                        }
                                    } else {
                                        perror("Error: ");
                                    }
                                }
                            } else { //-long -hid
                                for (int i = totalops+1; i < ntrozos; i++) {
                                    if (lstat(trozos[i], &statbuf) != -1) {
                                        if (isDirectory(trozos[i]) == 0) {
                                            printf("************%s\n",trozos[i]);
                                            DIR *dirpointer;
                                            struct dirent *contains;
                                            dirpointer = opendir(trozos[i]);
                                            if (dirpointer != NULL) {
                                                while ((contains = readdir(dirpointer)) != NULL) {
                                                    strcpy(tolist,trozos[i]);
                                                    strcat(strcat(tolist,"/"),contains->d_name);
                                                    if (!((!strcmp(contains->d_name, ".")) || (!strcmp(contains->d_name, "..")))) {
                                                        lstat(tolist, &statbuf);
                                                        char *permisos="---------- ";
                                                        permisos = ConvierteModo2(statbuf.st_mode);
                                                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);

                                                    }
                                                }
                                                closedir(dirpointer);
                                            } else {
                                                perror("Error: ");
                                            }
                                        } else {
                                            char *permisos="---------- ";
                                            permisos = ConvierteModo2(statbuf.st_mode);
                                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);
                                        }
                                    } else {
                                        perror("Error: ");
                                    }
                                }
                            }
                        } else {
                            if (acc!=0) { //exist -acc
                                if (link==0) {
                                    if (hid==0) { //-long -acc
                                        for (int i = totalops+1; i < ntrozos; i++) {
                                            if (lstat(trozos[i], &statbuf) != -1) {
                                                if (isDirectory(trozos[i]) == 0) {
                                                    printf("************%s\n",trozos[i]);
                                                    DIR *dirpointer;
                                                    struct dirent *contains;
                                                    dirpointer = opendir(trozos[i]);
                                                    if (dirpointer != NULL) {
                                                        while ((contains = readdir(dirpointer)) != NULL) {
                                                            strcpy(tolist,trozos[i]);
                                                            strcat(strcat(tolist,"/"),contains->d_name);
                                                            if (!((!strcmp(contains->d_name, ".")) || (!strcmp(contains->d_name, "..")))) {
                                                                if (contains->d_name[0]!='.') {
                                                                    lstat(tolist, &statbuf);
                                                                    char *permisos="---------- ";
                                                                    permisos = ConvierteModo2(statbuf.st_mode);
                                                                    printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                                                                }
                                                            }
                                                        }
                                                        closedir(dirpointer);
                                                    } else {
                                                        perror("Error: ");
                                                    }
                                                } else { //is not a directory
                                                    char *permisos="---------- ";
                                                    permisos = ConvierteModo2(statbuf.st_mode);
                                                    printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);
                                                }
                                            } else {
                                                perror("Error: ");
                                            }
                                        }
                                    } else { //-long -acc -hid
                                        for (int i = totalops+1; i < ntrozos; i++) {
                                            if (lstat(trozos[i], &statbuf) != -1) {
                                                if (isDirectory(trozos[i]) == 0) {
                                                    printf("************%s\n",trozos[i]);
                                                    DIR *dirpointer;
                                                    struct dirent *contains;
                                                    dirpointer = opendir(trozos[i]);
                                                    if (dirpointer != NULL) {
                                                        while ((contains = readdir(dirpointer)) != NULL) {
                                                            strcpy(tolist,trozos[i]);
                                                            strcat(strcat(tolist,"/"),contains->d_name);
                                                            if (!((!strcmp(contains->d_name, ".")) || (!strcmp(contains->d_name, "..")))) {
                                                                lstat(tolist, &statbuf);
                                                                char *permisos="---------- ";
                                                                permisos = ConvierteModo2(statbuf.st_mode);
                                                                printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);

                                                            }
                                                        }
                                                        closedir(dirpointer);
                                                    } else {
                                                        perror("Error: ");
                                                    }
                                                } else {
                                                    char *permisos="---------- ";
                                                    permisos = ConvierteModo2(statbuf.st_mode);
                                                    printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);
                                                }
                                            } else {
                                                perror("Error: ");
                                            }
                                        }
                                    }
                                } else {
                                    if (hid==0) { //-long -acc -link
                                        for (int i = totalops+1; i < ntrozos; i++) {
                                            if (lstat(trozos[i], &statbuf) != -1) {
                                                if (isDirectory(trozos[i]) == 0) {
                                                    printf("************%s\n",trozos[i]);
                                                    DIR *dirpointer;
                                                    struct dirent *contains;
                                                    dirpointer = opendir(trozos[i]);
                                                    if (dirpointer != NULL) {
                                                        while ((contains = readdir(dirpointer)) != NULL) {
                                                            strcpy(tolist,trozos[i]);
                                                            strcat(strcat(tolist,"/"),contains->d_name);
                                                            if (!((!strcmp(contains->d_name, ".")) || (!strcmp(contains->d_name, "..")))) {
                                                                if (contains->d_name[0]!='.') {
                                                                    lstat(tolist, &statbuf);
                                                                    char *permisos="---------- ";
                                                                    permisos = ConvierteModo2(statbuf.st_mode);
                                                                    char isSimbolic = LetraTF(statbuf.st_mode);
                                                                    if (isSimbolic=='l') {
                                                                        char buf[200];
                                                                        if (readlink(tolist,buf,200)!=-1) {
                                                                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name,buf);
                                                                        } else {
                                                                            perror("Error: ");
                                                                        }
                                                                    } else {
                                                                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                                                                    }
                                                                }
                                                            }
                                                        }
                                                        closedir(dirpointer);
                                                    } else {
                                                        perror("Error: ");
                                                    }
                                                } else { //is not a directory
                                                    char *permisos="---------- ";
                                                    permisos = ConvierteModo2(statbuf.st_mode);
                                                    printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);
                                                }
                                            } else {
                                                perror("Error: ");
                                            }
                                        }
                                    } else { //-long -acc -link -hid
                                        for (int i = totalops+1; i < ntrozos; i++) {
                                            if (lstat(trozos[i], &statbuf) != -1) {
                                                if (isDirectory(trozos[i]) == 0) {
                                                    printf("************%s\n",trozos[i]);
                                                    DIR *dirpointer;
                                                    struct dirent *contains;
                                                    dirpointer = opendir(trozos[i]);
                                                    if (dirpointer != NULL) {
                                                        while ((contains = readdir(dirpointer)) != NULL) {
                                                            strcpy(tolist,trozos[i]);
                                                            strcat(strcat(tolist,"/"),contains->d_name);
                                                            if (!((!strcmp(contains->d_name, ".")) || (!strcmp(contains->d_name, "..")))) {
                                                                lstat(tolist, &statbuf);
                                                                char *permisos="---------- ";
                                                                permisos = ConvierteModo2(statbuf.st_mode);
                                                                char isSimbolic = LetraTF(statbuf.st_mode);
                                                                if (isSimbolic=='l') {
                                                                    char buf[200];
                                                                    if (readlink(tolist,buf,200)!=-1) {
                                                                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name,buf);
                                                                    } else {
                                                                        perror("Error: ");
                                                                    }
                                                                } else {
                                                                    printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                                                                }
                                                            }
                                                        }
                                                        closedir(dirpointer);
                                                    } else {
                                                        perror("Error: ");
                                                    }
                                                } else { //is not a directory
                                                    char *permisos="---------- ";
                                                    permisos = ConvierteModo2(statbuf.st_mode);
                                                    printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);
                                                }
                                            } else {
                                                perror("Error: ");
                                            }
                                        }
                                    }
                                }
                            } else {
                                if (hid==0) { //-long -link
                                    for (int i = totalops+1; i < ntrozos; i++) {
                                        if (lstat(trozos[i], &statbuf) != -1) {
                                            if (isDirectory(trozos[i]) == 0) {
                                                printf("************%s\n",trozos[i]);
                                                DIR *dirpointer;
                                                struct dirent *contains;
                                                dirpointer = opendir(trozos[i]);
                                                if (dirpointer != NULL) {
                                                    while ((contains = readdir(dirpointer)) != NULL) {
                                                        strcpy(tolist,trozos[i]);
                                                        strcat(strcat(tolist,"/"),contains->d_name);
                                                        if (!((!strcmp(contains->d_name, ".")) || (!strcmp(contains->d_name, "..")))) {
                                                            if (contains->d_name[0]!='.') {
                                                                lstat(tolist, &statbuf);
                                                                char *permisos="---------- ";
                                                                permisos = ConvierteModo2(statbuf.st_mode);
                                                                char isSimbolic = LetraTF(statbuf.st_mode);
                                                                if (isSimbolic=='l') {
                                                                    char buf[200];
                                                                    if (readlink(tolist,buf,200)!=-1) {
                                                                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name,buf);
                                                                    } else {
                                                                        perror("Error: ");
                                                                    }
                                                                } else {
                                                                    printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                                                                }
                                                            }
                                                        }
                                                    }
                                                    closedir(dirpointer);
                                                } else {
                                                    perror("Error: ");
                                                }
                                            } else { //is not a directory
                                                char *permisos="---------- ";
                                                permisos = ConvierteModo2(statbuf.st_mode);
                                                printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);
                                            }
                                        } else {
                                            perror("Error: ");
                                        }
                                    }
                                } else { //-long -link -hid
                                    for (int i = totalops+1; i < ntrozos; i++) {
                                        if (lstat(trozos[i], &statbuf) != -1) {
                                            if (isDirectory(trozos[i]) == 0) {
                                                printf("************%s\n",trozos[i]);
                                                DIR *dirpointer;
                                                struct dirent *contains;
                                                dirpointer = opendir(trozos[i]);
                                                if (dirpointer != NULL) {
                                                    while ((contains = readdir(dirpointer)) != NULL) {
                                                        strcpy(tolist,trozos[i]);
                                                        strcat(strcat(tolist,"/"),contains->d_name);
                                                        if (!((!strcmp(contains->d_name, ".")) || (!strcmp(contains->d_name, "..")))) {
                                                            lstat(tolist, &statbuf);
                                                            char *permisos="---------- ";
                                                            permisos = ConvierteModo2(statbuf.st_mode);
                                                            char isSimbolic = LetraTF(statbuf.st_mode);
                                                            if (isSimbolic=='l') {
                                                                char buf[200];
                                                                if (readlink(tolist,buf,200)!=-1) {
                                                                    printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name,buf);
                                                                } else {
                                                                    perror("Error: ");
                                                                }
                                                            } else {
                                                                printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,contains->d_name);
                                                            }
                                                        }
                                                    }
                                                    closedir(dirpointer);
                                                } else {
                                                    perror("Error: ");
                                                }
                                            } else { //is not a directory
                                                char *permisos="---------- ";
                                                permisos = ConvierteModo2(statbuf.st_mode);
                                                printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);
                                            }
                                        } else {
                                            perror("Error: ");
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        if (reca!=0) { //exist reca
                            if ((acc==0)&&(link==0)) {
                                if (hid==0) { //-long -reca
                                    for (int i = totalops+1; i < ntrozos; i++) {
                                        if (lstat(trozos[i], &statbuf) != -1) {
                                            if (isDirectory(trozos[i]) == 0) {
                                                if (reca3(trozos[i])==-1) {
                                                    perror("Error: ");
                                                }
                                            } else {
                                                lstat(trozos[i], &statbuf);
                                                char *permisos="---------- ";
                                                permisos = ConvierteModo2(statbuf.st_mode);
                                                printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);
                                            }
                                        } else {
                                            perror("Error: ");
                                        }
                                    }
                                } else { //-long -reca -hid
                                    for (int i = totalops+1; i < ntrozos; i++) {
                                        if (lstat(trozos[i], &statbuf) != -1) {
                                            if (isDirectory(trozos[i]) == 0) {
                                                if (reca4(trozos[i])==-1) {
                                                    perror("Error: ");
                                                }
                                            } else {
                                                lstat(trozos[i], &statbuf);
                                                char *permisos="---------- ";
                                                permisos = ConvierteModo2(statbuf.st_mode);
                                                printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);

                                            }
                                        } else {
                                            perror("Error: ");
                                        }
                                    }
                                }
                            } else {
                                if (acc!=0) { //exist acc
                                    if (link==0) {
                                        if (hid==0) { //-long -reca -acc
                                            for (int i = totalops+1; i < ntrozos; i++) {
                                                if (lstat(trozos[i], &statbuf) != -1) {
                                                    if (isDirectory(trozos[i]) == 0) {
                                                        if (reca5(trozos[i])==-1) {
                                                            perror("Error: ");
                                                        }
                                                    } else {
                                                        lstat(trozos[i], &statbuf);
                                                        char *permisos="---------- ";
                                                        permisos = ConvierteModo2(statbuf.st_mode);
                                                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);
                                                    }
                                                } else {
                                                    perror("Error: ");
                                                }
                                            }
                                        } else { //-long -reca -acc -hid
                                            for (int i = totalops+1; i < ntrozos; i++) {
                                                if (lstat(trozos[i], &statbuf) != -1) {
                                                    if (isDirectory(trozos[i]) == 0) {
                                                        if (reca6(trozos[i])==-1) {
                                                            perror("Error: ");
                                                        }
                                                    } else {
                                                        lstat(trozos[i], &statbuf);
                                                        char *permisos="---------- ";
                                                        permisos = ConvierteModo2(statbuf.st_mode);
                                                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);

                                                    }
                                                } else {
                                                    perror("Error: ");
                                                }
                                            }
                                        }
                                    } else {
                                        if (hid==0) { //-long -reca -acc -link
                                            for (int i = totalops+1; i < ntrozos; i++) {
                                                if (lstat(trozos[i], &statbuf) != -1) {
                                                    if (isDirectory(trozos[i]) == 0) {
                                                        if (reca7(trozos[i])==-1) {
                                                            perror("Error: ");
                                                        }
                                                    } else {
                                                        lstat(trozos[i], &statbuf);
                                                        char *permisos="---------- ";
                                                        permisos = ConvierteModo2(statbuf.st_mode);
                                                        char isSimbolic = LetraTF(statbuf.st_mode);
                                                        if (isSimbolic=='l') {
                                                            char buf[200];
                                                            if (readlink(tolist,buf,200)!=-1) {
                                                                printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,tolist,buf);
                                                            } else {
                                                                perror("Error: ");
                                                            }
                                                        } else {
                                                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,tolist);
                                                        }
                                                    }
                                                } else {
                                                    perror("Error: ");
                                                }
                                            }
                                        } else { //-long -reca -acc -link -hid
                                            for (int i = totalops+1; i < ntrozos; i++) {
                                                if (lstat(trozos[i], &statbuf) != -1) {
                                                    if (isDirectory(trozos[i]) == 0) {
                                                        if (reca8(trozos[i])==-1) {
                                                            perror("Error: ");
                                                        }
                                                    } else {
                                                        lstat(trozos[i], &statbuf);
                                                        char *permisos="---------- ";
                                                        permisos = ConvierteModo2(statbuf.st_mode);
                                                        char isSimbolic = LetraTF(statbuf.st_mode);
                                                        if (isSimbolic=='l') {
                                                            char buf[200];
                                                            if (readlink(tolist,buf,200)!=-1) {
                                                                printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,tolist,buf);
                                                            } else {
                                                                perror("Error: ");
                                                            }
                                                        } else {
                                                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,tolist);
                                                        }
                                                    }
                                                } else {
                                                    perror("Error: ");
                                                }
                                            }
                                        }
                                    }
                                } else { //exist link
                                    if (hid==0) { //-long -reca -link
                                        for (int i = totalops+1; i < ntrozos; i++) {
                                            if (lstat(trozos[i], &statbuf) != -1) {
                                                if (isDirectory(trozos[i]) == 0) {
                                                    if (reca9(trozos[i])==-1) {
                                                        perror("Error: ");
                                                    }
                                                } else {
                                                    lstat(trozos[i], &statbuf);
                                                    char *permisos="---------- ";
                                                    permisos = ConvierteModo2(statbuf.st_mode);
                                                    char isSimbolic = LetraTF(statbuf.st_mode);
                                                    if (isSimbolic=='l') {
                                                        char buf[200];
                                                        if (readlink(tolist,buf,200)!=-1) {
                                                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,tolist,buf);
                                                        } else {
                                                            perror("Error: ");
                                                        }
                                                    } else {
                                                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,tolist);
                                                    }
                                                }
                                            } else {
                                                perror("Error: ");
                                            }
                                        }
                                    } else { //-long -reca -link -hid
                                        for (int i = totalops+1; i < ntrozos; i++) {
                                            if (lstat(trozos[i], &statbuf) != -1) {
                                                if (isDirectory(trozos[i]) == 0) {
                                                    if (reca10(trozos[i])==-1) {
                                                        perror("Error: ");
                                                    }
                                                } else {
                                                    lstat(trozos[i], &statbuf);
                                                    char *permisos="---------- ";
                                                    permisos = ConvierteModo2(statbuf.st_mode);
                                                    char isSimbolic = LetraTF(statbuf.st_mode);
                                                    if (isSimbolic=='l') {
                                                        char buf[200];
                                                        if (readlink(tolist,buf,200)!=-1) {
                                                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,tolist,buf);
                                                        } else {
                                                            perror("Error: ");
                                                        }
                                                    } else {
                                                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,tolist);
                                                    }
                                                }
                                            } else {
                                                perror("Error: ");
                                            }
                                        }
                                    }
                                }
                            }
                        } else { //exist recb
                            if ((acc==0)&&(link==0)) {
                                if (hid==0) { //-long -recb
                                    for (int i = totalops+1; i < ntrozos; i++) {
                                        if (lstat(trozos[i], &statbuf) != -1) {
                                            if (isDirectory(trozos[i]) == 0) {
                                                if (recb3(trozos[i])==-1) {
                                                    perror("Error: ");
                                                }
                                            } else {
                                                lstat(trozos[i], &statbuf);
                                                char *permisos="---------- ";
                                                permisos = ConvierteModo2(statbuf.st_mode);
                                                printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);
                                            }
                                        } else {
                                            perror("Error: ");
                                        }
                                    }
                                } else { //-long -recb -hid
                                    for (int i = totalops+1; i < ntrozos; i++) {
                                        if (lstat(trozos[i], &statbuf) != -1) {
                                            if (isDirectory(trozos[i]) == 0) {
                                                if (recb4(trozos[i])==-1) {
                                                    perror("Error: ");
                                                }
                                            } else {
                                                lstat(trozos[i], &statbuf);
                                                char *permisos="---------- ";
                                                permisos = ConvierteModo2(statbuf.st_mode);
                                                printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);

                                            }
                                        } else {
                                            perror("Error: ");
                                        }
                                    }
                                }
                            } else {
                                if (acc!=0) { //exist acc
                                    if (link==0) {
                                        if (hid==0) { //-long -recb -acc
                                            for (int i = totalops+1; i < ntrozos; i++) {
                                                if (lstat(trozos[i], &statbuf) != -1) {
                                                    if (isDirectory(trozos[i]) == 0) {
                                                        if (recb5(trozos[i])==-1) {
                                                            perror("Error: ");
                                                        }
                                                    } else {
                                                        lstat(trozos[i], &statbuf);
                                                        char *permisos="---------- ";
                                                        permisos = ConvierteModo2(statbuf.st_mode);
                                                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);
                                                    }
                                                } else {
                                                    perror("Error: ");
                                                }
                                            }
                                        } else { //-long -recb -acc -hid
                                            for (int i = totalops+1; i < ntrozos; i++) {
                                                if (lstat(trozos[i], &statbuf) != -1) {
                                                    if (isDirectory(trozos[i]) == 0) {
                                                        if (recb6(trozos[i])==-1) {
                                                            perror("Error: ");
                                                        }
                                                    } else {
                                                        lstat(trozos[i], &statbuf);
                                                        char *permisos="---------- ";
                                                        permisos = ConvierteModo2(statbuf.st_mode);
                                                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,trozos[i]);

                                                    }
                                                } else {
                                                    perror("Error: ");
                                                }
                                            }
                                        }
                                    } else { //exist link
                                        if (hid==0) { //-long -recb -acc -link
                                            for (int i = totalops+1; i < ntrozos; i++) {
                                                if (lstat(trozos[i], &statbuf) != -1) {
                                                    if (isDirectory(trozos[i]) == 0) {
                                                        if (recb7(trozos[i])==-1) {
                                                            perror("Error: ");
                                                        }
                                                    } else {
                                                        lstat(trozos[i], &statbuf);
                                                        char *permisos="---------- ";
                                                        permisos = ConvierteModo2(statbuf.st_mode);
                                                        char isSimbolic = LetraTF(statbuf.st_mode);
                                                        if (isSimbolic=='l') {
                                                            char buf[200];
                                                            if (readlink(tolist,buf,200)!=-1) {
                                                                printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,tolist,buf);
                                                            } else {
                                                                perror("Error: ");
                                                            }
                                                        } else {
                                                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,tolist);
                                                        }
                                                    }
                                                } else {
                                                    perror("Error: ");
                                                }
                                            }
                                        } else { //-long -recb -acc -link -hid
                                            for (int i = totalops+1; i < ntrozos; i++) {
                                                if (lstat(trozos[i], &statbuf) != -1) {
                                                    if (isDirectory(trozos[i]) == 0) {
                                                        if (recb8(trozos[i])==-1) {
                                                            perror("Error: ");
                                                        }
                                                    } else {
                                                        lstat(trozos[i], &statbuf);
                                                        char *permisos="---------- ";
                                                        permisos = ConvierteModo2(statbuf.st_mode);
                                                        char isSimbolic = LetraTF(statbuf.st_mode);
                                                        if (isSimbolic=='l') {
                                                            char buf[200];
                                                            if (readlink(tolist,buf,200)!=-1) {
                                                                printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,tolist,buf);
                                                            } else {
                                                                perror("Error: ");
                                                            }
                                                        } else {
                                                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_atime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,tolist);
                                                        }
                                                    }
                                                } else {
                                                    perror("Error: ");
                                                }
                                            }
                                        }
                                    }
                                } else { //no acc and exist link
                                    if (hid==0) { //-long -recb -link
                                        for (int i = totalops+1; i < ntrozos; i++) {
                                            if (lstat(trozos[i], &statbuf) != -1) {
                                                if (isDirectory(trozos[i]) == 0) {
                                                    if (recb9(trozos[i])==-1) {
                                                        perror("Error: ");
                                                    }
                                                } else {
                                                    lstat(trozos[i], &statbuf);
                                                    char *permisos="---------- ";
                                                    permisos = ConvierteModo2(statbuf.st_mode);
                                                    char isSimbolic = LetraTF(statbuf.st_mode);
                                                    if (isSimbolic=='l') {
                                                        char buf[200];
                                                        if (readlink(tolist,buf,200)!=-1) {
                                                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,tolist,buf);
                                                        } else {
                                                            perror("Error: ");
                                                        }
                                                    } else {
                                                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,tolist);
                                                    }
                                                }
                                            } else {
                                                perror("Error: ");
                                            }
                                        }
                                    } else { //-long -recb -link -hid
                                        for (int i = totalops+1; i < ntrozos; i++) {
                                            if (lstat(trozos[i], &statbuf) != -1) {
                                                if (isDirectory(trozos[i]) == 0) {
                                                    if (recb10(trozos[i])==-1) {
                                                        perror("Error: ");
                                                    }
                                                } else {
                                                    lstat(trozos[i], &statbuf);
                                                    char *permisos="---------- ";
                                                    permisos = ConvierteModo2(statbuf.st_mode);
                                                    char isSimbolic = LetraTF(statbuf.st_mode);
                                                    if (isSimbolic=='l') {
                                                        char buf[200];
                                                        if (readlink(tolist,buf,200)!=-1) {
                                                            printf("%s    %ld %ld    %ld %ld  %s      %lld %s -> %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,tolist,buf);
                                                        } else {
                                                            perror("Error: ");
                                                        }
                                                    } else {
                                                        printf("%s    %ld %ld    %ld %ld  %s      %lld %s\n",ctime(&statbuf.st_mtime),(long) statbuf.st_nlink,(long) statbuf.st_ino,(long) statbuf.st_gid,(long) statbuf.st_gid, permisos,(long long)statbuf.st_size,tolist);
                                                    }
                                                }
                                            } else {
                                                perror("Error: ");
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else {
            imprimirActual();
        }
    } else {
        imprimirActual();
    }
    return 0;
}
