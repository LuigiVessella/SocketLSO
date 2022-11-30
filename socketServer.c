#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <postgresql/libpq-fe.h>

void dbConnection();
 
void main(int argc, char* argv[])
{
   int    portNum = 8080;
   int    server;
   int    newSocket;

   int    sendrc;
   int    bndrc;
  
   char*  greeting;
   char rbuff[1024];
   int    check_mess;

   
   struct sockaddr_in  serverAddr;
   struct sockaddr_in newAddr;
   socklen_t addr_size = sizeof(serverAddr);
   
   //settiamo tutti i parametri, famiglia, e porta.
   memset(&serverAddr,0x00,sizeof(serverAddr));
   serverAddr.sin_family = AF_INET;
   serverAddr.sin_port = htons(portNum);
   serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   
   #pragma convert (819)
   greeting = "This is a message from the C socket server.";
   #pragma convert (0)

   dbConnection();
 
   //socket allocata
   if((server = socket(AF_INET, SOCK_STREAM, 0))<0)
   {
      printf("failure on socket allocation\n");
      perror(NULL);
      exit(-1);
   }

   printf("socket allocata\n");
 
   //bind della socket
   if((bndrc = bind(server,(struct sockaddr*)&serverAddr, sizeof(serverAddr)))<0)
   {
     printf("Bind failed\n");
     perror(NULL);
     exit(-1);
   }
   printf("bind() completato\n");
 
   //socket in ascolto
   listen(server, 10);

   printf("socket in ascolto\n");

   int pid, counter = 0;
   while (1) {
      //attente richieste dal cliente, e, se ok, le accetta
      if((newSocket = accept(server,(struct sockaddr*)&newAddr, &addr_size))<0)
      {
         printf("accept failed\n");
         perror(NULL);
         exit(-1);
      }
      printf("questo e il %d client servito\n", counter++);
      printf("client accettato\n");
      
      //forkiamo 
      pid = fork();
      if(pid < 0) {
         printf("error in process creation\n");
         perror(NULL);
         exit(-1);
      }

      if(pid == 0) {
         close(server);
         
         //riceve messaggi dal cliente, e li mette in rbuff
         if(check_mess = recv(newSocket, rbuff, 1024 * sizeof(char), 0) < 0) {
            printf("receive failed\n");      
         }

         printf("messaggio client: %s\n", rbuff);

         //invia i messaggi a/ai client, contenuti in greeting
         if((sendrc = send(newSocket, greeting, strlen(greeting),0))<0)
         {
            printf("Send failed\n");
            perror(NULL);
            exit(-1);
         }
      } else {
         close(newSocket);
      }
   }
}



void dbConnection() {
   PGconn *conn;
   PGresult *res;

   char *stringConn = "host=projectpotholes.postgres.database.azure.com dbname=postgres port=5432 user=adminpotholes password=potholes2.";
 
   conn = PQconnectdb(stringConn);                

   if(PQstatus(conn) == CONNECTION_BAD) {
      printf("unable to connect\n");
      return;
   }
   else{
      printf("connesso al db\n"); 
   }
   res = PQexec(conn, "select * from rilevazionebuca");
   
   int res_count = PQntuples(res);

   printf("ci sono %d record \n", res_count);

}