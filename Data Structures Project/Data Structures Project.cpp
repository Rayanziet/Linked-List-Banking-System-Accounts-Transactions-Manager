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



const double DOLLAR_TO_EURO = 0.92;   
const double DOLLAR_TO_LEBANESE_LIRA = 89500.0;
const double EURO_TO_LEBANESE_LIRA = 97250.0; 
const double EURO_TO_DOLLAR = 1.09;     
const double LEBANESE_LIRA_TO_DOLLAR = 0.000011; 
const double LEBANESE_LIRA_TO_EURO = 0.000010;  

double convertCurrency(double amount, string fromCurrency, string toCurrency) {
    if (fromCurrency == toCurrency) return amount;

    if (fromCurrency == "$" && toCurrency == "Euro") {
        return amount * DOLLAR_TO_EURO;
    }
    if (fromCurrency == "$" && toCurrency == "L.L") {
        return amount * DOLLAR_TO_LEBANESE_LIRA;
    }
    if (fromCurrency == "Euro" && toCurrency == "$") {
        return amount * EURO_TO_DOLLAR;
    }
    if (fromCurrency == "Euro" && toCurrency == "L.L") {
        return amount * EURO_TO_LEBANESE_LIRA;
    }
    if (fromCurrency == "L.L" && toCurrency == "$") {
        return amount * LEBANESE_LIRA_TO_DOLLAR;
    }
    if (fromCurrency == "L.L" && toCurrency == "Euro") {
        return amount * LEBANESE_LIRA_TO_EURO;
    }

    cout << "Error: Unknown currency conversion" << endl;
    return 0.0;
}

void trans(userList& ul, double amount, string amountCurrency, string fromIBAN, string toIBAN) {
    user* currentUser = ul.head;
    account* fromAccount = NULL;
    account* toAccount = NULL;

    while (currentUser) {
        account* currentAccount = currentUser->acct;
        while (currentAccount) {
            if (currentAccount->IBAN == fromIBAN) {
                fromAccount = currentAccount;
            }
            if (currentAccount->IBAN == toIBAN) {
                toAccount = currentAccount;
            }
            if (fromAccount && toAccount) break;
            currentAccount = currentAccount->next;
        }
        if (fromAccount && toAccount) break;
        currentUser = currentUser->next;
    }

    if (!fromAccount || !toAccount) {
        cout << "Error: One or both accounts not found." << endl;
        return;
    }

    // Convert amount to source account's currency for balance check
    double convertedWithdrawAmount = convertCurrency(amount, amountCurrency, fromAccount->currency);

    if (convertedWithdrawAmount > fromAccount->balance) {
        cout << "Error: Insufficient balance." << endl;
        cout << "  Withdrawal Amount: " << convertedWithdrawAmount << " " << fromAccount->currency << endl;
        cout << "  Current Balance: " << fromAccount->balance << " " << fromAccount->currency << endl;
        return;
    }

    // Convert amount to destination account's currency
    double depositAmount = convertCurrency(amount, amountCurrency, toAccount->currency);

    cout << "Transfer Amount Details:" << endl;
    cout << "  Original Amount: " << amount << " " << amountCurrency << endl;
    cout << "  Converted Withdrawal Amount: " << convertedWithdrawAmount << " " << fromAccount->currency << endl;
    cout << "  Converted Deposit Amount: " << depositAmount << " " << toAccount->currency << endl;

    // Check deposit limit
    if (depositAmount > toAccount->limitDepositPerDay) {
        cout << "Error: Deposit amount exceeds daily limit." << endl;
        cout << "  Deposit Amount: " << depositAmount << " " << toAccount->currency << endl;
        cout << "  Daily Limit: " << toAccount->limitDepositPerDay << " " << toAccount->currency << endl;
        return;
    }

    // Check withdrawal limit
    if (convertedWithdrawAmount > fromAccount->limitWithdrawPerMonth) {
        cout << "Error: Withdrawal amount exceeds monthly limit." << endl;
        return;
    }

    // Perform transfer
    fromAccount->balance -= convertedWithdrawAmount;
    toAccount->balance += depositAmount;

    // Create transactions
    transaction* withdrawTxn = new transaction;
    withdrawTxn->date = "17/12/2024";
    withdrawTxn->amount = -convertedWithdrawAmount;
    withdrawTxn->next = NULL;
    AddTransactionToList(fromAccount, withdrawTxn);

    transaction* depositTxn = new transaction;
    depositTxn->date = "17/12/2024";
    depositTxn->amount = depositAmount;
    depositTxn->next = NULL;
    AddTransactionToList(toAccount, depositTxn);

    cout << "Transfer successful: "
        << convertedWithdrawAmount << " " << fromAccount->currency
        << " transferred from " << fromAccount->IBAN
        << " to " << toAccount->IBAN
        << " (" << depositAmount << " " << toAccount->currency << ")" << endl;
}



int main() {
    return 0;
}