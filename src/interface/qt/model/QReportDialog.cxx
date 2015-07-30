/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QReportDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QReportDialog.h>
#include <ui_QReportDialog.h>

#include <Application.h>
#include <Configuration.h>
#include <Modality.h>
#include <Rating.h>
#include <User.h>

#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <QDate>
#include <QDateTime>
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>
#include <QList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QTime>

#include <stdexcept>
#include <vector>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QReportDialog::QReportDialog( QWidget* parent )
  : QDialog( parent )
{
  int index = 0;
  this->ui = new Ui_QReportDialog;
  this->ui->setupUi( this );
  QStringList labels;

  labels << "Name";
  this->columnText[index] = "Name";
  this->columnIndex["Name"] = index++;
  this->ui->userTableWidget->horizontalHeader()->setResizeMode(
    this->columnIndex["Name"], QHeaderView::Stretch );

  // list all modalities (to see if user rates them)
  std::vector< vtkSmartPointer< Alder::Modality > > modalityList;
  Alder::Modality::GetAll( &modalityList );

  this->ui->userTableWidget->setColumnCount( index + modalityList.size() );
  for( auto modalityListIt = modalityList.begin(); modalityListIt != modalityList.end(); ++modalityListIt )
  {
    std::string name = (*modalityListIt)->Get( "Name" ).ToString();
    labels << name.c_str();
    this->ui->userTableWidget->horizontalHeader()->setResizeMode( index, QHeaderView::ResizeToContents );
    this->columnText[index] = name;
    this->columnIndex[name] = index++;
  }

  this->ui->userTableWidget->setHorizontalHeaderLabels( labels );
  this->ui->userTableWidget->horizontalHeader()->setClickable( true );
  this->ui->userTableWidget->verticalHeader()->setVisible( false );
  this->ui->userTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  this->ui->userTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );

  this->sortColumn = 0;
  this->sortOrder = Qt::AscendingOrder;

  QObject::connect(
    this->ui->sendPushButton, SIGNAL( clicked( bool ) ),
    this, SLOT( slotSend() ) );
  QObject::connect(
    this->ui->closePushButton, SIGNAL( clicked( bool ) ),
    this, SLOT( slotClose() ) );
  QObject::connect(
    this->ui->userTableWidget->horizontalHeader(), SIGNAL( sectionClicked( int ) ),
    this, SLOT( slotHeaderClicked( int ) ) );
  QObject::connect(
    this->ui->smtpServerComboBox, SIGNAL( currentIndexChanged( int ) ),
    this, SLOT( slotServerChanged( int ) ) );
  QObject::connect(
    this->ui->userTableWidget, SIGNAL( itemChanged( QTableWidgetItem* ) ),
    this, SLOT( slotItemChanged( QTableWidgetItem* ) ) );

  // setup the smtp server selection UI
  Alder::Application *app =  Alder::Application::GetInstance();
  Alder::Configuration *config = app->GetConfig();
  QString smtp1Name = config->GetValue( "SmtpServer1", "Name" ).c_str();
  QString smtp2Name = config->GetValue( "SmtpServer2", "Name" ).c_str();

  this->ui->smtpServerComboBox->clear();
  if( !smtp1Name.isEmpty() )
  {
    QMap<QString, QVariant> qmap;
    qmap["host"] = QVariant(config->GetValue( "SmtpServer1", "Host" ).c_str());
    qmap["port"] = QVariant(config->GetValue( "SmtpServer1", "Port" ).c_str());
    qmap["ssl"]  = QVariant(config->GetValue( "SmtpServer1", "SSL" ).c_str());
    this->ui->smtpServerComboBox->addItem( smtp1Name, QVariant(qmap) );
  }
  if( !smtp2Name.isEmpty() )
  {
    QMap<QString, QVariant> qmap;
    qmap["host"] = QVariant(config->GetValue( "SmtpServer2", "Host" ).c_str());
    qmap["port"] = QVariant(config->GetValue( "SmtpServer2", "Port" ).c_str());
    qmap["ssl"]  = QVariant(config->GetValue( "SmtpServer2", "SSL" ).c_str());
    this->ui->smtpServerComboBox->addItem( smtp2Name, QVariant(qmap) );
  }

  bool hasSmtp = ( 0 < this->ui->smtpServerComboBox->count() );

  this->ui->emailLineEdit->setEnabled( hasSmtp );
  this->ui->passwordLineEdit->setEnabled( hasSmtp );
  this->ui->sendPushButton->setEnabled( hasSmtp );
  this->ui->smtpServerComboBox->setEnabled( hasSmtp );

  if( hasSmtp )
    this->ui->smtpServerComboBox->setCurrentIndex(0);

  this->mailSender.setSubject( "(Alder rating report)" );
  this->mailSender.setPriority( QMailSender::HighPriority );
  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QReportDialog::~QReportDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QReportDialog::slotItemChanged( QTableWidgetItem* item )
{
  bool enableSmtp = false;
  if( Qt::Checked == item->checkState() )
  {
    enableSmtp = true;
  }
  else
  {
    for( int row = 0; row < this->ui->userTableWidget->rowCount() && !enableSmtp; ++row )
    {
      for( int col = 0; col < this->ui->userTableWidget->columnCount(); ++col )
      {
        QTableWidgetItem* it = this->ui->userTableWidget->item( row, col );
        if( Qt::Checked == it->checkState() )
        {
          enableSmtp = true;
          break;
        }
      }
    }
  }

  this->ui->emailLineEdit->setEnabled( enableSmtp );
  this->ui->passwordLineEdit->setEnabled( enableSmtp );
  this->ui->sendPushButton->setEnabled( enableSmtp );
  this->ui->smtpServerComboBox->setEnabled( enableSmtp );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QReportDialog::slotSend()
{
  QString msgBoxTitle = "Alder Rating Report Delivery";
  QString msgBoxMessage;
  QMessageBox::Icon msgBoxIcon;

  if( this->buildReport() )
  {
    QString emailStr = this->ui->emailLineEdit->text();

    this->mailSender.setFrom( emailStr );
    this->mailSender.setTo( emailStr );
    if( this->mailSender.getSsl() )
    {
      QString pwdStr = this->ui->passwordLineEdit->text();
      this->mailSender.setLogin( emailStr, pwdStr );
    }

    this->mailSender.setBody(
      "Hello,\nattached is your requested Alder rating report." );
    this->mailSender.setAttachments( QStringList( this->currentReportFileName ) );

    if( this->mailSender.send() )
    {
      msgBoxMessage =
        "Rating report was successfully mailed to " + emailStr;
      msgBoxIcon = QMessageBox::Information;
    }
    else
    {
      std::string err = "Error: failed to mail rating report " + this->currentReportFileName.toStdString();
      err += "\nMail sender info: ";
      err += this->mailSender.getLastError().toStdString();
      err += " / ";
      err += this->mailSender.getLastCmd().toStdString();
      err += " / ";
      err += this->mailSender.getLastResponse().toStdString();
      Alder::Application::GetInstance()->Log( err );
      msgBoxMessage =
        "Rating report failed delivery to " + emailStr;
      msgBoxIcon = QMessageBox::Warning;
    }
  }
  else
  {
    msgBoxMessage =
      "Rating report failed to build from the Alder database";
    msgBoxIcon = QMessageBox::Critical;
  }

  QMessageBox msgBox( msgBoxIcon, msgBoxTitle, msgBoxMessage, QMessageBox::Ok );
  msgBox.exec();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QReportDialog::buildReport()
{
  bool result = true;

  // find all checked items
  std::map<std::string,std::vector<std::string>> userModalityMap;
  for( int row = 0; row < this->ui->userTableWidget->rowCount(); ++row )
  {
    std::vector<std::string> modalityVec;
    std::string name;
    for( int col = 0; col < this->ui->userTableWidget->columnCount(); ++col )
    {
      QTableWidgetItem* item = this->ui->userTableWidget->item( row, col );
      if( 0 == col )
      {
        name = item->text().toStdString();
        continue;
      }
      if( Qt::Checked == item->checkState() )
        modalityVec.push_back( this->columnText[col] );
    }
    if( !modalityVec.empty() )
      userModalityMap[name] = modalityVec;
  }

  std::vector<std::map<std::string,std::string>> global;
  vtkSmartPointer< Alder::User > user = vtkSmartPointer< Alder::User >::New();
  for( auto it = userModalityMap.begin(); it != userModalityMap.end(); ++it )
  {
    std::string name = it->first;
    if( user->Load( "Name", name ) )
    {
      std::vector<std::string> modalityVec = it->second;
      for( auto vit = modalityVec.begin(); vit != modalityVec.end(); ++vit )
      {
        try
        {
          std::vector<std::map<std::string,std::string>> data =
            Alder::Rating::GetRatingReportData( user, *vit );
          if( !data.empty() )
          {
            global.insert(global.end(),data.begin(),data.end());
          }
        }
        catch( std::runtime_error& e )
        {
          Alder::Application::GetInstance()->Log( e.what() );
          result = false;
          return result;
        }
      }
    }
  }

  if( global.empty() )
    result = false;
  else
  {
    // build the file name
    Alder::Application *app = Alder::Application::GetInstance();
    QString path =
      app->GetConfig()->GetValue( "Path", "ImageData" ).c_str();
    QDate today = QDate::currentDate();
    QTime now   = QTime::currentTime();
    QDateTime todayNow( today, now );
    this->currentReportFileName =
      path
      + "/Alder_Rating_Report_"
      +  todayNow.toString().replace(" ","_")
      + ".csv";

    // open the file and write the report data
    QFile file( this->currentReportFileName );
    if( file.open( QIODevice::WriteOnly ) )
    {
      QTextStream stream( &file );
      bool first = true;
      std::string delim = "\",\"";
      for( auto vit = global.begin(); vit != global.end(); ++vit )
      {
        std::map<std::string,std::string> gmap = *vit;
        if( first )
        {
          // write the header row
          std::string str = "\"";
          for( auto it = gmap.begin(); it != gmap.end(); ++it )
          {
            str += it->first;
            str += delim;
          }
          str.replace( str.rfind(delim), delim.length(), "\"\n" );
          stream << str.c_str();
          first = false;
        }
        std::string str = "\"";
        for( auto it = gmap.begin(); it != gmap.end(); ++it )
        {
          str += it->second;
          str += delim;
        }
        str.replace( str.rfind(delim), delim.length(), "\"\n" );
        stream << str.c_str();
      }
      file.close();
    }
    else
      result = false;
  }

  return result;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QReportDialog::slotClose()
{
  this->accept();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QReportDialog::slotHeaderClicked( int index )
{
  // NOTE: currently the columns with checkboxes cannot be sorted.  In order to do this we would need
  // to either override QSortFilterProxyModel::lessThan() or QAbstractTableModel::sort()
  // For now we'll just ignore requests to sort by these columns
  if( index == this->columnIndex["Name"] )
  {
    // reverse order if already sorted
    if( index == this->sortColumn )
      this->sortOrder = Qt::AscendingOrder == this->sortOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
    this->sortColumn = index;
    this->ui->userTableWidget->sortItems( this->sortColumn, this->sortOrder );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QReportDialog::slotServerChanged( int item )
{
  QVariant data = this->ui->smtpServerComboBox->itemData( item );
  QMap<QString, QVariant> qmap = data.toMap();

  QString host = qmap["host"].toString();
  int port = qmap["port"].toInt();
  bool ssl = qmap["ssl"].toBool();

  this->mailSender.setSsl( ssl );
  this->mailSender.setPort( port );
  this->mailSender.setSmtpServer( host );

  if( ssl )
  {
    this->ui->passwordLineEdit->setEchoMode( QLineEdit::PasswordEchoOnEdit );
    this->ui->passwordLineEdit->setText( "" );
    this->ui->passwordLineEdit->setEnabled( true );
  }
  else
  {
    this->ui->passwordLineEdit->setEchoMode( QLineEdit::Normal );
    this->ui->passwordLineEdit->setText( "NOT REQUIRED" );
    this->ui->passwordLineEdit->setEnabled( false );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QReportDialog::updateInterface()
{
  this->ui->userTableWidget->blockSignals( true );

  this->ui->userTableWidget->setRowCount( 0 );
  QTableWidgetItem *item;
  std::vector< vtkSmartPointer< Alder::Modality > > modalityList;
  Alder::Modality::GetAll( &modalityList );

  std::vector< vtkSmartPointer< Alder::User > > userList;
  vtkSmartPointer<Alder::QueryModifier> modifier = vtkSmartPointer< Alder::QueryModifier >::New();
  modifier->Join( "Rating", "Rating.UserId", "User.Id" );
  modifier->Group( "User.Id" );

  Alder::User::GetAll( &userList, modifier );
  for( auto it = userList.begin(); it != userList.end(); ++it )
  { // for every user, add a new row
    Alder::User *user = (*it);
    this->ui->userTableWidget->insertRow( 0 );

    // add name to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    item->setText( QString( user->Get( "Name" ).ToString().c_str() ) );
    this->ui->userTableWidget->setItem( 0, this->columnIndex["Name"], item );

    // add all modalities (one per column)
    for( auto modalityListIt = modalityList.begin(); modalityListIt != modalityList.end(); ++modalityListIt )
    {
      std::string name = (*modalityListIt)->Get( "Name" ).ToString();
      item = new QTableWidgetItem;

      // does this user have any ratings associated with the current modality?

      std::map<std::string,int> ratings = Alder::Rating::GetNumberOfRatings( user );
      bool hasModalityRating = ( ratings.end() != ratings.find( name ) );
      if( hasModalityRating )
      {
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
        item->setCheckState( Qt::Checked );
      }
      item->setText( hasModalityRating ? QString::number( ratings.find( name )->second ) : QString( "NA" ) );
      this->ui->userTableWidget->setItem( 0, this->columnIndex[name], item );
    }
  }

  this->ui->userTableWidget->sortItems( this->sortColumn, this->sortOrder );

  this->ui->userTableWidget->blockSignals( false );
}
