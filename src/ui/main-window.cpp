#include "main-window.h"
#include "ui_main-window.h"

#include "key-import-dialog.h"
#include "generate-key-pair-dialog.h"
#include "progress-dialog.h"

#include "core/pgp-task.h"
#include "config.h"

#include <crypto/crypto.h>


MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent), ui(new Ui::MainWindow),
    public_keys_model(QDir(config::savePath).filePath("public_keys"), KeyInfo::PUBLIC_KEY),
    private_keys_model(QDir(config::savePath).filePath("private_keys"), KeyInfo::PRIVATE_KEY)
{
    ui->setupUi(this);

    ui->listView_PublicKey->setModel(&public_keys_model);
    ui->listView_PublicKey->addAction(ui->actionDeleteSelectedPublicKey);
    ui->listView_PublicKey->addAction(ui->actionImportPublicKey);

    ui->listView_PrivateKey->setModel(&private_keys_model);
    ui->listView_PrivateKey->addAction(ui->actionDeleteSelectedPrivateKey);
    ui->listView_PrivateKey->addAction(ui->actionImportPrivateKey);

    connect(ui->listView_PublicKey->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::check_status);
    connect(ui->listView_PrivateKey->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::check_status);
    connect(ui->lineEdit_EncryptInput, &QLineEdit::textChanged, this, &MainWindow::check_status);
    connect(ui->lineEdit_EncryptOutput, &QLineEdit::textChanged, this, &MainWindow::check_status);
    connect(ui->comboBox_EncryptMethod, &QComboBox::currentTextChanged, this, &MainWindow::check_status);
    connect(ui->lineEdit_DecryptInput, &QLineEdit::textChanged, this, &MainWindow::check_status);
    connect(ui->lineEdit_DecryptOutput, &QLineEdit::textChanged, this, &MainWindow::check_status);

    check_status();
}

MainWindow::~MainWindow()
{ delete ui; }


void MainWindow::on_actionAbout_triggered()
{
    QString content = tr("<h1>PGP-Again</h1>"
                         "<p>An asymmetric encryption tool that reinvented PGP.</p>");
    QMessageBox::about(this, tr("About"), content);
}

void MainWindow::on_actionAboutQt_triggered()
{ QMessageBox::aboutQt(this); }

void MainWindow::on_actionGenerateKeyPair_triggered()
{
    GenerateKeyPairDialog dialog(this);
    connect(&dialog, &GenerateKeyPairDialog::importPublicKey, &public_keys_model, qOverload<const KeyInfo &>(&KeysModel::import));
    connect(&dialog, &GenerateKeyPairDialog::importPrivateKey, &private_keys_model, qOverload<const KeyInfo &>(&KeysModel::import));
    dialog.exec();
}

void MainWindow::on_actionImportPublicKey_triggered()
{
    KeyImportDialog dialog(KeyInfo::PUBLIC_KEY, this);
    connect(&dialog, &KeyImportDialog::import, &public_keys_model, qOverload<const QList<KeyInfo> &>(&KeysModel::import));
    dialog.exec();
}

void MainWindow::on_actionImportPrivateKey_triggered()
{
    KeyImportDialog dialog(KeyInfo::PRIVATE_KEY, this);
    connect(&dialog, &KeyImportDialog::import, &private_keys_model, qOverload<const QList<KeyInfo> &>(&KeysModel::import));
    dialog.exec();
}

void MainWindow::on_actionDeleteSelectedPublicKey_triggered()
{
    auto row = ui->listView_PublicKey->selectionModel()->selection().indexes().at(0);
    if (QMessageBox::question(this, tr("Remove confirm"),
                              tr("Are you sure to delete public key of '%1'?")
                              .arg(public_keys_model.getAt(row).comment)) == QMessageBox::Yes) {
        public_keys_model.removeAt(row);
        ui->listView_PublicKey->selectionModel()->clearSelection();
    }
}

void MainWindow::on_actionDeleteSelectedPrivateKey_triggered()
{
    auto row = ui->listView_PrivateKey->selectionModel()->selection().indexes().at(0);
    if (QMessageBox::question(this, tr("Remove confirm"),
                              tr("Are you sure to delete private key of '%1'?")
                              .arg(private_keys_model.getAt(row).comment)) == QMessageBox::Yes) {
        private_keys_model.removeAt(row);
        ui->listView_PrivateKey->selectionModel()->clearSelection();
    }
}

void MainWindow::check_status()
{
    ui->actionDeleteSelectedPublicKey->setEnabled(ui->listView_PublicKey->selectionModel()->hasSelection());
    ui->actionDeleteSelectedPrivateKey->setEnabled(ui->listView_PrivateKey->selectionModel()->hasSelection());

    bool can_encrypt = !ui->lineEdit_EncryptInput->text().isEmpty()
                       && !ui->lineEdit_EncryptOutput->text().isEmpty()
                       && !ui->comboBox_EncryptMethod->currentText().isEmpty()
                       && ui->listView_PublicKey->selectionModel()->hasSelection();
    ui->pushButton_Encrypt->setEnabled(can_encrypt);

    bool can_decrypt = !ui->lineEdit_DecryptInput->text().isEmpty()
                       && !ui->lineEdit_DecryptOutput->text().isEmpty()
                       && ui->listView_PrivateKey->selectionModel()->hasSelection();
    ui->pushButton_Decrypt->setEnabled(can_decrypt);
}

void MainWindow::on_toolButton_EncryptInput_clicked()
{ ui->lineEdit_EncryptInput->setText(QFileDialog::getOpenFileName(this)); }

void MainWindow::on_toolButton_EncryptOutput_clicked()
{ ui->lineEdit_EncryptOutput->setText(QFileDialog::getSaveFileName(this)); }

void MainWindow::on_toolButton_DecryptInput_clicked()
{ ui->lineEdit_DecryptInput->setText(QFileDialog::getOpenFileName(this)); }

void MainWindow::on_toolButton_DecryptOutput_clicked()
{ ui->lineEdit_DecryptOutput->setText(QFileDialog::getSaveFileName(this)); }

void MainWindow::on_pushButton_Encrypt_clicked()
{
    QString input_path = ui->lineEdit_EncryptInput->text();
    QString output_path = ui->lineEdit_EncryptOutput->text();
    QString method = ui->comboBox_EncryptMethod->currentText();
    const KeyInfo &key = public_keys_model.getAt(ui->listView_PublicKey->selectionModel()->selection().indexes().at(0));

    auto task = new EncryptTask(input_path, output_path, key, method);
    connect(task, &PgpTask::finished, task, &QObject::deleteLater);

    QString info = tr("<p><b>Input File:</b> %1</p>"
                      "<p><b>Output File:</b> %2</p>"
                      "<p><b>Encrypt Method:</b> %3 + %4</p>")
                   .arg(input_path, output_path, KeyInfo::ALGO_STR[key.algo], method);
    auto progress = new ProgressDialog(task, tr("PGP Encryption"), info, this);
    progress->setAttribute(Qt::WA_DeleteOnClose);

    progress->show();
    task->work();
}

void MainWindow::on_pushButton_Decrypt_clicked()
{
    QString input_path = ui->lineEdit_DecryptInput->text();
    QString output_path = ui->lineEdit_DecryptOutput->text();
    const KeyInfo &key = private_keys_model.getAt(ui->listView_PrivateKey->selectionModel()->selection().indexes().at(0));

    auto task = new DecryptTask(input_path, output_path, key);
    connect(task, &PgpTask::finished, task, &QObject::deleteLater);

    QString info = tr("<p><b>Input File:</b> %1</p>"
                      "<p><b>Output File:</b> %2</p>"
                      "<p><b>Private Key Algorithm:</b> %3</p>")
        .arg(input_path, output_path, KeyInfo::ALGO_STR[key.algo]);
    auto progress = new ProgressDialog(task, tr("PGP Decryption"), info, this);
    progress->setAttribute(Qt::WA_DeleteOnClose);

    progress->show();
    task->work();
}
