/*=========================================================================

  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Module:    QMailSender.cxx
  Language:  C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QMailSender.h>

// Qt includes
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QHostInfo>
#include <QSslSocket>
#include <QString>
#include <QTextCodec>
#include <QTextStream>

// C includes
#include <time.h>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
static QString encodeBase64(const QString& s)
{
  QByteArray text;
  text.append(s);
  return text.toBase64();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
static QString timeStamp()
{
  QTime now = QTime::currentTime();
  QDate today = QDate::currentDate();
  QStringList monthList;
  monthList << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun"
            << "Jul" << "Aug" << "Sep" << "Oct" << "Nov" << "Dec";
  QString month = monthList.value(today.month() - 1);
  QString day = QString::number(today.day());
  QString year = QString::number(today.year());
  QString result = QString("Date: %1 %2 %3 %4").arg(
    day, month, year, now.toString("hh:mm:ss"));
  return result;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
static QString createBoundary()
{
  QByteArray hash =
    QCryptographicHash::hash(
      QString(QString::number(qrand())).toUtf8(), QCryptographicHash::Md5);
  QString boundary = hash.toHex();
  boundary.truncate(26);
  boundary.prepend("----=_NextPart_");
  return boundary;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMailSender::QMailSender()
{
  this->setPort(25);
  this->setTimeout(30000);
  this->setSubject("(no subject)");
  this->setPriority(QMailSender::NormalPriority);
  this->setContentType(QMailSender::TextContent);
  this->setEncoding(QMailSender::Encoding_7bit);
  this->setISO(QMailSender::utf8);
  this->setSsl(false);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMailSender::QMailSender(
  const QString& smtpServer, const QString& from, const QStringList& to)
{
  this->setSmtpServer(smtpServer);
  this->setPort(25);
  this->setTimeout(30000);
  this->setFrom(from);
  this->setTo(to);
  this->setSubject("(no subject)");
  this->setPriority(QMailSender::NormalPriority);
  this->setContentType(QMailSender::TextContent);
  this->setEncoding(QMailSender::Encoding_7bit);
  this->setISO(QMailSender::utf8);
  this->setSsl(false);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMailSender::~QMailSender()
{
  if (this->socket)
  {
    delete this->socket;
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMailSender::setFrom(const QString& from)
{
  this->from = from;
  this->fromName = from;
  this->replyTo = from;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMailSender::setISO(const QMailSender::ISO& iso)
{
  switch (iso)
  {
    case QMailSender::iso88591:
      this->charset = "iso-8859-1";
      this->bodyCodec = "ISO 8859-1";
      break;
    case QMailSender::utf8:
      this->charset = "utf-8";
      this->bodyCodec = "UTF-8";
      break;
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMailSender::setEncoding(const QMailSender::Encoding& encoding)
{
  switch (encoding)
  {
    case QMailSender::Encoding_7bit:
      this->encoding = "7bit";
      break;
    case QMailSender::Encoding_8bit:
      this->encoding = "8bit";
      break;
    case QMailSender::Encoding_base64:
      this->encoding = "base64";
      break;
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QMailSender::getContentType() const
{
  switch (this->contentType)
  {
    case QMailSender::HtmlContent:
      return "text/html";
    case QMailSender::TextContent:
    default:
      return "text/plain";
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QMailSender::getPriorityString() const
{
  QString result;
  switch (this->priority)
  {
    case QMailSender::LowPriority:
      result.append("X-Priority: 5\n");
      result.append("Priority: Non-Urgent\n");
      result.append("X-MSMail-Priority: Low\n");
      result.append("Importance: low\n");
      break;
    case QMailSender::HighPriority:
      result.append("X-Priority: 1\n");
      result.append("Priority: Urgent\n");
      result.append("X-MSMail-Priority: High\n");
      result.append("Importance: high\n");
      break;
    default:
      result.append("X-Priority: 3\n");
      result.append("X-MSMail-Priority: Normal\n");
  }

  return result;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMailSender::addMimeAttachment(
  QString* pdata, const QString& fileName) const
{
  QFile file(fileName);
  bool ok = file.open(QIODevice::ReadOnly);
  if (!ok)
  {
    pdata->append("Error attaching file: " + fileName + "\r\n");
    return;
  }

  QFileInfo fileinfo(fileName);
  QString name = fileinfo.fileName();
  QString type = this->getMimeType(fileinfo.suffix());
  pdata->append("Content-Type: " + type + ";\n");
  pdata->append("  name=" + name + "\n");
  pdata->append("Content-Transfer-Encoding: base64\n");
  pdata->append("Content-Disposition: attachment\n");
  pdata->append("  filename=" + name + "\n\n");

  QDataStream stream(&file);
  quint8 byte;
  QString str;
  while (!stream.atEnd())
  {
    stream >> byte;
    str.append(static_cast<char>(byte));
  }
  QString encodedFile = encodeBase64(str);
  pdata->append(encodedFile);
  pdata->append("\r\n\n");
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMailSender::addMimeBody(QString* pdata) const
{
  pdata->append("Content-Type: " + this->getContentType() + ";\n");
  pdata->append("  charset=" + this->charset + "\r\n");
  pdata->append("Content-Transfer-Encoding: " + this->encoding + "\r\n");
  pdata->append("\r\n\n");

  if (QMailSender::HtmlContent == this->contentType)
  {
    pdata->append(
      "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
    pdata->append(
      "<HTML><HEAD>\r\n");
    pdata->append(
      "<META HTTP-EQUIV=\"CONTENT-TYPE\" CONTENT=\"text/html; charset=" +
      this->charset + "\">\r\n");
    pdata->append(
      "<META content=\"MSHTML 6.00.2900.2802\" name=GENERATOR>\r\n");
    pdata->append(
      "<STYLE></STYLE>\r\n");
    pdata->append(
      "</head>\r\n");
    pdata->append(
      "<body bgColor=#ffffff>\r\n");
  }

  QByteArray encodedBody(this->body.toLatin1());
  QTextCodec* codec = QTextCodec::codecForName(this->bodyCodec.toLatin1());
  pdata->append(codec->toUnicode(encodedBody) + "\r\n");

  if (QMailSender::HtmlContent == this->contentType)
  {
    pdata->append("<DIV>&nbsp;</DIV></body></html>\r\n\n");
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QMailSender::getMailData() const
{
  QString data;
  QByteArray hash =
    QCryptographicHash::hash(
      QString(QString::number(qrand())).toUtf8(), QCryptographicHash::Md5);
  QString id = hash.toHex();

  data.append("Message-ID: " + id + "@" + QHostInfo::localHostName() + "\n");
  data.append("From: \"" + this->from + "\" <" + this->fromName + ">\n");

  if (0 < this->to.count())
  {
    data.append("To: ");
    bool first = true;
    foreach(QString val, this->to)
    {
      if (!first)
      {
        data.append(",");
        first = false;
      }
      data.append("<" + val + ">");
    }
    data.append("\n");
  }

  if (0 < this->cc.count())
  {
    data.append("Cc: ");
    bool first = true;
    foreach(QString val, this->cc)
    {
      if (!first)
      {
        data.append(",");
        first = false;
      }
      data.append(val);
    }
    data.append("\n");
  }

  data.append("Subject: " + this->subject + "\n");
  data.append(timeStamp() + "\n");
  data.append("MIME-Version: 1.0\n");

  QString boundary = createBoundary();
  data.append("Content-Type: Multipart/Mixed; boundary=\"" + boundary + "\"\n");
  data.append(this->getPriorityString());
  data.append("X-Mailer: QT4\r\n");

  if (!this->confirmTo.isEmpty())
  {
    data.append("Disposition-Notification-To: " + this->confirmTo + "\n");
  }

  if (!this->replyTo.isEmpty() && this->confirmTo != this->from)
  {
    data.append("Reply-to: " + this->replyTo + "\n");
    data.append("Return-Path: <" + this->replyTo + ">\n");
  }

  data.append("\n");
  data.append("This is a multi-part message in MIME format.\r\n\n");

  data.append("--" + boundary + "\n");

  this->addMimeBody(&data);

  if (0 < this->attachments.count())
  {
    foreach(QString val, this->attachments)
    {
      data.append("--" + boundary + "\n");
      this->addMimeAttachment(&data, val);
    }
  }

  data.append("--" + boundary + "--\r\n\n");

  return data;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QMailSender::getMimeType(const QString& ext) const
{
  QString result = "text/plain";
  // Text
  if ("txt" == ext)
    result = "text/plain";
  else if ("htm" == ext || "html" == ext)
    result = "text/html";
  else if ("css" == ext)
    result = "text/css";
  // Images
  else if ("png" == ext)
    result = "image/png";
  else if ("gif" == ext)
    result = "image/gif";
  else if ("jpg" == ext || "jpeg" == ext)
    result = "image/jpeg";
  else if ("bmp" == ext)
    result = "image/bmp";
  else if ("tif" == ext || "tiff" == ext)
    result = "image/tiff";
  // Archives
  else if ("bz2" == ext)
    result = "application/x-bzip";
  else if ("gz" == ext)
    result = "application/x-gzip";
  else if ("tar" == ext)
    result = "application/x-tar";
  else if ("zip" == ext)
    result = "application/zip";
  // Audio
  else if ("aif" == ext || "aiff" == ext)
    result = "audio/aiff";
  else if ("mid" == ext || "midi" == ext)
    result = "audio/mid";
  else if ("mp3" == ext)
    result = "audio/mpeg";
  else if ("ogg" == ext)
    result = "audio/ogg";
  else if ("wav" == ext)
    result = "audio/wav";
  else if ("wma" == ext)
    result = "audio/x-ms-wma";
  // Video
  else if ("asf" == ext || "asx" == ext)
    result = "video/x-ms-asf";
  else if ("avi" == ext)
    result = "video/avi";
  else if ("mpg" == ext || "mpeg" == ext)
    result = "video/mpeg";
  else if ("wmv" == ext)
    result = "video/x-ms-wmv";
  else if ("wmx" == ext)
    result = "video/x-ms-wmx";
  // XML
  else if ("xml" == ext)
    result = "text/xml";
  else if ("xsl" == ext)
    result = "text/xsl";
  // Microsoft
  else if ("doc" == ext || "rtf" == ext)
    result = "application/msword";
  else if ("xls" == ext || "xlsx" == ext)
    result = "application/excel";
  else if ("ppt" == ext || "pps" == ext)
    result = "application/vnd.ms-powerpoint";
  // Adobe
  else if ("pdf" == ext)
    result = "application/pdf";
  else if ("ai" == ext || "eps" == ext)
    result = "application/postscript";
  else if ("psd" == ext)
    result = "image/psd";
  // Macromedia
  else if ("swf" == ext)
    result = "application/x-shockwave-flash";
  // Real
  else if ("ra" == ext)
    result = "audio/vnd.rn-realaudio";
  else if ("ram" == ext)
    result = "audio/x-pn-realaudio";
  else if ("rm" == ext)
    result = "application/vnd.rn-realmedia";
  else if ("rv" == ext)
    result = "video/vnd.rn-realvideo";
  // Other
  else if ("exe" == ext)
    result = "application/x-msdownload";
  else if ("pls" == ext)
    result = "audio/scpls";
  else if ("m3u" == ext)
    result = "audio/x-mpegurl";

  return result;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMailSender::errorReceived(QAbstractSocket::SocketError socketError)
{
  QString msg;
  switch (socketError)
  {
    case QAbstractSocket::ConnectionRefusedError:
      msg = "ConnectionRefusedError";
      break;
    case QAbstractSocket::RemoteHostClosedError:
      msg = "RemoteHostClosedError";
      break;
    case QAbstractSocket::HostNotFoundError:
      msg = "HostNotFoundError";
      break;
    case QAbstractSocket::SocketAccessError:
      msg = "SocketAccessError";
      break;
    case QAbstractSocket::SocketResourceError:
      msg = "SocketResourceError";
      break;
    case QAbstractSocket::SocketTimeoutError:
      msg = "SocketTimeoutError";
      break;
    case QAbstractSocket::DatagramTooLargeError:
      msg = "DatagramTooLargeError";
      break;
    case QAbstractSocket::NetworkError:
      msg = "NetworkError";
      break;
    case QAbstractSocket::AddressInUseError:
      msg = "AddressInUseError";
      break;
    case QAbstractSocket::SocketAddressNotAvailableError:
      msg = "SocketAddressNotAvailableError";
      break;
    case QAbstractSocket::UnsupportedSocketOperationError:
      msg = "UnsupportedSocketOperationError";
      break;
    case QAbstractSocket::ProxyAuthenticationRequiredError:
      msg = "ProxyAuthenticationRequiredError";
      break;
    default:
      msg = "Unknown Error";
  }

  this->error("Socket error [" + msg + "]");
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QMailSender::send()
{
  this->lastError = "";

  if (this->socket)
  {
    delete this->socket;
  }

#ifndef QT_NO_OPENSSL
  this->socket = this->ssl ? new QSslSocket(this) : new QTcpSocket(this);
#else
  this->socket = new QTcpSocket(this);
#endif

  QObject::connect(this->socket, SIGNAL(error(QAbstractSocket::SocketError)),
    this, SLOT(errorReceived(QAbstractSocket::SocketError)));
  QObject::connect(this->socket,
    SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)),
    this, SLOT(proxyAuthentication(const QNetworkProxy &, QAuthenticator *)));

  bool auth = !this->login.isEmpty();
  this->socket->connectToHost(this->smtpServer, this->port);
  if (!this->socket->waitForConnected(this->timeout))
  {
    this->error("Time out connecting host");
    return false;
  }

  if (!this->read("220"))
  {
    return false;
  }

  if (!this->sendCommand("EHLO there", "250"))
  {
    if (!this->sendCommand("HELO there", "250"))
    {
      return false;
    }
  }

#ifndef QT_NO_OPENSSL
  if (this->ssl)
  {
    if (!this->sendCommand("STARTTLS", "220"))
    {
      return false;
    }

    QSslSocket* pssl = qobject_cast<QSslSocket*>(this->socket);
    if (0 == pssl)
    {
      this->error("Internal error casting to QSslSocket");
      return false;
    }
    pssl->startClientEncryption();
  }
#endif

  if (auth)
  {
    if (!this->sendCommand("AUTH LOGIN", "334"))
    {
      return false;
    }
    if (!this->sendCommand(encodeBase64(this->login), "334"))
    {
      return false;
    }
    if (!this->sendCommand(encodeBase64(this->password), "235"))
    {
      return false;
    }
  }

  if (!this->sendCommand(
    QString::fromLatin1("MAIL FROM:<") +
    this->from + QString::fromLatin1(">"), "250"))
  {
    return false;
  }

  QStringList recipients = this->to + this->cc + this->bcc;
  for (int i = 0; i < recipients.count(); ++i)
  {
    if (!this->sendCommand(
      QString::fromLatin1("RCPT TO:<") +
      recipients.at(i) + QString::fromLatin1(">"), "250"))
    {
      return false;
    }
  }

  if (!this->sendCommand(QString::fromLatin1("DATA"), "354"))
  {
    return false;
  }
  if (!this->sendCommand(
    this->getMailData() + QString::fromLatin1("\r\n."), "250"))
  {
    return false;
  }
  if (!this->sendCommand(QString::fromLatin1("QUIT"), "221"))
  {
    return false;
  }

  this->socket->disconnectFromHost();

  return true;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QMailSender::read(const QString& waitfor)
{
  if (!this->socket->waitForReadyRead(this->timeout))
  {
    this->error("Read timeout");
    return false;
  }

  if (!this->socket->canReadLine())
  {
    this->error("Can't read");
    return false;
  }

  QString responseLine;
  do
  {
    responseLine = this->socket->readLine();
  } while (this->socket->canReadLine() && ' ' != responseLine[3]);

  this->lastResponse = responseLine;

  QString prefix = responseLine.left(3);
  bool isOk = (prefix == waitfor);
  if (!isOk)
  {
    this->error("Waiting for " + waitfor + ", received " + prefix);
  }

  return isOk;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QMailSender::sendCommand(const QString& cmd, const QString& waitfor)
{
  QTextStream stream(this->socket);
  stream << cmd + "\r\n";
  stream.flush();
  this->lastCmd = cmd;

  return this->read(waitfor);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMailSender::error(const QString& msg)
{
  this->lastError = msg;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMailSender::proxyAuthentication(
  const QNetworkProxy& notused, QAuthenticator* authenticator)
{
  *authenticator = this->authenticator;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMailSender::setProxyAuthenticator(const QAuthenticator& authenticator)
{
  this->authenticator = authenticator;
}
