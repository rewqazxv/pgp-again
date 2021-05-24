#ifndef GENERATE_KEY_PAIR_DIALOG_H
#define GENERATE_KEY_PAIR_DIALOG_H

#include "model/key-info.h"


QT_BEGIN_NAMESPACE
namespace Ui { class GenerateKeyPairDialog; }
QT_END_NAMESPACE

class GenerateKeyPairDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GenerateKeyPairDialog(QWidget *parent = nullptr);
    ~GenerateKeyPairDialog();

signals:
    void importPublicKey(const KeyInfo &key_info);
    void importPrivateKey(const KeyInfo &key_info);

private:
    Ui::GenerateKeyPairDialog *ui;
    std::optional<KeyInfo> public_key, private_key;
    void check_status();

private slots:
    void on_pushButton_PublicKeyImport_clicked();
    void on_pushButton_PublicKeySave_clicked();
    void on_pushButton_PrivateKeyImport_clicked();
    void on_pushButton_PrivateKeySave_clicked();
    void on_pushButton_Generate_clicked();
};

#endif // GENERATE_KEY_PAIR_DIALOG_H
