#include "pgp-task.h"

using namespace crypto;


void PgpTask::work()
{
    auto thread = new QThread();
    moveToThread(thread);

    connect(this, &PgpTask::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    thread->start();
    QMetaObject::invokeMethod(this, &PgpTask::workImpl);
}

static constexpr int READ_BUFFER_SIZE = 128 * 1024; // 128k

void EncryptTask::workImpl()
{
    if (key.algo != KeyInfo::RSA) {
        emit finished(1);
        return;
    }
    if (key.key_type != KeyInfo::PUBLIC_KEY) {
        emit finished(2);
        return;
    }
    if (method != "AES_128_CBC") {
        emit finished(3);
        return;
    }

    QFile inFile(input_path);
    if (!inFile.open(QIODevice::ReadOnly)) {
        emit finished(4);
        return;
    }

    QFile outFile(output_path);
    if (!outFile.open(QIODevice::WriteOnly)) {
        emit finished(5);
        return;
    }

    emit total(inFile.size());
    emit progress(0);

    // The header part encrypted by rsa. Data uses big-endian encoding
    // raw:
    // ^ meta_string | '\0' | session_key $
    // encrypted:
    // ^ size: 4 bytes | header_encrypted $

    QByteArray header_raw;
    header_raw.append(method.toUtf8());
    header_raw.append(char(0));
    Bytes session_key = crypto::random::get_bytes(AES::BLOCK_SIZE);
    header_raw.append((char *)session_key.data(), session_key.size());

    BigInt header_raw_int = bigint::from_bytes((Byte *)header_raw.data(), header_raw.size(), ENDIAN::BIG);
    BigInt header_encrypted_int = rsa::rsa(header_raw_int, key.getRsaKeyData());
    Bytes header_encrypted = bigint::to_bytes(header_encrypted_int, ENDIAN::BIG);

    QByteArray header_encrypted_size(4, '\0');
    qToBigEndian(quint32(header_encrypted.size()), header_encrypted_size.data());
    outFile.write(header_encrypted_size);
    outFile.write((char *)header_encrypted.data(), header_encrypted.size());

    // The body part encrypted by aes
    // encrypted data:
    // ^ iv_block | file_encrypted $

    Bytes iv = crypto::random::get_bytes(AES::BLOCK_SIZE);
    outFile.write((char *)iv.data(), iv.size());

    auto aes_cbc = CbcMode<AES>::Encrypter(iv.data(), session_key);

    Bytes input_buffer(READ_BUFFER_SIZE);
    Bytes output_buffer(aes_cbc.output_buffer_size(READ_BUFFER_SIZE));

    qint64 read_count = 0;
    while (!inFile.atEnd()) {
        auto read_size = inFile.read((char *)input_buffer.data(), READ_BUFFER_SIZE);
        auto write_size = aes_cbc.use(input_buffer.data(), read_size, output_buffer.data(), output_buffer.size());
        outFile.write((char *)output_buffer.data(), write_size);
        emit progress(read_count += read_size);
    }
    auto latest_size = aes_cbc.finish(output_buffer.data(), output_buffer.size());
    outFile.write((char *)output_buffer.data(), latest_size);

    emit finished(0);
}


void DecryptTask::workImpl()
{
    if (key.algo != KeyInfo::RSA) {
        emit finished(1);
        return;
    }
    if (key.key_type != KeyInfo::PRIVATE_KEY) {
        emit finished(2);
        return;
    }

    QFile inFile(input_path);
    if (!inFile.open(QIODevice::ReadOnly)) {
        emit finished(4);
        return;
    }

    emit total(inFile.size());
    emit progress(0);

    // header, copied from above
    // encrypted:
    // ^ size: 4 bytes | header_encrypted $
    // decrypted:
    // ^ meta_string | '\0' | session_key $

    const rsa::Key &rsa_key = key.getRsaKeyData();

    qint64 read_size;
    Bytes input_buffer(READ_BUFFER_SIZE);

    read_size = inFile.read((char *)input_buffer.data(), 4);
    if (read_size != 4) {
        emit finished(6);
        return;
    }
    auto header_encrypted_size = qFromBigEndian<quint32>(input_buffer.data());
    if (header_encrypted_size > bigint::size(rsa_key.mod, 256)) {
        emit finished(6);
        return;
    }

    read_size = inFile.read((char *)input_buffer.data(), header_encrypted_size);
    if (read_size != header_encrypted_size) {
        emit finished(6);
        return;
    }
    BigInt header_encrypted_int = bigint::from_bytes(input_buffer.data(), read_size, ENDIAN::BIG);
    BigInt header_raw_int = rsa::rsa(header_encrypted_int, rsa_key);
    Bytes header_raw = bigint::to_bytes(header_raw_int, ENDIAN::BIG);

    auto zero_it = std::find(header_raw.begin(), header_raw.end(), Byte(0));
    if (zero_it == header_raw.end()) {
        emit finished(7);
        return;
    }
    size_t zero_pos = zero_it - header_raw.begin();

    std::string meta_string((char *)header_raw.data(), zero_pos);
    if (meta_string != "AES_128_CBC") {
        emit finished(8);
        return;
    }

    Bytes session_key(zero_it + 1, header_raw.end());
    if (session_key.size() != AES::BLOCK_SIZE) {
        emit finished(9);
        return;
    }

    qint64 read_count = 4 + header_encrypted_size;
    emit progress(read_count);

    // body, copied from above
    // encrypted data:
    // ^ iv_block | file_encrypted $

    read_size = inFile.read((char *)input_buffer.data(), AES::BLOCK_SIZE);
    if (read_size != AES::BLOCK_SIZE) {
        emit finished(10);
        return;
    }
    auto aes_cbc = CbcMode<AES>::Decrypter(input_buffer.data(), session_key);
    emit progress(read_count += read_size);

    QFile outFile(output_path);
    if (!outFile.open(QIODevice::WriteOnly)) {
        emit finished(5);
        return;
    }
    Bytes output_buffer(aes_cbc.output_buffer_size(READ_BUFFER_SIZE));

    while (!inFile.atEnd()) {
        read_size = inFile.read((char *)input_buffer.data(), READ_BUFFER_SIZE);
        auto write_size = aes_cbc.use(input_buffer.data(), read_size, output_buffer.data(), output_buffer.size());
        outFile.write((char *)output_buffer.data(), write_size);
        emit progress(read_count += read_size);
    }
    try {
        auto latest_size = aes_cbc.finish(output_buffer.data(), output_buffer.size());
        outFile.write((char *)output_buffer.data(), latest_size);
    } catch (const std::domain_error &e) {
        emit finished(11);
        return;
    }

    emit finished(0);
}
