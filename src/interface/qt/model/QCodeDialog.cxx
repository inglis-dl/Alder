/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QCodeDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QCodeDialog.h>
#include <QMessageBox>
#include <ui_QCodeDialog.h>
#include <vtkSmartPointer.h>
#include <CodeGroup.h>
#include <CodeType.h>
#include <ScanType.h>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QCodeDialog::QCodeDialog( QWidget* parent )
  : QDialog( parent )
{
  this->ui = new Ui_QCodeDialog;
  this->ui->setupUi( this );

  std::vector< vtkSmartPointer< Alder::ScanType > > scanTypeList;
  Alder::ScanType::GetAll( &scanTypeList );

  // list of all scan types that can have a code
  for( auto it = scanTypeList.begin(); it != scanTypeList.end(); ++it )
  {
    Alder::ScanType *scanType = (*it);
    this->ui->scanTypeComboBox->addItem(
      scanType->Get("Type").ToString().c_str(),
      QVariant(scanType->Get("Id").ToInt()) );
  }

  this->ui->codeGroupComboBox->addItem("",QVariant(-1));
  this->ui->codeGroupComboBox->setCurrentIndex(0);
  this->ui->groupValueSpinBox->setMinimum(-5);
  this->ui->groupValueSpinBox->setMaximum(0);
  this->ui->groupValueSpinBox->setSingleStep(-1);

  this->ui->codeValueSpinBox->setMinimum(-5);
  this->ui->codeValueSpinBox->setMaximum(0);
  this->ui->codeValueSpinBox->setSingleStep(-1);

  this->ui->codeTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );
  this->ui->codeTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  this->ui->groupTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );
  this->ui->groupTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );

  QHeaderView* header = this->ui->codeTableWidget->horizontalHeader();
  header->setClickable( true );
  QObject::connect(
    header, SIGNAL( sectionDoubleClicked(int) ),
    this, SLOT( slotHeaderClicked( int ) ) );
  header = this->ui->groupTableWidget->horizontalHeader();
  header->setClickable( true );
  QObject::connect(
    header, SIGNAL( sectionDoubleClicked( int ) ),
    this, SLOT( slotHeaderClicked( int ) ) );
   
  QObject::connect(  
    this->ui->groupTableWidget, SIGNAL( itemSelectionChanged() ),
    this, SLOT( slotSelectionChanged() ) );
    
  QObject::connect(  
    this->ui->groupTableWidget, SIGNAL( itemDoubleClicked(QTableWidgetItem*) ),
    this, SLOT( slotTableDoubleClicked() ) );

  QObject::connect(
    this->ui->tabWidget, SIGNAL( currentChanged(int)),
    this, SLOT( slotTabChanged() ) );

  QObject::connect(
    this->ui->groupAddPushButton, SIGNAL( clicked() ),
    this, SLOT( slotGroupAdd() ) );

  this->lastSelectedGroup = NULL;
  this->ui->codeAddPushButton->setEnabled(true);
  this->ui->codeEditPushButton->setEnabled(false);
  this->ui->codeRemovePushButton->setEnabled(false);
  this->ui->groupAddPushButton->setEnabled(true);
  this->ui->groupEditPushButton->setEnabled(false);
  this->ui->groupRemovePushButton->setEnabled(false);

  QRegExp grx("\\S+");
  QValidator* gv = new QRegExpValidator(grx,this);
  this->ui->groupLineEdit->setValidator(gv);
  QRegExp crx("[A-Z,a-z]{1,3}");
  QValidator* cv = new QRegExpValidator(crx,this);
  this->ui->codeLineEdit->setValidator(cv);

  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QCodeDialog::~QCodeDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotHeaderClicked(int index)
{
  QHeaderView* header = qobject_cast<QHeaderView*>(sender());
  if(header && header->parentWidget()==this->ui->codeTableWidget)
  {
    Qt::SortOrder order = this->codeTableSortColumnOrder[index];
    order = ( Qt::AscendingOrder == order ) ? Qt::DescendingOrder : Qt::AscendingOrder;
    this->ui->codeTableWidget->sortByColumn( index, order );
    this->codeTableSortColumnOrder[index] = order;
  }
  else
  {
    Qt::SortOrder order = this->groupTableSortColumnOrder[index];
    order = ( Qt::AscendingOrder == order ) ? Qt::DescendingOrder : Qt::AscendingOrder;
    this->ui->groupTableWidget->sortByColumn( index, order );
    this->groupTableSortColumnOrder[index] = order;
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::updateInterface()
{
  this->updateCodeTable();
  this->updateGroupTable();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::updateCodeTable()
{
  this->ui->codeTableWidget->blockSignals( true );
  this->ui->codeTableWidget->setSortingEnabled(false);
  this->ui->codeTableWidget->setRowCount(0);

  QStringList columns;
  columns << "Type" << "Code" << "Value" << "Usage" << "Group" << "Group Value" << "Active";
  for( int i=0; i < columns.size(); ++i )
  {
    this->codeTableSortColumnOrder[i] = Qt::AscendingOrder;
    this->codeTableColumnIndex[columns.at(i).toStdString()]=i;
  }
  this->ui->codeTableWidget->setHorizontalHeaderLabels(columns);

  QTableWidgetItem *item;

  vtkSmartPointer< Alder::QueryModifier > modifier = vtkSmartPointer< Alder::QueryModifier >::New();
  modifier->Join( "CodeType", "CodeType.Id", "ScanTypeHasCodeType.CodeTypeId" );
  std::string override = "ScanTypeHasCodeType";

  std::vector< vtkSmartPointer< Alder::CodeType > > codeTypeList;
  Alder::CodeType::GetAll( &codeTypeList );
  for( auto it = codeTypeList.begin(); it != codeTypeList.end(); ++it )
  { // for every codeType, add a new row
    Alder::CodeType *codeType = (*it);

    std::vector< vtkSmartPointer< Alder::ScanType > > scanTypeList;
    codeType->GetList( &scanTypeList, modifier, override );
    QStringList typeList;
    if( !scanTypeList.empty() )
    {
      for( auto sit = scanTypeList.begin(); sit != scanTypeList.end(); ++sit )
        typeList << (*sit)->Get("Type").ToString().c_str();
    }
    else
    {
      typeList << "NA";
    }

    QStringList::const_iterator sit;
    for( sit = typeList.constBegin(); sit != typeList.constEnd(); ++sit )
    {
      this->ui->codeTableWidget->insertRow( 0 );

      // add Type to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( (*sit) );
      this->ui->codeTableWidget->setItem( 0, this->codeTableColumnIndex["Type"], item );

      // add Code to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( QString( codeType->Get( "Code" ).ToString().c_str() ) );
      this->ui->codeTableWidget->setItem( 0, this->codeTableColumnIndex["Code"], item );

      // add Value to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( QString( codeType->Get( "Value" ).ToString().c_str() ) );
      this->ui->codeTableWidget->setItem( 0, this->codeTableColumnIndex["Value"], item );

      // add Active to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
      item->setCheckState( 0 < codeType->Get( "Active" ).ToInt() ? Qt::Checked : Qt::Unchecked );
      this->ui->codeTableWidget->setItem( 0, this->codeTableColumnIndex["Active"], item );

      // add Usage to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( QString::number( codeType->GetUsage() ) );
      this->ui->codeTableWidget->setItem( 0, this->codeTableColumnIndex["Usage"], item );

      vtkSmartPointer< Alder::CodeGroup > group;
      if( codeType->GetRecord( group ) )
      {
        // add Group to row
        item = new QTableWidgetItem;
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        item->setText( QString( group->Get( "Name" ).ToString().c_str() ) );
        this->ui->codeTableWidget->setItem( 0, this->codeTableColumnIndex["Group"], item );

        // add Group Value to row
        item = new QTableWidgetItem;
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        item->setText( QString( group->Get( "Value" ).ToString().c_str() ) );
        this->ui->codeTableWidget->setItem( 0, this->codeTableColumnIndex["Group Value"], item );
      }
    }
  }

  this->ui->codeTableWidget->setSortingEnabled(true);
  this->ui->codeTableWidget->blockSignals( false );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::updateGroupTable()
{
  this->ui->groupTableWidget->blockSignals( true );
  this->ui->groupTableWidget->setSortingEnabled(false);
  this->ui->groupTableWidget->setRowCount(0);

  QStringList columns;
  int index=0;
  columns.clear();
  columns << "Group" << "Group Value" << "Usage";
  for( int i=0; i < columns.size(); ++i )
  {
    this->groupTableSortColumnOrder[i] = Qt::AscendingOrder;
    this->groupTableColumnIndex[columns.at(i).toStdString()]=i;
  }
  this->ui->groupTableWidget->setHorizontalHeaderLabels(columns);

  QTableWidgetItem *item;

  std::vector< vtkSmartPointer< Alder::CodeGroup > > codeGroupList;
  Alder::CodeGroup::GetAll( &codeGroupList );

  // list of all groups
  this->ui->codeGroupComboBox->clear();
  this->ui->codeGroupComboBox->addItem("",QVariant(-1));
  this->ui->codeGroupComboBox->setCurrentIndex(0);
  for( auto it = codeGroupList.begin(); it != codeGroupList.end(); ++it )
  {
    Alder::CodeGroup *group = (*it);
    QString name = group->Get( "Name" ).ToString().c_str();
    this->ui->groupTableWidget->insertRow( 0 );

    // add Group to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    item->setText( name );
    this->ui->groupTableWidget->setItem( 0, this->groupTableColumnIndex["Group"], item );

    // add Group Value to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    item->setText( QString( group->Get( "Value" ).ToString().c_str() ) );
    this->ui->groupTableWidget->setItem( 0, this->groupTableColumnIndex["Group Value"], item );

    // add Usage to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    item->setText( QString::number( group->GetUsage() ) );
    this->ui->groupTableWidget->setItem( 0, this->groupTableColumnIndex["Usage"], item );

    this->ui->codeGroupComboBox->addItem(
      name, QVariant(group->Get("Id").ToInt()) );
  }

  this->ui->groupTableWidget->setSortingEnabled(true);
  this->ui->groupTableWidget->blockSignals( false );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotTabChanged()
{
  this->ui->groupTableWidget->clearSelection();
  this->ui->codeTableWidget->clearSelection();
  this->ui->codeAddPushButton->setEnabled(true);
  this->ui->codeEditPushButton->setEnabled(false);
  this->ui->codeRemovePushButton->setEnabled(false);
  this->ui->groupAddPushButton->setEnabled(true);
  this->ui->groupEditPushButton->setEnabled(false);
  this->ui->groupRemovePushButton->setEnabled(false);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotTableDoubleClicked()
{
  this->ui->groupTableWidget->clearSelection();
  this->ui->groupAddPushButton->setEnabled(true);
  this->ui->groupEditPushButton->setEnabled(false);
  this->ui->groupRemovePushButton->setEnabled(false);
  std::cout << "double click" << std::endl;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotGroupAdd()
{
  // add if the group name is filled and the value is non-zero
  int value = this->ui->groupValueSpinBox->value();
  QString name = this->ui->groupLineEdit->text().trimmed();
  // make sure the proposed group name is unique
  if( name.isEmpty() || 0 == value ) return;
  
  bool unique = Alder::CodeGroup::IsUnique(name.toStdString(), value);
  if(unique)
  {
    std::cout << name.toStdString().c_str() << ", " << value << std::endl;
    vtkNew<Alder::CodeGroup> group;
    group->Set("Name",name.toStdString());
    group->Set("Value",value);
    group->Save();
    this->updateInterface();
  }
  else
  {
    QMessageBox errorMessage( this );
    errorMessage.setWindowModality( Qt::WindowModal );
    errorMessage.setIcon( QMessageBox::Warning );
    std::string msg = "Invalid code group: name and value are not unique";
    errorMessage.setText( tr( msg.c_str() ) );
    errorMessage.exec();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotSelectionChanged()
{
  QList< QTableWidgetItem*> items = this->ui->groupTableWidget->selectedItems();
  /*
    if( items.first() == this->lastSelectedGroup )
    {
      this->ui->groupTableWidget->clearSelection();
      this->lastSelectedGroup = NULL;
    }
  */  
  bool selected = !items.empty();
  this->ui->groupAddPushButton->setEnabled( !selected );
  this->ui->groupEditPushButton->setEnabled( selected );
  this->ui->groupRemovePushButton->setEnabled( selected );
  std::cout << "select changed " << (selected ? "yes":"no") << std::endl;

  if( selected )
  {
    // get the selected row
    // set the spinbox value
    // set the active drop down name item
   
    /*
    int uiCol = this->columnIndex["UId"];
    int dateCol = this->columnIndex["VisitDate"];

    vtkSmartPointer< Alder::Interview > interview =
      vtkSmartPointer< Alder::Interview >::New();

    std::map< std::string, std::string > map;

    for( QList< QTableWidgetSelectionRange >::const_iterator it =
         ranges.constBegin(); it != ranges.constEnd(); ++it )
    {
      for( int row = (*it).topRow(); row <= (*it).bottomRow(); ++row )
      {
        QTableWidgetItem* item = this->ui->interviewTableWidget->item( row, uiCol );
        map["UId"] = item->text().toStdString();

        item = this->ui->interviewTableWidget->item( row, dateCol );
        map["VisitDate"] = item->text().toStdString();

        interview->Load( map );

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
   */
  }
}

