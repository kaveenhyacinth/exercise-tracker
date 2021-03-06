#include "dataadapter.h"
#include "dbgateway.h"
#include "util.h"

#include <QMessageBox>
#include <QDate>

DataAdapter::DataAdapter()
{

}

void DataAdapter::ReadAccounts(QTableView *tbl)
{
    DbGateway db;
    QSqlQuery *qry;
    QSqlQueryModel *model;

    if(!db.Connect()) {
        qDebug() << "Failed to open the database connection @ ReadAccounts";
        return;
    }

    model = new QSqlQueryModel();
    qry = new QSqlQuery(db.getDb());

    qry->prepare("SELECT account_name as 'Account', balance as 'Balance' FROM account");

    if(!qry->exec())
    {
        qDebug() << "Something went wrong while fetching account details";
        return;
    }

    model->setQuery(*qry);
    tbl->setModel(model);
    db.Disconnect();
}

void DataAdapter::ReadTransactions(QTableView *tbl)
{
    DbGateway db;
    QSqlQuery *qry;
    QSqlQueryModel *model;

    if(!db.Connect()) {
        qDebug() << "Failed to open the database connection @ ReadTransactions";
        return;
    }

    model = new QSqlQueryModel();
    qry = new QSqlQuery(db.getDb());

    qry->prepare("SELECT a.account_name as 'Account', "
                 "t.amount as 'Amount', "
                 "t.record_type as 'Type', "
                 "t.date as 'Date', "
                 "c.type as 'Category' "
                 "FROM ((record as t "
                 "INNER JOIN account as a ON t.account_id = a._id) "
                 "INNER JOIN category as c ON t.category = c._id) "
                 "ORDER BY t.date desc");

    if(!qry->exec())
    {
        qDebug() << "Something went wrong while fetching transaction details";
        return;
    }

    model->setQuery(*qry);
    tbl->setModel(model);
    db.Disconnect();
}

void DataAdapter::ReadAccounts(QComboBox *cmb)
{
    DbGateway db;
    QSqlQuery *qry;
    QSqlQueryModel *model;

    if(!db.Connect()) {
        qDebug() << "Failed to open the database connection @ ReadAccounts";
        return;
    }

    model = new QSqlQueryModel();
    qry = new QSqlQuery(db.getDb());

    qry->prepare("SELECT account_name FROM account ORDER BY _id");

    if(!qry->exec())
    {
        qDebug() << "Something went wrong while fetching account details";
        return;
    }

    model->setQuery(*qry);
    cmb->setModel(model);
    db.Disconnect();
}

void DataAdapter::ReadCategories(QComboBox *cmb, QString ref)
{
    DbGateway db;
    QSqlQuery *qry;
    QSqlQueryModel *model;

    if(!db.Connect()) {
        qDebug() << "Failed to open the database connection @ ReadCategories";
        return;
    }

    model = new QSqlQueryModel();
    qry = new QSqlQuery(db.getDb());

    qry->prepare("SELECT type FROM category WHERE ref = ? ORDER BY _id");
    qry->bindValue(0, ref);

    if(!qry->exec())
    {
        qDebug() << "Something went wrong while fetching category details";
        return;
    }

    model->setQuery(*qry);
    cmb->setModel(model);
    db.Disconnect();
}

bool DataAdapter::UpdateAccountIncome(int accountId, int incomeBalance)
{
    Util util;
    DbGateway db;
    QSqlQuery qry;
    QSqlQuery tempqry;
    QString tempBalanceStr;
    QString balanceStr;
    int tempBalance;
    int balance;

    if(!db.Connect()) {
        qDebug() << "Failed to open the database connection @ UpdateAccountIncome";
        return false;
    }

    tempqry.prepare("SELECT balance FROM account WHERE _id = ?");
    tempqry.bindValue(0, accountId);

    if(!tempqry.exec())
    {
        qDebug() << "Something went wrong while parsing account balance";
        return false;
    }

    while (tempqry.next())
    {
        tempBalanceStr = tempqry.value(0).toString();
    }

    tempBalance = util.FormatMoney(tempBalanceStr);
    balance = tempBalance + incomeBalance;
    balanceStr = util.FormatBalance(balance);

    qDebug() << "New account balance" << balanceStr;

    qry.prepare("UPDATE account SET balance = ? WHERE _id = ?");
    qry.bindValue(0, balanceStr);
    qry.bindValue(1, accountId);

    if(!qry.exec())
    {
        qDebug() << "Something went wrong while updating account balance";
        return false;
    }

    db.Disconnect();
    qDebug() << "Account balance updated";
    return true;
}

bool DataAdapter::UpdateAccountExpense(int accountId, int expenseBalance)
{
    Util util;
    DbGateway db;
    QSqlQuery qry;
    QSqlQuery tempqry;
    QString tempBalanceStr;
    QString balanceStr;
    int tempBalance;
    int balance;

    if(!db.Connect()) {
        qDebug() << "Failed to open the database connection @ UpdateAccountIncome";
        return false;
    }

    tempqry.prepare("SELECT balance FROM account WHERE _id = ?");
    tempqry.bindValue(0, accountId);

    if(!tempqry.exec())
    {
        qDebug() << "Something went wrong while parsing account balance";
        return false;
    }

    while (tempqry.next())
    {
        tempBalanceStr = tempqry.value(0).toString();
    }

    tempBalance = util.FormatMoney(tempBalanceStr);

    if(tempBalance < expenseBalance)
    {
        qDebug() << "Account has not enough funds";
        QMessageBox msgBox;
        msgBox.setWindowTitle("Transaction Denied!");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Selected account has not enough funds, Please select another account or add more funds");
        msgBox.exec();
        return false;
    }


    balance = tempBalance - expenseBalance;
    balanceStr = util.FormatBalance(balance);

    qDebug() << "New account balance" << balanceStr;

    qry.prepare("UPDATE account SET balance = ? WHERE _id = ?");
    qry.bindValue(0, balanceStr);
    qry.bindValue(1, accountId);

    if(!qry.exec())
    {
        qDebug() << "Something went wrong while updating account balance";
        return false;
    }

    db.Disconnect();
    qDebug() << "Account balance updated";
    return true;
}

int DataAdapter::FetchTotalByType(QString recordType)
{
    Util util;
    DbGateway db;
    QSqlQuery qry;
    int balance = 0;

    if(!db.Connect()) {
        qDebug() << "Failed to open the database connection @ FetchTotalIncome";
        return -1;
    }

    qry.prepare("SELECT amount FROM record WHERE record_type = ?");
    qry.bindValue(0, recordType);

    if(!qry.exec())
    {
        qDebug() << "Something went wrong while fetching total " << recordType << " details";
        return -1;
    }

    while (qry.next())
    {
         balance += util.FormatMoney(qry.value(0).toString());
    }

    db.Disconnect();
    return balance;
}

int DataAdapter::FetchTotalOfTypeByDate(QString recordType, QDate startDate, QDate endDate)
{
    Util util;
    DbGateway db;
    QSqlQuery qry;
    int balance = 0;

    if(!db.Connect()) {
        qDebug() << "Failed to open the database connection @ FetchTotalIncome";
        return -1;
    }

    if(startDate > endDate) return -1;

    qry.prepare("SELECT amount FROM record WHERE record_type = ? AND date >= ? AND date <= ?");
    qry.bindValue(0, recordType);
    qry.bindValue(1, startDate);
    qry.bindValue(2, endDate);

    if(!qry.exec())
    {
        qDebug() << "Something went wrong while fetching total " << recordType << " details";
        return -1;
    }

    while (qry.next())
    {
         balance += util.FormatMoney(qry.value(0).toString());
    }

    db.Disconnect();
    return balance;
}

QString DataAdapter::FetchTotalStringByType(QString recordType)
{
    Util util;
    DbGateway db;
    QSqlQuery qry;
    QString totalBalance;
    int balance = 0;

    QDate date = QDate::currentDate().addDays(-30);
    qDebug() << "Date debugger at fetchTotalByType" << date;

    if(!db.Connect()) {
        qDebug() << "Failed to open the database connection @ FetchTotalIncome";
        return "";
    }

    qry.prepare("SELECT amount FROM record WHERE record_type = ?");
    qry.bindValue(0, recordType);

    if(!qry.exec())
    {
        qDebug() << "Something went wrong while fetching total " << recordType << " details";
        return "";
    }

    while (qry.next())
    {
         balance += util.FormatMoney(qry.value(0).toString());
    }

    totalBalance = util.FormatBalance(balance);
    db.Disconnect();
    return totalBalance;
}

QString DataAdapter::FetchTotalStringOfTypeByDate(QString recordType, QDate startDate, QDate endDate)
{
    Util util;
    DbGateway db;
    QSqlQuery qry;
    QString totalBalance;
    int balance = 0;

    QDate date = QDate::currentDate().addDays(-30);
    qDebug() << "Date debugger at fetchTotalByType" << date;

    if(!db.Connect()) {
        qDebug() << "Failed to open the database connection @ FetchTotalIncome";
        return "";
    }

    if(startDate > endDate) return "";

    qry.prepare("SELECT amount FROM record WHERE record_type = ? AND date >= ? AND date <= ?");
    qry.bindValue(0, recordType);
    qry.bindValue(1, startDate);
    qry.bindValue(2, endDate);

    if(!qry.exec())
    {
        qDebug() << "Something went wrong while fetching total " << recordType << " details";
        return "";
    }

    while (qry.next())
    {
         balance += util.FormatMoney(qry.value(0).toString());
    }

    totalBalance = util.FormatBalance(balance);
    db.Disconnect();
    return totalBalance;
}

QString DataAdapter::FetchTotalBalance()
{
    Util util;
    DbGateway db;
    QSqlQuery qry;
    QString totalBalance;
    int tempBalance = 0;

    if(!db.Connect()) {
        qDebug() << "Failed to open the database connection @ FetchTotalIncome";
        return "";
    }

    qry.prepare("SELECT balance FROM account");

    if(!qry.exec())
    {
        qDebug() << "Something went wrong while fetching total details";
        return "";
    }

    while (qry.next())
    {
         tempBalance += util.FormatMoney(qry.value(0).toString());
    }

    totalBalance = util.FormatBalance(tempBalance);
    db.Disconnect();
    return totalBalance;
}

void DataAdapter::LoadAccountData(QTableView *tbl, QComboBox *cmb)
{
    this->ReadAccounts(tbl);
    this->ReadAccounts(cmb);
}

void DataAdapter::LoadTransactionData
(QTableView *tbl, QComboBox *cmbInAcc, QComboBox *cmbInCat, QComboBox *cmbExAcc, QComboBox *cmbExCat)
{
    this->ReadTransactions(tbl);
    this->ReadAccounts(cmbInAcc);
    this->ReadAccounts(cmbExAcc);
    this->ReadCategories(cmbInCat, "I");
    this->ReadCategories(cmbExCat, "E");
}
