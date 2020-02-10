#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INPUT_FILE_NAME "Assignment_3_input_file.txt"
#define OUTPUT_FILE_NAME "Assignment_3_output_file.txt"

char line[256];
char transLine[256];
int accAmt[256];
int accNum;
int accA;
int accB;
int amt;
int arraySize;
int i;
int transNums;
char* token;

//Pthread function to perform the mutex functions
pthread_mutex_t lock;

//Input file pointer
FILE *fp;
//Another input file pointer to count the number of transactions
FILE *transactionsFp;
//Output file pointer to write the file
FILE *of;

/*
 * Function:  is_blank_line
 * --------------------
 * Detects whether a line is blank. Depositor functions are seperated from client functions by a blank line.
 * To separate these functions, the is_blank_line test takes the parameter line and tests.
 *
 *  returns: type int (either 0 or 1) with 1 being if the line is tab, return or new line only
 */

int is_blank_line(const char *line) {
    const char accept[]=" \t\r\n";
    return (strspn(line, accept) == strlen(line));
}

/*
 * Function:  starts_with
 * --------------------
 * Tests strings to see what character they start with. This is used in the functions when trying to determine if the
 * tokens correspond with clients 1 - n.
 *
 *  returns: bool of either 0 [FALSE] or 1 [TRUE] if the string in a starts with the char or char array in b.
 */

bool starts_with(const char *a, const char *b)
{
   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
}

/*
 * Function:  initial_depositor_functions
 * --------------------
 * Reads the input file and determines lines that are designated depositor lines. 
 * These lines update an array at position accountNumber - 1 with the balance of the account (accAmt).
 *
 */

void initial_depositor_functions() {
    //While the file pointer for the imput file is not null.
    while (!feof(fp)) {
    //Handles which line is being read from
        fgets(line, sizeof(line), fp);
    //Calls the is_blank_line function which determines if there is a line break (meaning the end of the depositor functions).
        if (is_blank_line(line)) {
    //Break from this loop if a blank line is hit (no more depositor functions)
            break;
        }
        token = strtok(line, " ");
    //If the token starts with dep (which means it is a deposit)
        if (strstr(token, "dep") != NULL) {
    //Increment the token to the next spot
            token = strtok(NULL, " ");
    //While there are still tokens on the line
            while(token){
    //If the next token is d, it is a deposit
            if(strstr(token, "d") != NULL){
    //Increment the token to get the account #
                token = strtok(NULL, " ");
    //Because account number is in the format accN where N = a number, strip the first 3 letters acc to just get the acc #
                token += 3;
    //Take this number and turn it into an integer. Then subtract 1 from it as arrays start at 0 and not 1.
                accNum = atoi (token) - 1;
    //Keep track of the array size, if the current account number is greater than the arraySize variable, increase it by 1.
                if ((accNum+1) > arraySize){
                    arraySize = accNum+1;
                }
    //Move the token another spot, this is to get the deposit amount.
                token = strtok(NULL, " ");
    //Convert the deposit amount to an integer.
                amt = atoi (token);
    //The accAmt array stores the amount in each account.
    //Because it is an array where N = any number, accN balance will be stored at spot accAmt[N-1]. 
    //This is because arrays start at spot 0.
                accAmt[accNum] += amt;
            }
    //Increment the token, which should be the next deposit item on the current line (unless the line is empty).
    //If this is the case while(token) will fall through, and the line will increment.
            token = strtok(NULL, " ");
            }
        } 
    }
}

/*
 * Function:  client_transaction_functions
 * --------------------
 * Called by the threads per token set to perform transactions on the accounts.
 * The mutex locks once the function is entered and unlocks once it has completed allowing for another thread to enter.
 * Uses the accAmt[] array to keep track of account balances and uses if statements to test and makes sure 
 * the account is not going into the negative balance. Ignores withdrawals and transfers that will make the balance negative.
 */

void *client_transaction_functions(){
    //Mutex lock to only allow one thread at a time.
    pthread_mutex_lock(&lock);
    //If the token starts with c, it is telling you the client id, and is not important to the operation of the transactions.
    //Do nothing when this token is read.
        if(starts_with(token,"c")){
        }
    //In this instance if the token contains d, it is a deposit function. 
    //Deposit functions are positive by assignment specification and do not need to be compared to the account balance.
        else if(strstr(token, "d") != NULL){
    //Increment the token to get the account #
            token = strtok(NULL, " ");
    //Because account number is in the format accN where N = a number, strip the first 3 letters acc to just get the acc #
            token += 3;
    //Take this number and turn it into an integer. Then subtract 1 from it as arrays start at 0 and not 1.
            accNum = atoi (token) - 1;
    //Move the token another spot, this is to get the deposit amount.
            token = strtok(NULL, " ");
    //Convert the deposit amount to an integer.
            amt = atoi (token);
    //The accAmt array stores the amount in each account.
    //Because it is an array where N = any number, accN balance will be stored at spot accAmt[N-1]. 
    //This is because arrays start at spot 0.
            accAmt[accNum] += amt;
        }
    //In this instance if the token contains w, it is a withdrawal function. 
    //Withdrawal functions are checked against the current account balance to make sure they are not going into the negative.
        else if(strstr(token, "w") != NULL){
    //Increment the token to get the account #
            token = strtok(NULL, " ");
    //Because account number is in the format accN where N = a number, strip the first 3 letters acc to just get the acc #
            token += 3;
    //Take this number and turn it into an integer. Then subtract 1 from it as arrays start at 0 and not 1.
            accNum = atoi (token) - 1;
    //Move the token another spot, this is to get the withdrawal amount.
            token = strtok(NULL, " ");
    //Convert the withdrawal amount to an integer.
            amt = atoi (token);
    //Check the withdrawal amount against the accAmt array (account balance) at the accNum (account number) spot.
    //If the withdrawal amount is less than or equal to than the account balance, let it go through. Else this is skipped.
            if(accAmt[accNum]>= amt){
            accAmt[accNum] -= amt;
            }
        }
    //In this instance, if the token contains t, it is a transfer function. 
    //Transfer functions must be checked against the source account to make sure that it is not transferring more than the account contents.
        else if(strstr(token, "t") != NULL){
    //Increment the token to get the account # that the transfer is coming from.
            token = strtok(NULL, " ");
    //Because account number is in the format accN where N = a number, strip the first 3 letters acc to just get the acc #
            token += 3;
    //Take this number and turn it into an integer. Then subtract 1 from it as arrays start at 0 and not 1.
    //accA is the source account the transfer is coming from.
            accA = atoi (token) - 1;
    //Increment the token to get the account # that the transfer is going to.
            token = strtok(NULL, " ");
    //Because account number is in the format accN where N = a number, strip the first 3 letters acc to just get the acc #
            token += 3;
    //Take this number and turn it into an integer. Then subtract 1 from it as arrays start at 0 and not 1.
    //accB is the destination account the transfer is going to.
            accB = atoi (token) - 1;
    //Move the token another spot, this is to get the transfer amount.
            token = strtok(NULL, " ");
    //Convert the transfer amount to an integer.
            amt = atoi (token);
    //Check the transfer amount against the balance at the accAmt array at position accA. If the source account (accA)
    //has enough funds to complete the transfer, subtract the amount from the source account accA, and add it to destination account (accB)
            if(accAmt[accA]>=amt){
                accAmt[accA] -= amt;
                accAmt[accB] += amt;
            }
        }
    //Increment the token, which should be the next transaction spot on the current line (unless the line is empty).
   token = strtok(NULL, " ");
   //Mutex unlock as the thread has completed this function.
   pthread_mutex_unlock(&lock);
   //Return
   return 0;
}

/*
 * Function:  count_transactions
 * --------------------
 * Counts the number of transactions in the text file. This function is used to determine how many transactions there are in the
 * text file which consequently is how many threads will be created.
 */

void count_transactions() {
    //While the file pointer for the imput file copy is not null.
    while (!feof(transactionsFp)) {
    //Read through the file and handle line functions.
        fgets(transLine, sizeof(transLine), transactionsFp);
    //Set the token on the start of the line.
        token = strtok(transLine, " ");
    //If the line is blank or the first part of the line is "dep" meaning it's a deposit line and not a transaction line, omit.
        if (is_blank_line(transLine) || (strstr(token, "dep") != NULL)) {
        continue;
        }
    //Otherwise, increment the token.
        token = strtok(NULL, " ");
    //While there are still tokens on the line.
        while(token){
    //If the next token is d, it is a deposit.
        if(strstr(token, "d") != NULL){
    //Deposits have the format d acc# depAmt - so increment by two more tokens to move on to the next transaction.
            token = strtok(NULL, " ");
            token = strtok(NULL, " ");
    //Add 1 to the number of transactions counter.
            transNums += 1;
        }
    //If the next token is w, it is a withdrawal.
        else if(strstr(token, "w") != NULL){
    //Withdrawals have the format w acc# withAmt - so increment by two more tokens to move on to the next transaction.
            token = strtok(NULL, " ");
            token = strtok(NULL, " ");
    //Add 1 to the number of transactions counter.
            transNums += 1;
        }
    //If the next token is t, it is a transfer.
        else if(strstr(token, "t") != NULL){
    //Transfers have the format t srcAcc# destAcc# transAmt - so increment by thre more tokens to move on to the next transaction.
            token = strtok(NULL, " ");
            token = strtok(NULL, " ");
            token = strtok(NULL, " ");
    //Add 1 to the number of transactions counter.
            transNums += 1;
        }
    //Increment the token, which should be the next transaction spot on the current line (unless the line is empty).
        token = strtok(NULL, " ");
        }
    }
}

/*
 * Function:  main
 * --------------------
 * Opens the input file, creates an output file. Also handles thread creation and calls the functions 
 * to count the number of transactions and also handle the intial depositor functions.
 * Also creates threads based on the number of transactions and loops through calling the client_transaction_functions per thread
 */

int main() {
   //Open the input file and throw an error if it does not exist.
    fp = fopen(INPUT_FILE_NAME, "r");
    if (!(fp = fopen(INPUT_FILE_NAME, "r"))||ferror(fp)||fp == NULL){
        fprintf( stderr, "Unable to open file.\nPlease make sure the file is located in the same directory as the run file.\n");
        return 0;
    }
    //Open an instance of the input file which will be used to count the number of transactions.
    transactionsFp = fopen(INPUT_FILE_NAME, "r");
    //Create the output file with write permissions.
    of = fopen(OUTPUT_FILE_NAME, "w");
    //Run the count transactions function to count the number of transactions, which will be used to create the correct number of threads.
    count_transactions();
    //Run the intial deposit functions.
    initial_depositor_functions();
    //Create threads that are sized appropriately based on the number of transactions.
    pthread_t * thread = malloc(sizeof(pthread_t)*transNums);
    //Handle mutex errors
        if (pthread_mutex_init(&lock, NULL) != 0){
        printf("\n mutex init failed\n");
        return 1;
        }
    //While the file pointer for the imput file is not null.
        while (!feof(fp)) {
    //Read through the file and handle line functions.
        fgets(line, sizeof(line), fp);
    //Set the token on the start of the line based on where the fp has left off from (after depositor functions).
        token = strtok(line, " ");
    //Let i denote the transaction number. For each transaction, create a thread.
        for( i = 0; i< transNums; i++){
    //While there are still tokens on the line.
            while(token){
    //Create a thread and have it run through the client_transaction_functions which handles the mutex locks and threads.
    //Handle errors are return 1 if there is an error in creation.
                if(pthread_create(&thread[i], NULL, &client_transaction_functions, NULL)){
                    printf ("Error: Thread\n");
                    return 1;
                    }
    //Handle thread join and errors
                if(pthread_join(thread[i],NULL)) {
                    printf ("Error: joining\n");
                    return 1;
                    }
                }
            }

        }
    //Reset i to 0 as it will now be used to traverse the array and compare to the size of the array to print the account balances to the file.
    i = 0;
    //While i is less than the array size, print the account balance at each spot to the file.
    while(i<arraySize){
        fprintf(of,"acc%d %d ",i+1,accAmt[i]);
        i++;
    }
    //Destroy the mutex
    pthread_mutex_destroy(&lock);
    //Return
    return 0;
}