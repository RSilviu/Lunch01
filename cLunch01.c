
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <time.h>

#define SERVER_PORT 3000 //port server

/* codul de eroare returnat de anumite apeluri */
extern int errno;


int main ()
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  int T;
  int dish;
  char dish_state[32];
  int denials = 0;
  int id;

  srand(time(NULL));
  //T = rand() % 120;

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[client] Eroare la socket().\n");
      return errno;
    }
  

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  /* portul de conectare */
  server.sin_port = htons (SERVER_PORT);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }
   
   read(sd, &id, sizeof(int));  //citeste id client


   while(denials < 3){
    
    dish = rand() % 5 + 1;
    //sleep(T);
  
    if (write (sd, &dish, sizeof(int)) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }

    if (read (sd, dish_state, sizeof(dish_state)) <= 0)
    {
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
    }

    if(strcmp(dish_state,"Indisponibil") == 0){
        
        ++denials;
        printf("\n[client %d] Respins\n", id); fflush(stdout);
        
        if(denials < 3){ T = rand() % 10 + 1; sleep(T); }
    }

    else if(strcmp(dish_state,"Masa e servita") == 0) break;

}
  
  if(denials < 3){
    printf("\n[client %d] Satul!\n", id); fflush(stdout);
  }

  else{ 
    printf("\n[client %d] Schimb cantina! Aici mor de foame!\n", id);
    fflush(stdout);
  }

  close(sd);
  exit(0);  
}