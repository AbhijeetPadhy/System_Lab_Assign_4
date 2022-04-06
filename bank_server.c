// We can allow at most one login at a time otherwise we will have to maintain login state and multithreading!
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define QUIT "QUIT"
#define LOGIN "LOGIN"
#define CUSTOMER "C"
#define ADMIN "A"
#define POLICE "P"
#define BALANCE "BAL"
#define MINI_STATEMENT "STMT"
#define CREDIT "CREDIT"
#define DEBIT "DEBIT"

struct user{
	char username[250];
	char password[250];
	char type_of_user[2];
};

char print_data[1000][1000];
int sockfd, newsockfd[1000];
int client_no = 0;
pthread_t tid[1000];

void error(const char *msg){
    printf("%s",msg);
    exit(1);
}

void print(char data[1000]){
	int i;
	for(i=0;i<client_no;i++){
		if(tid[i] == pthread_self())
			break;
	}
	strcat(print_data[i], data);
}

void send_data(){
	int i;
	for(i=0;i<client_no;i++){
		if(tid[i] == pthread_self())
			break;
	}
	int n = write(newsockfd[i], print_data[i], strlen(print_data[i]));
	if(n < 0){
			error("Error on writing.");
	}
}

void receive_data(char buffer[1000]){
	int i;
	for(i=0;i<client_no;i++){
		if(tid[i] == pthread_self())
			break;
	}
	send_data();
	bzero(buffer, 1000);
	int n = read(newsockfd[i], buffer, 1000);
	if(n < 0){
			error("Error on reading.");
	}
	print_data[i][0] = '\0';
}

void receive_integer(int *data){
	int i;
	for(i=0;i<client_no;i++){
		if(tid[i] == pthread_self())
			break;
	}
	send_data();
	char buffer[1000];
	bzero(buffer, 1000);
	int n = read(newsockfd[i], buffer, 1000);
	if(n < 0){
			error("Error on reading.");
	}
	sscanf(buffer, "%d", data);
	print_data[i][0] = '\0';
}

struct user* validate(char username[250], char password[250]){
	FILE *fptr;
	char line[255];
	char user_from_file[250];
	char pass_from_file[250];
	char type_from_file[2];
	struct user *new_user;

	if ((fptr = fopen("database/login_file.txt","r")) == NULL){
    	print("Error! opening file");
    	return NULL;
	}
	while(fscanf(fptr,"%s", line) != EOF){
		int i=0;
		while(line[i] != ',')i++;
		strncpy(user_from_file, line, i);
		user_from_file[i] = '\0';
		if(strcmp(username, user_from_file) != 0)
			continue;

		int j = i+1;
		while(line[j] != ',')j++;
		strncpy(pass_from_file, line+i+1, j-i-1);
		pass_from_file[j-i-1] = '\0';
		if(strcmp(password, pass_from_file) != 0)
			continue;

		strncpy(type_from_file, line+j+1, 1);
		type_from_file[1] = '\0';
		new_user = (struct user *)malloc(sizeof(struct user));
		strcpy(new_user->username, user_from_file);
		strcpy(new_user->password, pass_from_file);
		strcpy(new_user->type_of_user, type_from_file);

		fclose(fptr);
		return new_user;
	}
	fclose(fptr);
	return NULL;
}

struct user * user_authenticate(){
	char username[1000];
	char password[1000];
	for(int i=0;i<3;i++){
		print("\n------------------------- User Authentication -------------------------\n\n");
		print("username: ");
		receive_data(username);
		print("password: ");
		receive_data(password);
		struct user *new_user = validate(username, password);
		if(new_user != NULL)
			return new_user;
	}
	return NULL;
}

int quit_client(){
	print("Bye");
	send_data();
	int i;
	for(i=0;i<client_no;i++){
		if(tid[i] == pthread_self())
			break;
	}
	print_data[i][0] = '\0';
	return 0;
}

void print_balance(struct user* new_user){
	FILE *fptr;
	char file_name[270] = "database/";
	char print_data[1000] = "";
	strcat(file_name, new_user->username);
	strcat(file_name, ".txt");
	if ((fptr = fopen(file_name,"r")) == NULL){
    	print("Error! opening file");
    	return;
	}
	int balance;
	fscanf(fptr,"%d", &balance);
	print("----------------------------------\n");
	char formatted_string[1000] = "";
	sprintf(formatted_string, "Available Balance: %d\n", balance);
	print(formatted_string);
	print("----------------------------------\n");
	fclose(fptr);
}

void mini_statement(struct user* new_user){
	FILE *fptr;
	char file_name[270] = "database/";
	strcat(file_name, new_user->username);
	strcat(file_name, ".txt");
	if ((fptr = fopen(file_name,"r")) == NULL){
    	print("Error: User account does not exist!");
    	return;
	}
	int balance;
	fscanf(fptr,"%d", &balance);
	print("\n\n------------------------- MINI STATEMENT -------------------------\n\n");
	print("\t\t|----------------------------------\n");
	char formatted_string[1000] = "";
	sprintf(formatted_string, "\t\t|User: %s \t Available Balance: %d\n", new_user->username, balance);
	print(formatted_string);
	print("\t\t|----------------------------------\n");
	char type_of_transaction[250];
	int amount;
	while(fscanf(fptr,"%s", type_of_transaction) != EOF){
		fscanf(fptr,"%d", &amount);
		sprintf(formatted_string, "\t\t|%s:%d\n", type_of_transaction, amount);
		print(formatted_string);
	}
	print("\t\t|----------------------------------\n");
	fclose(fptr);
}

void customer_panel(struct user* new_user){
	char buffer[1000];
	do{
		print("\n\n-------------------------Customer Panel-------------------------\n");
		print("Available commands\n");
		print("------------------\n");
		print("1. BAL\n");
		print("2. STMT\n");
		print("3. QUIT\n");
		print("Enter the command: ");
		receive_data(buffer);

		if(strcmp(buffer, BALANCE) == 0){
			print_balance(new_user);
		}else if(strcmp(buffer, MINI_STATEMENT) == 0){
			mini_statement(new_user);
		}
	}while(strcmp(buffer, QUIT) != 0);
}

int debit(struct user* new_user, int amount){
	FILE *fptr, *new_fptr;
	char file_name[270] = "database/";
	char type_of_transaction[250];
	strcat(file_name, new_user->username);
	strcat(file_name, ".txt");
	if ((fptr = fopen(file_name,"r")) == NULL){
    	print("Error! opening file");
    	return -1;
	}
	if ((new_fptr = fopen("database/temp.txt","w")) == NULL){
    	print("Error! opening file");
    	return -1;
	}
	int balance;
	fscanf(fptr,"%d", &balance);
	if(balance < amount)
		return 0;
	fprintf(new_fptr, "%d\n", balance-amount);
	while(fscanf(fptr,"%s", type_of_transaction) != EOF){
		fprintf(new_fptr, "%s\n", type_of_transaction);
		fscanf(fptr,"%d", &balance);
		fprintf(new_fptr, "%d\n", balance);
	}
	fprintf(new_fptr, "%s", DEBIT);
	fprintf(new_fptr, "\n%d", amount);
	fclose(fptr);
	fclose(new_fptr);
	if(rename("database/temp.txt", file_name) == 1)
		print("rename success\n");
	return 1;
}

void credit(struct user* new_user, int amount){
	FILE *fptr, *new_fptr;
	char file_name[270] = "database/";
	char type_of_transaction[250];
	strcat(file_name, new_user->username);
	strcat(file_name, ".txt");
	if ((fptr = fopen(file_name,"r")) == NULL){
    	print("Error! opening file");
    	return;
	}
	if ((new_fptr = fopen("database/temp.txt","w")) == NULL){
    	print("Error! opening file");
    	return;
	}
	int balance;
	fscanf(fptr,"%d", &balance);
	fprintf(new_fptr, "%d\n", balance+amount);
	while(fscanf(fptr,"%s", type_of_transaction) != EOF){
		fprintf(new_fptr, "%s\n", type_of_transaction);
		fscanf(fptr,"%d", &balance);
		fprintf(new_fptr, "%d\n", balance);
	}
	fprintf(new_fptr, "%s", CREDIT);
	fprintf(new_fptr, "\n%d", amount);
	fclose(fptr);
	fclose(new_fptr);
	rename("database/temp.txt", file_name);
	return;
}
void admin_panel(){
	char buffer[1000];
	char username[250];
	int amount = 0;
	struct user *new_user = NULL;
	do{
		print("\n\n-------------------------Admin Panel-------------------------\n");
		print("Available commands\n");
		print("------------------\n");
		print("1. CREDIT\n");
		print("2. DEBIT\n");
		print("3. QUIT\n");
		print("Enter the command: ");
		receive_data(buffer);

		if(strcmp(buffer, DEBIT) == 0){
			print("Enter username: ");
			receive_data(username);
			new_user = (struct user *)malloc(sizeof(struct user));
			strcpy(new_user->username, username);
			print("Enter the amount to be debited: ");
			receive_integer(&amount);
			if(debit(new_user, amount) == 1)
				print("Amount has been successfully debited to your account!\n");
			else
				print("Transaction failed due to insufficient balance\n");
			print_balance(new_user);
			free(new_user);
		}else if(strcmp(buffer, CREDIT) == 0){
			print("Enter username: ");
			receive_data(username);
			new_user = (struct user *)malloc(sizeof(struct user));
			strcpy(new_user->username, username);
			print("Enter the amount to be credited: ");
			receive_integer(&amount);
			credit(new_user, amount);
			print_balance(new_user);
			free(new_user);
		}
	}while(strcmp(buffer, QUIT) != 0);
}

void police_panel(struct user* new_user){
	char buffer[1000];
	char username[250];
	char line[255];
	char user_from_file[250];
	char user_file_name[270] = "";
	char type_from_file[250] = "";
	int amount;
	FILE *fptr;
	do{
		print("\n\n-------------------------Police Panel-------------------------\n");
		print("Available commands\n");
		print("------------------\n");
		print("1. BAL(Show Balance of All Customers)\n");
		print("2. STMT\n");
		print("3. QUIT\n");
		print("Enter the command: ");
		receive_data(buffer);

		if(strcmp(buffer, BALANCE) == 0){
			if ((fptr = fopen("database/login_file.txt","r")) == NULL){
		    	print("Error! opening file");
		    	return;
			}
			while(fscanf(fptr,"%s", line) != EOF){
				int i=0;
				while(line[i] != ',')i++;
				strncpy(user_from_file, line, i);
				user_from_file[i] = '\0';

				int j = i+1;
				while(line[j] != ',')j++;

				strncpy(type_from_file, line+j+1, 1);
				type_from_file[1] = '\0';

				if(type_from_file[0] != 'C')
					continue;

				FILE *fptr2;
				user_file_name[0] = '\0';
				strcpy(user_file_name, "database/");
				strcat(user_file_name, user_from_file);
				strcat(user_file_name, ".txt");

				if ((fptr2 = fopen(user_file_name,"r")) == NULL){
			    	print("Error! opening file");
			    	return;
				}
				fscanf(fptr2,"%d", &amount);
				char formatted_string[1000] = "";
				sprintf(formatted_string, "%s : %d\n", user_from_file, amount);
				print(formatted_string);
				fclose(fptr2);
			}
			fclose(fptr);
		}else if(strcmp(buffer, MINI_STATEMENT) == 0){
			print("Enter username: ");
			receive_data(username);
			struct user* new_user = (struct user*)malloc(sizeof(struct user));
			strcpy(new_user->username, username);
			mini_statement(new_user);
		}
	}while(strcmp(buffer, QUIT) != 0);
}

void user_panel(struct user* new_user){
	if(strcmp(new_user->type_of_user, CUSTOMER) == 0)
		customer_panel(new_user);
	else if(strcmp(new_user->type_of_user, ADMIN) == 0)
		admin_panel();
	else if(strcmp(new_user->type_of_user, POLICE) == 0)
		police_panel(new_user);
}

void *run_server(){
	char buffer[1000];
	struct user * new_user;
	do{
		print("\n------------------------- Banking Dashboard-------------------------\n");
		print("Available commands\n");
		print("------------------\n");
		print("1. LOGIN\n");
		print("2. QUIT\n");
		print("Enter the command: ");
		receive_data(buffer);

		if(strcmp(buffer, LOGIN) == 0){
			new_user = user_authenticate();
			if(new_user != NULL){
				print("Authentication Successfull!\n");
				user_panel(new_user);
				free(new_user);
			}
		}
	}while(strcmp(buffer, QUIT) != 0);
	printf("Closing Connection\n");
	quit_client();
	int i;
	for(i=0;i<client_no;i++){
		if(tid[i] == pthread_self())
			break;
	}
	close(newsockfd[i]);
}

int main(int argc, char *argv[]){
	if(argc < 2){
			fprintf(stderr, "Port number not provided. Program terminated\n");
			exit(1);
	}
	int portno, n;
	char buffer[255];

	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
			error("Error opening Socket.");
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno); // htons - host to network short

	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
			error("Binding Failed.");
	}

	listen(sockfd, 1000); // Max No of clients that can connect to the server at a time
	clilen = sizeof(cli_addr);
	while(1){
		printf("\nWaiting for new connections...\n");
		newsockfd[client_no] = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if(newsockfd[client_no] < 0){
				error("Error on accept");
		}
		printf("Connection Established.\n");
		printf("Starting Banking Prompt...\n");
		pthread_create(&tid[client_no], NULL, run_server, NULL);
		client_no++;
	}
	close(sockfd);
	return 0;
}
