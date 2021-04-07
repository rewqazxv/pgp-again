#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "pch.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    std::pair<QString, QString> keygen(QString prefixpath);
    void encrypt(QString orig_path, QString pubkey_path, QString save_path);
    void decrypt(QString orig_path, QString prikey_path, QString save_path);

    void write2num(QString path, const mpz_class &a, const mpz_class &b);
    std::pair<mpz_class, mpz_class> read2num(QString path);
};

#endif // MAINWINDOW_H
