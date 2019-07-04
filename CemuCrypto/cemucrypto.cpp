#include <utility>

#include <utility>

#include "cemucrypto.h"

CemuCrypto::CemuCrypto() = default;

CemuCrypto::CemuCrypto(QString titleKey, QString basedir)
{
    TitleKey = std::move(titleKey);
    Directory = std::move(basedir);
}

CemuCrypto *CemuCrypto::initialize(QString titleKey, QString basedir)
{
    return new CemuCrypto(std::move(titleKey), std::move(basedir));
}

void CemuCrypto::Start()
{
    qInfo() << "Decrypt:" << Directory;
    qInfo() << "Decrypt Exit Code:" << Decrypt();
}

quint16 CemuCrypto::bs16(quint16 s)
{
    return static_cast<quint16>( ((s)>>8) | ((s)<<8) );
}

quint32 CemuCrypto::bs24(quint32 i)
{
    return ((i & 0xFF0000) >> 16) | ((i & 0xFF) << 16) | (i & 0x00FF00);
}

quint32 CemuCrypto::bs32(quint32 s)
{
    return static_cast<quint32>( (((s)&0xFF0000)>>8) | (((s)&0xFF00)<<8) | ((s)>>24) | ((s)<<24) );
}

qulonglong CemuCrypto::bs64(qulonglong i)
{
    return static_cast<qulonglong>(((static_cast<qulonglong>(bs32(i & 0xFFFFFFFF))) << 32) | (bs32(i >> 32)));
}

char* CemuCrypto::ReadFile(const QString& file, quint32 *len)
{
    QFile in(file);
    if (!in.open(QIODevice::ReadOnly))
    {
        return nullptr;
    }

    *len = static_cast<quint32>(in.size());
    QByteArray bytearray(in.readAll());
    in.close();

    auto size = static_cast<size_t>(bytearray.size());
    char* data = new char[size];
    memcpy(data, bytearray.data(), size);
    return data;
}

#define BLOCK_SIZE  0x10000
void CemuCrypto::ExtractFileHash(QFile * in, qulonglong PartDataOffset, qulonglong FileOffset, qulonglong Size, const QString& FileName, quint16 ContentID, int i1, int i2)
{
    char decdata[BLOCK_SIZE];
    unsigned char IV[16];
    unsigned char hash[SHA_DIGEST_LENGTH];
    unsigned char H0[SHA_DIGEST_LENGTH];
    unsigned char Hashes[0x400];

    qulonglong Wrote = 0;
    qulonglong WriteSize = 0xFC00; //Hash block size
    qulonglong Block = (FileOffset / 0xFC00) & 0xF;

    auto* out = new QFile(FileName);
    if (!out->open(QIODevice::WriteOnly))
    {
        qCritical() << out->errorString();
        exit(0);
    }

    qulonglong roffset = FileOffset / 0xFC00 * BLOCK_SIZE;
    qulonglong soffset = FileOffset - (FileOffset / 0xFC00 * 0xFC00);

    if (soffset + Size > WriteSize)
        WriteSize = WriteSize - soffset;

    in->seek(static_cast<qlonglong>(PartDataOffset + roffset));
    while (Size > 0)
    {
        if (WriteSize > Size)
            WriteSize = Size;

        auto* temp_encdata = new char[BLOCK_SIZE]();
        auto len = static_cast<qulonglong>(in->read(temp_encdata, BLOCK_SIZE));

        auto* encdata = new quint8[len]();
        memcpy(encdata, temp_encdata, len);

        memset(static_cast<void*>(IV), 0, sizeof(IV));
        IV[1] = static_cast<unsigned char>(ContentID);

        AES_cbc_encrypt(encdata, static_cast<quint8*>(Hashes), 0x400, &_key, static_cast<unsigned char*>(IV), AES_DECRYPT);

        memcpy(static_cast<void*>(H0), Hashes + 0x14 * Block, SHA_DIGEST_LENGTH);
        memcpy(static_cast<void*>(IV), Hashes + 0x14 * Block, sizeof(IV));

        if (Block == 0)
            IV[1] ^= ContentID;

        AES_cbc_encrypt(encdata + 0x400, reinterpret_cast<quint8*>(decdata), 0xFC00, &_key, static_cast<unsigned char*>(IV), AES_DECRYPT);

        SHA1(reinterpret_cast<const quint8*>(decdata), 0xFC00, hash);

        if (Block == 0)
            hash[1] ^= ContentID;
        H0Count++;

        if (memcmp(hash, H0, SHA_DIGEST_LENGTH) != 0)
        {
            H0Fail++;
            qCritical() << "failed to verify H0 hash:" << out->fileName();
            return;
        }

        Size -= static_cast<qulonglong>(out->write(decdata + soffset, static_cast<qint64>(WriteSize)));

        Wrote += WriteSize;

        Block++;
        if (Block >= 16)
            Block = 0;

        if (soffset)
        {
            WriteSize = 0xFC00;
            soffset = 0;
        }
        emit Progress(i1, i2);
    }

    out->close();
    delete out;
}
#undef BLOCK_SIZE

#define BLOCK_SIZE  0x8000
void CemuCrypto::ExtractFile(QFile * in, qulonglong PartDataOffset, qulonglong FileOffset, qulonglong Size, const QString& FileName, quint16 ContentID, int i1, int i2)
{
    char decdata[BLOCK_SIZE];
    qulonglong Wrote = 0;

    qulonglong roffset = FileOffset / BLOCK_SIZE * BLOCK_SIZE;
    qulonglong soffset = FileOffset - (FileOffset / BLOCK_SIZE * BLOCK_SIZE);

    auto* out = new QFile(FileName);
    if (!out->open(QIODevice::WriteOnly))
    {
        qCritical() << out->errorString();
        exit(0);
    }

    quint8 IV[16];
    memset(static_cast<void*>(IV), 0, sizeof(IV));
    IV[1] = static_cast<quint8>(ContentID);
    qulonglong WriteSize = BLOCK_SIZE;

    if (soffset + Size > WriteSize)
        WriteSize = WriteSize - soffset;

    in->seek(static_cast<qlonglong>(PartDataOffset + roffset));

    while (Size > 0)
    {
        if (WriteSize > Size)
            WriteSize = Size;

        auto* temp_encdata = new char[BLOCK_SIZE]();
        auto len = static_cast<qulonglong>(in->read(temp_encdata, BLOCK_SIZE));

        auto* encdata = new quint8[len]();
        memcpy(encdata, temp_encdata, len);

        in->read(temp_encdata, BLOCK_SIZE);

        AES_cbc_encrypt(encdata, reinterpret_cast<quint8*>(decdata), BLOCK_SIZE, &_key, static_cast<unsigned char*>(IV), AES_DECRYPT);
        Size -= static_cast<qulonglong>(out->write(decdata + soffset, static_cast<qint64>(WriteSize)));
        Wrote += WriteSize;

        if (soffset) {
            WriteSize = BLOCK_SIZE;
            soffset = 0;
        }
        emit Progress(i1, i2);
    }

    out->close();
    delete out;
}

qint32 CemuCrypto::Decrypt()
{
    quint32 TMDLen;
    char* TMD = ReadFile(QDir(Directory).filePath("tmd"), &TMDLen);
    if (TMD == nullptr)
    {
        qCritical() << "failed to open tmd" << QDir(Directory).filePath("tmd");
        return EXIT_FAILURE;
    }

    if (TitleKey.isEmpty())
    {
        quint32 TIKLen;
        char* TIK = ReadFile(QDir(Directory).filePath("cetk"), &TIKLen);
        if (TIK == nullptr)
        {
            qCritical() << "failed to open cetk" << QDir(Directory).filePath("cetk");
            return EXIT_FAILURE;
        }
        memcpy(static_cast<void*>(enc_title_key), TIK + 0x1BF, 16);
        return Decrypt(TMD, TIK, Directory);
    }

    return Decrypt(TMD, nullptr, Directory);
}

qint32 CemuCrypto::Decrypt(char* TMD, const char* TIK, const QString& basedir)
{
    qInfo() << "Original CDecrypt v2.0b written by crediar";

    if (TMD == nullptr)
    {
        qCritical() << "failed to open tmd" << QDir(Directory).filePath("tmd");
        return EXIT_FAILURE;
    }

    if (TIK == nullptr)
    {
        memcpy(enc_title_key, QByteArray::fromHex(TitleKey.toLatin1()), 16);
    }

    auto tmd = reinterpret_cast<TitleMetaData*>(TMD);

    if (tmd->Version != 1)
    {
        qCritical() << QString("Unsupported TMD Version:%1").arg(tmd->Version);
        return EXIT_FAILURE;
    }

    qInfo() << QString("Title version:%1").arg(bs16(tmd->TitleVersion));
    qInfo() << QString("Content Count:%1").arg(bs16(tmd->ContentCount));

    if (strcmp(TMD + 0x140, "Root-CA00000003-CP0000000b") == 0)
    {
        AES_set_decrypt_key(reinterpret_cast<const quint8*>(WiiUCommenKey), sizeof(WiiUCommenKey) * 8, &_key);
    }
    else if (strcmp(TMD + 0x140, "Root-CA00000004-CP00000010") == 0)
    {
        AES_set_decrypt_key(reinterpret_cast<const quint8*>(WiiUCommenDevKey), sizeof(WiiUCommenDevKey) * 8, &_key);
    }
    else
    {
        printf("Unknown Root type:\"%s\"\n", TMD + 0x140);
        return EXIT_FAILURE;
    }

    memset(title_id, 0, sizeof(title_id));
    memcpy(title_id, TMD + 0x18C, 8);

    AES_cbc_encrypt(enc_title_key, dec_title_key, sizeof(dec_title_key), &_key, title_id, AES_DECRYPT);
    AES_set_decrypt_key(dec_title_key, sizeof(dec_title_key) * 8, &_key);

    char iv[16];
    memset(iv, 0, sizeof(iv));

    QString _str;
    _str = basedir + QString().sprintf("/%08x.app", bs32(tmd->Contents[0].ID));

    quint32 CNTLen;
    char* CNT = ReadFile(_str, &CNTLen);
    if (CNT == static_cast<char*>(nullptr))
    {
        _str = basedir + QString().sprintf("/%08x", bs32(tmd->Contents[0].ID));
        CNT = ReadFile(_str, &CNTLen);
        if (CNT == static_cast<char*>(nullptr))
        {
            qInfo() << QString("Failed to open content:%1").arg(bs32(tmd->Contents[0].ID));
            return EXIT_FAILURE;
        }
    }

    if (bs64(tmd->Contents[0].Size) != static_cast<qulonglong>(CNTLen))
    {
        qInfo() << QString("Size of content:%1 is wrong: %2:%3").arg(bs32(tmd->Contents[0].ID)).arg(CNTLen).arg(bs64(tmd->Contents[0].Size));
        return EXIT_FAILURE;
    }

    AES_cbc_encrypt(reinterpret_cast<const quint8*>(CNT), reinterpret_cast<quint8*>(CNT), CNTLen, &_key, reinterpret_cast<quint8*>(iv), AES_DECRYPT);

    if (bs32(*reinterpret_cast<quint32*>(CNT)) != 0x46535400)
    {
        qCritical() << "Failure decrypting";
        return EXIT_FAILURE;
    }

    FST* fst = reinterpret_cast<FST*>(CNT);
    qInfo() << QString("FSTInfo Entries:%1").arg(bs32(fst->EntryCount));
    if (bs32(fst->EntryCount) > 90000)
    {
        return EXIT_FAILURE;
    }

    auto* fe = reinterpret_cast<FEntry*>(CNT + 0x20 + bs32(fst->EntryCount) * 0x20);

    quint32 Entries = bs32(*reinterpret_cast<quint32*>(CNT + 0x20 + bs32(fst->EntryCount) * 0x20 + 8));
    quint32 NameOff = 0x20 + bs32(fst->EntryCount) * 0x20 + Entries * 0x10;

    qInfo() << QString("FST entries:%1").arg(Entries);

    char* Path = new char[1024];
    QDir dir(basedir);
    qint32 Entry[16];
    qint32 LEntry[16];

    qint32 level = 0;

    emit Started();
    for (quint32 i = 1; i < Entries; ++i)
    {
        if (level)
        {
            while (static_cast<quint32>(LEntry[level - 1]) == i)
            {
                //printf("[%03X]leaving :\"%s\" Level:%d\n", i, CNT + NameOff + bs24( fe[Entry[level-1]].NameOffset ), level );
                level--;
            }
        }

        if (fe[i].u1.s1.Type & 1)
        {
            Entry[level] = static_cast<qint32>(i);
            LEntry[level++] = static_cast<qint32>(bs32(fe[i].u2.s3.NextOffset));
            if (level > 15)
            { // something is wrong!
                qCritical() << QString("level error:%1").arg(level);
                break;
            }
        }
        else
        {
            memset(Path, 0, 1024);

            for (qint32 j = 0; j < level; ++j)
            {
                if (j)
                {
                    Path[strlen(Path)] = '\\';
                }
                memcpy(Path + strlen(Path), CNT + NameOff + bs24(fe[Entry[j]].u1.s1.NameOffset), strlen(CNT + NameOff + bs24(fe[Entry[j]].u1.s1.NameOffset)));
                QDir().mkdir(dir.filePath(Path));
            }
            if (level)
            {
                Path[strlen(Path)] = '\\';
            }
            memcpy(Path + strlen(Path), CNT + NameOff + bs24(fe[i].u1.s1.NameOffset), strlen(CNT + NameOff + bs24(fe[i].u1.s1.NameOffset)));

            quint32 CNTSize = bs32(fe[i].u2.s2.FileLength);
            qulonglong CNTOff = (static_cast<qulonglong>(bs32(fe[i].u2.s2.FileOffset)));

            if ((bs16(fe[i].Flags) & 4) == 0)
            {
                CNTOff <<= 5;
            }

            auto msg(QString().sprintf("Size:%1 Offset:0x%2 CID:%3 U:%4 %5", CNTSize, CNTOff, bs16(fe[i].ContentID), bs16(fe[i].Flags), Path));
            qInfo() << msg.arg(CNTSize).arg(CNTOff).arg(bs16(fe[i].ContentID)).arg(bs16(fe[i].Flags)).arg(Path);

            quint32 ContFileID = bs32(tmd->Contents[bs16(fe[i].ContentID)].ID);

            auto fei = fe[i];
            if (!(fei.u1.s1.Type & 0x80))
            {
                QString filepath = basedir + QString().sprintf("/%08x", ContFileID);
                QFile* in = new QFile(filepath);
                if (!in->open(QIODevice::ReadOnly))
                {
                    qWarning() << QString("Could not open:\"%1\"").arg(filepath);
                    continue;
                }
                QString output(dir.filePath(Path));
                QFile outputFile(output);
                auto sz = bs32(fei.u2.s2.FileLength);
                if ((bs16(fei.Flags) & 0x440))
                {
                    if (!outputFile.exists() || outputFile.size() != sz)
                    {
                        ExtractFileHash(in, 0, CNTOff, sz, output, bs16(fei.ContentID), static_cast<int>(i), static_cast<int>(Entries - 1));
                    }
                    else
                    {
                        emit Progress(static_cast<int>(i), static_cast<int>(Entries - 1));
                    }
                }
                else
                {
                    if (!outputFile.exists() || outputFile.size() != sz)
                    {
                        ExtractFile(in, 0, CNTOff, sz, output, bs16(fei.ContentID), static_cast<int>(i), static_cast<int>(Entries - 1));
                    }
                    else
                    {
                        emit Progress(static_cast<int>(i), static_cast<int>(Entries - 1));
                    }
                }
                in->close();
                delete in;
            }
        }
    }
    emit Finished();
    return EXIT_SUCCESS;
}
