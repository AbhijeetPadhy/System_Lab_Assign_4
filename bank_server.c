// We can allow at most one login at a time otherwise we will have to maintain login state and multithreading!
#include<stdio.h>
#include<string.h>

#define QUIT "QUIT"
#define LOGIN "LOGIN"

int validate(char username[250], char password[250]){
	FILE *fptr;
	char line[255];
	char user_from_file[250];
	char pass_from_file[250];
	if ((fptr = fopen("login_file.txt","r")) == NULL){
    	printf("Error! opening file");
    	return 1;
	}
	while(fscanf(fptr,"%s", line) != EOF){
		
	}
	
	return 0;
}

int user_authenticate(){
	char username[250];
	char password[250];
	for(int i=0;i<3;i++){
		printf("username: ");
		scanf("%s", username);
		printf("password: ");
		scanf("%s", password);
		if(validate(username, password) == 1)
			return 1;
	}
	return 0;
}

int quit_server(){
	return 0;
}

int main(){
	printf("Welcome to Banking System!\n");
	char buffer[250];
	do{
		printf("Available commands: LOGIN, QUIT\n");
		printf("Enter the command: ");
		scanf("%s", buffer);
		if(strcmp(buffer, LOGIN) == 0){
			user_authenticate();
		}
	}while(strcmp(buffer, QUIT) != 0);
	quit_server();
	return 0;
}
