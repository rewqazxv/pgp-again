#include "key-import-dialog.h"
#include "ui_key-import-dialog.h"


KeyImportDialog::KeyImportDialog(KeyInfo::KeyType key_type, QWidget *parent):
    QDialog(parent), ui(new Ui::KeyImportDialog), key_type(key_type)
{
    ui->setupUi(this);
    ui->plainTextEdit_KeyInput->setWordWrapMode(QTextOption::WrapAnywhere);

    if (key_type == KeyInfo::PUBLIC_KEY) {
        setWindowTitle(tr("Import Public Key"));
        ui->label_Tips->setText(tr("Paste your public key here:"));
    } else if (key_type == KeyInfo::PRIVATE_KEY) {
        setWindowTitle(tr("Import Private Key"));
        ui->label_Tips->setText(tr("Paste your private key here:"));
    } else throw std::runtime_error("unknown key type");

    connect(ui->plainTextEdit_KeyInput, &QPlainTextEdit::textChanged, [this] { ui->label_Info->clear(); });
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &KeyImportDialog::onReturn);
}

KeyImportDialog::~KeyImportDialog()
{ delete ui; }

void KeyImportDialog::onReturn()
{
    QStringList lines;
    for (const auto &i :
         ui->plainTextEdit_KeyInput->toPlainText().split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts)) {
        QString s = i.trimmed();
        if (!s.isEmpty()) lines.append(s);
    }
    try {
        QList<KeyInfo> res;
        for (const auto &i : lines)
            res.append(KeyInfo(i, key_type));
        if (res.empty()) {
            ui->label_Info->setText(tr("Import content is empty"));
        } else {
            if (res.size() > 1 && QMessageBox::question(this, tr("Confirm Import"),
                    tr("Import %1 keys?").arg(res.size())) != QMessageBox::Yes)
                return;
            emit import(res);
            this->accept();
        }
    } catch (const std::invalid_argument &e) {
        ui->label_Info->setText(tr("Key format error"));
    }
}
