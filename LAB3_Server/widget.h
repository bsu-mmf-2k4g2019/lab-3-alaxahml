#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QVector>
#include <QDataStream>
#include <QTcpServer>

// int is used for this constants

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void sendFortune();
    void hanleNewConnection();
    void hanleReadyRead();
    void dropClient(QTcpSocket *client);

private:
    QLabel *statusLabel = nullptr;
    QTcpServer *tcpServer = nullptr;
    QVector<QString> fortunes;
    QVector<QTcpSocket*> sockets;

    QDataStream in;
};

#endif // WIDGET_H
