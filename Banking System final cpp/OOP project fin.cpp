#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <limits>
#include<cstdio>
using namespace std;
// ACCOUNT TYPES
class UserClient;
class CompanyClient;
class BankingEmployee;
void updateUserBalanceInFile(const string& loginID, long double newBalance);
void updateCompanyBalanceInFile(const string& companyID, long double newBalance);
UserClient* currentUserClient = nullptr;
CompanyClient* currentCompanyClient = nullptr;
BankingEmployee* currentEmployee = nullptr;

class AccountType
{
protected:
    string typeName;
    long double withDrawalLimit;
public:
    AccountType(string tName, long double wLimit) : typeName(tName), withDrawalLimit(wLimit) {}
    virtual long double getWithDrawalLimit() const = 0;
    virtual string getTypeName() const { return typeName; }
    virtual void displayAccountTypeInfo() = 0;
    virtual ~AccountType() {}
};

class Bronze : public AccountType
{
public:
    Bronze() : AccountType("Bronze", 100000) {}
    long double getWithDrawalLimit() const override { return withDrawalLimit; }
    void displayAccountTypeInfo() override
    {
        cout << "Account Type: " << typeName << ", Daily Withdrawal Limit: " << withDrawalLimit << " PKR" << endl;
    }
};

class Gold : public AccountType
{
public:
    Gold() : AccountType("Gold", 500000) {}
    long double getWithDrawalLimit() const override
    {
        return withDrawalLimit;
    }
    void displayAccountTypeInfo() override
    {
        cout << "Account Type: " << typeName << ", Daily Withdrawal Limit: " << withDrawalLimit << " PKR" << endl;
    }
};

class Business : public AccountType
{
public:
    Business() : AccountType("Business", 20000000) {}
    long double getWithDrawalLimit() const override
    {
        return withDrawalLimit;
    }
    void displayAccountTypeInfo() override
    {
        cout << "Account Type: " << typeName << ", Daily Withdrawal Limit: " << withDrawalLimit << " PKR" << endl;
    }
};
class Card
{
    string cardNumber;
    string cardPin;
    bool isFraud;
    int wrongAttempts;
public:
    Card(const string& number = "", const string& pin = "0000")
    {
        cardNumber = number.empty() ? generateCardNumber() : number;
        cardPin = pin;
        isFraud = false;
        wrongAttempts = 0;
    }
    string getCardNumber() const { return cardNumber; }
    string getCardPin() const { return cardPin; }
    Card* setFraud(const bool s)
    {
        isFraud = s;
        return this;
    }
    bool getIsFraud() const { return isFraud; }
    bool verifyPin(const string& pin) {
        if (isFraud) {
            cout << "Card marked as FRAUD. Please contact bank.\n";
            return false;
        }
        if (pin == cardPin) {
            wrongAttempts = 0;
            return true;
        }
        wrongAttempts++;
        cout << "Wrong PIN! Attempts remaining: " << (3 - wrongAttempts) << endl;
        if (wrongAttempts >= 3) {
            isFraud = true;
            cout << "Card has been marked as FRAUD due to multiple failed attempts.\n";
        }
        return false;
    }
    void setPin(const string& pin) {
        if (pin.length() == 4) {
            cardPin = pin;
            wrongAttempts = 0;
        }
    }
    void resetFraudStatus() {
        isFraud = false;
        wrongAttempts = 0;
    }
    string generateCardNumber() {
        string number = "4000";
        for (int i = 0; i < 12; i++)
        {
            number += to_string(rand() % 10);
        }
        return number;
    }
};
class Transactions
{
    long double balance;
public:
    Transactions(long double bal = 0) : balance(bal) {}
    long double getBalance() const { return balance; }
    void depositMoney(long double amount) { balance += amount; }
    bool withDrawMoney(long double amount) {
        if (amount > balance) return false;
        balance -= amount;
        return true;
    }
    void displayBalance() const
    {
        cout << "Current Balance: " << balance << " PKR" << endl;
    }
};
class User {
protected:
    string userName, userID, loginID, userPassword;
    Transactions accountTransactions;
    AccountType* account;
public:
    long double DailyWithdrawalLimit;
    User(string uName, string uID, string uLoginID, string uPassword,
        long double eWLimit, long double bal)
        : userName(uName), userID(uID), loginID(uLoginID),
        userPassword(uPassword), DailyWithdrawalLimit(eWLimit),
        accountTransactions(bal)
    {

    }
    virtual void canDeposit(string date, long double amount) = 0;
    virtual void canWithdraw(string date, long double amount) = 0;
    virtual void canTransfer(string date, User& user, long double amount) = 0;
    virtual void viewTransactionHistory() = 0;
    virtual void displayUserInfo() = 0;
    virtual void checkBalance() = 0;
    virtual void changePassword() = 0;
    string getLoginID() const { return loginID; }
    string getUserID() const { return userID; }
    string getUserName() const { return userName; }
    string getUserPassword() const { return userPassword; }
    void setUserPassword(const string& pass) { userPassword = pass; }
    void depositInternal(long double amount) { accountTransactions.depositMoney(amount); }
    long double getBalance() const { return accountTransactions.getBalance(); }
    AccountType* getAccountType() const { return account; }
    void resetDailyLimit() { DailyWithdrawalLimit = 0; }
    virtual ~User() { if (account) delete account; }
};
void loadUserCards(UserClient* user);
// USER CLIENT 
class UserClient : public User
{
    string address, phoneNo, CNIC;
    Card* userCards[5];
    int totalCards;

    int chooseCardAndVerify()
    {
        if (totalCards == 0)
        {
            cout << "No cards added to your account.\n";
            return -1;
        }

        cout << "\nSelect a card:\n";
        for (int i = 0; i < totalCards; i++)
        {
            cout << i + 1 << ". Card Ending With "
                << userCards[i]->getCardNumber().substr(12)
                << (userCards[i]->getIsFraud() ? "  [FRAUD - BLOCKED]" : "")
                << endl;
        }

        int choice;
        cin >> choice;

        if (choice < 1 || choice > totalCards)
        {
            cout << "Invalid choice.\n";
            return -1;
        }

        int idx = choice - 1;

        if (userCards[idx]->getIsFraud())
        {
            cout << "This card is BLOCKED due to suspected FRAUD.\n";
            return -1;
        }

        string pin;
        cout << "Enter PIN: ";
        cin >> pin;

        if (!userCards[idx]->verifyPin(pin))
            return -1;     // verifyPin() handles wrong attempts & fraud marking

        return idx;
    }


public:
    static int nextUserID;

    UserClient(string uName, string uLoginID, string uPassword,
        long double eWLimit, long double bal,
        string addr, string phone, string cnic, string existingUserID)
        : User(uName, existingUserID.empty() ? "U" + to_string(nextUserID++) : existingUserID, uLoginID, uPassword,
            eWLimit, bal),
        address(addr), phoneNo(phone), CNIC(cnic)
    {
        if (this->DailyWithdrawalLimit < 100000)
        {
            this->account = new Bronze();
        }
        else if (this->DailyWithdrawalLimit < 500000)
        {
            this->account = new Gold();
        }
        else
        {
            //Default to Business because it exceeds max limit

            this->account = new Business();
        }
        totalCards = 0;
        for (int i = 0; i < 5; i++)
            userCards[i] = nullptr;
    }

    string getAddress() const { return address; }
    string getPhone() const { return phoneNo; }
    string getCNIC() const { return CNIC; }
    void showCards()
    {
        if (totalCards == 0)
        {
            cout << "No cards available.\n";
            return;
        }
        cout << "Your Cards:\n";
        for (int i = 0; i < totalCards; i++)
        {
            cout << i + 1 << ". " << userCards[i]->getCardNumber();
            if (userCards[i]->getIsFraud()) cout << " [FRAUD]";
            cout << endl;
        }
    }
    void addLoadedCard(const string& number, const string& pin)
    {
        if (totalCards < 5)
        {
            userCards[totalCards] = new Card(number, pin);
            totalCards++;
        }
    }

    void addCard()
    {
        if (totalCards >= 5)
        {
            cout << "Maximum cards limit (5) reached.\n";
            return;
        }

        string pin;
        cout << "Enter PIN for new card: ";
        cin >> pin;

        // Auto-generate card number properly
        Card temp;                          // temporary card object
        string number = temp.generateCardNumber();

        // Create new card object
        userCards[totalCards] = new Card(number, pin);
        totalCards++;

        // Save to file
        ofstream cards("cards.txt", ios::app);
        cards << this->getLoginID() << "," << number << "," << pin << "\n";
        cards.close();

        cout << "New card assigned successfully.\nCard Number: " << number << endl;
    }

    bool verifyCard(string cardNumber, string cardPin)
    {
        for (int i = 0; i < totalCards; i++) {
            if (userCards[i]->getCardNumber() == cardNumber && userCards[i]->getCardPin() == cardPin)
                return true;
        }
        return false;
    }

    void changePassword() override
    {
        string oldPass, newPass;
        bool updated = false;

        cout << "Enter current password: ";
        cin >> oldPass;
        ifstream infile("users.txt");
        ofstream temp("temp.txt");
        string line;
        while (getline(infile, line))
        {
            stringstream ss(line);
            string userID, lID, name, address, phone, cnic, password, balance, accType;
            getline(ss, userID, ',');
            getline(ss, lID, ',');
            getline(ss, name, ',');
            getline(ss, address, ',');
            getline(ss, phone, ',');
            getline(ss, cnic, ',');
            getline(ss, password, ',');
            getline(ss, balance, ',');
            getline(ss, accType, ',');

            if (lID == loginID)
            {
                if (password != oldPass)
                {
                    cout << "Incorrect current password.\n";
                    temp << line << "\n";
                    continue;
                }

                cout << "Enter new password: ";
                cin >> newPass;

                temp << userID << "," << lID << "," << name << "," << address << "," << phone
                    << "," << cnic << "," << newPass << "," << balance << "," << accType << "\n";
                updated = true;

            }
            else
            {
                temp << line << "\n";
            }

        }
        infile.close();
        temp.close();
        remove("users.txt");
        rename("temp.txt", "users.txt");
        if (updated)
        {
            cout << "Password changed successfully!\n";
        }
        else
        {
            cout << "Password not changed.\n";
        }

    }
    void displayCards() {
        if (totalCards == 0) {
            cout << "No cards available.\n";
            return;
        }
        cout << "Your Cards:\n";
        for (int i = 0; i < totalCards; i++) {
            cout << i + 1 << ". " << userCards[i]->getCardNumber();
            if (userCards[i]->getIsFraud()) cout << " [FRAUD]";
            cout << endl;
        }
    }
    // save file
    void saveToFile()
    {
        ofstream file("userclient.txt", ios::app);
        file << getUserID() << ","
            << getUserName() << ","
            << getLoginID() << ","
            << getUserPassword() << ","
            << address << ","
            << phoneNo << ","
            << CNIC << ","
            << accountTransactions.getBalance() << ","
            << account->getTypeName()
            << endl;
        file.close();
    }
    void checkCardNo()
    {
        ifstream infile("cards.txt");
        string line;
        for (int i = 0; i < this->totalCards;i++)
        {
            while (getline(infile, line))
            {
                stringstream ss(line);
                string lID, cNumber, cPin;
                getline(ss, lID, ',');
                getline(ss, cNumber, ',');
                getline(ss, cPin, ',');
                if (lID == getLoginID() && cNumber == userCards[i]->getCardNumber())
                {
                    cout << lID << "," << cNumber << "," << cPin << "\n";
                }

            }
        }
        infile.close();
    }
    void changeCardPin()
    {
        int selectedCard;
        cout << "Select your card number to change PIN: ";
        cin >> selectedCard;
        string oldPin;
        cout << "Enter current PIN: ";
        cin >> oldPin;
        if (userCards[selectedCard - 1]->getCardPin() != oldPin)
        {
            cout << "Incorrect current PIN!\n";
            return;
        }
    xyz:
        string newPin;
        cout << "Enter new PIN (4 digits): ";
        cin >> newPin;
        if (newPin.length() != 4)
        {
            cout << "PIN must be 4 digits!\n";
            goto xyz;
        }
        userCards[selectedCard - 1]->setPin(newPin);
        cout << "PIN changed successfully!\n";
        // Update cards file
        ifstream infile("cards.txt");
        ofstream outfile("temp_cards.txt");
        string line;
        while (getline(infile, line))
        {
            stringstream ss(line);
            string lID, cNumber, cPin;
            getline(ss, lID, ',');
            getline(ss, cNumber, ',');
            getline(ss, cPin, ',');
            if (lID == getLoginID() && cNumber == userCards[selectedCard - 1]->getCardNumber())
            {
                outfile << lID << "," << cNumber << "," << newPin << "\n";
            }
            else
            {
                outfile << line << "\n";
            }
        }
        infile.close();
        outfile.close();
        remove("cards.txt");
        rename("temp_cards.txt", "cards.txt");
    }

    void canDeposit(string date, long double amount) override {
        int idx = chooseCardAndVerify();
        if (idx < 0) return;

        if (amount <= 0) {
            cout << "Invalid amount. Please enter positive value.\n";
            return;
        }

        this->accountTransactions.depositMoney(amount);
        updateUserBalanceInFile(getLoginID(), this->accountTransactions.getBalance());

        ofstream file("transactions.txt", ios::app);
        file << date << "," << userID << "," << amount << ",Deposit,-1" << endl;
        file.close();

        cout << "Successfully deposited " << amount << " PKR.\n";
    }

    void canWithdraw(string date, long double amount) override {
        int idx = chooseCardAndVerify();
        if (idx < 0) return;

        if (amount <= 0) {
            cout << "Invalid amount. Please enter positive value.\n";
            return;
        }

        if (DailyWithdrawalLimit + amount > this->account->getWithDrawalLimit()) {
            cout << "Daily withdrawal limit exceeded! Available: "
                << (this->account->getWithDrawalLimit() - DailyWithdrawalLimit) << " PKR\n";
            return;
        }

        if (amount > this->accountTransactions.getBalance()) {
            cout << "Insufficient balance! Current balance: " << this->accountTransactions.getBalance() << " PKR\n";
            return;
        }

        if (this->accountTransactions.withDrawMoney(amount)) {
            this->DailyWithdrawalLimit += amount;
            updateUserBalanceInFile(getLoginID(), this->accountTransactions.getBalance());

            ofstream file("transactions.txt", ios::app);
            file << date << "," << userID << "," << amount << ",Withdraw,-1" << endl;
            file.close();

            cout << "Successfully withdrawn " << amount << " PKR.\n";
        }
    }

    void canTransfer(string date, User& user, long double amount) override {
        int idx = chooseCardAndVerify();
        if (idx < 0) return;

        if (amount <= 0) {
            cout << "Invalid amount.\n";
            return;
        }

        if (amount > this->accountTransactions.getBalance()) {
            cout << "Insufficient balance! Current balance: " << this->accountTransactions.getBalance() << " PKR\n";
            return;
        }

        this->accountTransactions.withDrawMoney(amount);
        updateUserBalanceInFile(getLoginID(), this->accountTransactions.getBalance());

        user.depositInternal(amount);
        updateUserBalanceInFile(user.getLoginID(), user.getBalance());

        ofstream file("transactions.txt", ios::app);
        file << date << "," << getUserID() << "," << amount << ",Transfer," << user.getUserID() << endl;
        file.close();

        cout << "Successfully transferred " << amount << " PKR to " << user.getUserID() << ".\n";
    }

  

    void viewTransactionHistory() override {
        ifstream file("transactions.txt");
        string line;
        bool found = false;

        cout << "Transaction History for " << userName << ":\n";
        cout << "-----------------------------\n";

        while (getline(file, line)) {
            if (line.find(userID) != string::npos) {
                cout << line << endl;
                found = true;
            }
        }
        file.close();

        if (!found) {
            cout << "No transactions found.\n";
        }
    }

    void displayUserInfo() override {
        cout << "\n=== User Information ===\n";
        cout << "Name: " << userName << "\n";
        cout << "User ID: " << userID << "\n";
        cout << "Address: " << address << "\n";
        cout << "Phone: " << phoneNo << "\n";
        cout << "CNIC: " << CNIC << "\n";
        account->displayAccountTypeInfo();
        accountTransactions.displayBalance();
        cout << "Daily Withdrawal Used: " << DailyWithdrawalLimit << " PKR\n";
        cout << "Remaining Daily Withdrawal: " << (account->getWithDrawalLimit() - DailyWithdrawalLimit) << " PKR\n";
    }

    void checkBalance() override {
        cout << "\n=== Account Balance ===\n";
        accountTransactions.displayBalance();
    }

    ~UserClient() {
        for (int i = 0; i < totalCards; i++)
            delete userCards[i];
    }
};

// COMPANY CLIENT 
class CompanyClient : public User
{
    string companyName, companyAddress, companyTaxNumber;

public:
    static int nextCompanyID;

    CompanyClient(string companyName, string uLoginID, string uPassword,
        long double eWLimit, long double bal,
        string cAddress, string cTaxNumber, AccountType* acc)
        : User(companyName, "C" + to_string(nextCompanyID++),
            uLoginID, uPassword,
            eWLimit, bal), companyName(companyName),
        companyAddress(cAddress), companyTaxNumber(cTaxNumber)
    {
        this->account = acc;
    }



    string getCompanyName() const { return companyName; }
    string getCompanyAddress() const { return companyAddress; }
    string getCompanyTaxNumber() const { return companyTaxNumber; }
    CompanyClient* loginCompanyClient(const string& loginInput, const string& password) {
        ifstream file("companies.txt");
        string line;

        while (getline(file, line)) {
            stringstream ss(line);
            string companyID, companyName, address, tax, loginID, pass, balanceStr;

            getline(ss, companyID, ',');
            getline(ss, companyName, ',');
            getline(ss, address, ',');
            getline(ss, tax, ',');
            getline(ss, loginID, ',');
            getline(ss, pass, ',');
            getline(ss, balanceStr, ',');

            if (loginID == loginInput && pass == password) {

                long double balance = stold(balanceStr);
                AccountType* accType = new Business();

                return new CompanyClient(
                    companyName,
                    loginID,
                    pass,
                    0,
                    balance,
                    address,
                    tax,
                    accType
                );

            }
        }
        return nullptr;
    }
    void changePassword() override
    {
        string oldPass, newPass;
        bool updated = false;

        cout << "Enter current password: ";
        cin >> oldPass;
        ifstream infile("companies.txt");
        ofstream temp("temp.txt");
        string line;
        while (getline(infile, line))
        {
            stringstream ss(line);
            string cid, name, address, tax, lID, password, balance;
            getline(ss, cid, ',');
            getline(ss, name, ',');
            getline(ss, address, ',');
            getline(ss, tax, ',');
            getline(ss, lID, ',');
            getline(ss, password, ',');
            getline(ss, balance, ',');

            if (lID == loginID)
            {
                if (password != oldPass)
                {
                    cout << "Incorrect current password.\n";
                    temp << line << "\n";
                    continue;
                }

                cout << "Enter new password: ";
                cin >> newPass;

                temp << cid << "," << name << "," << address << "," << tax << ","
                    << lID << "," << newPass << "," << balance << "\n";

                updated = true;

            }
            else
            {
                temp << line << "\n";
            }

        }
        infile.close();
        temp.close();
        remove("companies.txt");
        rename("temp.txt", "companies.txt");
        if (updated)
        {
            cout << "Password changed successfully!\n";
        }
        else
        {
            cout << "Password not changed.\n";
        }

    }
    // save to file
    void saveToFile()
    {
        ofstream file("companyclient.txt", ios::app);
        file << getUserID() << ","
            << companyName << ","
            << companyAddress << ","
            << companyTaxNumber << ","
            << getLoginID() << ","
            << getUserPassword() << ","
            << accountTransactions.getBalance() << ","
            << account->getTypeName()
            << endl;
        file.close();
    }

    void canDeposit(string date, long double amount) override {
        if (amount <= 0) {
            cout << "Invalid amount. Please enter positive value.\n";
            return;
        }

        this->accountTransactions.depositMoney(amount);
        updateCompanyBalanceInFile(getUserID(), this->accountTransactions.getBalance());

        ofstream file("transactions.txt", ios::app);
        file << date << "," << getUserID() << "," << amount << ",Deposit,-1" << endl;
        file.close();

        cout << "Successfully deposited " << amount << " PKR.\n";
    }

    void canWithdraw(string date, long double amount) override {
        if (amount <= 0) {
            cout << "Invalid amount.\n";
            return;
        }

        if (this->DailyWithdrawalLimit + amount > this->account->getWithDrawalLimit()) {
            cout << "Daily withdrawal limit exceeded! Available: "
                << (this->account->getWithDrawalLimit() - this->DailyWithdrawalLimit) << " PKR\n";
            return;
        }

        if (amount > this->accountTransactions.getBalance()) {
            cout << "Insufficient balance! Current balance: " << this->accountTransactions.getBalance() << " PKR\n";
            return;
        }

        if (this->accountTransactions.withDrawMoney(amount)) {
            this->DailyWithdrawalLimit += amount;
            updateCompanyBalanceInFile(getUserID(), this->accountTransactions.getBalance());

            ofstream file("transactions.txt", ios::app);
            file << date << "," << getUserID() << "," << amount << ",Withdraw,-1" << endl;
            file.close();

            cout << "Successfully withdrawn " << amount << " PKR.\n";
        }
    }

    void canTransfer(string date, User& targetUser, long double amount) override {
        if (amount <= 0) {
            cout << "Invalid amount.\n";
            return;
        }

        if (amount > this->accountTransactions.getBalance()) {
            cout << "Insufficient balance! Current balance: " << this->accountTransactions.getBalance() << " PKR\n";
            return;
        }

        this->accountTransactions.withDrawMoney(amount);
        updateCompanyBalanceInFile(getUserID(), this->accountTransactions.getBalance());

        targetUser.depositInternal(amount);
        updateUserBalanceInFile(targetUser.getLoginID(), targetUser.getBalance());

        ofstream file("transactions.txt", ios::app);
        file << date << "," << getUserID() << "," << amount << ",Transfer," << targetUser.getUserID() << endl;
        file.close();

        cout << "Successfully transferred " << amount << " PKR to " << targetUser.getUserID() << ".\n";
    }

    void viewTransactionHistory() override
    {
        ifstream file("transactions.txt");
        string line;
        bool found = false;

        cout << "Transaction History for " << companyName << ":\n";
        cout << "------------------------------------\n";

        while (getline(file, line)) {
            if (line.find(userID) != string::npos)
            {
                cout << line << endl;
                found = true;
            }
        }
        file.close();

        if (!found)
        {
            cout << "No transactions found.\n";
        }
    }

    void displayUserInfo() override
    {
        cout << "\n=== Company Information ===\n";
        cout << "Company Name: " << companyName << "\n";
        cout << "Company ID: " << userID << "\n";
        cout << "Address: " << companyAddress << "\n";
        cout << "Tax Number: " << companyTaxNumber << "\n";
        account->displayAccountTypeInfo();
        accountTransactions.displayBalance();
        cout << "Daily Withdrawal Used: " << DailyWithdrawalLimit << " PKR\n";
        cout << "Remaining Daily Withdrawal: " << (account->getWithDrawalLimit() - DailyWithdrawalLimit) << " PKR\n";
    }

    void checkBalance() override {
        cout << "\n=== Company Account Balance ===\n";
        accountTransactions.displayBalance();
    }

    void requestLoan() {
        long double amount;
        cout << "Enter loan amount: ";
        cin >> amount;

        if (amount <= 0) {
            cout << "Invalid loan amount!\n";
            return;
        }

        ofstream file("loan_requests.txt", ios::app);
        file << "Company: " << companyName << ", ID: " << userID << ", Amount: " << amount << " PKR" << endl;
        file.close();

        cout << "Loan request of " << amount << " PKR submitted successfully!\n";
    }
};

void appendUserToUsersFile(const string& userID, const string& loginID, const string& name, const string& address, const string& phone, const string& cnic, const string& password, long double balance, const string& accType);

//  BANKING EMPLOYEE 
class BankingEmployee
{
    string employeeID, password;
public:
    BankingEmployee(string id = "admin", string pass = "admin123") :employeeID(id), password(pass) {}

    bool login(string id, string pass) {
        return id == employeeID && pass == password;
    }
    void reviewFraudReports()
    {
        ifstream inFile("fraud_report.txt");
        string line;
        while (getline(inFile, line)) {
            cout << line << endl; // show fraud reports
        }
        inFile.close();

        
    }
    // new reject and approve func:
    void approveOrRejectAccount()
    {
        string targetLogin;
        cout << "Enter applicant login ID to process: ";
        cin >> targetLogin;
        ifstream fin("pending_users.txt");
        if (!fin) { cout << "No pending applications.\n"; return; }
        ofstream temp("temp_pending.txt");
        string line;
        bool found = false;
        string approvedLine;
        while (getline(fin, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            // expecting: loginID,name,address,cnic,loginID,password,0,accountType
            string loginID, name, address, cnic, loginID2, password, zero, accType;
            getline(ss, loginID, ',');
            getline(ss, name, ',');
            getline(ss, address, ',');
            getline(ss, cnic, ',');
            getline(ss, loginID2, ',');
            getline(ss, password, ',');
            getline(ss, zero, ',');
            getline(ss, accType);
            if (loginID == targetLogin) {
                found = true;
                char ch;
                cout << "Approve (A) or Reject (R) for " << loginID << " (" << name << ")? ";
                cin >> ch;
                if (toupper(ch) == 'A') {
                    approvedLine = line;
                    cout << "Approved.\n";
                    // generate card and write to cards.txt (cardNumber,pin)
                    ofstream cards("cards.txt", ios::app);
                    string cardNumber = "4000";
                    for (int i = 0; i < 12; i++) cardNumber += char('0' + (rand() % 10));
                    string pin = "";
                    for (int i = 0; i < 4; i++) pin += char('0' + (rand() % 10));
                    cards << loginID << "," << cardNumber << "," << pin << "\n";
                    cards.close();
                    // create userID based on static counter (or derive next available)
                    string userID = "U" + to_string(UserClient::nextUserID++);
                    long double balance = 0.0;
                    // append in standardized users.txt format:
                    appendUserToUsersFile(userID, loginID, name, address, /*phone*/"", cnic, password, balance, accType);
                }
                else {
                    cout << "Rejected.\n";
                }
            }
            else {
                temp << line << "\n";
            }
        }
        fin.close();
        temp.close();
        remove("pending_users.txt");
        rename("temp_pending.txt", "pending_users.txt");
        if (!found) cout << "No application found for login ID: " << targetLogin << "\n";
    }

   

    void viewAllAccounts() {
        cout << "\n=== ALL USER ACCOUNTS ===\n";
        ifstream f("users.txt");
        string line;

        if (!f) {
            cout << "No user accounts found.\n";
        }
        else {
            cout << "User Accounts:\n";
            while (getline(f, line)) {
                if (!line.empty()) cout << line << endl;
            }
            f.close();
        }

        cout << "\n=== ALL COMPANY ACCOUNTS ===\n";
        ifstream f2("companies.txt");

        if (!f2) {
            cout << "No company accounts found.\n";
        }
        else {
            cout << "Company Accounts:\n";
            while (getline(f2, line)) {
                if (!line.empty()) cout << line << endl;
            }
            f2.close();
        }
    }

    void viewTransactions() {
        cout << "\n=== ALL TRANSACTIONS ===\n";
        ifstream f("transactions.txt");
        string line;

        if (!f) {
            cout << "No transactions found.\n";
            return;
        }

        while (getline(f, line)) {
            if (!line.empty()) cout << line << endl;
        }
        f.close();
    }

    void viewLoanRequests() {
        cout << "\n=== PENDING LOAN REQUESTS ===\n";
        ifstream f("loan_requests.txt");
        string line;

        if (!f) {
            cout << "No loan requests found.\n";
            return;
        }

        while (getline(f, line)) {
            if (!line.empty()) cout << line << endl;
        }
        f.close();
    }

private:
    string generateCardNumber() {
        string number = "4000";
        for (int i = 0; i < 12; i++) number += to_string(rand() % 10);
        return number;
    }

    string generatePIN() {
        string pin = "";
        for (int i = 0; i < 4; i++) pin += to_string(rand() % 10);
        return pin;
    }
};

void forgotPassword(bool isCompany)
{
    string loginID, cnicOrTax;
    cout << "Enter Login ID: ";
    cin >> loginID;

    if (isCompany)
    {
        cout << "Enter Company Tax Number: ";
        cin >> cnicOrTax;
    }
    else
    {
        cout << "Enter CNIC (without dashes): ";
        cin >> cnicOrTax;
    }

    string fileName = isCompany ? "companies.txt" : "users.txt";

    ifstream fin(fileName);
    ofstream temp("temp.txt");
    string line;

    bool found = false;

    while (getline(fin, line))
    {
        if (line.empty()) continue;

        stringstream ss(line);

        if (!isCompany)
        {
            // USER FORMAT:
            string userID, lID, name, address, phone, cnic, password, balance, accType;

            getline(ss, userID, ',');
            getline(ss, lID, ',');
            getline(ss, name, ',');
            getline(ss, address, ',');
            getline(ss, phone, ',');
            getline(ss, cnic, ',');
            getline(ss, password, ',');
            getline(ss, balance, ',');
            getline(ss, accType, ',');

            if (lID == loginID && cnic == cnicOrTax)
            {
                found = true;
                string newPass;
                cout << "Identity verified! Enter new password: ";
                cin >> newPass;

                temp << userID << "," << lID << "," << name << "," << address << "," << phone
                    << "," << cnic << "," << newPass << "," << balance << "," << accType << "\n";
            }
            else temp << line << "\n";
        }
        else
        {
            // COMPANY FORMAT:
            string cid, name, address, tax, lID, password, balance;

            getline(ss, cid, ',');
            getline(ss, name, ',');
            getline(ss, address, ',');
            getline(ss, tax, ',');
            getline(ss, lID, ',');
            getline(ss, password, ',');
            getline(ss, balance, ',');

            if (lID == loginID && tax == cnicOrTax)
            {
                found = true;
                string newPass;
                cout << "Identity verified! Enter new password: ";
                cin >> newPass;

                temp << cid << "," << name << "," << address << "," << tax
                    << "," << lID << "," << newPass << "," << balance << "\n";
            }
            else temp << line << "\n";
        }
    }

    fin.close();
    temp.close();
    remove(fileName.c_str());
    rename("temp.txt", fileName.c_str());

    if (found) cout << "Password reset successfully!\n";
    else cout << "Incorrect login ID or CNIC/Tax Number.\n";
}


// Function to check if user has any cards in the system
bool userHasCards(const string& userID)
{
    ifstream file("cards.txt");
    string line;

    if (!file) return false;

    while (getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }


        stringstream ss(line);
        string storedUserID;
        getline(ss, storedUserID, ',');

        if (storedUserID == userID)
        {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

// returns true if loginID already exists in users.txt
bool loginIDExists(const string& loginID) {
    ifstream fin("users.txt");
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string userID, fLoginID;
        getline(ss, userID, ',');     // userID
        getline(ss, fLoginID, ',');   // loginID 
        if (fLoginID == loginID) return true;
    }
    return false;
}

// Save a newly created approved user to users.txt (use the standardized format)
void appendUserToUsersFile(const string& userID, const string& loginID, const string& name,
    const string& address, const string& phone, const string& cnic,
    const string& password, long double balance, const string& accType) {
    ofstream fout("users.txt", ios::app);
    fout << userID << "," << loginID << "," << name << "," << address << "," << phone << "," << cnic
        << "," << password << "," << balance << "," << accType << "\n";
}

void loadUserCards(UserClient* user)
{
    if (!user) return;

    ifstream fin("cards.txt");
    string line;

    while (getline(fin, line))
    {
        if (line.empty()) continue;
        stringstream ss(line);

        string loginID, number, pin;
        getline(ss, loginID, ',');
        getline(ss, number, ',');
        getline(ss, pin, ',');

        if (loginID == user->getLoginID())
        {
            user->addLoadedCard(number, pin);
        }
    }

    fin.close();
}



UserClient* loadUserByLoginID(const string& loginID) {
    ifstream fin("users.txt");
    if (!fin) return nullptr;

    string line;
    while (getline(fin, line))
    {
        if (line.empty()) continue;

        stringstream ss(line);
        string userID, fLoginID, name, address, phone, cnic, password, balanceStr, accType;

        getline(ss, userID, ',');      // userID (e.g., U1)
        getline(ss, fLoginID, ',');    // loginID (your login identifier)
        getline(ss, name, ',');        // name
        getline(ss, address, ',');     // address
        getline(ss, phone, ',');       // phone
        getline(ss, cnic, ',');        // cnic
        getline(ss, password, ',');    // password
        getline(ss, balanceStr, ',');  // balance
        getline(ss, accType, ',');     // account type

        if (fLoginID == loginID)
        {
            long double balance = 0;
            try { balance = stold(balanceStr); }
            catch (...) { balance = 0; }

            UserClient* user = new UserClient(name, fLoginID, password, 0, balance, address, phone, cnic, userID);

            // load cards into the user object
            loadUserCards(user);

            return user;
        }
    }

    return nullptr;
}

//newly added
UserClient* loginUserClient(const string& loginInput, const string& password) {
    ifstream file("users.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string userID, loginID, userName, address, phone, cnic, pass, balanceStr, accType;
        getline(ss, userID, ',');
        getline(ss, loginID, ',');
        getline(ss, userName, ',');
        getline(ss, address, ',');
        getline(ss, phone, ',');
        getline(ss, cnic, ',');
        getline(ss, pass, ',');
        getline(ss, balanceStr, ',');
        getline(ss, accType);

        if (loginID == loginInput && pass == password) {
            long double balance = 0;
            try { balance = stold(balanceStr); }
            catch (...) { balance = 0; }
            // Choose account type
            AccountType* accPtr = nullptr;
            if (accType == "Gold") accPtr = new Gold();
            else if (accType == "Business") accPtr = new Business();
            else accPtr = new Bronze();

            UserClient* u = new UserClient(userName, loginID, pass, 0, balance, address, phone, cnic, userID);
           
            loadUserCards(u);
            return u;
        }
    }
    return nullptr;
}


// new one
void addEmployeeToCompany(const string& employeeLoginID, const string& companyID) {
    ofstream f("companies_employees.txt", ios::app);
    f << employeeLoginID << "," << companyID << "\n";
}
vector<string> getCompaniesOfUser(const string& loginID)
{
    vector<string> result;
    ifstream f("companies_employees.txt");
    string line;

    while (getline(f, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string employee, company;
        getline(ss, employee, ',');
        getline(ss, company, ',');
        if (employee == loginID)
            result.push_back(company);
    }
    return result;
}
CompanyClient* loadCompanyByID(const string& companyID)
{
    ifstream file("companyclient.txt");   
    if (!file)
    {
        cout << "Error opening companyclient.txt\n";
        return nullptr;
    }

    string line;
    while (getline(file, line))
    {
        if (line.empty()) continue;

        stringstream ss(line);
        string cID, name, loginID, password, address, tax, balanceStr, withdrawLimitStr;

        getline(ss, cID, ',');
        getline(ss, name, ',');
        getline(ss, loginID, ',');
        getline(ss, password, ',');
        getline(ss, address, ',');
        getline(ss, tax, ',');
        getline(ss, withdrawLimitStr, ',');
        getline(ss, balanceStr, ',');

        if (cID == companyID)
        {
            long double balance = stold(balanceStr);
            long double wLimit = stold(withdrawLimitStr);

            // assign correct account type
            AccountType* acc = nullptr;
            if (wLimit <= 100000)
            {
                acc = new Bronze();
            }
            else if (wLimit > 100000 && wLimit <= 500000)
            {
                acc = new Gold();
            }
            else
            {
                acc = new Business();
            }

            return new CompanyClient(
                name,
                loginID,
                password,
                wLimit,
                balance,
                address,
                tax,
                acc
            );
        }
    }

    return nullptr;
}


void signupUserClient()
{
    string name, address, phone, cnic, loginID, password;
    long double estimatedLimit;

    cout << "\n=== USER CLIENT SIGNUP ===\n";
    cout << "Full Name: ";
    cin.ignore();
    getline(cin, name);
    cout << "Address: ";
    getline(cin, address);
    cout << "Phone: ";
    getline(cin, phone);
    cout << "CNIC: ";
    getline(cin, cnic);
    cout << "Login ID: ";
    getline(cin, loginID);
    cout << "Password: ";
    getline(cin, password);
    cout << "Estimated Daily Withdrawal Limit: ";
    cin >> estimatedLimit;

    // Determine account type based on estimated limit
    string accountType = "Bronze";
    if (estimatedLimit >= 100000 && estimatedLimit < 500000) accountType = "Gold";

    // Save to pending file for employee approval
    ofstream file("pending_users.txt", ios::app);
    file << loginID << "," << name << "," << address << "," << cnic << ","
        << loginID << "," << password << ",0," << accountType << endl;
    file.close();

    cout << "Account application submitted! Please wait for employee approval.\n";
}


void signupCompanyClient() {
    string companyName, address, taxNumber, loginID, password;
    long double estimatedLimit;

    cout << "\n=== COMPANY CLIENT SIGNUP ===\n";
    cout << "Company Name: ";
    cin.ignore();
    getline(cin, companyName);
    cout << "Company Address: ";
    getline(cin, address);
    cout << "Tax Number: ";
    getline(cin, taxNumber);
    cout << "Login ID: ";
    getline(cin, loginID);
    cout << "Password: ";
    getline(cin, password);
    cout << "Estimated Daily Withdrawal Limit: ";
    cin >> estimatedLimit;

    // Save to companies file
    ofstream file("companies.txt", ios::app);
    file << "C" << time(nullptr) << "," << companyName << "," << address << ","
        << taxNumber << "," << loginID << "," << password << ",0" << endl;
    file.close();

    cout << "Company account created successfully!\n";
}



CompanyClient* loginCompanyClient(const string& loginInput, const string& password) {
    ifstream file("companies.txt");
    string line;

    while (getline(file, line))
    {
        if (line.empty()) continue;
        stringstream ss(line);

        string companyID, companyName, address, tax, loginID, pass, balanceStr;
        getline(ss, companyID, ',');
        getline(ss, companyName, ',');
        getline(ss, address, ',');
        getline(ss, tax, ',');
        getline(ss, loginID, ',');
        getline(ss, pass, ',');
        getline(ss, balanceStr, ',');

        if (loginID == loginInput && pass == password)
        {

            long double balance = stold(balanceStr);
            AccountType* acc = new Business();

            return new CompanyClient(
                companyName,
                loginID,
                pass,
                0,
                balance,
                address,
                tax,
                acc
            );
        }
    }
    return nullptr;
}


//new one
void updateUserBalanceInFile(const string& loginID, long double newBalance) {
    ifstream fin("users.txt");
    ofstream temp("temp.txt");
    string line;

    while (getline(fin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string userID, lID, name, address, phone, cnic, password, balanceStr, accType;
        getline(ss, userID, ',');
        getline(ss, lID, ',');
        getline(ss, name, ',');
        getline(ss, address, ',');
        getline(ss, phone, ',');
        getline(ss, cnic, ',');
        getline(ss, password, ',');
        getline(ss, balanceStr, ',');
        getline(ss, accType, ',');

        if (lID == loginID) balanceStr = to_string(newBalance);

        temp << userID << "," << lID << "," << name << "," << address << "," << phone
            << "," << cnic << "," << password << "," << balanceStr << "," << accType << "\n";
    }

    fin.close();
    temp.close();
    remove("users.txt");
    rename("temp.txt", "users.txt");
}

void updateCompanyBalanceInFile(const string& companyID, long double newBalance) {
    ifstream fin("companies.txt");
    ofstream temp("temp.txt");
    string line;

    while (getline(fin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string cID, name, address, tax, loginID, password, balanceStr;
        getline(ss, cID, ',');
        getline(ss, name, ',');
        getline(ss, address, ',');
        getline(ss, tax, ',');
        getline(ss, loginID, ',');
        getline(ss, password, ',');
        getline(ss, balanceStr, ',');

        if (cID == companyID) balanceStr = to_string(newBalance);

        temp << cID << "," << name << "," << address << "," << tax << ","
            << loginID << "," << password << "," << balanceStr << "\n";
    }

    fin.close();
    temp.close();
    remove("companies.txt");
    rename("temp.txt", "companies.txt");
}


void companyClientMenu();
void userClientMenu()
{
    int choice;
    do
    {
        cout << "\n=== USER CLIENT MENU ===\n";
        cout << "1. Login\n";
        cout << "2. Signup\n";
        cout << "3. Forgot Password\n";
        cout << "4. Back to Main Menu\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
        {
            string userID, password;
            cout << "Login ID: ";
            cin >> userID;
            cout << "Password: ";
            cin >> password;

            currentUserClient = loginUserClient(userID, password);
            if (currentUserClient)
            {
                cout << "Login successful! Welcome " << currentUserClient->getUserName() << "!\n";
                vector<string> companies = getCompaniesOfUser(currentUserClient->getLoginID());
                if (!companies.empty())
                {
                    cout << "You are an employee of company: " << companies[0] << endl;
                    cout << "Access which account?\n1. Personal\n2. Company\n";
                    int ch; cin >> ch;

                    if (ch == 2)
                    {
                        currentCompanyClient = loadCompanyByID(companies[0]);
                        companyClientMenu();
                        return; // IMPORTANT
                    }
                }

                // User logged in menu
                int userChoice;
                do {
                    cout << "\n--- User Dashboard ---\n";
                    cout << "1. Deposit\n";
                    cout << "2. Withdraw\n";
                    cout << "3. Transfer\n";
                    cout << "4. Check Balance\n";
                    cout << "5. View Transaction History\n";
                    cout << "6. Change Password\n";
                    cout << "7. View My Information\n";
                    cout << "8. View My Cards\n";
                    cout << "9. Add New Card\n";
                    cout << "10. Change Card Pin\n";
                    cout << "11. Check Card Number\n";
                    cout << "0. Logout\n";
                    cout << "Enter choice: ";
                    cin >> userChoice;

                    string date; 

                    switch (userChoice)
                    {
                    case 1: {
                        long double amount;
                        cout << "Enter deposit amount: ";
                        cin >> amount;
                        cout << "Enter date (YYYY-MM-DD): ";
                        cin >> date;

                        currentUserClient->canDeposit(date, amount);
                        break;
                    }
                    case 2: {
                        long double amount;
                        cout << "Enter withdrawal amount: ";
                        cin >> amount;
                        cout << "Enter date (YYYY-MM-DD): ";
                        cin >> date;

                        currentUserClient->canWithdraw(date, amount);
                        break;
                    }
                    case 3:
                    {
                        long double amount;
                        string targetID;
                        cout << "Enter Other's User ID: ";
                        cin >> targetID;
                        cout << "Enter transfer amount: ";
                        cin >> amount;
                        cout << "Enter date (YYYY-MM-DD): ";
                        cin >> date;

                        UserClient* targetUser = loadUserByLoginID(targetID);
                        if (!targetUser) {
                            cout << "Target user not found!\n";
                            break;
                        }

                        currentUserClient->canTransfer(date, *targetUser, amount);

                        delete targetUser; 
                        break;
                    }

                    case 4:
                    {
                        currentUserClient->checkBalance();
                        break;
                    }
                    case 5:
                    {
                        currentUserClient->viewTransactionHistory();
                        break;
                    }
                    case 6:
                    {
                        currentUserClient->changePassword();
                        break;
                    }
                    case 7:
                    {
                        currentUserClient->displayUserInfo();
                        break;
                    }
                    case 8:
                    {
                        currentUserClient->displayCards();
                        break;
                    }
                    case 9:
                    {
                        currentUserClient->addCard();
                        break;
                    }
                    case 10:
                    {
                        currentUserClient->changeCardPin();
                        break;
                    }
                    case 11:
                    {
                        currentUserClient->checkCardNo();
                        break;

                    }
                    case 0:
                    {
                        cout << "Logging out...\n";
                        delete currentUserClient;
                        currentUserClient = nullptr;
                        break;
                    }
                    default:
                    {
                        cout << "Invalid choice!\n";
                    }
                    }
                } while (userChoice != 0 && currentUserClient != nullptr);
            }
            else {
                cout << "Invalid login credentials!\n";
            }
            break;
        }
        case 2:
        {
            signupUserClient();
            break;
        }
        case 3:
        {
            forgotPassword(false);
            break;
        }
        case 4:
        {
            cout << "Returning to main menu...\n";
            break;
        }
        default:
        {
            cout << "Invalid choice!\n";
        }

        }
    } while (choice != 4);
}
void companyClientMenu()
{
    int choice;
    do
    {
        cout << "\n=== COMPANY CLIENT MENU ===\n";
        cout << "1. Login\n";
        cout << "2. Signup\n";
        cout << "3. Forgot Password\n";
        cout << "4. Back to Main Menu\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
        {
            string companyID, password;
            cout << "Company ID: ";
            cin >> companyID;
            cout << "Password: ";
            cin >> password;

            currentCompanyClient = loginCompanyClient(companyID, password);
            if (currentCompanyClient) {
                cout << "Login successful! Welcome " << currentCompanyClient->getCompanyName() << "!\n";

                // Company logged in menu
                int companyChoice;
                do {
                    cout << "\n--- Company Dashboard ---\n";
                    cout << "1. Deposit\n";
                    cout << "2. Withdraw\n";
                    cout << "3. Transfer\n";
                    cout << "4. Check Balance\n";
                    cout << "5. View Transaction History\n";
                    cout << "6. Change Password\n";
                    cout << "7. Request Loan\n";
                    cout << "8. View Company Information\n";
                    cout << "0. Logout\n";
                    cout << "Enter choice: ";
                    cin >> companyChoice;

                    string date = "2024-01-01"; // Simplified date

                    switch (companyChoice) {
                    case 1: {
                        long double amount;
                        cout << "Enter deposit amount: ";
                        cin >> amount;
                        currentCompanyClient->canDeposit(date, amount);
                        break;
                    }
                    case 2: {
                        long double amount;
                        cout << "Enter withdrawal amount: ";
                        cin >> amount;
                        currentCompanyClient->canWithdraw(date, amount);
                        break;
                    }
                    case 3:
                    {
                        long double amount;
                        string targetID;
                        cout << "Enter Other's User ID: ";
                        cin >> targetID;
                        cout << "Enter transfer amount: ";
                        cin >> amount;
                        cout << "Enter date (YYYY-MM-DD): ";
                        cin >> date;

                        UserClient* targetUser = loadUserByLoginID(targetID);
                        if (!targetUser) {
                            cout << "Target user not found!\n";
                            break;
                        }

                        currentCompanyClient->canTransfer(date, *targetUser, amount);


                        delete targetUser; // avoid memory leak
                        break;
                    }

                    case 4:
                        currentCompanyClient->checkBalance();
                        break;
                    case 5:
                        currentCompanyClient->viewTransactionHistory();
                        break;
                    case 6:
                        currentCompanyClient->changePassword();
                        break;
                    case 7:
                        currentCompanyClient->requestLoan();
                        break;
                    case 8:
                        currentCompanyClient->displayUserInfo();
                        break;
                    case 0:
                        cout << "Logging out...\n";
                        delete currentCompanyClient;
                        currentCompanyClient = nullptr;
                        break;
                    default:
                        cout << "Invalid choice!\n";
                    }
                } while (companyChoice != 0 && currentCompanyClient != nullptr);
            }
            else {
                cout << "Invalid login credentials!\n";
            }
            break;
        }
        case 2:
        {
            signupCompanyClient();
            break;
        }
        case 3:
        {
            forgotPassword(true);
            break;
        }
        case 4:
        {
            cout << "Returning to main menu...\n";
            break;
        }
        default:
        {
            cout << "Invalid choice!\n";
        }
        }
    } while (choice != 4);
}

void employeeMenu()
{
    BankingEmployee emp;
    string empID, password;

    cout << "\n=== BANK EMPLOYEE LOGIN ===\n";
    cout << "Employee ID: ";
    cin >> empID;
    cout << "Password: ";
    cin >> password;

    if (emp.login(empID, password)) {
        cout << "Login successful! Welcome Employee!\n";

        int empChoice;
        do {
            cout << "\n--- Employee Dashboard ---\n";
            cout << "1. View All Accounts\n";
            cout << "2. Approve/Reject Accounts\n";
            cout << "3. View Transactions\n";
            cout << "4. View Loan Requests\n";
            cout << "0. Logout\n";
            cout << "Enter choice: ";
            cin >> empChoice;

            switch (empChoice) {
            case 1:
                emp.viewAllAccounts();
                break;
            case 2:
                emp.approveOrRejectAccount();
                break;
            case 3:
                emp.viewTransactions();
                break;
            case 4:
                emp.viewLoanRequests();
                break;
            case 0:
                cout << "Logging out...\n";
                break;
            default:
                cout << "Invalid choice!\n";
            }
        } while (empChoice != 0);
    }
    else {
        cout << "Invalid employee credentials!\n";
    }
}
int UserClient::nextUserID = 1;
int CompanyClient::nextCompanyID = 1;

void mainMenu() {
    int choice;
    do {
        cout << "\n=================================\n";
        cout << "        MMA BANK LIMITED         \n";
        cout << "=================================\n";
        cout << "           MAIN MENU            \n";
        cout << "=================================\n";
        cout << "1. User Client\n";
        cout << "2. Company Client\n";
        cout << "3. Bank Employee\n";
        cout << "0. Exit\n";
        cout << "=================================\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
            userClientMenu();
            break;
        case 2:
            companyClientMenu();
            break;
        case 3:
            employeeMenu();
            break;
        case 0:
            cout << "Thank you for using MMA Bank Limited. Goodbye!\n";
            break;
        default:
            cout << "Invalid choice! Please try again.\n";
        }
    } while (choice != 0);
}
//newly added
int findHighestNumericID(const string& filename, char prefix) {
    ifstream fin(filename);
    string line;
    int maxID = 0;

    while (getline(fin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string id;
        getline(ss, id, ',');
        if (id.size() > 1 && id[0] == prefix) {
            int number = stoi(id.substr(1));
            maxID = max(maxID, number);
        }
    }
    return maxID;
}

int main()
{
    UserClient::nextUserID = findHighestNumericID("users.txt", 'U') + 1;
    CompanyClient::nextCompanyID = findHighestNumericID("companies.txt", 'C') + 1;


    srand(time(nullptr)); // Seed for random number generation

    // Create necessary files if they don't exist
    ofstream users("users.txt", ios::app);
    ofstream companies("companies.txt", ios::app);
    ofstream transactions("transactions.txt", ios::app);
    ofstream pending("pending_users.txt", ios::app);
    ofstream loans("loan_requests.txt", ios::app);
    ofstream cards("cards.txt", ios::app);

    users.close();
    companies.close();
    transactions.close();
    pending.close();
    loans.close();
    cards.close();

    cout << "=================================\n";
    cout << "    WELCOME TO MMA BANK LIMITED  \n";
    cout << "=================================\n";

    mainMenu();

    // Clean up
    if (currentUserClient) delete currentUserClient;
    if (currentCompanyClient) delete currentCompanyClient;
    if (currentEmployee) delete currentEmployee;

    return 0;
}

