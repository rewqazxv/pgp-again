#include "generate-key-pair-dialog.h"
#include "ui_generate-key-pair-dialog.h"

using namespace crypto;


GenerateKeyPairDialog::GenerateKeyPairDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::GenerateKeyPairDialog)
{
    ui->setupUi(this);
    ui->plainTextEdit_PublicKey->setWordWrapMode(QTextOption::WrapAnywhere);
    ui->plainTextEdit_PrivateKey->setWordWrapMode(QTextOption::WrapAnywhere);

    connect(ui->lineEdit_Comment, &QLineEdit::textChanged, [this] {
        public_key.reset();
        private_key.reset();
        check_status();
    });
    check_status();
}

GenerateKeyPairDialog::~GenerateKeyPairDialog()
{ delete ui; }

void GenerateKeyPairDialog::check_status()
{
    if (!public_key) {
        ui->pushButton_PublicKeyImport->setEnabled(false);
        ui->pushButton_PublicKeySave->setEnabled(false);
        ui->pushButton_PrivateKeyImport->setEnabled(false);
        ui->pushButton_PrivateKeySave->setEnabled(false);
        ui->plainTextEdit_PublicKey->clear();
        ui->plainTextEdit_PrivateKey->clear();
    } else {
        if (!private_key) throw std::runtime_error("internal error: public_key has value but private_key not");
        ui->pushButton_PublicKeyImport->setEnabled(true);
        ui->pushButton_PublicKeySave->setEnabled(true);
        ui->pushButton_PrivateKeyImport->setEnabled(true);
        ui->pushButton_PrivateKeySave->setEnabled(true);
        ui->plainTextEdit_PublicKey->setPlainText(public_key.value().toString());
        ui->plainTextEdit_PrivateKey->setPlainText(private_key.value().toString());
    }
}

void GenerateKeyPairDialog::on_pushButton_Generate_clicked()
{
    QString comment = ui->lineEdit_Comment->text().trimmed();
    if (comment.isEmpty()) {
        ui->lineEdit_Comment->setStyleSheet("background-color: pink");
        return;
    } else {
        ui->lineEdit_Comment->setStyleSheet("background-color:");
    }
    QString algo = ui->comboBox_Algorithm->currentText();

    if (algo == "RSA-2048") {
        auto key_pair = rsa::keygen(2048);
        public_key.emplace(key_pair.public_key, KeyInfo::PUBLIC_KEY, comment);
        private_key.emplace(key_pair.private_key, KeyInfo::PRIVATE_KEY, comment);
        check_status();
    } else if (algo == "RSA-4096") {
        auto key_pair = rsa::keygen(4096);
        public_key.emplace(key_pair.public_key, KeyInfo::PUBLIC_KEY, comment);
        private_key.emplace(key_pair.private_key, KeyInfo::PRIVATE_KEY, comment);
        check_status();
    } else {
        throw std::runtime_error("unsupported asymmetric encryption algorithm");
    }
}

void GenerateKeyPairDialog::on_pushButton_PublicKeyImport_clicked()
{
    assert(public_key.has_value());
    emit importPublicKey(public_key.value());
}

void GenerateKeyPairDialog::on_pushButton_PublicKeySave_clicked()
{
    assert(public_key.has_value());
    auto path = QFileDialog::getSaveFileName(
                    this,
                    QString(),
                    "public-key"
                );
    if (path.isNull()) return;

    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(public_key.value().toString().toUtf8());
        f.close();
    } else {
        QMessageBox::warning(this, tr("File Save Failed"),
                             tr("Can not save to file '%1'.").arg(path));
    }
}

void GenerateKeyPairDialog::on_pushButton_PrivateKeyImport_clicked()
{
    assert(private_key.has_value());
    emit importPrivateKey(private_key.value());
}

void GenerateKeyPairDialog::on_pushButton_PrivateKeySave_clicked()
{
    assert(private_key.has_value());
    auto path = QFileDialog::getSaveFileName(
                    this,
                    QString(),
                    "private-key"
                );
    if (path.isNull()) return;

    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(private_key.value().toString().toUtf8());
        f.close();
    } else {
        QMessageBox::warning(this, tr("File Save Failed"),
                             tr("Can not save to file '%1'.").arg(path));
    }
}
