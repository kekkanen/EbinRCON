#ifndef EBINRCON_H
#define EBINRCON_H

#include <QMainWindow>
#include <QTcpSocket>

namespace Ui {
class EbinRCON;
}

class EbinRCON : public QMainWindow
{
    Q_OBJECT

public:
    explicit EbinRCON(QWidget *parent = 0);
    ~EbinRCON();

    QByteArray buildRconPacket(qint32 type, QByteArray body = 0x00);

public slots:
    void cancelClicked();
    void connectClicked();
    void disconnectClicked();
    void sendCommandClicked();
    void onConnected();
    void onDisconnected();
    void sendRconCommand(QByteArray packet);
    void readResponse();

private:
    Ui::EbinRCON *ui;

    QTcpSocket* sock;
    qint32 id;
};

#endif // EBINRCON_H
