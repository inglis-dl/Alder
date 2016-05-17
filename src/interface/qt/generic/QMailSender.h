/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QLoginDialog.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QMailSender_h
#define __QMailSender_h

// Qt includes
#include <QAuthenticator>
#include <QPointer>
#include <QSslSocket>
#include <QStringList>
#include <QTcpSocket>

class QMailSender : public QObject
{
  Q_OBJECT
  Q_ENUMS(Priority)
  Q_ENUMS(ContentType)
  Q_ENUMS(Encoding)
  Q_ENUMS(ISO)

  public:
    enum Priority { HighPriority, NormalPriority, LowPriority };

    enum ContentType { TextContent, HtmlContent };

    enum Encoding { Encoding_7bit, Encoding_8bit, Encoding_base64 };

    enum ISO { utf8, iso88591 };

    // constructors
    QMailSender();
    QMailSender(const QString& smtpServer,
                const QString& from, const QStringList& to);
    // destructor
    ~QMailSender();

    bool send();

    QString getLastError() {
      return this->lastError;
    }
    QString getLastCmd() {
      return this->lastCmd;
    }
    QString getLastResponse() {
      return this->lastResponse;
    }

    void setSmtpServer(const QString& smtpServer) {
      this->smtpServer = smtpServer;
    }
    void setPort(const int& port) {
      this->port = port;
    }
    void setTimeout(const int& timeout) {
      this->timeout = timeout;
    }
    void setLogin(const QString& login, const QString& passwd) {
      this->login = login;
      this->password = passwd;
    }
    void setSsl(const bool& ssl) {
      this->ssl = ssl;
    }
    bool getSsl() {
      return this->ssl;
    }
    void sslOn() {
      this->setSsl(true);
    }
    void sslOff() {
      this->setSsl(false);
    }
    void setCc(const QStringList& cc) {
      this->cc = cc;
    }
    void setBcc(const QStringList& bcc) {
      this->bcc = bcc;
    }
    void setAttachments(const QStringList& attachments) {
      this->attachments = attachments;
    }
    void addAttachment(const QString& attachment) {
      this->attachments << attachment;
    }
    void setReplyTo(const QString& replyTo) {
      this->replyTo = replyTo;
    }
    void setPriority(const QMailSender::Priority& priority) {
      this->priority = priority;
    }
    void setFrom(const QString& from);
    void setTo(const QStringList& to) {
      this->to = to;
    }
    void setTo(const QString& to) {
      QStringList q(to);
      this->setTo(q);
    }
    void setSubject(const QString& subject) {
      this->subject = subject;
    }
    void setBody(const QString& body) {
      this->body = body;
    }
    void setFromName(const QString& fromName) {
      this->fromName = fromName;
    }
    void setContentType(const QMailSender::ContentType& contentType) {
      this->contentType = contentType;
    }
    void setISO(const QMailSender::ISO& iso);
    void setEncoding(const QMailSender::Encoding& encoding);
    void setProxyAuthenticator(const QAuthenticator& authenticator);

    void clearTo() {
      this->to.clear();
    }
    void clearAttachments() {
      this->attachments.clear();
    }
    void clearCc() {
      this->cc.clear();
    }
    void clearBcc() {
      this->bcc.clear();
    }

  private Q_SLOTS:
    void errorReceived(QAbstractSocket::SocketError socketError);
    void proxyAuthentication(
      const QNetworkProxy& proxy, QAuthenticator* authenticator);

  private:
    QString getMimeType(const QString& ext) const;
    QString getPriorityString() const;
    void addMimeAttachment(QString* pdata, const QString& filename) const;
    void addMimeBody(QString* pdata) const;
    QString getMailData() const;
    QString getContentType() const;
    bool read(const QString& waitfor);
    bool sendCommand(const QString& cmd, const QString& waitfor);
    void error(const QString& msg);

    QString smtpServer;
    int port;
    int timeout;
    QString login;
    QString password;
    QPointer<QTcpSocket> socket;
    bool ssl;
    QAuthenticator authenticator;
    QString lastError;
    QString lastCmd;
    QString lastResponse;
    QString from;
    QStringList to;
    QString subject;
    QString body;
    QStringList cc;
    QStringList bcc;
    QStringList attachments;
    QString fromName;
    QString replyTo;
    Priority priority;
    ContentType contentType;
    QString encoding;
    QString charset;
    QString bodyCodec;
    QString confirmTo;
};

#endif
