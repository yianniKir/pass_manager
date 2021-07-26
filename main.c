/*
Made by Yianni Kiritsis for the CS50 final project. 
Slite3 library can be found here -> https://www.sqlite.org/download.html
This project took 2 focused days to make

Thank you to all the CS50 staff, I never thought I would be making large programs like this.
Ive learned so much and will keep learning to make even bigger programs.
*/
#include <stdio.h>
#include <sqlite3.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>

#define MAX_STR_LEN 30

void sigintHandler(int sig_num)
{
    signal(SIGINT, sigintHandler);
    fprintf(stdout, "\n");
    fflush(stdout);
}

int main(int argc, char* argv[]){
    //Don't allow user to press CTRL+C and make it exit the program
    signal(SIGINT, sigintHandler);
    //Get operator IE when you run ./passwords -i , 'i' will be the operator
    char* ops = "vi";
    char op = getopt(argc, argv, ops);
    //Respond to operator
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
    //Declare slite variables
    sqlite3* pass_data;
    sqlite3_stmt* pass_stmt;
    const char* pass_tail;
    char* pass_err;
    //Open database file
    if(sqlite3_open("database.db", &pass_data)){
        fprintf(stderr, "Could not open the .db file\n");
        return 1;
    }
    //Create password table if it does not exist
    if(sqlite3_exec(pass_data, "CREATE TABLE IF NOT EXISTS password(pass TEXT NOT NULL);", NULL, NULL, &pass_err)){
        fprintf(stderr, "ERROR: %s\n", pass_err);
        return 1;
    }
    //Get users system name so you can say "Hello, user"
    uid_t uid = getuid();
    struct passwd *id = getpwuid(uid);
    char usrpass[MAX_STR_LEN];
    //Select the password from the .db file so you can get in
    char* first_query = sqlite3_mprintf("SELECT pass FROM password;");
    if(sqlite3_prepare_v2(pass_data, first_query, 128,  &pass_stmt, &pass_tail) != SQLITE_OK){
        fprintf(stderr, "ERROR: %s\n", pass_err);
        return 1;
    }
    int rowcount = 0;
    while (SQLITE_ROW == sqlite3_step(pass_stmt)) {
        rowcount ++; 
    }
    char password[30];
    char* query;
    switch(rowcount){
        case 0:                
            fprintf(stdout, "I see this is your first time running this password, enter a default password\n");
            fscanf(stdin, "%30s", password);
                
            query = sqlite3_mprintf("INSERT INTO password(pass) VALUES('%s');", password);

            if(sqlite3_exec(pass_data, query, NULL, NULL, &pass_err)){
                fprintf(stderr, "ERROR: %s\n", pass_err);
                return 1;
            }
            fprintf(stdout, "Great! You have completed the setup, your password is %s, don't forget it!\n", password);
            break;
        case 1:
            break;
        default:
            fprintf(stdout, "Something is wrong with your program password. We will have to reset, enter a new password\n");
            fscanf(stdin, "%30s", password);

            query = sqlite3_mprintf("INSERT INTO password(pass) VALUES('%s');", password);
            if(sqlite3_exec(pass_data, "DELETE FROM password;", NULL, NULL, &pass_err)){
                fprintf(stderr, "ERROR: %s\n", pass_err);
                return 1;
            }
            if(sqlite3_exec(pass_data, query, NULL, NULL, &pass_err)){
                fprintf(stderr, "ERROR: %s\n", pass_err);
                return 1;
            }
            fprintf(stdout, "Great! You have completed the setup, your password is %s, don't forget it!\n", password);
        }
    //Copy the password in the .db file to a string
    while((sqlite3_step(pass_stmt)) == SQLITE_ROW)
        strcpy(usrpass, sqlite3_column_text (pass_stmt, 0));
    char* enteredpass;
    //Get correct password to let user access
    do{
        fprintf(stdout, "Hello, %s. Enter password to access: ", id->pw_name);
        enteredpass = getpass("");
    }while(strcmp(enteredpass, usrpass) != 0);
   //Create table that stores all the passwords
    if(sqlite3_exec(pass_data, "CREATE TABLE IF NOT EXISTS passwords(username TEXT NOT NULL, password TEXT NOT NULL, email TEXT NOT NULL, website TEXT NOT NULL);", NULL, NULL, &pass_err)){
        fprintf(stderr, "ERROR: %s\n", pass_err);
        return 1;
    }
    //Manage whatever the user decides to do
    int choice;
    while(1){
        do{
            /*
            ================MENU================
            1) Create new password
            2) Find all sites and apps connected to an email
            3) Find a password for a site or app
            4) Change program password
            5) List everything
            0) Exit
            ====================================
            */
            fprintf(stdout, "================MENU================\n1) Create new password\n2) Find all sites and apps connected to an email\n3) Find a password for a site or app\n4) Change program password\n5) List everything\n0) Exit\n====================================\n");
            fscanf(stdin, "%i", &choice);
            if(choice < 0 || choice > 6){
                fprintf(stdout, "Invalid selection\n");
            }
        }while(choice < 0 || choice > 6);
        //Handle choice 1
        if(choice == 1){
            //Declare vars
            char username[MAX_STR_LEN];
            char password[MAX_STR_LEN];
            char email[MAX_STR_LEN];
            char website[MAX_STR_LEN];
            //Scan for username
            fprintf(stdout, "Provide username\n");
            fscanf(stdin, "%30s", username);
            //Scan for password
            fprintf(stdout, "Provide password\n");
            fscanf(stdin, "%30s", password);
            //Scan for email
            fprintf(stdout, "Provide email\n");
            fscanf(stdin, "%30s", email);
            //Scan for website or application
            fprintf(stdout, "Provide website/application\n");
            fscanf(stdin, "%30s", website);
            //Save query as a string
            char* query = sqlite3_mprintf("INSERT INTO passwords(username, password, email, website) VALUES('%s', '%s', '%s', '%s');", username, password, email, website);
            //Execute query
            if(sqlite3_exec(pass_data, query, NULL, NULL, &pass_err)){
                fprintf(stderr, "ERROR: %s\n", pass_err);
                return 1;
            }
        }
        //Handle choice 2
        if(choice == 2){
            //Declare variable and scan
            char email[MAX_STR_LEN];
            fprintf(stdout, "Provide email\n");
            fscanf(stdin, "%30s", email);
            //Save query as a string
            char* query = sqlite3_mprintf("SELECT username, password, email, website FROM passwords WHERE email = '%s' ORDER BY username ASC;", email);
            //Execute query
            if(sqlite3_prepare_v2(pass_data, query, 128,  &pass_stmt, &pass_tail) != SQLITE_OK){
                fprintf(stderr, "ERROR: %s\n", pass_err);
                return 1;
            }
            //Print formatted results
            while((sqlite3_step(pass_stmt)) == SQLITE_ROW){
                fprintf(stdout, "\nUsername: %s", sqlite3_column_text (pass_stmt, 0));
                fprintf(stdout, "\nPassword: %s", sqlite3_column_text (pass_stmt, 1));
                fprintf(stdout, "\nEmail: %s", sqlite3_column_text (pass_stmt, 2));
                fprintf(stdout, "\nWebsite: %s", sqlite3_column_text (pass_stmt, 3));
                fprintf(stdout, "\n"); 
            }   
            fprintf(stdout, "\n");
        }
        //Handle choice 3
        if(choice == 3){
            //Declare variable and scan
            char website[MAX_STR_LEN];
            fprintf(stdout, "Provide website\n");
            fscanf(stdin, "%30s", website);
            //Save query as a string
            char* query = sqlite3_mprintf("SELECT username, password, email, website FROM passwords WHERE website = '%s' ORDER BY username ASC;", website);
            //Execute query
            if(sqlite3_prepare_v2(pass_data, query, 128,  &pass_stmt, &pass_tail) != SQLITE_OK){
                fprintf(stderr, "ERROR: %s\n", pass_err);
                return 1;
            }
            //Print formatted results
            while((sqlite3_step(pass_stmt)) == SQLITE_ROW){
                fprintf(stdout, "\nUsername: %s", sqlite3_column_text(pass_stmt, 0));
                fprintf(stdout, "\nPassword: %s", sqlite3_column_text(pass_stmt, 1));
                fprintf(stdout, "\nEmail: %s", sqlite3_column_text(pass_stmt, 2));
                fprintf(stdout, "\nWebsite: %s", sqlite3_column_text(pass_stmt, 3));
                fprintf(stdout, "\n"); 
            }   
            fprintf(stdout, "\n");
        }
        //Handle choice 4
        if(choice == 4){
            //Declare variable and scan
            char newpass[MAX_STR_LEN];
            fprintf(stdout, "Provide new password\n");
            fscanf(stdin, "%30s", newpass);
            //Execute query to delete original password
            if(sqlite3_exec(pass_data, "DELETE FROM password;", NULL, NULL, &pass_err)){
                fprintf(stderr, "ERROR: %s\n", pass_err);
                return 1;
            }
            //Save query as a string 
            char* query = sqlite3_mprintf("INSERT INTO password(pass) VALUES('%s');", newpass);
            //Execute query
            if(sqlite3_exec(pass_data, query, NULL, NULL, &pass_err)){
                fprintf(stderr, "ERROR: %s\n", pass_err);
                return 1;
            }else{
                //Show user their new password
                fprintf(stdout, "Your new password is: %s\n", newpass);
            }
        }
        //Handle choice 5
        if(choice == 5){
            //Save query as string
            char* query = sqlite3_mprintf("SELECT * FROM passwords ORDER BY username ASC;");
            //Execute query
            if(sqlite3_prepare_v2(pass_data, query, 128,  &pass_stmt, &pass_tail) != SQLITE_OK){
                fprintf(stderr, "ERROR: %s\n", pass_err);
                return 1;
            }
            //Print formatted results
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
            //Exit out of loop
            break;
        }
    }
    //Finalize statements and close the .db file
    sqlite3_finalize(pass_stmt);
    sqlite3_close(pass_data);
    //Show user that the application was successfullly closed.
    fprintf(stdout, "Application closed succesfully\n");
}
