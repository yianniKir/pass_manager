#include <stdio.h>
#include <sqlite3.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#define MAX_STR_LEN 30
int main(int argc, char* argv[]){
    char* ops = "vi";
    char op = getopt(argc, argv, ops);
    if(op == 'v'){
        fprintf(stdout, "Version 0.1.0\n");
        return 0;
    }
    if(op == 'i'){
        fprintf(stdout, "Developed by Yianni Kiritsis for CS50 final project\nFor information on how to use this manager, go to INSERT GITHUB LINK HERE\n");
        return 0;
    }
    if(op == '?'){
        fprintf(stderr, "Try running ./passwords -i for more information\n");
        return 1;
    }

    uid_t uid = getuid();
    struct passwd *id = getpwuid(uid);
    char usrpass[MAX_STR_LEN] = "1";
    char* enteredpass;
    do{
        fprintf(stdout, "Hello, %s. Enter password to access: ", id->pw_name);
        enteredpass = getpass("");
    }while(strcmp(enteredpass, usrpass) != 0);

    sqlite3* db;
    sqlite3_stmt* stmt;
    sqlite3_open("database.db", &db);
    char* err;
    int rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS passwords(username TEXT NOT NULL, password TEXT NOT NULL, email TEXT NOT NULL, website TEXT NOT NULL);", NULL, NULL, &err);
    
    int choice;
    while(1){
        do{
            fprintf(stdout, "================MENU================\n1) Create new password\n2) Find all sites and apps connected to an email\n3) Find a password for a site or app\n4)Exit\n====================================\n");
            fscanf(stdin, "%i", &choice);
        }while(choice < 1 || choice > 4);
        
        if(choice == 1){
            char username[MAX_STR_LEN];
            char password[MAX_STR_LEN];
            char email[MAX_STR_LEN];
            char website[MAX_STR_LEN];

            fprintf(stdout, "Provide username\n");
            fscanf(stdin, "%30s", username);

            fprintf(stdout, "Provide password\n");
            fscanf(stdin, "%30s", password);

            fprintf(stdout, "Provide email\n");
            fscanf(stdin, "%30s", email);
            
            fprintf(stdout, "Provide website/application\n");
            fscanf(stdin, "%30s", website);
            
            char* query = sqlite3_mprintf("INSERT INTO passwords(username, password, email, website) VALUES('%s', '%s', '%s', '%s');", username, password, email, website);
            printf("%s\n", query);
            sqlite3_exec(db, query, NULL, NULL, &err);
            if(err != SQLITE_OK){
                printf("ERROR: %s\n", err);
                return 1;
            }
        }
        if(choice == 4){
            break;
        }
    }
    sqlite3_close(db);
}