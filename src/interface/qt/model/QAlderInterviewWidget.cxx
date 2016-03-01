/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderInterviewWidget.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QAlderInterviewWidget.h>
#include <QAlderInterviewWidget_p.h>

// Alder includes
#include <Application.h>
#include <Code.h>
#include <CodeType.h>
#include <Common.h>
#include <Exam.h>
#include <Image.h>
#include <ImageNote.h>
#include <Interview.h>
#include <Modality.h>
#include <QueryModifier.h>
#include <Rating.h>
#include <Site.h>
#include <User.h>
#include <Wave.h>
#include <QAlderImageWidget.h>
#include <QVTKProgressDialog.h>

// VTK includes
#include <vtkEventQtSlotConnect.h>
#include <vtkNew.h>

// Qt includes
#include <QMessageBox>
#include <QTreeWidgetItem>

#include <stdexcept>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QAlderInterviewWidgetPrivate methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderInterviewWidgetPrivate::QAlderInterviewWidgetPrivate(QAlderInterviewWidget& object)
  : QObject(&object), q_ptr(&object)
{
  this->qvtkConnection = vtkSmartPointer<vtkEventQtSlotConnect>::New();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderInterviewWidgetPrivate::~QAlderInterviewWidgetPrivate()
{
  this->modalityLookup.clear();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::setupUi( QWidget* widget )
{
  Q_Q(QAlderInterviewWidget);

  this->Ui_QAlderInterviewWidget::setupUi( widget );

  Alder::Application *app = Alder::Application::GetInstance();

  // set up child widgets
  this->examTreeWidget->header()->hide();

  QObject::connect(
    this->previousPushButton, SIGNAL( clicked() ),
    this, SLOT( previous() ) );
  QObject::connect(
    this->nextPushButton, SIGNAL( clicked() ),
    this, SLOT( next() ) );
  QObject::connect(
    this->examTreeWidget, SIGNAL( itemSelectionChanged() ),
    this, SLOT( treeSelectionChanged() ) );

  QObject::connect(
     this->codeTableWidget, SIGNAL( itemClicked(QTableWidgetItem*) ),
     this, SLOT( codeChanged(QTableWidgetItem*) ) );

  QObject::connect(
    this->ratingSlider, SIGNAL( valueChanged( int ) ),
    this, SLOT( ratingChanged( int ) ) );
  QObject::connect(
    this->noteTextEdit, SIGNAL( textChanged() ),
    this, SLOT( noteChanged() ) );
  QObject::connect(
    this->resetRatingPushButton, SIGNAL( clicked() ),
    this, SLOT( resetRating() ) );
  QObject::connect(
    this->useDerivedCheckBox, SIGNAL( clicked() ),
    this, SLOT( derivedRatingToggle() ) );

  this->qvtkConnection = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->qvtkConnection->Connect( app,
    Alder::Common::ActiveInterviewEvent,
    q, SLOT( activeInterviewChanged() ) );

  this->qvtkConnection->Connect( app,
    Alder::Common::ActiveInterviewUpdateImageDataEvent,
    q, SLOT( activeInterviewChanged() ) );

  this->qvtkConnection->Connect( app,
    Alder::Common::ActiveImageEvent,
    q, SLOT( activeImageChanged() ) );

  std::vector< vtkSmartPointer< Alder::Modality > > modalityList;
  Alder::Modality::GetAll( &modalityList );
  QTreeWidgetItem *item = NULL;
  this->modalityLookup.clear();

  QTreeWidgetItem *root = new QTreeWidgetItem( this->examTreeWidget );
  root->setText( 0, "Interview" );
  root->setExpanded( true );
  root->setFlags( Qt::ItemIsEnabled );
  this->examTreeWidget->addTopLevelItem( root );

  for( auto modalityIt = modalityList.begin();
       modalityIt != modalityList.end(); ++modalityIt )
  {
    Alder::Modality *modality = modalityIt->GetPointer();
    QString name = modality->Get( "Name" ).ToString().c_str();
    item = new QTreeWidgetItem( root );
    item->setText( 0, name );
    item->setExpanded( false );
    item->setDisabled( true );
    this->modalityLookup[name] = item;
  }
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::previous()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Interview *activeInterview = app->GetActiveInterview();
  vtkSmartPointer< Alder::Interview > interview;
  if( activeInterview )
  {
    interview = activeInterview->GetPrevious(
      this->loadedCheckBox->isChecked(),
      this->unratedCheckBox->isChecked() );

    this->setActiveInterview( interview );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::next()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Interview *activeInterview = app->GetActiveInterview();
  vtkSmartPointer< Alder::Interview > interview;
  if( activeInterview )
  {
    interview = activeInterview->GetNext(
      this->loadedCheckBox->isChecked(),
      this->unratedCheckBox->isChecked() );
    this->setActiveInterview( interview );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::setActiveInterview( Alder::Interview* interview )
{
  Q_Q(QAlderInterviewWidget);
  if( !interview ) return;

  Alder::Application *app = Alder::Application::GetInstance();
  // warn user if the neighbouring interview is an empty record (ie: no neighbour found)
  vtkVariant vId = interview->Get( "Id" );
  if( !vId.IsValid() || 0 == vId.ToInt() )
  {
    QMessageBox errorMessage( q );
    errorMessage.setWindowModality( Qt::WindowModal );
    errorMessage.setIcon( QMessageBox::Warning );
    errorMessage.setText(
      QDialog::tr( "There are no remaining studies available which meet your criteria." ) );
    errorMessage.exec();
  }
  else
  {
    if( !interview->HasExamData() )
    {
      interview->UpdateExamData();
    }
    Alder::User *user = app->GetActiveUser();
    vtkSmartPointer< Alder::QueryModifier > modifier =
      vtkSmartPointer< Alder::QueryModifier >::New();
    user->InitializeExamModifier( modifier );

    if( !interview->HasImageData( modifier ) )
    {
      interview->UpdateImageData();
      // create a progress dialog to observe the progress of the update
      // QVTKProgressDialog dialog( this );
      //Alder::SingleInterviewProgressFunc func( interview );
      //dialog.Run(
      // "Downloading Exam Images",
      // "Please wait while the interview's images are downloaded.",
      // func );
    }

    app->SetActiveInterview( interview );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::treeSelectionChanged()
{
  QList<QTreeWidgetItem*> list = this->examTreeWidget->selectedItems();
  if( !list.isEmpty() )
  {
    QMap<QTreeWidgetItem*, vtkSmartPointer<Alder::ActiveRecord>>::iterator it = 
      this->treeModelMap.find( list.front() );
    if( it != this->treeModelMap.end() )
    {
      Alder::ActiveRecord *record = it.value();
      Alder::Application::GetInstance()->SetActiveImage(
        Alder::Image::SafeDownCast( record ) );
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::ratingChanged( int value )
{
  Alder::Application *app = Alder::Application::GetInstance();

  // make sure we have an active user and image
  Alder::User *user = app->GetActiveUser();
  Alder::Image *image = app->GetActiveImage();
  if( user && image )
  {
    // See if we have a rating for this user and image
    std::map< std::string, std::string > map;
    map["UserId"] = user->Get( "Id" ).ToString();
    map["ImageId"] = image->Get( "Id" ).ToString();
    vtkNew< Alder::Rating > rating;
    if( !rating->Load( map ) )
    { // no record exists, set the user and image ids
      rating->Set( "UserId", user->Get( "Id" ).ToInt() );
      rating->Set( "ImageId", image->Get( "Id" ).ToInt() );
    }

    if( 0 == value ) rating->SetNull( "Rating" );
    else rating->Set( "Rating", value );

    rating->Save();
  }
  this->ratingValueLabel->setText( 0 == value ?
    QLabel::tr( "N/A" ) : QString::number( value ) );
}


//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::noteChanged()
{
  Alder::Application *app = Alder::Application::GetInstance();

  // make sure we have an active user and image
  Alder::User *user = app->GetActiveUser();
  Alder::Image *image = app->GetActiveImage();
  std::string noteStr = this->noteTextEdit->toPlainText().toStdString();
  if( user && image )
  {
    vtkNew< Alder::ImageNote > note;
    std::map< std::string, std::string > map;
    map[ "UserId" ] = user->Get( "Id" ).ToString();
    map[ "ImageId" ] = image->Get( "Id" ).ToString();
    if( note->Load( map ) )
    {
      if( noteStr.empty() )
      {
        note->Remove();
      }
      else
      {
        note->Set( "Note", noteStr );
        note->Save();
      }
    }
    else
    {
      if( !noteStr.empty() )
      {
        map[ "Note" ] = noteStr;
        note->Save();
      }
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::updateInfo()
{
  QString noteString = "";
  QString interviewerString = QLabel::tr( "N/A" );
  QString siteString = QLabel::tr( "N/A" );
  QString dateString = QLabel::tr( "N/A" );
  QString codeString = QLabel::tr( "N/A" );

  // fill in the active exam information
  Alder::Application *app = Alder::Application::GetInstance();

  Alder::Interview *interview = app->GetActiveInterview();
  Alder::Image *image = app->GetActiveImage();
  Alder::User *user = app->GetActiveUser();
  if( interview && image )
  {
    // get exam from active image
    vtkSmartPointer< Alder::Exam > exam;
    if( image->GetRecord( exam ) )
    {
      if( user )
      {
        vtkNew< Alder::ImageNote > note;
        std::map< std::string, std::string > map;
        map[ "UserId" ] = user->Get( "Id" ).ToString();
        map[ "ImageId" ] = image->Get( "Id" ).ToString();
        if( note->Load( map ) )
          noteString = note->Get( "Note" ).ToString();
      }
      interviewerString = QLabel::tr(exam->Get( "Interviewer" ).ToString().c_str());
      vtkSmartPointer< Alder::Site > site = vtkSmartPointer< Alder::Site >::New();
      interview->GetRecord( site );
      vtkSmartPointer< Alder::Wave > wave = vtkSmartPointer< Alder::Wave >::New();
      interview->GetRecord( wave );
      siteString = QLabel::tr(site->Get( "Name" ).ToString().c_str());
      siteString += "/";
      siteString += QLabel::tr(wave->Get( "Name" ).ToString().c_str());
      dateString = QLabel::tr(exam->Get( "DatetimeAcquired" ).ToString().c_str());
      codeString = QLabel::tr(image->GetCode().c_str());
    }
  }

  // set the text edit content from the exam note
  bool oldSignalState = this->noteTextEdit->blockSignals( true );
  this->noteTextEdit->setPlainText( noteString );
  this->noteTextEdit->blockSignals( oldSignalState );

  this->infoInterviewerValueLabel->setText( interviewerString );
  this->infoSiteValueLabel->setText( siteString );
  this->infoDateValueLabel->setText( dateString );
  this->infoCodeValueLabel->setText( codeString );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::updateCodeList()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *image = app->GetActiveImage();
  if( !image ) return;
  vtkSmartPointer< Alder::Exam > exam;
  image->GetRecord( exam );
  std::map<int,std::string> codeMap = exam->GetCodeTypeData();

  int colCount = 4;
  int imax = codeMap.size();
  if( colCount > imax ) colCount = imax;
  int rowCount = 0;
  for( int i = 0; i < imax; i += colCount ) rowCount++;

  this->codeTableWidget->clear();
  this->codeTableWidget->setColumnCount(colCount);
  this->codeTableWidget->setRowCount(rowCount);

  auto it = codeMap.begin();
  for( int row = 0; row < rowCount;  ++row )
  {
    for( int column = 0; column < colCount; ++column )
    {
      if( it != codeMap.end() )
      {
        QTableWidgetItem* item = new QTableWidgetItem();
        this->codeTableWidget->setItem(row,column,item);
        QCheckBox* box = new QCheckBox(tr(it->second.c_str()));
        this->codeTableWidget->setCellWidget(row,column, box );
        box->setProperty("column", QVariant(column));
        box->setProperty("row", QVariant(row));
        box->setProperty("codeTypeId", QVariant(it->first));
        QObject::connect(
          box, SIGNAL( toggled(bool) ),
          this, SLOT( codeSelected() ) );
        Qt::ItemFlags flags = item->flags();
        flags &= ~(Qt::ItemIsDropEnabled|Qt::ItemIsDragEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);
        item->setFlags(flags);
        ++it;
      }
      else
      {
        QTableWidgetItem* item = this->codeTableWidget->itemAt( row, column );
        Qt::ItemFlags flags = Qt::NoItemFlags;
        item->setFlags(flags);
      }
    }
  }
  this->codeTableWidget->resizeColumnsToContents();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::codeChanged(QTableWidgetItem* item)
{
  QCheckBox* box = qobject_cast<QCheckBox*>(this->codeTableWidget->cellWidget(item->row(), item->column()));
  if( box )
  {
    box->setChecked(!box->isChecked());
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::codeSelected()
{
  // who sent this?
  QCheckBox* box = qobject_cast<QCheckBox*>(sender());
  if( box )
  {
    int column = box->property("column").toInt();
    int row = box->property("row").toInt();
    int codeTypeId = box->property("codeTypeId").toInt();
    bool codeSelected = box->isChecked();
    std::string codeStr = box->text().toStdString();
    // update the rating, the string of codes, and the slider

    Alder::Application *app = Alder::Application::GetInstance();

    // make sure we have an active image
    Alder::User *user = app->GetActiveUser();
    Alder::Image *image = app->GetActiveImage();

    if( user && image )
    {
      // get the codetype
      vtkNew< Alder::CodeType > codeType;
      if( codeType->Load( "Id", codeTypeId ) )
      {
        std::map< std::string, std::string > map;
        vtkVariant userId = user->Get( "Id" );
        vtkVariant imageId = image->Get( "Id" );
        map["UserId"] = userId.ToString();
        map["ImageId"] = imageId.ToString();
        vtkNew< Alder::Rating > rating;
        if( !rating->Load( map ) )
        { // no record exists, set the user and image ids
          rating->Set( "UserId", userId.ToInt() );
          rating->Set( "ImageId", imageId.ToInt() );
          rating->Save();
        }

        vtkNew< Alder::Code > code;
        map["CodeTypeId"] = codeTypeId;
        if( codeSelected )
        {
          if( !code->Load( map ) )
          {
            code->Set( "UserId", userId.ToInt() );
            code->Set( "ImageId", imageId.ToInt() );
            code->Set( "CodeTypeId", codeTypeId );
            code->Save();
          }
        }
        else
        {
          if( code->Load( map ) )
            code->Remove();
        }

        bool useDerived = this->useDerivedCheckBox->isChecked();
        rating->UpdateDerivedRating( useDerived );
        vtkVariant derivedRating = rating->Get("DerivedRating");
        this->derivedRatingLabel->setText( QLabel::tr( "Derived Rating " ) + derivedRating.ToString().c_str() );
        if( useDerived )
        {
          bool oldSignalState = this->ratingSlider->blockSignals( true );
          this->ratingSlider->setValue( derivedRating.ToInt() );
          this->ratingValueLabel->setText( QString::number( derivedRating.ToInt() ) );
          this->ratingSlider->blockSignals( oldSignalState );
        }
      }
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::updateExamTreeWidget()
{
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Interview *interview = app->GetActiveInterview();

  // stop the tree's signals until we are done
  bool oldSignalState = this->examTreeWidget->blockSignals( true );

  // if an interview is open then populate the interview tree
  QTreeWidgetItem *selectedItem = NULL;
  this->treeModelMap.clear();

  for( QMap<QString,QTreeWidgetItem*>::iterator it = this->modalityLookup.begin();
       it != this->modalityLookup.end(); ++it )
  {
     QTreeWidgetItem* item = it.value();
     item->setDisabled(true);
     item->setExpanded( false );
     qDeleteAll( item->takeChildren() );
  }

  if( interview )
  {
    // get the active image so that we can highlight it
    Alder::User *user = app->GetActiveUser();
    Alder::Image *activeImage = app->GetActiveImage();
    vtkVariant activeImageId;
    if( activeImage ) activeImageId = activeImage->Get( "Id" );

    // make root the interview's UID and date
    QTreeWidgetItem *item = this->examTreeWidget->topLevelItem(0);
    QString name = "Interview: ";
    name += interview->Get( "UId" ).ToString().c_str();
    name += " (";
    name += interview->Get( "VisitDate" ).ToString().c_str();
    name += ")";
    item->setText( 0, name );

    // make each modality type a child of the root
    std::vector< vtkSmartPointer< Alder::Modality > > modalityList;
    user->GetList( &modalityList );
    for( auto modalityIt = modalityList.begin(); modalityIt != modalityList.end(); ++modalityIt )
    {
      Alder::Modality *modality = modalityIt->GetPointer();
      QString name = modality->Get( "Name" ).ToString().c_str();
      item = this->modalityLookup[name];
      item->setDisabled( false );
    }

    std::vector< vtkSmartPointer< Alder::Exam > > examList;
    interview->GetList( &examList );

    for( auto examIt = examList.begin(); examIt != examList.end(); ++examIt )
    {
      Alder::Exam *exam = examIt->GetPointer();
      QString modalityName = exam->GetModalityName().c_str();
      item = this->modalityLookup[ modalityName ];
      if( item->isDisabled() ) continue;

      name = "Exam: ";
      QString sideStr = exam->Get( "Side" ).ToString().c_str();
      if( "none" != sideStr )
      {
        name += sideStr;
        name += " ";
      }

      QString examType = exam->GetScanType().c_str();
      name += examType;

      QTreeWidgetItem *examItem = new QTreeWidgetItem( item );
      this->treeModelMap[examItem] = *examIt;
      examItem->setText( 0, name );
      examItem->setExpanded( true );
      examItem->setFlags( Qt::ItemIsEnabled );

      // add the images for this exam
      std::vector< vtkSmartPointer< Alder::Image > > imageList;
      vtkSmartPointer< Alder::QueryModifier > mod = vtkSmartPointer< Alder::QueryModifier >::New();
      mod->Where( "ParentImageId", "=", vtkVariant(), false );
      exam->GetList( &imageList, mod );

      // display the status of the exam
      examItem->setIcon( 0,
        imageList.empty() ?
        QIcon(":/icons/x-icon" ) :
        QIcon(":/icons/eye-visible-icon" ) );

      for( auto imageIt = imageList.begin(); imageIt != imageList.end(); ++imageIt )
      {
        Alder::Image *image = imageIt->GetPointer();

        name = "Image #";
        name += image->Get( "Acquisition" ).ToString().c_str();
        QTreeWidgetItem *imageItem = new QTreeWidgetItem( examItem );
        this->treeModelMap[imageItem] = *imageIt;
        imageItem->setText( 0, name );

        if( 3 == image->Get( "Dimensionality" ).ToInt() )
        {
          imageItem->setIcon(0, QIcon(":/icons/movie-icon" ) );
        }

        imageItem->setExpanded( true );
        imageItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

        if( activeImage && activeImageId == image->Get( "Id" ) )
          selectedItem = imageItem;

        // add child images for this image
        std::vector< vtkSmartPointer< Alder::Image > > childImageList;
        image->GetList( &childImageList, "ParentImageId" );
        for( auto childImageIt = childImageList.begin();
             childImageIt != childImageList.end();
             ++childImageIt )
        {
          Alder::Image *childImage = childImageIt->GetPointer();
          name = "Image #";
          name += childImage->Get( "Acquisition" ).ToString().c_str();
          QTreeWidgetItem *childImageItem = new QTreeWidgetItem( imageItem );
          this->treeModelMap[childImageItem] = *childImageIt;
          childImageItem->setText( 0, name );
          childImageItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

          if( activeImage && activeImageId == childImage->Get( "Id" ) )
            selectedItem = childImageItem;
        }
      }
    }
  }

  // re-enable the tree's signals
  this->examTreeWidget->blockSignals( oldSignalState );

  // set and expand the selected item after restoring signals
  // so that other UI elements get updated
  if( selectedItem )
  {
    QTreeWidgetItem* item = selectedItem;
    while( item->parent() ) item = item->parent();
    this->examTreeWidget->expandItem( item );
    this->examTreeWidget->setCurrentItem( selectedItem );
  }

  QApplication::restoreOverrideCursor();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::resetRating()
{
  Alder::Application *app = Alder::Application::GetInstance();

  // make sure we have an active image
  Alder::User *user = app->GetActiveUser();
  Alder::Image *image = app->GetActiveImage();

  if( user && image )
  {
    std::map< std::string, std::string > map;
    vtkVariant userId = user->Get( "Id" );
    vtkVariant imageId = image->Get( "Id" );
    map["UserId"] = userId.ToString();
    map["ImageId"] = imageId.ToString();
    vtkNew< Alder::Rating > rating;
    if( rating->Load( map ) )
      rating->Remove();

    std::vector< vtkSmartPointer< Alder::Code > > codeList;
    vtkSmartPointer< Alder::QueryModifier > modifier = vtkSmartPointer< Alder::QueryModifier >::New();
    modifier->Where( "UserId", "=" , userId );
    modifier->Where( "ImageId", "=" , imageId );
    Alder::Code::GetAll( &codeList, modifier );
    for( auto it = codeList.begin(); it != codeList.end(); ++it )
      (*it)->Remove();

    for( int row = 0; row < this->codeTableWidget->rowCount(); ++row )
      for( int col = 0; col < this->codeTableWidget->columnCount(); ++col )
      {
        QCheckBox* box = qobject_cast<QCheckBox*>(this->codeTableWidget->cellWidget( row, col ));
        if( box )
        {
          box->blockSignals( true );
          box->setChecked( false );
          box->blockSignals( false );
        }
      }

    this->derivedRatingLabel->setText( tr( "Derived Rating N/A" ) );

    bool oldSignalState = this->ratingSlider->blockSignals( true );
    this->ratingSlider->setValue( 0 );
    this->ratingValueLabel->setText( QLabel::tr( "N/A" ) );
    this->ratingSlider->blockSignals( oldSignalState );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::updateRating()
{
  // stop the rating slider's signals until we are done
  bool oldSignalState = this->ratingSlider->blockSignals( true );

  int ratingValue = 0;
  int derivedRating = 0;
  Alder::Application *app = Alder::Application::GetInstance();

  // make sure we have an active image
  Alder::User *user = app->GetActiveUser();
  Alder::Image *image = app->GetActiveImage();
  vtkVariant userId;
  vtkVariant imageId;

  if( user && image )
  {
    std::map< std::string, std::string > map;
    userId = user->Get( "Id" );
    imageId = image->Get( "Id" );
    map["UserId"] = userId.ToString();
    map["ImageId"] = imageId.ToString();
    vtkNew< Alder::Rating > rating;
    if( rating->Load( map ) )
    {
      vtkVariant v = rating->Get( "Rating" );
      if( v.IsValid() ) ratingValue = v.ToInt();
      v = rating->Get( "DerivedRating" );
      if( v.IsValid() ) derivedRating = v.ToInt();
    }
    std::vector< vtkSmartPointer< Alder::Code > > codeList;
    vtkSmartPointer< Alder::QueryModifier > modifier = vtkSmartPointer< Alder::QueryModifier >::New();
    modifier->Where( "UserId", "=" , userId );
    modifier->Where( "ImageId", "=" , imageId );
    Alder::Code::GetAll( &codeList, modifier );
    std::vector<std::string> codeString;
    QStringList qlist;
    for( auto it = codeList.begin(); it != codeList.end(); ++it )
    {
      vtkSmartPointer< Alder::CodeType > codeType;
      if( (*it)->GetRecord(codeType) )
        qlist << codeType->Get("Code").ToString().c_str();
    }
    qlist.removeDuplicates();

    for( int row = 0; row < this->codeTableWidget->rowCount(); ++row )
      for( int col = 0; col < this->codeTableWidget->columnCount(); ++col )
      {
        QCheckBox* box = qobject_cast<QCheckBox*>(this->codeTableWidget->cellWidget( row, col ));
        if( box && -1 != qlist.indexOf(box->text()) )
        {
          box->blockSignals( true );
          box->setChecked( true );
          box->blockSignals( false );
        }
      }
  }

  this->ratingSlider->setValue( ratingValue );
  this->ratingValueLabel->setText( 0 == ratingValue ?
    QLabel::tr( "N/A" ) :
    QString::number( ratingValue ) );
  this->derivedRatingLabel->setText( "Derived Rating " +
    ( 0 == derivedRating ? QLabel::tr( "N/A" ) : QString::number( derivedRating ) ) );

  this->useDerivedCheckBox->setChecked(
    ( derivedRating == ratingValue ? true : false ) );

  // re-enable the rating slider's signals
  this->ratingSlider->blockSignals( oldSignalState );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::derivedRatingToggle()
{
  bool useDerived = this->useDerivedCheckBox->isChecked();
  if( useDerived )
  {
    Alder::Application *app = Alder::Application::GetInstance();

    // make sure we have an active image
    Alder::User *user = app->GetActiveUser();
    Alder::Image *image = app->GetActiveImage();
    if( user && image )
    {
      std::map< std::string, std::string > map;
      map["UserId"] = user->Get("Id").ToString();
      map["ImageId"] = image->Get("Id").ToString();
      vtkNew< Alder::Rating > rating;
      int ratingValue;
      int derivedRating;
      if( rating->Load( map ) )
      {
        vtkVariant v = rating->Get( "Rating" );
        if( v.IsValid() ) ratingValue = v.ToInt();
        v = rating->Get( "DerivedRating" );
        if( v.IsValid() ) derivedRating = v.ToInt();
        if( derivedRating != ratingValue )
        {
          this->ratingSlider->setEnabled( true );
          this->ratingSlider->setValue( v.ToInt() );
        }
      }
    }
  }
  this->ratingSlider->setEnabled( !useDerived );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::updateViewer()
{
  Alder::Image *image = Alder::Application::GetInstance()->GetActiveImage();
  if( image )
    this->imageWidget->load( image->GetFileName().c_str() );
  else
    this->imageWidget->reset();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::updateEnabled()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Interview *interview = app->GetActiveInterview();
  Alder::Image *image = app->GetActiveImage();

  // set all widget enable states
  this->unratedCheckBox->setEnabled( interview );
  this->loadedCheckBox->setEnabled( interview );
  this->previousPushButton->setEnabled( interview );
  this->nextPushButton->setEnabled( interview );
  this->examTreeWidget->setEnabled( interview );

  this->useDerivedCheckBox->setEnabled( image );
  this->resetRatingPushButton->setEnabled( image );
  this->codeTableWidget->setEnabled( image );
  this->ratingSlider->setEnabled( image && !this->useDerivedCheckBox->isChecked() );
  this->noteTextEdit->setEnabled( image );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QAlderInterviewWidget methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderInterviewWidget::QAlderInterviewWidget( QWidget* parent )
  : Superclass( parent )
  , d_ptr(new QAlderInterviewWidgetPrivate(*this))
{
  Q_D(QAlderInterviewWidget);
  d->setupUi(this);
  d->updateExamTreeWidget();
  d->updateEnabled();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderInterviewWidget::~QAlderInterviewWidget()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::saveImage( const QString& fileName )
{
  Q_D(QAlderInterviewWidget);
  d->imageWidget->save( fileName );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::activeInterviewChanged()
{
  Q_D(QAlderInterviewWidget);
  d->updateExamTreeWidget();
  d->updateEnabled();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::hideControls( bool hide )
{
  Q_D(QAlderInterviewWidget);
  // hide/show all the widgets to show only the image viewer
  QList<QLayout*> layouts;
  layouts.append( d->verticalLayout );
  layouts.append( d->infoLayout );
  layouts.append( d->ratingLayout );
  layouts.append( d->buttonLayout );

  QList<QWidget*> widgets;
  for( int i = 0; i < layouts.count(); ++i )
  {
     QLayout* layout = layouts.at( i );
    for( int j = 0; j < layout->count(); ++j )
    {
      if( QWidgetItem* item = dynamic_cast<QWidgetItem*>(
          layout->itemAt( j ) ) )
      {
        widgets.append( item->widget() );
      }
    }
  }
  for( int i = 0; i < widgets.count(); ++i )
  {
    QWidget* item = qobject_cast<QWidget*>( widgets.at( i ) );
    if( hide ) item->hide();
    else item->show();
  }
}
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::activeImageChanged()
{
  Q_D(QAlderInterviewWidget);
  d->updateInfo();
  d->updateCodeList();
  d->updateRating();
  d->updateViewer();
  d->updateEnabled();
}
