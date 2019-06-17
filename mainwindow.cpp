#include "mainwindow.h"
#include "ui_mainwindow.h"

extern "C" {
#include "encrypt/aes.h"
}
#include "encrypt/rsa.hpp"

#include "tools.h"

class file_io_error: public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    file_io_error(QString s): file_io_error(s.toStdString()) {}
};

class data_error: public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    data_error(QString s): data_error(s.toStdString()) {}
};

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    QList<QToolButton *> toolButtons{ui->toolButton_file, ui->toolButton_pubkey, ui->toolButton_prikey};
    QList<QLineEdit *> lineEdits{ui->lineEdit_file, ui->lineEdit_pubkey, ui->lineEdit_prikey};
    assert(toolButtons.size() == lineEdits.size());
    for (int i = 0; i < toolButtons.size(); i++) {
        auto curToolButton = toolButtons[i];
        auto curLineEdit = lineEdits[i];
        connect(curToolButton, &QToolButton::clicked, [ = ] {
            QString path = QFileDialog::getOpenFileName(this);
            if (!path.isEmpty())
                curLineEdit->setText(path);
        });
        connect(curLineEdit, &QLineEdit::textChanged, [ = ] {
            curLineEdit->setStyleSheet("background-color:");
        });
    }

    connect(ui->pushButton_keygen, &QPushButton::clicked, [&] {
        QString prefixpath = QFileDialog::getSaveFileName(this);
        if (!prefixpath.isEmpty()) {
            auto keys_path = keygen(prefixpath);
            ui->lineEdit_pubkey->setText(keys_path.first);
            ui->lineEdit_prikey->setText(keys_path.second);
        }
    });

    connect(ui->pushButton_encrypt, &QPushButton::clicked, [&] {
        QString orig_path = ui->lineEdit_file->text();
        if (orig_path.isEmpty()) {
            ui->lineEdit_file->setStyleSheet("background-color: pink");
            return;
        }
        QString pubkey_path = ui->lineEdit_pubkey->text();
        if (pubkey_path.isEmpty()) {
            ui->lineEdit_pubkey->setStyleSheet("background-color: pink");
            return;
        }
        QString save_path = QFileDialog::getSaveFileName(this);
        if (!save_path.isEmpty())
            encrypt(orig_path, pubkey_path, save_path);
    });

    connect(ui->pushButton_decrypt, &QPushButton::clicked, [&] {
        QString orig_path = ui->lineEdit_file->text();
        if (orig_path.isEmpty()) {
            ui->lineEdit_file->setStyleSheet("background-color: pink");
            return;
        }
        QString prikey_path = ui->lineEdit_prikey->text();
        if (prikey_path.isEmpty()) {
            ui->lineEdit_prikey->setStyleSheet("background-color: pink");
            return;
        }
        QString save_path = QFileDialog::getSaveFileName(this);
        if (!save_path.isEmpty())
            decrypt(orig_path, prikey_path, save_path);
    });
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::write2num(QString path, const mpz_class &a, const mpz_class &b) {
    QFile f{path};
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        throw file_io_error(tr("Cannot write file '%1'").arg(path));
    QTextStream out{&f};
    out << QString(a.get_str(16).data()) << '\n';
    out << QString(b.get_str(16).data()) << '\n';
}

std::pair<mpz_class, mpz_class> MainWindow::read2num(QString path) {
    QFile f{path};
    if (!f.open(QIODevice::ReadOnly))
        throw file_io_error(tr("Cannot read file '%1'").arg(path));
    QString s1, s2;
    QTextStream in(&f);
    in >> s1 >> s2;
    return std::make_pair(mpz_class("0x" + s1.toStdString()), mpz_class("0x" + s2.toStdString()));
}

// return public key file path and private key file path
std::pair<QString, QString> MainWindow::keygen(QString prefixpath) {
    try {
        auto keys = rsa_keygen(tools::getgmprand());
        QString pubkey_path = prefixpath + ".pub";
        QString prikey_path = prefixpath + ".key";
        write2num(pubkey_path, keys.n, keys.e);
        write2num(prikey_path, keys.n, keys.d);
        return std::make_pair(pubkey_path, prikey_path);
    } catch (file_io_error &e) {
        QMessageBox::critical(this, tr("File IO Error"), tr("Error info: %1").arg(e.what()));
        return std::make_pair(QString(), QString());
    }
}

void MainWindow::encrypt(QString orig_path, QString pubkey_path, QString save_path) {
    try {
        // open files
        QFile savefile{save_path};
        if (!savefile.open(QIODevice::WriteOnly | QIODevice::Truncate))
            throw file_io_error(tr("Cannot write save file"));

        QFile origfile{orig_path};
        if (!origfile.open(QIODevice::ReadOnly))
            throw file_io_error(tr("Cannot read original file"));
        QByteArray orig = origfile.readAll();

        QByteArray buf;

        // generate aes key
        QByteArray aeskey;
        aeskey.resize(16);
        for (int i = 0; i < aeskey.size(); i++)
            aeskey[i] = tools::randbyte();

        // encrypt aes key
        auto aeskeynum = tools::readint<mpz_class>((Byte *)aeskey.data(), aeskey.size());
        auto [n, e] = read2num(pubkey_path);
        if (n == 0 || e == 0)
            throw std::invalid_argument(tr("Zero Number").toStdString());
        auto encryptedkey = rsa(aeskeynum, e, n);
        buf.resize(1100); // buffer length > rsa length (byte)
        size_t ekeylen = tools::writempz((Byte *)buf.data() + 2, buf.size() - 2, encryptedkey);
        if (ekeylen > UINT16_MAX)
            throw data_error(tr("Encrypted symmetric key is too long"));
        tools::writeint((Byte *)buf.data(), (uint16_t)ekeylen);
        savefile.write(buf.data(), ekeylen + 2);

        // encrypt data
        buf.resize(aes128cbc_chiperlen(orig.size()));
        aes128cbc_encrypt((Byte *)buf.data(), (Byte *)orig.data(), orig.size(), (Byte *)aeskey.data(), tools::randbyte);
        savefile.write(buf);
    } catch (file_io_error &e) {
        QMessageBox::critical(this, tr("File IO Error"), tr("Error info: %1").arg(e.what()));
    } catch (data_error &e) {
        QMessageBox::critical(this, tr("Data Error"), tr("Error info: %1").arg(e.what()));
    } catch (std::invalid_argument &e) {
        QMessageBox::critical(this, tr("Invalid Argument"), tr("Error info: %1\nMaybe some problems with your public key file.").arg(e.what()));
    }
    // QFile object will closes resource automaticly
}

void MainWindow::decrypt(QString orig_path, QString prikey_path, QString save_path) {
    try {
        // open files
        QFile savefile{save_path};
        if (!savefile.open(QIODevice::WriteOnly | QIODevice::Truncate))
            throw file_io_error(tr("Cannot write save file"));

        QFile origfile{orig_path};
        if (!origfile.open(QIODevice::ReadOnly))
            throw file_io_error(tr("Cannot read original file"));
        QByteArray orig = origfile.readAll();

        QByteArray buf;

        // decrypt aes key
        auto [n, d] = read2num(prikey_path);
        if (n == 0 || d == 0)
            throw std::invalid_argument(tr("Zero Number").toStdString());
        uint16_t ekeylen = tools::readint<uint16_t>((Byte *)orig.data(), 2);
        if (ekeylen + 2 > orig.size())
            throw data_error(tr("Symmetric key length error"));
        auto encryptedkey = tools::readint<mpz_class>((Byte *)orig.data() + 2, ekeylen);
        auto aeskeynum = rsa(encryptedkey, d, n);
        QByteArray aeskey;
        aeskey.resize(56);
        size_t aeskeylen = tools::writempz((Byte *)aeskey.data(), aeskey.size(), aeskeynum);
        if (aeskeylen != 16)
            throw data_error(tr("Symmetric key length error"));

        // decrypt data
        size_t chiperlen = orig.size() - 2 - ekeylen;
        if (chiperlen < 32 || chiperlen % 16)
            throw data_error(tr("Data length error"));
        buf.resize(chiperlen);
        size_t datalen = aes128cbc_decrypt((Byte *)buf.data(), (Byte *)orig.data() + 2 + ekeylen, chiperlen, (Byte *)aeskey.data());
        if (datalen == SIZE_MAX)
            throw data_error(tr("Decrypt error"));
        savefile.write(buf.data(), datalen);
    } catch (file_io_error &e) {
        QMessageBox::critical(this, tr("File IO Error"), tr("Error info: %1").arg(e.what()));
    } catch (data_error &e) {
        QMessageBox::critical(this, tr("Data Error"), tr("Error info: %1").arg(e.what()));
    } catch (std::invalid_argument &e) {
        QMessageBox::critical(this, tr("Invalid Argument"), tr("Error info: %1\nMaybe some problems with your private key file.").arg(e.what()));
    }
}
