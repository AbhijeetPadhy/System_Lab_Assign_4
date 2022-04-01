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

struct user* validate(char username[250], char password[250]){
	FILE *fptr;
	char line[255];
	char user_from_file[250];
	char pass_from_file[250];
	char type_from_file[2];
	struct user *new_user;

	if ((fptr = fopen("database/login_file.txt","r")) == NULL){
    	printf("Error! opening file");
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
		printf("\n------------------------- User Authentication -------------------------\n\n");
		printf("username: ");
		scanf("%s", username);
		fflush(stdin);
		printf("password: ");
		scanf("%s", password);
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
	strcat(file_name, new_user->username);
	strcat(file_name, ".txt");
	if ((fptr = fopen(file_name,"r")) == NULL){
    	printf("Error! opening file");
    	return;
	}
	int balance;
	fscanf(fptr,"%d", &balance);
	printf("----------------------------------\n");
	printf("Available Balance: %d\n", balance);
	printf("----------------------------------\n");
	fclose(fptr);
}

void mini_statement(struct user* new_user){
	FILE *fptr;
	char file_name[270] = "database/";
	strcat(file_name, new_user->username);
	strcat(file_name, ".txt");
	if ((fptr = fopen(file_name,"r")) == NULL){
    	printf("Error! opening file");
    	return;
	}
	int balance;
	fscanf(fptr,"%d", &balance);
	printf("\n\n------------------------- MINI STATEMENT -------------------------\n\n");
	printf("\t\t|----------------------------------\n");
	printf("\t\t|Available Balance: %d\n", balance);
	printf("\t\t|----------------------------------\n");
	char type_of_transaction[250];
	int amount;
	while(fscanf(fptr,"%s", type_of_transaction) != EOF){
		fscanf(fptr,"%d", &amount);
		printf("\t\t|%s:%d\n", type_of_transaction, amount);
	}
	printf("\t\t|----------------------------------\n");
	fclose(fptr);
}

void customer_panel(struct user* new_user){
	char buffer[250];
	do{
		printf("\n\n-------------------------Customer Panel-------------------------\n");
		printf("Available commands\n");
		printf("------------------\n");
		printf("1. BAL\n");
		printf("2. STMT\n");
		printf("3. QUIT\n");
		printf("Enter the command: ");
		scanf("%s", buffer);

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
    	printf("Error! opening file");
    	return -1;
	}
	if ((new_fptr = fopen("database/temp.txt","w")) == NULL){
    	printf("Error! opening file");
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
	rename("temp.txt", file_name);
	return 1;
}

void credit(struct user* new_user, int amount){

}
void admin_panel(){
	char buffer[250];
	char username[250];
	int amount = 0;
	struct user *new_user = NULL;
	do{
		printf("\n\n-------------------------Admin Panel-------------------------\n");
		printf("Available commands\n");
		printf("------------------\n");
		printf("1. CREDIT\n");
		printf("2. DEBIT\n");
		printf("3. QUIT\n");
		printf("Enter the command: ");
		scanf("%s", buffer);

		if(strcmp(buffer, DEBIT) == 0){
			printf("Enter username: ");
			scanf("%s", username);
			new_user = (struct user *)malloc(sizeof(struct user));
			strcpy(new_user->username, username);
			printf("Enter the amount to be debited: ");
			scanf("%d", &amount);
			if(debit(new_user, amount) == 1)
				printf("Amount has been successfully debited to your account!\n");
			else
				printf("Transaction failed due to insufficient balance");
			print_balance(new_user);
			free(new_user);
		}else if(strcmp(buffer, CREDIT) == 0){
			printf("Enter username: ");
			scanf("%s", username);
			new_user = (struct user *)malloc(sizeof(struct user));
			strcpy(new_user->username, username);
			printf("Enter the amount to be credited: ");
			scanf("%d", &amount);
			credit(new_user, amount);
			print_balance(new_user);
			free(new_user);
		}
	}while(strcmp(buffer, QUIT) != 0);
}

void police_panel(struct user* new_user){

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
		printf("\n------------------------- Banking Dashboard-------------------------\n");
		printf("Available commands\n");
		printf("------------------\n");
		printf("1. LOGIN\n");
		printf("2. QUIT\n");
		printf("Enter the command: ");
		scanf("%s", buffer);

		if(strcmp(buffer, LOGIN) == 0){
			new_user = user_authenticate();
			if(new_user != NULL){
				printf("Auth Successfull!\n");
				user_panel(new_user);
				free(new_user);
			}
		}
	}while(strcmp(buffer, QUIT) != 0);
	quit_server();
	return 0;
}
