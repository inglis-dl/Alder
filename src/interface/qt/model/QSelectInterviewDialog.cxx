/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QSelectInterviewDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QSelectInterviewDialog.h>
#include <QSelectInterviewDialog_p.h>

// Alder includes
#include <Application.h>
#include <Database.h>
#include <Exam.h>
#include <Interview.h>
#include <Modality.h>
#include <QueryModifier.h>
#include <Site.h>
#include <User.h>
#include <Wave.h>

// VTK includes
#include <vtkSmartPointer.h>

// Qt includes
#include <QInputDialog>
#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <vector>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QSelectInterviewDialogPrivate methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSelectInterviewDialogPrivate::QSelectInterviewDialogPrivate(QSelectInterviewDialog& object)
  : QObject(&object), q_ptr(&object)
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSelectInterviewDialogPrivate::~QSelectInterviewDialogPrivate()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialogPrivate::setupUi( QWidget* widget )
{
  Q_Q(QSelectInterviewDialog);

  this->Ui_QSelectInterviewDialog::setupUi( widget );

  int index = 0;
  QStringList labels;

  labels << "Site";
  this->columnIndex.insert(labels.last(),index++);

  labels << "UId";
  this->columnIndex.insert(labels.last(),index++);

  labels << "VisitDate";
  this->columnIndex.insert(labels.last(),index++);

  labels << "Wave";
  this->columnIndex.insert(labels.last(),index++);

  // user allowed modalities will fill up the remainder of the table
  std::vector< vtkSmartPointer< Alder::Modality > > vecModality;
  Alder::User *user = Alder::Application::GetInstance()->GetActiveUser();
  user->GetList( &vecModality );

  // make enough columns for all modalities and set their names
  this->interviewTableWidget->setColumnCount( index + vecModality.size() );
  for( auto it = vecModality.begin(); it != vecModality.end(); ++it )
  {
    labels << (*it)->Get( "Name" ).ToString().c_str();
    this->columnIndex.insert(labels.last(),index++);
  }

  this->interviewTableWidget->setHorizontalHeaderLabels( labels );
  this->interviewTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  this->interviewTableWidget->horizontalHeader()->setClickable( true );
  this->interviewTableWidget->verticalHeader()->setVisible( false );
  this->interviewTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  this->interviewTableWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );

  this->sortColumn = 1;
  this->sortOrder = Qt::AscendingOrder;

  QObject::connect(
    this->searchPushButton, SIGNAL( clicked( bool ) ),
    this, SLOT( search() ) );
  QObject::connect(
    this->buttonBox, SIGNAL( accepted() ),
    q, SLOT( accepted() ) );
  QObject::connect(
    this->interviewTableWidget, SIGNAL( itemSelectionChanged() ),
    this, SLOT( selectionChanged() ) );
  QObject::connect(
    this->interviewTableWidget->horizontalHeader(), SIGNAL( sectionClicked( int ) ),
    this, SLOT( sort( int ) ) );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialogPrivate::search()
{
  Q_Q(QSelectInterviewDialog);

  bool ok;
  QString text = QInputDialog::getText(
    q, QDialog::tr( "Search Term" ),
    QDialog::tr(
      "Provide some or all of the interviews to search for,\nusing commas to separate terms:" ),
    QLineEdit::Normal, QString(), &ok );

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
    this->updateUi();
    QApplication::restoreOverrideCursor();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialogPrivate::sort( int col )
{
  // reverse order if already sorted
  if( col == this->sortColumn )
  {
    this->sortOrder =
      Qt::AscendingOrder == this->sortOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
  }

  this->sortColumn = col;
  this->updateUi();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialogPrivate::updateRow( const int &row, Alder::Interview *interview )
{
  // interview meta data
  QString strUId = QString( interview->Get( "UId" ).ToString().c_str() );
  QVariant vId = QVariant( interview->Get( "Id" ).ToInt() );

  QString strSite = "N/A";
  vtkSmartPointer< Alder::Site > site;
  if( interview->GetRecord( site ) )
    strSite = QString( site->Get( "Name" ).ToString().c_str() );

  QString strVisitDate = QString( interview->Get( "VisitDate" ).ToString().c_str() );
  QString strWave = "N/A";
  vtkSmartPointer< Alder::Wave > wave;
  if( interview->GetRecord( wave ) )
    strWave = QString( wave->Get( "Name" ).ToString().c_str() );

  // exam meta data by modality
  QMap< QString, bool > updateItemText;
  QMap< QString, int > examCount;
  QMap< QString, int > ratedCount;
  QMap< QString, int > downloadCount;
  QMap< QString, QString > itemText;

  Alder::User *user = Alder::Application::GetInstance()->GetActiveUser();
  std::vector< vtkSmartPointer< Alder::Modality > > vecModality;
  user->GetList( &vecModality );

  for( auto it = vecModality.begin(); it != vecModality.end(); ++it )
  {
    QString name( (*it)->Get( "Name" ).ToString().c_str() );
    updateItemText.insert(name, false);
    examCount.insert(name, 0);
    ratedCount.insert(name, 0) ;
    downloadCount.insert( name, 0);
    itemText.insert(name, "?");
  }

  QString date = tr( "N/A" );
  std::vector< vtkSmartPointer< Alder::Exam > > vecExam;
  interview->GetList( &vecExam );

  // examCount the number of exams of each modality and whether they have been rated
  for( auto it = vecExam.begin(); it != vecExam.end(); ++it )
  {
    Alder::Exam *exam = it->GetPointer();
    QString name = exam->GetModalityName().c_str();

    // the modalities and hence exams permitted to this user has already been determined
    if( !updateItemText.contains(name) )
      continue;

    updateItemText.insert(name, true);
    if( exam->HasImageData() )
    {
      examCount[name]++;
      if( 1 == exam->Get( "Downloaded" ).ToInt() )
      {
        downloadCount[name]++;
        if( exam->IsRatedBy( user ) )
          ratedCount[name]++;
      }
    }
  }

  // set the text, if updated
  for(QMap<QString,bool>::iterator it = updateItemText.begin(); it != updateItemText.end(); ++it )
  {
    if( it.value() )
    {
      QString name = it.key();
      QString s = QString::number( downloadCount.value(name) );
      s += QWidget::tr( " of " );
      s += QString::number( examCount.value(name) );
      s += QWidget::tr( ", " );
      s += QString::number( ratedCount.value(name) );
      s += QWidget::tr( " rated" );
      itemText.insert(name, s);
    }
  }

  if( this->searchText.isEmpty() || this->searchTextInUId( strUId ) )
  {
    QTableWidgetItem *item;
    item = this->interviewTableWidget->item( row, this->columnIndex.value("Site") );
    if( item ) item->setText( strSite );
    item = this->interviewTableWidget->item( row, this->columnIndex.value("UId") );
    if( item )
    {
      item->setText( strUId );
      item->setData( Qt::UserRole, vId );
    }
    item = this->interviewTableWidget->item( row, this->columnIndex.value("VisitDate") );
    if( item )
      item->setText( strVisitDate );
    item = this->interviewTableWidget->item( row, this->columnIndex.value("Wave") );
    if( item )
      item->setText( strWave );

    // add all modalities to the table
    for(QMap<QString,QString>::iterator it = itemText.begin(); it != itemText.end(); ++it )
    {
      item = this->interviewTableWidget->item( row, this->columnIndex.value(it.key()) );
      if( item )
        item->setText( it.value() );
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QSelectInterviewDialogPrivate::searchTextInUId( const QString &UId )
{
  for( QStringList::const_iterator it = this->searchText.constBegin();
       it != this->searchText.constEnd(); ++it )
  {
    if( UId.contains( (*it), Qt::CaseInsensitive ) )
      return true;
  }
  return false;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialogPrivate::updateUi()
{
  Q_Q(QSelectInterviewDialog);
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
  std::vector< vtkSmartPointer< Alder::Interview > > vecInterview;
  Alder::Interview::GetAll( &vecInterview, modifier );

  // if the search fails to find any UId's, inform the user and return
  if( vecInterview.empty() )
  {
    QMessageBox messageBox( q );
    messageBox.setWindowModality( Qt::WindowModal );
    messageBox.setIcon( QMessageBox::Information );
    QString s = QDialog::tr("No matches for search criteria ");
    s += modifier->GetSql().c_str();
    s += QDialog::tr(", please try again.");
    messageBox.setText( s );
    messageBox.exec();
    return;
  }

  this->interviewTableWidget->setRowCount( 0 );
  QTableWidgetItem *item;
  Alder::User *user = Alder::Application::GetInstance()->GetActiveUser();
  std::vector< vtkSmartPointer< Alder::Modality > > vecModality;
  user->GetList( &vecModality );

  for( auto it = vecInterview.begin(); it != vecInterview.end(); ++it )
  { // for every interview, add a new row
    Alder::Interview *interview = *it;
    QString UId = interview->Get( "UId" ).ToString().c_str();

    if( this->searchText.empty() || this->searchTextInUId( UId ) )
    {
      this->interviewTableWidget->insertRow( 0 );

      // add site to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      this->interviewTableWidget->setItem( 0, this->columnIndex.value("Site"), item );

      // add UId to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      this->interviewTableWidget->setItem( 0, this->columnIndex.value("UId"), item );

      // add visit date to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      this->interviewTableWidget->setItem( 0, this->columnIndex.value("VisitDate"), item );

      // add wave to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      this->interviewTableWidget->setItem( 0, this->columnIndex.value("Wave"), item );

      // add all modalities (one per column)
      for( auto vit = vecModality.begin(); vit != vecModality.end(); ++vit )
      {
        QString name = (*vit)->Get( "Name" ).ToString().c_str();
        item = new QTableWidgetItem;
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        this->interviewTableWidget->setItem(
          0, this->columnIndex.value(name), item );
      }

      this->updateRow( 0, interview );
    }
  }

  this->interviewTableWidget->sortItems( this->sortColumn, this->sortOrder );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialogPrivate::selectionChanged()
{
  QList< QTableWidgetSelectionRange > ranges = this->interviewTableWidget->selectedRanges();
  this->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( !ranges.empty() );

  if( !ranges.empty() )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

    int col = this->columnIndex.value("UId");

    vtkSmartPointer< Alder::Interview > interview =
      vtkSmartPointer< Alder::Interview >::New();

    for( QList< QTableWidgetSelectionRange >::const_iterator it =
         ranges.constBegin(); it != ranges.constEnd(); ++it )
    {
      for( int row = (*it).topRow(); row <= (*it).bottomRow(); ++row )
      {
        QTableWidgetItem* item = this->interviewTableWidget->item( row, col );
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
QSelectInterviewDialog::QSelectInterviewDialog( QWidget* parent )
  : Superclass( parent )
  , d_ptr(new QSelectInterviewDialogPrivate(*this))
{
  Q_D(QSelectInterviewDialog);
  d->setupUi(this);
  d->updateUi();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSelectInterviewDialog::~QSelectInterviewDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialog::accepted()
{
  Q_D(QSelectInterviewDialog);
  QList< QTableWidgetSelectionRange > ranges = d->interviewTableWidget->selectedRanges();

  if( !ranges.empty() )
  {
    Alder::Application *app = Alder::Application::GetInstance();
    int col = d->columnIndex.value("UId");
    bool first = true;
    std::vector< vtkSmartPointer< Alder::Interview > > vecInterview;

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
        QTableWidgetItem* item = d->interviewTableWidget->item( row, col );
        QVariant vId = item->data( Qt::UserRole );
        if( vId.isValid() && !vId.isNull() && interview->Load( "Id", vId.toInt() ) )
        {
          if( interview->HasImageData( modifier ) )
          {
            if( first )
            {
              app->SetActiveInterview( interview );
              first = false;
            }
          }
          else
          {
            vecInterview.push_back( interview );
          }
        }
      }
    }

    // process the list of interviews that require image data download
    if( !vecInterview.empty() )
    {
      for( auto it = vecInterview.cbegin(); it != vecInterview.cend(); ++it )
      {
        (*it)->UpdateImageData();
        if( first && (*it)->HasImageData( modifier ) )
        {
          app->SetActiveInterview( *it );
          first = false;
        }
      }
    }
  }

  this->accept();
}
