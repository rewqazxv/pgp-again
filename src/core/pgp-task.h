#ifndef PGP_TASK_H
#define PGP_TASK_H

#include "model/key-info.h"
#include <crypto/crypto.h>


class PgpTask: public QObject
{
    Q_OBJECT

public:
    explicit PgpTask(QObject *parent): QObject(parent) {}
    void work();

    static inline const QHash<int, QString> ERROR_STR{
        {1, "Unsupported asymmetric encryption algorithm"},
        {2, "Key type error"},
        {3, "Unsupported symmetric encryption algorithm"},
        {4, "Unable to read input file"},
        {5, "Unable to write output file"},
        {6, "Header size error"},
        {7, "Header data error"},
        {8, "Header data error, or unknown symmetric encryption algorithm"},
        {9, "Session key size error"},
        {10, "IV block size error"},
        {11, "Decrypted data has incorrect ending"}
    };

protected slots:
    virtual void workImpl() = 0;

signals:
    void total(qint64 max);
    void progress(qint64 done); // done in [0, total_max]
    void finished(int status); // 0 for success
}; // class PgpTask


class EncryptTask: public PgpTask
{
    Q_OBJECT

public:
    EncryptTask(const QString &input_path, const QString &output_path, const KeyInfo &key, const QString &method, QObject *parent = nullptr):
        PgpTask(parent), input_path(input_path), output_path(output_path), key(key), method(method) {}

protected slots:
    void workImpl() final;

private:
    QString input_path, output_path;
    KeyInfo key;
    QString method;
}; // class EncryptTask


class DecryptTask: public PgpTask
{
    Q_OBJECT

public:
    DecryptTask(const QString &input_path, const QString &output_path, const KeyInfo &key, QObject *parent = nullptr):
        PgpTask(parent), input_path(input_path), output_path(output_path), key(key) {}

protected slots:
    void workImpl() final;

private:
    QString input_path, output_path;
    KeyInfo key;
}; // class DecryptTask

#endif // PGP_TASK_H
