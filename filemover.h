#ifndef FILEMOVER_H
#define FILEMOVER_H

#include <QObject>
#include <QFile>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>

bool isMovable(QString& from, QString& to);

class FileMover : public QObject
{
	Q_OBJECT
public:
	explicit FileMover(QString source, QString destination, QString action, QObject *parent = 0);
	~FileMover();
	//void execute();
	QMetaObject::Connection hz;

signals:
	void bytesProgress(uint);
	void completed(int);


public slots:
	void setStatus(int status);

private:
	QString destination;
	QString source;
	QString action;
	int status;
	QWaitCondition cond;

	int copy();
	int move();
};

#endif // FILEMOVER_H
