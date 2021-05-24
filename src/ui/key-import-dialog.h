#ifndef KEY_IMPORT_DIALOG_H
#define KEY_IMPORT_DIALOG_H

#include "model/key-info.h"


QT_BEGIN_NAMESPACE
namespace Ui { class KeyImportDialog; }
QT_END_NAMESPACE

class KeyImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KeyImportDialog(KeyInfo::KeyType key_type, QWidget *parent = nullptr);
    ~KeyImportDialog() override;

signals:
    void import(const QList<KeyInfo> &keys);

private:
    Ui::KeyImportDialog *ui;
    KeyInfo::KeyType key_type;
    void onReturn();
};

#endif // KEY_IMPORT_DIALOG_H
