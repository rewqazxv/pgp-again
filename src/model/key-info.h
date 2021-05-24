#ifndef KEY_INFO_H
#define KEY_INFO_H

#include <crypto/crypto.h>


class KeyInfo
{
public:
    enum Algorithm {
        RSA
    } algo;

    static inline const QHash<Algorithm, QString> ALGO_STR {
        {RSA, "RSA"}
    };

    enum KeyType {
        PUBLIC_KEY, PRIVATE_KEY
    } key_type;

    QString base64data;
    std::variant<std::monostate, crypto::rsa::Key> data;
    static_assert(sizeof(data) < 50); // make sure not to take up too much space

    QString comment;

    // construct from saved base64 string
    KeyInfo(const QString &s, KeyType key_type): key_type(key_type)
    {
        // split input string
        static const QRegularExpression pattern("([^\\s]+)\\s+([^\\s]+)\\s*(.*)");
        auto res = pattern.match(s);
        if (!res.hasMatch())
            throw std::invalid_argument("key string format error");

        // Algorithm
        if (res.captured(1) == "rsa")
            algo = Algorithm::RSA;
        else
            throw std::invalid_argument("unknown key algorithm");

        // data
        base64data = res.captured(2);
        switch (algo) {
        case Algorithm::RSA:
            data = parseRsaData(base64data);
            break;
        default:
            throw std::runtime_error("interval error: unsupported key algorithm");
        }

        // comment
        comment = res.captured(3);
    }

    // construct using the given rsa key
    KeyInfo(const crypto::rsa::Key &rsaKey, KeyType key_type, const QString &comment):
        algo(RSA), key_type(key_type), base64data(produceRsaData(rsaKey)), data(rsaKey), comment(comment) {}

    // convenience function for getting key data when the type is known
    const crypto::rsa::Key &getRsaKeyData() const
    { return std::get<crypto::rsa::Key>(data); }

    // string format used when saving
    QString toString() const
    {
        switch (algo) {
        case Algorithm::RSA:
            return QString("rsa %1 %2").arg(base64data, comment);
        default:
            throw std::runtime_error("interval error: unsupported key algorithm");
        }
    }

    QString toViewString() const
    {
        switch (algo) {
        case Algorithm::RSA: {
            auto mod_str = getRsaKeyData().mod.get_str(16);
            mod_str.resize(16);
            return QString("%1 (RSA: %2...)").arg(comment, QString::fromStdString(mod_str));
        }
        default:
            throw std::runtime_error("interval error: unsupported key algorithm");
        }
    }

private:
    // rsa key data format:
    // ^ len1: 4-byte integer | exp: $len1-byte long big integer | len2: 4-byte integer | mod: $len2-byte long big integer $
    // all integers are big-endian
    static crypto::rsa::Key parseRsaData(const QString &s)
    {
        auto rawData = QByteArray::fromBase64(s.toUtf8());
        auto p = rawData.data();
        auto data_end = p + rawData.size();

#define runtime_check(condition) \
    if (!(condition)) \
        throw std::invalid_argument(QString("key data error: runtime check '" #condition "' fail.\n" \
        "raw string: '%1'").arg(s).toStdString())

        runtime_check(p + 4 <= data_end);
        auto len1 = qFromBigEndian<quint32>(p);
        p += 4;

        runtime_check(p + len1 <= data_end);
        crypto::BigInt exp = crypto::bigint::from_bytes((crypto::Byte *)p, len1, crypto::ENDIAN::BIG);
        p += len1;

        runtime_check(p + 4 <= data_end);
        auto len2 = qFromBigEndian<quint32>(p);
        p += 4;

        runtime_check(p + len2 == data_end);
        crypto::BigInt mod = crypto::bigint::from_bytes((crypto::Byte *)p, len2, crypto::ENDIAN::BIG);

#undef runtime_check

        return {exp, mod};
    }

    // same format but mod part has a leading zero byte
    static QByteArray produceRsaData(const crypto::rsa::Key &rsa_key)
    {
        auto exp_data = crypto::bigint::to_bytes(rsa_key.exp, crypto::ENDIAN::BIG);
        auto mod_data = crypto::bigint::to_bytes(rsa_key.mod, crypto::ENDIAN::BIG);

        QByteArray len1(4, 0);
        qToBigEndian(quint32(exp_data.size()), len1.data());

        QByteArray len2(4, 0);
        qToBigEndian(quint32(mod_data.size() + 1), len2.data());

        QByteArray res;
        res.append(len1).append((char *)exp_data.data(), exp_data.size());
        res.append(len2).append('\0').append((char *)mod_data.data(), mod_data.size());

        return res.toBase64();
    }
};

#endif // KEY_INFO_H
