#include "progress-dialog.h"
#include "ui_progress-dialog.h"


ProgressDialog::ProgressDialog(PgpTask *task, const QString &title, const QString &info, QWidget *parent):
    QDialog(parent), ui(new Ui::ProgressDialog), timer(this)
{
    ui->setupUi(this);
    setWindowTitle(title);
    ui->textBrowser->setText(info);
    ui->textBrowser->setWordWrapMode(QTextOption::WrapAnywhere);

    connect(task, &PgpTask::total, this, &ProgressDialog::setTotal);
    connect(task, &PgpTask::progress, this, &ProgressDialog::setProgress);
    connect(task, &PgpTask::finished, this, &ProgressDialog::onFinish);

    connect(&timer, &QTimer::timeout, this, &ProgressDialog::updateTime);
    stopwatch.start();
    timer.start(200);
    updateTime();
}

ProgressDialog::~ProgressDialog()
{ delete ui; }

void ProgressDialog::setTotal(qint64 _total)
{ this->total = _total; }

void ProgressDialog::updateTime()
{
    qint64 msec = stopwatch.elapsed();
    qint64 sec = msec / 1000; msec %= 1000;
    qint64 min = sec / 60; sec %= 60;
    qint64 hour = min / 60; min %= 60;

    auto time_str = QString("%1:%2")
                    .arg(min, 2, 10, QChar('0'))
                    .arg(sec, 2, 10, QChar('0'));
    if (hour)
        time_str = QString("%1:").arg(hour) + time_str;
    ui->label_Timer->setText(time_str);
}

void ProgressDialog::setProgress(qint64 _done)
{
    status = PROCESSING;
    done = _done;
    ui->progressBar->setValue((long double)(done) / total * 100);
    ui->label_Status->setText(tr("Processing %1/%2").arg(done).arg(total));
}

void ProgressDialog::onFinish(int ret)
{
    timer.stop();
    status = FINISHED;
    updateTime();

    if (ret == 0) {
        ui->label_Status->setText(tr("Complete %1/%2").arg(done).arg(total));
    } else {
        QHash<int, QString> ERROR_STR{
            {1, tr("Unsupported asymmetric encryption algorithm")},
            {2, tr("Key type error")},
            {3, tr("Unsupported symmetric encryption algorithm")},
            {4, tr("Unable to read input file")},
            {5, tr("Unable to write output file")},
            {6, tr("Header size error")},
            {7, tr("Header data error")},
            {8, tr("Header data error, or unknown symmetric encryption algorithm")},
            {9, tr("Session key size error")},
            {10, tr("IV block size error")},
            {11, tr("Decrypted data has incorrect ending")}
        };
        ui->label_Status->setText(tr("Error %1: %2").arg(ret).arg(ERROR_STR[ret]));
    }
}

void ProgressDialog::closeEvent(QCloseEvent *e)
{
    if (status != FINISHED)
        e->ignore();
}
