// We can allow at most one login at a time otherwise we will have to maintain login state and multithreading!
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define QUIT "QUIT"
#define LOGIN "LOGIN"

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

	if ((fptr = fopen("login_file.txt","r")) == NULL){
    	printf("Error! opening file");
    	return NULL;
	}
	while(fscanf(fptr,"%s", line) != EOF){
		int i=0;
		while(line[i] != ',')i++;
		strncpy(user_from_file, line, i);
		if(strcmp(username, user_from_file) != 0)
			continue;

		int j = i+1;
		while(line[j] != ',')j++;
		strncpy(pass_from_file, line+i+1, j-i-1);
		if(strcmp(password, pass_from_file) != 0)
			continue;

		strncpy(type_from_file, line+j+1, 1);
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
		printf("username: ");
		scanf("%s", username);
		printf("password: ");
		scanf("%s", password);
		struct user *new_user = validate(username, password);
		if(new_user != NULL)
			return new_user;
	}
	return NULL;
}

int quit_server(){
	return 0;
}

void user_panel(struct user* new_user){
	
}

int main(){
	printf("Welcome to Banking System!\n");
	char buffer[250];
	struct user * new_user;
	do{
		printf("Available commands: LOGIN, QUIT\n");
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
