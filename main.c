#include <stdio.h>
#include <sqlite3.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pwd.h>

#define MAX_STR_LEN 30

void sigintHandler(int sig_num)
{
    signal(SIGINT, sigintHandler);
    fprintf(stdout, "\n");
    fflush(stdout);
}

int main(int argc, char* argv[]){
    signal(SIGINT, sigintHandler);
    char* ops = "vi";
    char op = getopt(argc, argv, ops);
    if(op == 'v'){
        fprintf(stdout, "Version 1.0.0\n");
        return 0;
    }
    if(op == 'i'){
        fprintf(stdout, "Developed by Yianni Kiritsis for CS50 final project\nFor information on how to use this manager, go to https://github.com/yianniKir/pass_manager\n");
        return 0;
    }
    if(op == '?'){
        fprintf(stderr, "Try running ./passwords -i for more information\n");
        return 1;
    }

    sqlite3* pass_data;
    sqlite3_stmt* pass_stmt;
    const char* pass_tail;
    char* pass_err;

    if(sqlite3_open("database.db", &pass_data)){
        fprintf(stderr, "Could not open the .db file\n");
        return 1;
    }

    if(sqlite3_exec(pass_data, "CREATE TABLE IF NOT EXISTS password(pass TEXT NOT NULL);", NULL, NULL, &pass_err)){
        fprintf(stderr, "ERROR: %s\n", pass_err);
        return 1;
    }

    uid_t uid = getuid();
    struct passwd *id = getpwuid(uid);
    char usrpass[MAX_STR_LEN];

    char* first_query = sqlite3_mprintf("SELECT pass FROM password LIMIT 1;");
    if(sqlite3_prepare_v2(pass_data, first_query, 128,  &pass_stmt, &pass_tail) != SQLITE_OK){
        fprintf(stderr, "ERROR: %s\n", pass_err);
        return 1;
    }

    while((sqlite3_step(pass_stmt)) == SQLITE_ROW)
        strcpy(usrpass, sqlite3_column_text (pass_stmt, 0));
    char* enteredpass;

    do{
        fprintf(stdout, "Hello, %s. Enter password to access: ", id->pw_name);
        enteredpass = getpass("");
    }while(strcmp(enteredpass, usrpass) != 0);
   
    if(sqlite3_exec(pass_data, "CREATE TABLE IF NOT EXISTS passwords(username TEXT NOT NULL, password TEXT NOT NULL, email TEXT NOT NULL, website TEXT NOT NULL);", NULL, NULL, &pass_err)){
        fprintf(stderr, "ERROR: %s\n", pass_err);
        return 1;
    }
    
    int choice;
    while(1){
        do{
            fprintf(stdout, "================MENU================\n1) Create new password\n2) Find all sites and apps connected to an email\n3) Find a password for a site or app\n4) Change program password\n5) List everything\n0) Exit\n====================================\n");
            fscanf(stdin, "%i", &choice);
            if(choice < 0 || choice > 6){
                fprintf(stdout, "Invalid selection\n");
            }
        }while(choice < 0 || choice > 6);
        
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
            
            if(sqlite3_exec(pass_data, query, NULL, NULL, &pass_err)){
                fprintf(stderr, "ERROR: %s\n", pass_err);
                return 1;
            }
        }
        
        if(choice == 2){
            char email[MAX_STR_LEN];
            fprintf(stdout, "Provide email\n");
            fscanf(stdin, "%30s", email);

            char* query = sqlite3_mprintf("SELECT username, password, email, website FROM passwords WHERE email = '%s' ORDER BY username ASC;", email);

            if(sqlite3_prepare_v2(pass_data, query, 128,  &pass_stmt, &pass_tail) != SQLITE_OK){
                fprintf(stderr, "ERROR: %s\n", pass_err);
                return 1;
            }

            while((sqlite3_step(pass_stmt)) == SQLITE_ROW){
                fprintf(stdout, "\nUsername: %s", sqlite3_column_text (pass_stmt, 0));
                fprintf(stdout, "\nPassword: %s", sqlite3_column_text (pass_stmt, 1));
                fprintf(stdout, "\nEmail: %s", sqlite3_column_text (pass_stmt, 2));
                fprintf(stdout, "\nWebsite: %s", sqlite3_column_text (pass_stmt, 3));
                fprintf(stdout, "\n"); 
            }   
            fprintf(stdout, "\n");
        }

        if(choice == 3){
            char website[MAX_STR_LEN];
            fprintf(stdout, "Provide website\n");
            fscanf(stdin, "%30s", website);

            char* query = sqlite3_mprintf("SELECT username, password, email, website FROM passwords WHERE website = '%s' ORDER BY username ASC;", website);

            if(sqlite3_prepare_v2(pass_data, query, 128,  &pass_stmt, &pass_tail) != SQLITE_OK){
                fprintf(stderr, "ERROR: %s\n", pass_err);
                return 1;
            }

            while((sqlite3_step(pass_stmt)) == SQLITE_ROW){
                fprintf(stdout, "\nUsername: %s", sqlite3_column_text(pass_stmt, 0));
                fprintf(stdout, "\nPassword: %s", sqlite3_column_text(pass_stmt, 1));
                fprintf(stdout, "\nEmail: %s", sqlite3_column_text(pass_stmt, 2));
                fprintf(stdout, "\nWebsite: %s", sqlite3_column_text(pass_stmt, 3));
                fprintf(stdout, "\n"); 
            }   
            fprintf(stdout, "\n");
        }

        if(choice == 4){
            char newpass[MAX_STR_LEN];
            fprintf(stdout, "Provide new password\n");
            fscanf(stdin, "%30s", newpass);
            if(sqlite3_exec(pass_data, "DELETE FROM password;", NULL, NULL, &pass_err)){
                fprintf(stderr, "ERROR: %s\n", pass_err);
                return 1;
            }

            char* query = sqlite3_mprintf("INSERT INTO password(pass) VALUES('%s');", newpass);

            if(sqlite3_exec(pass_data, query, NULL, NULL, &pass_err)){
                fprintf(stderr, "ERROR: %s\n", pass_err);
                return 1;
            }else{
                fprintf(stdout, "Your new password is: %s\n", newpass);
            }
        }

        if(choice == 5){
            char* query = sqlite3_mprintf("SELECT * FROM passwords ORDER BY username ASC;");

            if(sqlite3_prepare_v2(pass_data, query, 128,  &pass_stmt, &pass_tail) != SQLITE_OK){
                fprintf(stderr, "ERROR: %s\n", pass_err);
                return 1;
            }

            while((sqlite3_step(pass_stmt)) == SQLITE_ROW){
                fprintf(stdout, "\nUsername: %s", sqlite3_column_text(pass_stmt, 0));
                fprintf(stdout, "\nPassword: %s", sqlite3_column_text(pass_stmt, 1));
                fprintf(stdout, "\nEmail: %s", sqlite3_column_text(pass_stmt, 2));
                fprintf(stdout, "\nWebsite: %s", sqlite3_column_text(pass_stmt, 3));
                fprintf(stdout, "\n"); 
            }   
            fprintf(stdout, "\n");
        }

        if(choice == 0){
            break;
        }
    }

    sqlite3_finalize(pass_stmt);
    sqlite3_close(pass_data);
    fprintf(stdout, "Application closed succesfully\n");
}