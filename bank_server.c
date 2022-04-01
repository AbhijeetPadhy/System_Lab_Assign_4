// We can allow at most one login at a time otherwise we will have to maintain login state and multithreading!
#include<stdio.h>
#include<string.h>

#define QUIT "QUIT"

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
	}while(strcmp(buffer, QUIT) != 0);
	quit_server();
	return 0;
}