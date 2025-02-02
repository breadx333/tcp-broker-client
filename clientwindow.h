#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QFile>
#include <QDataStream>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private slots:
    void connectToServer();
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void sendTextMessage();
    void sendFile();

private:
    void processIncomingData(const QByteArray &data);
    void displayMessage(const QString &msg);

private:
    QTcpSocket* m_socket;
    QLineEdit* m_hostEdit;
    QLineEdit* m_portEdit;
    QPushButton* m_connectButton;
    QLabel* m_statusLabel;
    QTextEdit* m_textDisplay;
    QLineEdit* m_messageEdit;
    QPushButton* m_sendTextButton;
    QPushButton* m_sendFileButton;
    QLabel* m_clientCountLabel;
};

#endif // CLIENTWINDOW_H
