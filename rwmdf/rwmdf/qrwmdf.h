#pragma once

#include <QObject>

class QRWMDF : public QObject
{
	Q_OBJECT

public:
	QRWMDF(QObject *parent);
	~QRWMDF();
};
