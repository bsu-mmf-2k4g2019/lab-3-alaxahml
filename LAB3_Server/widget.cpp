#include "widget.h"

#include <QDebug>
#include <QTcpSocket>
#include <QNetworkInterface>

#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRandomGenerator>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    qDebug() << "Server constructor is called";
    statusLabel = new QLabel();
    statusLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen()) {
        qDebug() << "Unable to make server listen";
        statusLabel->setText(QString("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
        close();
        return;
    }

    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

    statusLabel->setText(QString("The server is running on\n\nIP: %1\nport: %2\n\n"
                            "Run the Fortune Client example now.")
                         .arg(ipAddress).arg(tcpServer->serverPort()));
    qDebug() << "Start server on: " << ipAddress << ":" << tcpServer->serverPort();

    auto quitButton = new QPushButton(tr("Quit"));
    connect(quitButton, &QAbstractButton::clicked, this, &QWidget::close);
    connect(tcpServer, &QTcpServer::newConnection, this, &Widget::hanleNewConnection);

    auto buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);
    buttonLayout->addStretch(1);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);

    // Initialize fortunes
    fortunes << "You've been leading a dog's life. Stay off the furniture."
             << "Computers are not intelligent. They only think they are.";

    in.setVersion(QDataStream::Qt_4_0);
}

Widget::~Widget()
{

}

void Widget::sendFortune()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    for(int i = 0;i < sockets.size();++i){

    out.setDevice(sockets[i]);
    out << fortunes[fortunes.size()-1];

   // QTcpSocket *clientConnection = dynamic_cast<QTcpSocket*>(sender());
   // clientConnection->write(block);

    //dropClient(clientConnection);
    sockets[i]->write(block);
    dropClient(sockets[i]);
    }
}

void Widget::hanleNewConnection()
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    bool is_exist = false;
    in.setDevice(clientConnection);

    /*for(int i = 0; i < sockets.size();++i){
        if(clientConnection == sockets[i]){
            is_exist = true;
            break;
        }
    }*/
        sockets.push_back(clientConnection);

    connect(clientConnection, &QAbstractSocket::readyRead, this, &Widget::hanleReadyRead);
}

void Widget::hanleReadyRead()
{
    qDebug() << "Read fortune is called";

        QString fortune;
        // Read fortune from client
        in.startTransaction();
        in >> fortune;
        if (!in.commitTransaction())
            return;
        qDebug() << "Fortune: " << fortune;
        if(fortunes.size() < 50) {
        fortunes.push_back(fortune);
        }
        /*else{
            fortunes.erase(0);
            fortunes.push_back(fortune);
        }*/

        sendFortune();
}

void Widget::dropClient(QTcpSocket *client)
{
    disconnect(client, &QAbstractSocket::readyRead, this, &Widget::hanleReadyRead);
    connect(client, &QAbstractSocket::disconnected,
            client, &QObject::deleteLater);
    client->disconnectFromHost();
}
