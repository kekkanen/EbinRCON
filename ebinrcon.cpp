#include "ebinrcon.h"
#include "ui_ebinrcon.h"

#include <QtNetwork>
#include <QMessageBox>
#include <QPushButton>
#include <QDataStream>
#include <QBuffer>
#include <QTextCodec>

EbinRCON::EbinRCON(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EbinRCON),
    sock(new QTcpSocket(this)),
    id(1234)
{
    // set ui from designer file
    ui->setupUi(this);

    // set send button to inactive
    ui->sendCommandButton->setEnabled(false);

    // set the password field to not show its contents
    ui->serverPassword->setEchoMode(QLineEdit::Password);

    ui->statusBar->addWidget(new QLabel(tr("Ready")));


    // testbutton
    QPushButton* test = new QPushButton(this);
    test->setText("test");

    // connect signals
    connect(ui->connectButton, &QPushButton::clicked, this, &EbinRCON::connectClicked);
    connect(sock, &QTcpSocket::connected, this, &EbinRCON::onConnected);
    connect(sock, &QTcpSocket::disconnected, this, &EbinRCON::onDisconnected);
    connect(sock, &QTcpSocket::readyRead, this, &EbinRCON::readResponse);
    connect(ui->sendCommandButton, &QPushButton::clicked, this, &EbinRCON::sendCommandClicked);
}

EbinRCON::~EbinRCON()
{
    delete ui;
}

QByteArray EbinRCON::buildRconPacket(qint32 type, QByteArray body)
{
    qint8 end = 0;
    const qint32 size = body.size() + 10;

    // create a byte array for the packet to be sent
    // and a buffer for writing to the array
    QByteArray packet;
    QBuffer buffer(&packet);
    buffer.open(QIODevice::WriteOnly);

    // use the buffer as data stream and write to the
    QDataStream out(&buffer);
    out.setByteOrder(QDataStream::LittleEndian);
    out << size << id << type;
    buffer.close();
    packet.append(body);
    packet.append(end);	// add two zero bytes as per rcon standard
    packet.append(end);

    return packet;
}

void EbinRCON::cancelClicked()
{
    sock->abort();

    onDisconnected();
    ui->statusBar->showMessage("Cancelled", 10);
}

void EbinRCON::connectClicked()
{
    // attempt to establish a connection to the server specified in the ui.
    // if the input fields are empty, a warning is displayed
    QString address = ui->serverName->text();
    if (address.isEmpty()) {
        QMessageBox msg;
        msg.setText("Enter a server address.");
        msg.exec();
    } else if (ui->serverPassword->text().isEmpty()) {
        QMessageBox msg;
        msg.setText("Enter password.");
        msg.exec();
    } else {

        ui->statusBar->showMessage("Connecting...");
        ui->connectButton->setText("Cancel");

        // button -connect
        // button +cancel
        disconnect(ui->connectButton, &QPushButton::clicked, this, &EbinRCON::connectClicked);
        connect(ui->connectButton, &QPushButton::clicked, this, &EbinRCON::cancelClicked);
        sock->connectToHost(address, 27015);

    }
}
void EbinRCON::disconnectClicked()
{

    ui->connectButton->setText("Disconnecting");

    // Button -disconnect
    disconnect(ui->connectButton, &QPushButton::clicked, this, &EbinRCON::disconnectClicked);

    // disconnect from the host
    sock->disconnectFromHost();
}

void EbinRCON::sendCommandClicked()
{
    QByteArray command = ui->commandLine->text().toUtf8();
    QByteArray packet = buildRconPacket(2, command);
    sendRconCommand(packet);
}

// when the socket is connected
void EbinRCON::onConnected()
{
    // set ui to match connected state
    ui->sendCommandButton->setEnabled(true);
    ui->statusBar->showMessage("Connected");

    // set the button to call disconnectClicked()
    ui->connectButton->setText("Disconnect");

    // button -cancel
    // button +disconnect
    disconnect(ui->connectButton, &QPushButton::clicked, this, &EbinRCON::cancelClicked);
    connect(ui->connectButton, &QPushButton::clicked, this, &EbinRCON::disconnectClicked);

	// get the server's password as user input
    QByteArray pw = ui->serverPassword->text().toUtf8();
    QByteArray packet = buildRconPacket(3, pw);
    sendRconCommand(packet);

}

// when the socket is disconnected
void EbinRCON::onDisconnected()
{
    // set ui to match state
    ui->statusBar->showMessage("Disconnected", 10);
    ui->connectButton->setText("Connect");
    ui->sendCommandButton->setEnabled(false);

    // button +connect
    connect(ui->connectButton, &QPushButton::clicked, this, &EbinRCON::connectClicked);
    QMessageBox msg;
    msg.setText("Disconnected.");
    msg.exec();
}


void EbinRCON::sendRconCommand(QByteArray packet)
{

    sock->write(packet);

}

// read the server's response
void EbinRCON::readResponse()
{
    QByteArray response;
    QDataStream in;
    in.setDevice(sock);
    in.setByteOrder(QDataStream::LittleEndian);
    in >> response;

    QString q(response);
    ui->commandLine->setText(q); // show server response, not final
}