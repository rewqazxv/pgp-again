#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "model/keys-model.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    KeysModel public_keys_model;
    KeysModel private_keys_model;

private slots:
    void on_actionAbout_triggered();
    void on_actionAboutQt_triggered();
    void on_actionGenerateKeyPair_triggered();
    void on_actionImportPublicKey_triggered();
    void on_actionImportPrivateKey_triggered();
    void on_actionDeleteSelectedPublicKey_triggered();
    void on_actionDeleteSelectedPrivateKey_triggered();

    void on_toolButton_EncryptInput_clicked();
    void on_toolButton_EncryptOutput_clicked();
    void on_toolButton_DecryptInput_clicked();
    void on_toolButton_DecryptOutput_clicked();

    void on_pushButton_Encrypt_clicked();
    void on_pushButton_Decrypt_clicked();

    void check_status();
};

#endif //MAIN_WINDOW_H
