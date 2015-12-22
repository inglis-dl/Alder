/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QSelectInterviewDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QSelectInterviewDialog.h>
#include <ui_QSelectInterviewDialog.h>

#include <Application.h>
#include <Database.h>
#include <Exam.h>
#include <Interview.h>
#include <Modality.h>
#include <QueryModifier.h>
#include <Site.h>
#include <User.h>
#include <Wave.h>

#include <QVTKProgressDialog.h>

#include <vtkSmartPointer.h>

#include <QInputDialog>
#include <QList>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <vector>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSelectInterviewDialog::QSelectInterviewDialog( QWidget* parent )
  : QDialog( parent )
{
  int index = 0;
  this->ui = new Ui_QSelectInterviewDialog;
  this->ui->setupUi( this );
  QStringList labels;

  labels << "Site";
  this->columnIndex[labels.last().toStdString()] = index++;

  labels << "UId";
  this->columnIndex[labels.last().toStdString()] = index++;

  labels << "VisitDate";
  this->columnIndex[labels.last().toStdString()] = index++;

  labels << "Wave";
  this->columnIndex[labels.last().toStdString()] = index++;

  // user allowed modalities will fill up the remainder of the table
  std::vector< vtkSmartPointer< Alder::Modality > > modalityList;
  Alder::User *user = Alder::Application::GetInstance()->GetActiveUser();
  user->GetList( &modalityList );

  // make enough columns for all modalities and set their names
  this->ui->interviewTableWidget->setColumnCount( index + modalityList.size() );
  for( auto modalityListIt = modalityList.begin(); modalityListIt != modalityList.end(); ++modalityListIt )
  {
    std::string name = (*modalityListIt)->Get( "Name" ).ToString();
    labels << name.c_str();
    this->columnIndex[labels.last().toStdString()] = index++;
  }

  this->ui->interviewTableWidget->setHorizontalHeaderLabels( labels );
  this->ui->interviewTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  this->ui->interviewTableWidget->horizontalHeader()->setClickable( true );
  this->ui->interviewTableWidget->verticalHeader()->setVisible( false );
  this->ui->interviewTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  this->ui->interviewTableWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );

  this->sortColumn = 1;
  this->sortOrder = Qt::AscendingOrder;

  QObject::connect(
    this->ui->searchPushButton, SIGNAL( clicked( bool ) ),
    this, SLOT( slotSearch() ) );
  QObject::connect(
    this->ui->buttonBox, SIGNAL( accepted() ),
    this, SLOT( slotAccepted() ) );
  QObject::connect(
    this->ui->interviewTableWidget, SIGNAL( itemSelectionChanged() ),
    this, SLOT( slotSelectionChanged() ) );
  QObject::connect(
    this->ui->interviewTableWidget->horizontalHeader(), SIGNAL( sectionClicked( int ) ),
    this, SLOT( slotHeaderClicked( int ) ) );

  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSelectInterviewDialog::~QSelectInterviewDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialog::slotSearch()
{
  bool ok;
  QString text = QInputDialog::getText(
    this,
    QObject::tr( "Search Term" ),
    QObject::tr(
      "Provide some or all of the interviews to search for,\nusing commas to separate terms:" ),
    QLineEdit::Normal,
    QString(),
    &ok );

  if( ok )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    this->searchText.clear();
    if( text.contains(",") )
    {
      this->searchText = text.split(",", QString::SkipEmptyParts );
      this->searchText.removeDuplicates();
    }
    else
    {
      this->searchText << text;
    }
    this->updateInterface();
    QApplication::restoreOverrideCursor();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialog::slotAccepted()
{
  QList< QTableWidgetSelectionRange > ranges = this->ui->interviewTableWidget->selectedRanges();

   // TODO: warn the user that multiple interviews are about to be downloaded
   // and could take time ... allow for and respond to abort signals
   // for multi selection, the first loaded interview will be set to the active
   // interview

  if( !ranges.empty() )
  {
    Alder::Application *app = Alder::Application::GetInstance();
    int uidCol = this->columnIndex["UId"];
    bool first = true;
    std::vector< vtkSmartPointer< Alder::Interview > > interviewList;

    vtkSmartPointer< Alder::Interview > interview =
      vtkSmartPointer< Alder::Interview >::New();

    // only worry about the modalities the user is allowed to access
    vtkSmartPointer< Alder::QueryModifier > modifier =
      vtkSmartPointer< Alder::QueryModifier >::New();

    Alder::User *user = app->GetActiveUser();
    user->InitializeExamModifier( modifier );

    for( QList< QTableWidgetSelectionRange >::const_iterator it =
         ranges.constBegin(); it != ranges.constEnd(); ++it )
    {
      for( int row = (*it).topRow(); row <= (*it).bottomRow(); ++row )
      {
        QTableWidgetItem* item = this->ui->interviewTableWidget->item( row, uidCol );
        QVariant vId = item->data( Qt::UserRole );
        if( vId.isValid() && !vId.isNull() && interview->Load( "Id", vId.toInt() ) )
        {
          if( !interview->HasImageData( modifier ) )
          {
            interviewList.push_back( interview );
          }
          else
          {
            if( first )
            {
              app->SetActiveInterview( interview );
              first = false;
            }
          }
        }
      }
    }

    // process the list of interviews that require image data download
    if( !interviewList.empty() )
    {
      /*
      QVTKProgressDialog dialog( this->parentWidget() );
      Alder::ListInterviewProgressFunc func( idList );
      this->hide();
      dialog.Run(
        "Downloading Images",
        "Please wait while the images are downloaded.",
        func );
      */
      for( auto it = interviewList.cbegin(); it != interviewList.cend(); ++it )
      {
        (*it)->UpdateImageData();
        if( first && (*it)->HasImageData( modifier ) )
        {
          app->SetActiveInterview( *it );
          first = false;
        }
      }
      /*
      if( first )
      {
        for( auto it = idList.begin(); it != idList.end(); ++it )
        {
          interview->Loat( "Id", *it )
          if( interview->Load( "Id", *it ) && interview->HasImageData( modifier ) )
          {
            app->SetActiveInterview( interview );
            break;
          }
        }
      }*/
    }
  }

  this->accept();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialog::slotSelectionChanged()
{
  QList< QTableWidgetSelectionRange > ranges = this->ui->interviewTableWidget->selectedRanges();
  this->ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( !ranges.empty() );

  if( !ranges.empty() )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

    int uidCol = this->columnIndex["UId"];

    vtkSmartPointer< Alder::Interview > interview =
      vtkSmartPointer< Alder::Interview >::New();

    for( QList< QTableWidgetSelectionRange >::const_iterator it =
         ranges.constBegin(); it != ranges.constEnd(); ++it )
    {
      for( int row = (*it).topRow(); row <= (*it).bottomRow(); ++row )
      {
        QTableWidgetItem* item = this->ui->interviewTableWidget->item( row, uidCol );        
        QVariant vId = item->data( Qt::UserRole );
        if( vId.isValid() && !vId.isNull() && interview->Load( "Id", vId.toInt() ) )
        {
          if( !interview->HasExamData() )
          {
            try
            {
              interview->UpdateExamData();
            }
            catch( std::runtime_error& e )
            {
              QApplication::restoreOverrideCursor();
              throw e;
            }
          }
          this->updateRow( row, interview );
        }
      }
    }
    QApplication::restoreOverrideCursor();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialog::slotHeaderClicked( int index )
{
  // reverse order if already sorted
  if( this->sortColumn == index )
  {
    this->sortOrder =
      Qt::AscendingOrder == this->sortOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
  }

  this->sortColumn = index;
  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialog::updateRow( const int &row, Alder::Interview *interview )
{
  std::vector< vtkSmartPointer< Alder::Exam > > examList;
  Alder::Exam *exam;
  Alder::User *user = Alder::Application::GetInstance()->GetActiveUser();
  QString strUId = QString( interview->Get( "UId" ).ToString().c_str() );
  QVariant vId = QVariant( interview->Get( "Id" ).ToInt() );
  QString strSite = "N/A";
  vtkSmartPointer< Alder::Site > site;
  if( interview->GetRecord( site ) )
  {
    strSite = QString( site->Get( "Name" ).ToString().c_str() );
  }
  QString strVisitDate = QString( interview->Get( "VisitDate" ).ToString().c_str() );
  QString strWave = "N/A";
  vtkSmartPointer< Alder::Wave > wave;
  if( interview->GetRecord( wave ) )
  {
    strWave = QString( wave->Get( "Name" ).ToString().c_str() );
  }

  std::map< std::string, bool > updateItemText;
  std::map< std::string, int > examCount;
  std::map< std::string, int > ratedCount;
  std::map< std::string, int > downloadCount;
  std::map< std::string, QString > itemText;
  std::string modalityName;
  std::vector< vtkSmartPointer< Alder::Modality > > modalityList;
  user->GetList( &modalityList );

  for( auto modalityListIt = modalityList.begin(); modalityListIt != modalityList.end(); ++modalityListIt )
  {
    modalityName = (*modalityListIt)->Get( "Name" ).ToString();
    updateItemText[modalityName] = false;
    examCount[modalityName] = 0;
    ratedCount[modalityName] = 0;
    downloadCount[modalityName] = 0;
    itemText[modalityName] = "?";
  }

  QString date = tr( "N/A" );
  interview->GetList( &examList );

  // examCount the number of exams of each modality and whether they have been rated
  for( auto examIt = examList.begin(); examIt != examList.end(); ++examIt )
  {
    exam = examIt->GetPointer();
    modalityName = exam->GetModalityName();

    // NOTE: it is possible that an exam with state "Ready" has valid data, but we are leaving
    // those exams out for now since we don't know for sure whether they are always valid

    // the modalities and hence exams permitted to this user has already been determined
    if( updateItemText.end() == updateItemText.find(modalityName) ) continue;

    updateItemText[modalityName] = true;
    if( "Completed" == exam->Get( "Stage" ).ToString() ) examCount[modalityName]++;
    if( exam->HasImageData() ) downloadCount[modalityName]++;
    if( exam->IsRatedBy( user ) ) ratedCount[modalityName]++;
  }

  // set the text, if updated
  for( auto updateItemTextIt = updateItemText.begin();
       updateItemTextIt != updateItemText.end();
       ++updateItemTextIt )
  {
    modalityName = updateItemTextIt->first;
    if( updateItemTextIt->second )
    {
      itemText[modalityName] = QString::number( downloadCount[modalityName] );
      itemText[modalityName] += tr( " of " );
      itemText[modalityName] += QString::number( examCount[modalityName] );
      itemText[modalityName] += tr( ", " );
      itemText[modalityName] += QString::number( ratedCount[modalityName] );
      itemText[modalityName] += tr( " rated" );
    }
  }

  if( this->searchText.empty() || this->searchTextInUId( strUId ) )
  {
    QTableWidgetItem *item;
    item = this->ui->interviewTableWidget->item( row, this->columnIndex["Site"] );
    if( item ) item->setText( strSite );
    item = this->ui->interviewTableWidget->item( row, this->columnIndex["UId"] );
    if( item ) 
    {
      item->setText( strUId );
      item->setData( Qt::UserRole, vId );
    }  
    item = this->ui->interviewTableWidget->item( row, this->columnIndex["VisitDate"] );
    if( item ) item->setText( strVisitDate );
    item = this->ui->interviewTableWidget->item( row, this->columnIndex["Wave"] );
    if( item ) item->setText( strWave );

    // add all modalities to the table
    for( auto itemTextIt = itemText.begin(); itemTextIt != itemText.end(); ++itemTextIt )
    {
      item = this->ui->interviewTableWidget->item( row, this->columnIndex[itemTextIt->first] );
      if( item ) item->setText( itemTextIt->second );
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QSelectInterviewDialog::searchTextInUId( const QString &UId )
{
  for( QStringList::const_iterator it = this->searchText.constBegin();
       it != this->searchText.constEnd(); ++it )
  {
    if( UId.contains( (*it), Qt::CaseInsensitive ) ) return true;
  }
  return false;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialog::updateInterface()
{
  if( this->searchText.empty() ) return;

  vtkSmartPointer< Alder::QueryModifier > modifier = vtkSmartPointer< Alder::QueryModifier >::New();
  for( QStringList::const_iterator it = this->searchText.constBegin();
       it != this->searchText.constEnd(); ++it )
  {
    // create a modifier using the search text
    std::string where = (*it).toStdString();
    where += "%";
    modifier->Where( "UId", "LIKE", vtkVariant( where ), true,
      (it != this->searchText.constBegin()) );
  }

  // get all the interviews given the search text
  std::vector< vtkSmartPointer< Alder::Interview > > interviewList;
  Alder::Interview::GetAll( &interviewList, modifier );

  // if the search fails to find any UId's, inform the user and return
  if( interviewList.empty() )
  {
    QMessageBox messageBox( this );
    messageBox.setWindowModality( Qt::WindowModal );
    messageBox.setIcon( QMessageBox::Information );
    std::string s = "No matches for search criteria ";
    s += modifier->GetSql();
    s += ", please try again.";
    messageBox.setText( tr( s.c_str() ) );
    messageBox.exec();
    return;
  }

  this->ui->interviewTableWidget->setRowCount( 0 );
  QTableWidgetItem *item;
  Alder::User *user = Alder::Application::GetInstance()->GetActiveUser();
  std::vector< vtkSmartPointer< Alder::Modality > > modalityList;
  user->GetList( &modalityList );

  for( auto it = interviewList.begin(); it != interviewList.end(); ++it )
  { // for every interview, add a new row
    Alder::Interview *interview = *it;
    QString UId = QString( interview->Get( "UId" ).ToString().c_str() );

    if( this->searchText.empty() || this->searchTextInUId( UId ) )
    {
      this->ui->interviewTableWidget->insertRow( 0 );

      // add site to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      this->ui->interviewTableWidget->setItem( 0, this->columnIndex["Site"], item );

      // add UId to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      this->ui->interviewTableWidget->setItem( 0, this->columnIndex["UId"], item );

      // add visit date to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      this->ui->interviewTableWidget->setItem( 0, this->columnIndex["VisitDate"], item );

      // add wave to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      this->ui->interviewTableWidget->setItem( 0, this->columnIndex["Wave"], item );

      // add all modalities (one per column)
      for( auto modalityListIt = modalityList.begin();
           modalityListIt != modalityList.end();
           ++modalityListIt )
      {
        item = new QTableWidgetItem;
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        this->ui->interviewTableWidget->setItem(
          0, this->columnIndex[(*modalityListIt)->Get( "Name" ).ToString()], item );
      }

      this->updateRow( 0, interview );
    }
  }

  this->ui->interviewTableWidget->sortItems( this->sortColumn, this->sortOrder );
}
