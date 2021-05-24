#ifndef PROGRESS_DIALOG_H
#define PROGRESS_DIALOG_H

#include "core/pgp-task.h"


QT_BEGIN_NAMESPACE
namespace Ui { class ProgressDialog; }
QT_END_NAMESPACE

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(PgpTask *task, const QString &title,  const QString &info, QWidget *parent = nullptr);
    ~ProgressDialog() override;

public slots:
    void setTotal(qint64 total);
    void setProgress(qint64 done);
    void onFinish(int ret);
    void updateTime();

private:
    Ui::ProgressDialog *ui;
    qint64 total = 100;
    qint64 done = 0;
    enum Status {
        PREPARING, PROCESSING, FINISHED
    } status = PREPARING;
    QElapsedTimer stopwatch;
    QTimer timer;

protected slots:
    void closeEvent(QCloseEvent *e) override;
};

#endif // PROGRESS_DIALOG_H
