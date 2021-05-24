#ifndef KEYS_MODEL_H
#define KEYS_MODEL_H

#include "key-info.h"
#include <crypto/crypto.h>


class KeysModel: public QAbstractListModel
{
    Q_OBJECT

private:
    QList<KeyInfo> keys_list;
    QString saved_filepath;
    KeyInfo::KeyType key_type;

public slots:
    void import(const crypto::rsa::Key &key, const QString &comment)
    {
        keys_list.append(KeyInfo(key, key_type, comment));
        emit dataChanged(QModelIndex(), QModelIndex());
        saveToFile();
    }

    void import(const KeyInfo &key)
    {
        keys_list.append(key);
        emit dataChanged(QModelIndex(), QModelIndex());
        saveToFile();
    }

    void import(const QList<KeyInfo> &keys)
    {
        for (const auto &i : keys) {
            if (i.key_type != key_type)
                throw std::invalid_argument("failed to insert a different type key");
            keys_list.append(i);
        }
        emit dataChanged(QModelIndex(), QModelIndex());
        saveToFile();
    }

    void removeAt(int i)
    {
        keys_list.removeAt(i);
        emit dataChanged(QModelIndex(), QModelIndex());
        saveToFile();
    }

    void removeAt(const QModelIndex &i)
    { removeAt(i.row()); }

    void saveToFile() const
    {
        QFile f(saved_filepath);
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QTextStream out(&f);
            for (const auto &i : keys_list) {
                out << i.toString() << '\n';
            }
            f.close();
        } else {
            throw std::runtime_error(
                QString("can not save keys data to file '%1'").arg(saved_filepath).toStdString());
        }
    }

public:
    explicit KeysModel(const QString &filepath, KeyInfo::KeyType key_type):
        saved_filepath(filepath), key_type(key_type)
    {
        QFile f(saved_filepath);
        if (f.open(QIODevice::ReadOnly)) {
            QTextStream in(&f);
            for (;;) {
                QString line = in.readLine();
                if (line.isNull()) break;
                keys_list.append(KeyInfo(line, key_type));
            }
            f.close();
        }
        // else do nothing
    }

    const KeyInfo &getAt(int i)
    { return keys_list[i]; }

    const KeyInfo &getAt(const QModelIndex &i)
    { return getAt(i.row()); }

    int rowCount(const QModelIndex &parent) const override
    {
        // parent must be invalid (parent is root)
        if (parent.isValid())
            return 0;
        return keys_list.size();
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.parent().isValid() && index.column() == 0
            && index.row() >= 0 && index.row() < keys_list.size()
            && role == Qt::DisplayRole)
            return keys_list.at(index.row()).toViewString();
        else
            return QVariant();
    }
};

#endif // KEYS_MODEL_H
