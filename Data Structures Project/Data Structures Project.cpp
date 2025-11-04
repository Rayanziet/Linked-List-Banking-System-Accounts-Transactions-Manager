#include <iostream>
#include<cstring>
#include<string>
#include <fstream>
#include <sstream>

using namespace std;


struct transaction {
    string date;
    double amount;
    transaction* next;
};


struct account {
    string IBAN;
    string accountName;
    double balance;
    string currency;
    double limitDepositPerDay;
    double limitWithdrawPerMonth;
    transaction* txn;
    account* next;
};

struct user {
    int userID;
    string fname;
    string lname;
    account* acct;
    user* next, * previous;
};

struct userList {
    user* head, * tail;
};



void AddUserToList(userList& ul, user* u) {
    if (ul.head==NULL) {
        ul.head = ul.tail = u;
    }
    else {
        ul.tail->next = u;    
        u->previous = ul.tail; 
        ul.tail = u;           
    }
}


void AddAccountToList(user* u, account* a) {
    if (u->acct == NULL) {
        u->acct = a;
    }
    else {
        account* temp = u->acct;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = a;
    }
}


void AddTransactionToList(account* a, transaction* t) {
    if (a->txn == NULL) {
        a->txn = t;
    }
    else {
        transaction* temp = a->txn;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = t;
    }
}


userList ReadFile(string filename) {
    userList ul = { NULL, NULL };

    ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        cout << "Failed to open file";
        return ul;
    }

    string line;
    user* currentUser = NULL;
    account* currentAccount = NULL;

    while (getline(inputFile, line)) {
        if (line.empty()) {
            continue;
        }
        if (line[0] == '-') {
            string data(line.substr(1));
            stringstream ss(data);
            string userID, firstName, lastName;

            getline(ss, userID, ',');
            getline(ss, firstName, ',');
            getline(ss, lastName);

            currentUser = new user;
            currentUser->userID = stoi(userID);
            currentUser->fname = firstName;
            currentUser->lname = lastName;
            currentUser->acct = NULL; 
            currentUser->next = NULL;
            currentUser->previous = NULL;
            AddUserToList(ul, currentUser);

            currentAccount = NULL;
        }
        else if (line[0] == '#' && currentUser) {
            string data(line.substr(1));
            stringstream ss(data);
            string IBAN, accountName, balance, currency, limitDepositPerDay, limitWithdrawPerMonth;

            getline(ss, IBAN, ',');
            getline(ss, accountName, ',');
            getline(ss, balance, ',');
            getline(ss, limitDepositPerDay, ',');
            getline(ss, limitWithdrawPerMonth, ',');

            string sum;
            int i;
            for (i = 0; i < balance.length(); i++) {
                if (isdigit(balance[i])) {
                    sum += balance[i];
                }
                else {
                    break;
                }
            }

            currentAccount = new account;
            currentAccount->IBAN = IBAN;
            currentAccount->accountName = accountName;
            currentAccount->balance = stod(sum);

            if (balance.substr(i) == "L.L") {
                currentAccount->currency = "L.L";
            }
            else if (balance.substr(i) == "$") {
                currentAccount->currency = "$";
            }
            else {
                currentAccount->currency = "Euro";
            }
           
            currentAccount->limitDepositPerDay = stod(limitDepositPerDay);
            currentAccount->limitWithdrawPerMonth = stod(limitWithdrawPerMonth);
            currentAccount->txn = NULL; 
            currentAccount->next = NULL;

            AddAccountToList(currentUser, currentAccount);
        }

        else if (line[0] == '*' && currentUser && currentAccount) {
            string data(line.substr(1));
            stringstream ss(data);
            string date, amount;
            getline(ss, date, ',');
            getline(ss, amount);

            transaction* newtransaction = new transaction;
            newtransaction->date = date;
            newtransaction->amount = stod(amount);
            newtransaction->next = NULL;

            AddTransactionToList(currentAccount, newtransaction);
        }
    }
    inputFile.close();  
    return ul;
}




int main() {
    cout << "Hello, World!" << endl;
    return 0;
}