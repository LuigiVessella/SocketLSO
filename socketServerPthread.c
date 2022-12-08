/*
    C socket server example, handles multiple clients using threads
    Compile
    gcc server.c -lpthread -o server
*/
 
#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include <string.h>
#include <postgresql/libpq-fe.h>
#include <sys/time.h>

struct arg_struct {
    int client_sock;
    PGconn * dbConn;
};
 
//the thread function
void *connection_handler(void *args);
PGconn *dbConnection();
void executeQuery(PGconn *conn, char * user);
void checkUserDb(PGconn *conn, char * user);
void executePhotolesQuery(PGconn * conn, char * query);

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
    PGconn *dbConn = NULL; 
    struct arg_struct args;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);
    
    //instauro la connessione al db
    args.dbConn =  dbConnection();
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
	pthread_t thread_id;
	
    while( (args.client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
         
        if(pthread_create( &thread_id , NULL ,  connection_handler , (void*) &args) != 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("Handler assigned");
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
     
    return 0;
}
 
/*
 * This will handle connection for each client
 * */
void *connection_handler(void *arguments)
{
    struct arg_struct *args = arguments; 
    //Get the socket descriptor
    int sock = args->client_sock;
    int read_size;
    char *message , client_message[2000];
     
    //Send some messages to the client
    message = "Greetings! I am your connection handler\n";
    write(sock , message , strlen(message));
     
    message = "Now type something and i shall repeat what you type \n";
    write(sock , message , strlen(message));
     
    //Receive a message from client
    while( (read_size = recv(sock , client_message , 3000 * sizeof(char) , 0)) > 0 )
    {
		
        //end of string marker
		client_message[read_size] = '\0';

        //printf("messaggio dal client: %s\n", client_message);

        checkUserDb(args->dbConn, client_message);
		
		//clear the message buffer
		memset(client_message, 0, 2000);
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
    // Client closed socket so clean up
    close(sock);
    return 0;
} 

PGconn *dbConnection() {

    printf("sono qui\n");
   PGconn *conn;

   char *stringConn = "host=projectpotholes.postgres.database.azure.com dbname=postgres port=5432 user=adminpotholes password=potholes2.";
   
   conn = PQconnectdb(stringConn);                
    
   if(PQstatus(conn) == CONNECTION_BAD) {
      printf("unable to connect\n");
      conn = NULL;
   }
   else{
      printf("connesso al db\n"); 
   }

   return conn;

}
void checkUserDb(PGconn *conn, char * user) {
    if(strlen(user) > 10 ) {
        printf("si tratta della query per la buca");
        executePhotolesQuery(conn, user);
    }
    else {
        PGresult *res;
        char * query_pt1 = "select 1 from utente where nome = '";
        char * query_pt2 = "'";
        char * query = (char *)malloc(1 + strlen(query_pt1) + strlen(user) + strlen(query_pt2));

        strcpy(query, query_pt1);
        strcat(query, user);
        strcat(query, query_pt2);

        printf("query: %s\n", query);
        
        res = PQexec(conn, query);
        
        int rows = PQntuples(res);
        if(rows > 0 ) {
            printf("utente esiste\n");
            return;
        }
        else {
            printf("utente non presente\n");
            executeQuery(conn, user);
        }
    }
}

void executeQuery(PGconn *conn, char * user) {

    PGresult *res;
    char * status;
    char * query_pt1 = "insert into utente(nome)values('";
    char * query_pt2 = "')";
    char * query = (char *)malloc(1 + strlen(query_pt1) + strlen(user) + strlen(query_pt2));

    strcpy(query, query_pt1);
    strcat(query, user);
    strcat(query, query_pt2);
    printf("la query: %s\n", query);
   
    res = PQexec(conn, query);

    status = PQresStatus(PQresultStatus(res));
    
    printf("%s\n", status);
}

void executePhotolesQuery(PGconn * conn, char * query){

    printf("\nla query ricevuta: %s\n", query);
    PGresult *res;
    char * status;
    res = PQexec(conn, query);
    status = PQresStatus(PQresultStatus(res));
    printf("%s\n", status);

}