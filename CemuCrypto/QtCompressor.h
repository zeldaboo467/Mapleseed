#ifndef QTCOMPRESSOR_H
#define QTCOMPRESSOR_H

#include "cemucrypto_global.h"
class CEMUCRYPTOSHARED_EXPORT QtCompressor : public QObject
{
    Q_OBJECT
public:
    explicit QtCompressor(QObject *parent = nullptr);

    //A recursive function that scans all files inside the source folder
    //and serializes all files in a row of file names and compressed
    //binary data in a single file
    bool compress(const QString& sourceFolder, const QString& destinationFile);

    //handles compression of indiviual directories, iterating through levels
    bool handleCompress(const QString& sourceFolder, const QString& prefex);

    //A function that deserializes data from the compressed file and
    //creates any needed subfolders before saving the file
    bool decompress(const QString& sourceFile, const QString& destinationFolder);

	//counts the number of files in a directory
    int count(const QString& directory);

private:
    QFile file;
    QDataStream dataStream;
    int curFile{};
    int numFiles{};
    int countDown{};
	
signals:
    void Progress(qint64 min, qint64 max);
};

#endif // QTCOMPRESSOR_H
