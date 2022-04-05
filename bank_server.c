// We can allow at most one login at a time otherwise we will have to maintain login state and multithreading!
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

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

char print_data[1000] = "";

void print(char data[1000]){
	strcat(print_data, data);
}

void send_data(){
	printf("%s", print_data);
}

void receive_data(char buffer[1000]){
	send_data();
	scanf("%s", buffer);
	print_data[0] = '\0';
}

void receive_integer(int *data){
	send_data();
	char buffer[1000];
	scanf("%s", buffer);
	sscanf(buffer, "%d", data);
	print_data[0] = '\0';
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
	char username[250];
	char password[250];
	for(int i=0;i<3;i++){
		print("\n------------------------- User Authentication -------------------------\n\n");
		print("username: ");
		receive_data(username);
		fflush(stdin);
		print("password: ");
		receive_data(password);
		fflush(stdin);
		struct user *new_user = validate(username, password);
		if(new_user != NULL)
			return new_user;
	}
	return NULL;
}

int quit_server(){
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
	//send_data(print_data);
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
	char buffer[250];
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
	char buffer[250];
	char username[250];
	int amount = 0;
	struct user *new_user = NULL;
	//char print_data[1000] = "Authentication Successfull!";
	do{
		print("\n\n-------------------------Admin Panel-------------------------\n");
		print("Available commands\n");
		print("------------------\n");
		print("1. CREDIT\n");
		print("2. DEBIT\n");
		print("3. QUIT\n");
		print("Enter the command: ");
		//send_data(print_data);
		receive_data(buffer);

		if(strcmp(buffer, DEBIT) == 0){
			print("Enter username: ");
			receive_data(username);
			new_user = (struct user *)malloc(sizeof(struct user));
			strcpy(new_user->username, username);
			print("Enter the amount to be debited: ");
			receive_integer(&amount);
			//print_data[0] = '\0';
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
		//print_data[0] = '\0';
	}while(strcmp(buffer, QUIT) != 0);
}

void police_panel(struct user* new_user){
	char buffer[250];
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
		//send_data(print_data);
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

int main(){
	char buffer[250];
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
	quit_server();
	return 0;
}
