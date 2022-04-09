// We can allow at most one login at a time otherwise we will have to maintain login state and multithreading!
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define CUSTOMER "C"
#define ADMIN "A"
#define POLICE "P"
#define CREDIT "CREDIT"
#define DEBIT "DEBIT"

struct user {
	char username[250];
	char password[250];
	char type_of_user[2];
};

char print_data[5000] = "";
int sockfd, newsockfd,maxfd,ready_fd;
int admin_active = 0;

void error(const char* msg) {
	printf("%s", msg);
	exit(1);
}

void print(char data[1000]) {
	strcat(print_data, data);
}

void send_data() {
	int n = write(newsockfd, print_data, strlen(print_data));
	if (n < 0) {
		error("Error on writing.");
	}
}

void receive_data(char buffer[1000]) {
	send_data();
	bzero(buffer, 1000);
	int n = read(newsockfd, buffer, 1000);
	if (n < 0) {
		error("Error on reading.");
	}
	print_data[0] = '\0';
}

int no_of_digits(int num){
	int i=0;
	while(num>0){
		num /= 10;
		i++;
	};
	return i;
}
int receive_integer() {
	send_data();
	char buffer[1000];
	bzero(buffer, 1000);
	int n = read(newsockfd, buffer, 1000);
	if (n < 0) {
		error("Error on reading.");
	}
	int num = atoi(buffer);
	print_data[0] = '\0';
	if(no_of_digits(num) == strlen(buffer))
		return num;
	return 0;
}

struct user* validate(char username[250], char password[250]) {
	FILE* fptr;
	char line[255];
	char user_from_file[250];
	char pass_from_file[250];
	char type_from_file[2];
	struct user* new_user;

	if ((fptr = fopen("database/login_file.txt", "r")) == NULL) {
		print("Error! opening file");
		return NULL;
	}
	while (fscanf(fptr, "%s", line) != EOF) {
		int i = 0;
		while (line[i] != ',')i++;
		strncpy(user_from_file, line, i);
		user_from_file[i] = '\0';
		if (strcmp(username, user_from_file) != 0)
			continue;

		int j = i + 1;
		while (line[j] != ',')j++;
		strncpy(pass_from_file, line + i + 1, j - i - 1);
		pass_from_file[j - i - 1] = '\0';
		if (strcmp(password, pass_from_file) != 0)
			continue;

		strncpy(type_from_file, line + j + 1, 1);
		type_from_file[1] = '\0';
		new_user = (struct user*)malloc(sizeof(struct user));
		strcpy(new_user->username, user_from_file);
		strcpy(new_user->password, pass_from_file);
		strcpy(new_user->type_of_user, type_from_file);

		fclose(fptr);
		return new_user;
	}
	fclose(fptr);
	return NULL;
}

struct user* user_authenticate() {
	char username[1000];
	char password[1000];
	char line[10];
	FILE* fptr;
	char admin_from_file[1];
	struct user* new_user;


	for (int i = 0; i < 3; i++) {
		print("\n------------------------- User Authentication -------------------------\n\n");
		print("username: ");
		receive_data(username);
		print("password: ");
		receive_data(password);
		new_user = validate(username, password);

		if (new_user != NULL) {
			if (strcmp(new_user->type_of_user, ADMIN) == 0) {


				if ((fptr = fopen("database/admin_login.txt", "r")) == NULL) {
					print("Error! opening file");
					return NULL;
				}

				fscanf(fptr, "%s", line);
				fclose(fptr);

				admin_active = atoi(line);



				if (admin_active) {
					print("\nAdmin user already logged in.");
					return NULL;
				}

				if ((fptr = fopen("database/admin_login_temp.txt", "w")) == NULL) {
					print("Error! opening file");
					return NULL;
				}
				fprintf(fptr, "%d", 1);
				fclose(fptr);
				rename("database/admin_login_temp.txt", "database/admin_login.txt");

			}
			return new_user;
		}else
			print("Athentication Failed!\n");
	}
	return NULL;
}

int quit_client() {
	print("Bye");
	send_data();
	print_data[0] = '\0';
	return 0;
}

int compute_balance(struct user* new_user){
	FILE* fptr;
	char file_name[270] = "database/";
	char bal_str[255];
	char date_time[255];
	char trans_type[255];
	char line[255];
	strcat(file_name, new_user->username);
	strcat(file_name, ".txt");
	if ((fptr = fopen(file_name, "r")) == NULL) {
		print("Error! opening file");
    	return -1;
	}
	int balance = 0;
	int amount = 0;
	while(fgets(line,255, fptr) != NULL){
		int i=0;
		while(line[i] != ',')i++;
		strncpy(date_time, line, i);
		date_time[i] = '\0';

		int j = i+1;
		while(line[j] != ',')j++;
		strncpy(trans_type, line+i+1, j-i-1);
		trans_type[j-i-1] = '\0';

		int k = j+1;
		while(line[k] != ',')k++;
		strncpy(bal_str, line+j+1, strlen(line)-j-1);
		bal_str[strlen(line)-j-1] = '\0';
		amount = atoi(bal_str);

		if(strcmp(trans_type, DEBIT) == 0)
			balance -= amount;
		else
			balance += amount;
	}
	fclose(fptr);
	return balance;
}

void print_balance(struct user* new_user){
	int balance = compute_balance(new_user);
	print("----------------------------------\n");
	char formatted_string[1000] = "";
	sprintf(formatted_string, "Available Balance: %d\n", balance);
	print(formatted_string);
	print("----------------------------------\n");
}

void mini_statement(struct user* new_user) {
	FILE* fptr;
	char file_name[270] = "database/";
	char bal_str[255];
	char date_time[255];
	char trans_type[255];
	char line[255];
	strcat(file_name, new_user->username);
	strcat(file_name, ".txt");
	if ((fptr = fopen(file_name, "r")) == NULL) {
			print("Error! Could not access user's account.\n");
		return;
	}
	int balance = 0;
	int amount = 0;
	print("\n\n------------------------- MINI STATEMENT -------------------------\n\n");
	print("\t\t|----------------------------------------------\n");
	char formatted_string[1000] = "";
	sprintf(formatted_string, "\t\t|User: %s\n", new_user->username);
	print(formatted_string);
	print("\t\t|----------------------------------------------\n");
	char type_of_transaction[250];
	while(fgets(line,255, fptr) != NULL){
		int i=0;
		while(line[i] != ',')i++;
		strncpy(date_time, line, i);
		date_time[i] = '\0';

		int j = i+1;
		while(line[j] != ',')j++;
		strncpy(trans_type, line+i+1, j-i-1);
		trans_type[j-i-1] = '\0';

		int k = j+1;
		while(line[k] != ',')k++;
		strncpy(bal_str, line+j+1, strlen(line)-j-1);
		bal_str[strlen(line)-j-1] = '\0';
		amount = atoi(bal_str);


		if(strcmp(trans_type, DEBIT) == 0)
			balance -= amount;
		else
			balance += amount;

		sprintf(formatted_string, "\t\t|%s\t%s\t%d\n", date_time, trans_type, amount);
		print(formatted_string);
	}
	print("\t\t|----------------------------------------------\n");
	formatted_string[0] = '\0';
	sprintf(formatted_string, "\t\t|Available Balance: %d\n", balance);
	print(formatted_string);
	print("\t\t|----------------------------------------------\n");
	fclose(fptr);
}

void customer_panel(struct user* new_user) {
	char buffer[1000];
	do {
		print("\n\n-------------------------Customer Panel-------------------------\n");
		print("Available commands\n");
		print("------------------\n");
		print("1. View Balance\n");
		print("2. Print Mini Statement\n");
		print("3. Log Out\n");
		print("Enter the option number: ");
		receive_data(buffer);

		if (strcmp(buffer, "1") == 0) {
			print_balance(new_user);
		}else if(strcmp(buffer, "2") == 0){
			mini_statement(new_user);
		}
	} while (strcmp(buffer, "3") != 0);
}

int debit(struct user* new_user, int trans_amount){
	if(trans_amount <= 0)
		return -2;
	FILE *fptr;
	char file_name[270] = "database/";
	char bal_str[255];
	char date_time[255];
	char trans_type[255];
	char line[255];
	strcat(file_name, new_user->username);
	strcat(file_name, ".txt");
	if ((fptr = fopen(file_name,"r+")) == NULL){
		print("Error! Could not access user's account.\n");
		return -1;
	}
	int balance = 0;
	int amount = 0;
	while(fgets(line,255, fptr) != NULL){
		int i=0;
		while(line[i] != ',')i++;
		strncpy(date_time, line, i);
		date_time[i] = '\0';

		int j = i+1;
		while(line[j] != ',')j++;
		strncpy(trans_type, line+i+1, j-i-1);
		trans_type[j-i-1] = '\0';

		int k = j+1;
		while(line[k] != ',')k++;
		strncpy(bal_str, line+j+1, strlen(line)-j-1);
		bal_str[strlen(line)-j-1] = '\0';
		amount = atoi(bal_str);

		if(strcmp(trans_type, DEBIT) == 0)
			balance -= amount;
		else
			balance += amount;
	}
	if(balance < trans_amount)
		return 0;

	time_t t;
  time(&t);
	sprintf(date_time, "%s", ctime(&t));
	date_time[strlen(date_time)-1] = '\0';

	char formatted_string[500];
	sprintf(formatted_string, "%s,%s,%d\n", date_time, "DEBIT" ,trans_amount);

	fprintf(fptr, "%s", formatted_string);
	fclose(fptr);
	return 1;
}

int credit(struct user* new_user, int amount) {
	if(amount <= 0)
		return -2;
	FILE *fptr;
	char file_name[270] = "database/";
	strcat(file_name, new_user->username);
	strcat(file_name, ".txt");
	if ((fptr = fopen(file_name,"r")) == NULL){
			print("Error! Could not access user's account.\n");
		return -1;
	}
	fclose(fptr);
	if ((fptr = fopen(file_name,"a")) == NULL){
			print("Error! Could not access user's account.\n");
		return -1;
	}
	time_t t;
  time(&t);
	char date_time[100];
	sprintf(date_time, "%s", ctime(&t));
	date_time[strlen(date_time)-1] = '\0';

	char formatted_string[500];
	sprintf(formatted_string, "%s,%s,%d\n", date_time, "CREDIT" ,amount);

	fprintf(fptr, "%s", formatted_string);
	fclose(fptr);
	return 1;
}

void unset_admin_login_file(){
	FILE* fptr;
	if ((fptr = fopen("database/admin_login_temp.txt", "w")) == NULL) {
		print("Error! opening file");
		return;
	}
	fprintf(fptr, "%d", 0);
	fclose(fptr);
	rename("database/admin_login_temp.txt", "database/admin_login.txt");
}

void admin_panel() {
	char buffer[1000];
	char username[250];
	int amount = 0;
	struct user* new_user = NULL;
	do {
		print("\n\n-------------------------Admin Panel-------------------------\n");
		print("Available commands\n");
		print("------------------\n");
		print("1. Credit\n");
		print("2. Debit\n");
		print("3. Log Out\n");
		print("Enter the option number: ");
		receive_data(buffer);

		if (strcmp(buffer, "2") == 0) {
			print("Enter username: ");
			receive_data(username);
			new_user = (struct user*)malloc(sizeof(struct user));
			strcpy(new_user->username, username);
			print("Enter the amount to be debited: ");
			amount = receive_integer();
			int result = debit(new_user, amount);
			if (result == 1){
				print("Amount has been successfully debited to your account!\n");
				print_balance(new_user);
			}else if(result == 0){
				print("Transaction failed due to insufficient balance\n");
				print_balance(new_user);
			}else if(result == -2)
				print("Incorrect Transaction Amount. Try again!\n");
			free(new_user);
		}else if(strcmp(buffer, "1") == 0){
			print("Enter username: ");
			receive_data(username);
			new_user = (struct user*)malloc(sizeof(struct user));
			strcpy(new_user->username, username);
			print("Enter the amount to be credited: ");
			amount = receive_integer();
			int result = credit(new_user, amount);
			if(result == 1){
				print("Amount successfully credited!\n");
				print_balance(new_user);
			}else if(result == -2)
				print("Incorrect Transaction Amount. Try again!\n");
			free(new_user);
		}
	} while (strcmp(buffer, "3") != 0);

	unset_admin_login_file();
}

void police_panel(struct user* new_user) {
	char buffer[1000];
	char username[250];
	char line[255];
	char user_from_file[250];
	char user_file_name[270] = "";
	char type_from_file[250] = "";
	int amount;
	FILE* fptr;
	do {
		print("\n\n-------------------------Police Panel-------------------------\n");
		print("Available commands\n");
		print("------------------\n");
		print("1. Show Balance of All Customers\n");
		print("2. Print Mini Statement\n");
		print("3. Log Out\n");
		print("Enter the option number: ");
		receive_data(buffer);

		if (strcmp(buffer, "1") == 0) {
			print("\nAvailable Balance of all the customers:\n");
			print("-------------------------------------------\n");
			if ((fptr = fopen("database/login_file.txt", "r")) == NULL) {
				print("Error! opening file");
				return;
			}
			while (fscanf(fptr, "%s", line) != EOF) {
				int i = 0;
				while (line[i] != ',')i++;
				strncpy(user_from_file, line, i);
				user_from_file[i] = '\0';

				int j = i + 1;
				while (line[j] != ',')j++;

				strncpy(type_from_file, line + j + 1, 1);
				type_from_file[1] = '\0';

				if (type_from_file[0] != 'C')
					continue;

				struct user *new_user = (struct user *)malloc(sizeof(struct user));
				strcpy(new_user->username, user_from_file);
				int balance = compute_balance(new_user);
				char formatted_string[1000] = "";
				sprintf(formatted_string, "%s : %d\n", user_from_file, balance);
				print(formatted_string);
			}
			fclose(fptr);
		}else if(strcmp(buffer, "2") == 0){
			print("Enter username: ");
			receive_data(username);
			struct user* new_user = (struct user*)malloc(sizeof(struct user));
			strcpy(new_user->username, username);
			mini_statement(new_user);
		}
	} while (strcmp(buffer, "3") != 0);
}

void user_panel(struct user* new_user) {
	if (strcmp(new_user->type_of_user, CUSTOMER) == 0)
		customer_panel(new_user);
	else if (strcmp(new_user->type_of_user, ADMIN) == 0)
		admin_panel();
	else if (strcmp(new_user->type_of_user, POLICE) == 0)
		police_panel(new_user);
}

void run_server() {
	char buffer[1000];
	struct user* new_user;
	do {
		print("\n------------------------- Banking Dashboard-------------------------\n");
		print("Available commands\n");
		print("------------------\n");
		print("1. LOGIN\n");
		print("2. QUIT\n");
		print("Enter the option number: ");
		receive_data(buffer);

		if (strcmp(buffer, "1") == 0) {
			new_user = user_authenticate();
			if (new_user != NULL) {
				print("Authentication Successfull!\n");
				user_panel(new_user);
				free(new_user);
			}
		}
	} while (strcmp(buffer, "2") != 0);
	quit_client();
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Port number not provided. Program terminated\n");
		exit(1);
	}
	unset_admin_login_file();
	int portno, n;
	char buffer[255];
	pid_t cpid;
	fd_set req_fds;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("Error opening Socket.");
	}
	bzero((char*)&serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno); // htons - host to network short

	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		error("Binding Failed.");
	}

	listen(sockfd, 10); // Max No of clients that can connect to the server at a time

	FD_ZERO(&req_fds);

	maxfd = sockfd + 1;
	printf("\nServer running...\n\n");
	for (;;)
	{
		FD_SET(sockfd,&req_fds);

		ready_fd = select(maxfd, &req_fds, NULL, NULL, NULL);

		if (FD_ISSET(sockfd, &req_fds)) {
			clilen = sizeof(cli_addr);

			newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
			if (newsockfd < 0) {
				error("Error on accept");
			}
			if ((cpid = fork())== 0){
				close(sockfd);
				printf("Connection Established With New Client.\n");
				printf("Starting Banking Prompt...\n\n");
				run_server();
				printf("Closing Connection\n\n");
				close(newsockfd);
				return 0;
			}
			close(newsockfd);
		}
	}
	close(sockfd);
	return 0;
}
