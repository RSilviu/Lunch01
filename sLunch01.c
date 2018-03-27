
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

extern int errno;		/* eroarea returnata de unele apeluri */

#define SERVER_PORT 3000 //port server
#define N 30 //numarul maxim de conexiuni concurente
//#define T 10
#define TOTAL_DISHES 5 
#define LISTEN_BACKLOG 5

pthread_mutex_t accept_lock=PTHREAD_MUTEX_INITIALIZER;              // variabila mutex ce va fi partajata de threaduri
pthread_mutex_t ord_by_dish_lock=PTHREAD_MUTEX_INITIALIZER;


#define NOT_AVAILABLE -1
#define READY 0
#define NOT_READY 1


int server_socket;
int ordersByDish[TOTAL_DISHES];
int favDish = NOT_AVAILABLE;

pthread_t thread_id[N];

/*
usage: 

type err_chk_fn(args){
  if(fn errs)
    treat err
}

*/


/* programul */
int
main ()
{
  struct sockaddr_in server;	/* structurile pentru server si clienti */
  struct sockaddr_in from;
      
  int optval=1; 			/* optiune folosita pentru setsockopt()*/ 

  int len;			/* lungimea structurii sockaddr_in */

  /* creare socket */
  if ((server_socket = socket (AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror ("[server] Eroare la socket().\n");
    exit(errno);
  }

  /*setam pentru socket optiunea SO_REUSEADDR */ 
  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,&optval,sizeof(optval));

  /* pregatim structurile de date */
  bzero (&server, sizeof (server));

  /* umplem structura folosita de server */
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl (INADDR_ANY);
  server.sin_port = htons (SERVER_PORT);

  /* atasam socketul */
  if (bind (server_socket, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
  {
    perror ("[server] Eroare la bind().\n");
    exit(errno);
  }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (server_socket, LISTEN_BACKLOG) == -1)
  {
    perror ("[server] Eroare la listen().\n");
    exit(errno);
  }

  printf ("[server] Asteptam la portul %d...\n", SERVER_PORT);
  fflush (stdout);

 
  void* treat(void *);

  int getFavDish(void);


  int master_client;
  len = sizeof (from);
  bzero (&from, len);
  if ( (master_client = accept (server_socket, (struct sockaddr *) &from, &len)) < 0)
  {
      perror ("[thread]Eroare la accept().\n");	  
      exit(errno);			
  }

  int i;
  for(i = 0; i < N; ++i) pthread_create(&thread_id[i],NULL,treat,NULL);


  int T;
  if(read(master_client, &T, sizeof(int)) < 0){
      perror("[server] Eroare la read()\n");
      exit(errno);
  }

  int max_load = N;

  if(write(master_client, &max_load, sizeof(int)) < 0)
	{
		perror("[server] Eroare la write()\n");
		exit(errno);
	}

  close(master_client);


  //afla felul cel mai preferat
  while(1)
  {
    sleep(T);

    pthread_mutex_lock(&ord_by_dish_lock);
    favDish = getFavDish();
    pthread_mutex_unlock(&ord_by_dish_lock);
    favDish = NOT_AVAILABLE;
  }

  exit(0);
}

int getFavDish(){
  int currentFav = 1, currentOrders = ordersByDish[0];
  int i = 1;
  
  while(i < TOTAL_DISHES)
  {
    if(ordersByDish[i] > currentOrders){     
      currentOrders = ordersByDish[i];
      currentFav = i + 1;    
    }
    ++i;
  }
  ordersByDish[currentFav - 1] = 0;

  return currentFav; 
}

 
void rmConn(int cliSock)
{
  printf ("[server] S-a deconectat clientul cu descriptorul %d.\n", cliSock);
  fflush (stdout);
  close (cliSock);		/* inchidem conexiunea cu clientul */
}

void manageDish(int cliSock);

char * conv_addr (struct sockaddr_in address);

void *treat(void * arg)
{		
		/*
    char* dish_state[2]; 
    dish_state[READY] = "Masa e servita", dish_state[NOT_READY] = "Indisponibil";
    */
    int cliSock, length;     
		struct sockaddr_in from; 
		
		printf ("[thread]- %d - pornit...\n", (int) arg);fflush(stdout);

		for( ; ; )
		{
      length = sizeof (from);
      bzero (&from, length);

			pthread_mutex_lock(&accept_lock);
			//printf("Thread %d trezit\n",(int)arg);
			if ( (cliSock = accept (server_socket, (struct sockaddr *) &from, &length)) < 0)
			{
	 			 perror ("[thread]Eroare la accept().\n");	  			
			}
			pthread_mutex_unlock(&accept_lock);
      
      printf("[server] S-a conectat clientul cu descriptorul %d, de la adresa %s.\n",cliSock, conv_addr (from));
      fflush (stdout);
      
			write(cliSock,&cliSock,sizeof(int)); //trimite id client la client

			manageDish(cliSock);
      rmConn(cliSock);
		}	
}

void send_dish_state(int cliSock, int state);

void manageDish(int cliSock)
{
int client_dish, current_favDish;  
int nbytes;

while((nbytes = read(cliSock, &client_dish, sizeof(int))) > 0){
    //contorizeaza cererea 
      pthread_mutex_lock(&ord_by_dish_lock);
      ++ordersByDish[client_dish - 1];
      pthread_mutex_unlock(&ord_by_dish_lock);
      
      while(favDish == NOT_AVAILABLE) ;
      
      current_favDish = favDish;  //favDish se poate schimba intre timp?
      if(client_dish == current_favDish){
        //am terminat cu clientu asta, iesim din while
        send_dish_state(cliSock, READY);
        break;
      }
      else{
        //continuam
        send_dish_state(cliSock, NOT_READY);
      }
}

if(nbytes < 0)
{
  perror("[server] Eroare la read()\n");
  //return errno;
}
}

void write_err_chk(int fd, const void* buf, size_t count)
{
    if(write(fd, buf, count) < 0)
          {
            perror("[server] Eroare la write()\n");
            //return errno;
          }
}

void send_dish_state(int cliSock, int state)
{
    const char* dish_state[2]; 
    dish_state[READY] = "Masa e servita"; dish_state[NOT_READY] = "Indisponibil";

    write_err_chk(cliSock, dish_state[state], strlen(dish_state[state]) + 1);    
}


/* functie de convertire a adresei IP a clientului in sir de caractere */
char * conv_addr (struct sockaddr_in address)
{
  static char str[25];
  char port[7];

  /* adresa IP a clientului */
  strcpy (str, inet_ntoa (address.sin_addr));	
  /* portul utilizat de client */
  bzero (port, 7);
  sprintf (port, ":%d", ntohs (address.sin_port));	
  strcat (str, port);
  return (str);
}