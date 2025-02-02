#include "ClientWindow.h"
#include <QFileDialog>
#include <QMessageBox>

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent),
    m_socket(nullptr)
{
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout* connectionLayout = new QHBoxLayout();
    m_hostEdit = new QLineEdit("127.0.0.1");
    m_portEdit = new QLineEdit("12345");
    m_connectButton = new QPushButton("Connect");
    m_statusLabel = new QLabel("Disconnected");

    connectionLayout->addWidget(new QLabel("Host:"));
    connectionLayout->addWidget(m_hostEdit);
    connectionLayout->addWidget(new QLabel("Port:"));
    connectionLayout->addWidget(m_portEdit);
    connectionLayout->addWidget(m_connectButton);
    connectionLayout->addWidget(m_statusLabel);

    m_textDisplay = new QTextEdit();
    m_textDisplay->setReadOnly(true);

    QHBoxLayout* sendLayout = new QHBoxLayout();
    m_messageEdit = new QLineEdit();
    m_sendTextButton = new QPushButton("Send Text");
    m_sendFileButton = new QPushButton("Send File");

    sendLayout->addWidget(m_messageEdit);
    sendLayout->addWidget(m_sendTextButton);
    sendLayout->addWidget(m_sendFileButton);

    m_clientCountLabel = new QLabel("Clients connected: 0");

    mainLayout->addLayout(connectionLayout);
    mainLayout->addWidget(m_textDisplay);
    mainLayout->addLayout(sendLayout);
    mainLayout->addWidget(m_clientCountLabel);

    setCentralWidget(centralWidget);

    m_socket = new QTcpSocket(this);

    connect(m_connectButton, &QPushButton::clicked, this, &ClientWindow::connectToServer);
    connect(m_socket, &QTcpSocket::connected, this, &ClientWindow::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientWindow::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &ClientWindow::onReadyRead);

    connect(m_sendTextButton, &QPushButton::clicked, this, &ClientWindow::sendTextMessage);
    connect(m_sendFileButton, &QPushButton::clicked, this, &ClientWindow::sendFile);
}

ClientWindow::~ClientWindow()
{
    if (m_socket->isOpen()) {
        m_socket->disconnectFromHost();
    }
}

void ClientWindow::connectToServer()
{
    QString host = m_hostEdit->text();
    quint16 port = m_portEdit->text().toUShort();
    m_socket->connectToHost(host, port);
    m_statusLabel->setText("Connecting...");
}

void ClientWindow::onConnected()
{
    m_statusLabel->setText("Connected");
    displayMessage("Connected to server.");
}

void ClientWindow::onDisconnected()
{
    m_statusLabel->setText("Disconnected");
    displayMessage("Disconnected from server.");
}

void ClientWindow::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    processIncomingData(data);
}

void ClientWindow::processIncomingData(const QByteArray &data)
{
    if (data.size() < 4) {
        return;
    }

    QDataStream in(data);
    qint32 messageType;
    in >> messageType;

    switch (messageType) {
    case 0: {
        QString msg;
        in >> msg;
        displayMessage(QString("[Peer]: %1").arg(msg));
        break;
    }
    case 1: {
        QByteArray fileContent;
        in >> fileContent;

        QString filePath = QFileDialog::getSaveFileName(this, "Save Received File");
        if (!filePath.isEmpty()) {
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(fileContent);
                file.close();
                displayMessage("File received and saved to: " + filePath);
            } else {
                displayMessage("Failed to save file.");
            }
        } else {
            displayMessage("File received but not saved.");
        }
        break;
    }
    case 2: {
        qint32 count;
        in >> count;
        m_clientCountLabel->setText(QString("Clients connected: %1").arg(count));
        break;
    }
    default:
        displayMessage("Received unknown message type.");
        break;
    }
}

void ClientWindow::sendTextMessage()
{
    if (!m_socket->isOpen()) {
        QMessageBox::warning(this, "Error", "Not connected to any server.");
        return;
    }

    QString msg = m_messageEdit->text();
    if (msg.isEmpty()) return;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (qint32)0 << msg;  // type=0 for text

    m_socket->write(block);
    displayMessage(QString("[Me]: %1").arg(msg));
    m_messageEdit->clear();
}

void ClientWindow::sendFile()
{
    if (!m_socket->isOpen()) {
        QMessageBox::warning(this, "Error", "Not connected to any server.");
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(this, "Open File to Send");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Cannot open file.");
        return;
    }

    QByteArray fileContent = file.readAll();
    file.close();

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (qint32)1 << fileContent;  // type=1 for file

    m_socket->write(block);
    displayMessage("Sent file: " + filePath);
}

void ClientWindow::displayMessage(const QString &msg)
{
    m_textDisplay->append(msg);
}
