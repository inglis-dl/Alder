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

  QStringList tables;
  tables << "CodeType" << "CodeGroup";
  for( int i = 0; i < tables.size(); ++i )
  {
    this->sortOrder[ tables[i].toStdString() ];
    this->columnIndex[ tables[i].toStdString() ];
  }

  std::vector< vtkSmartPointer< Alder::ScanType > > scanTypeList;
  Alder::ScanType::GetAll( &scanTypeList );

  // list of all scan types that can have a code
  for( auto it = scanTypeList.begin(); it != scanTypeList.end(); ++it )
  {
    Alder::ScanType *scanType = (*it);
    this->ui->scanTypeComboBox->addItem(
      scanType->Get( "Type" ).ToString().c_str(),
      QVariant(scanType->Get( "Id" ).ToInt()) );
  }

  this->ui->codeGroupComboBox->addItem( "", QVariant( -1 ) );
  this->ui->codeGroupComboBox->setCurrentIndex( 0 );

  this->ui->groupValueSpinBox->setMinimum( -5 );
  this->ui->groupValueSpinBox->setMaximum( 0 );
  this->ui->groupValueSpinBox->setSingleStep( -1 );

  this->ui->codeValueSpinBox->setMinimum( -5 );
  this->ui->codeValueSpinBox->setMaximum( 0 );
  this->ui->codeValueSpinBox->setSingleStep( -1 );

  this->ui->codeTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );
  this->ui->codeTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );

  this->ui->groupTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );
  this->ui->groupTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );

  QHeaderView* header = this->ui->codeTableWidget->horizontalHeader();
  header->setClickable( true );
  header->setResizeMode( QHeaderView::ResizeToContents );
  header->setStretchLastSection( true );

  QObject::connect(
    header, SIGNAL( sectionDoubleClicked(int) ),
    this, SLOT( slotHeaderClicked( int ) ) );
  header = this->ui->groupTableWidget->horizontalHeader();

  header->setClickable( true );
  header->setResizeMode( QHeaderView::ResizeToContents );
  header->setStretchLastSection( true );

  QObject::connect(
    header, SIGNAL( sectionDoubleClicked( int ) ),
    this, SLOT( slotHeaderClicked( int ) ) );

  QObject::connect(
    this->ui->closePushButton, SIGNAL( clicked( bool ) ),
    this, SLOT( slotClose() ) );

  QObject::connect(
    this->ui->groupTableWidget, SIGNAL( itemDoubleClicked( QTableWidgetItem* ) ),
    this, SLOT( slotTableDoubleClicked() ) );

  QObject::connect(
    this->ui->codeTableWidget, SIGNAL( itemDoubleClicked( QTableWidgetItem* ) ),
    this, SLOT( slotTableDoubleClicked() ) );

  QObject::connect(
    this->ui->tabWidget, SIGNAL( currentChanged( int ) ),
    this, SLOT( slotTabChanged() ) );

  QObject::connect(
    this->ui->groupTableWidget, SIGNAL( itemSelectionChanged() ),
    this, SLOT( slotGroupSelectionChanged() ) );

  QObject::connect(
    this->ui->groupAddPushButton, SIGNAL( clicked() ),
    this, SLOT( slotGroupAdd() ) );

  QObject::connect(
    this->ui->groupEditPushButton, SIGNAL( clicked() ),
    this, SLOT( slotGroupEdit() ) );

  QObject::connect(
    this->ui->groupRemovePushButton, SIGNAL( clicked() ),
    this, SLOT( slotGroupRemove() ) );

  QObject::connect(
    this->ui->groupApplyPushButton, SIGNAL( clicked() ),
    this, SLOT( slotGroupApply() ) );

  QObject::connect(
    this->ui->codeTableWidget, SIGNAL( itemSelectionChanged() ),
    this, SLOT( slotCodeSelectionChanged() ) );

  QObject::connect(
    this->ui->codeAddPushButton, SIGNAL( clicked() ),
    this, SLOT( slotCodeAdd() ) );

  QObject::connect(
    this->ui->codeEditPushButton, SIGNAL( clicked() ),
    this, SLOT( slotCodeEdit() ) );

  QObject::connect(
    this->ui->codeRemovePushButton, SIGNAL( clicked() ),
    this, SLOT( slotCodeRemove() ) );

  QObject::connect(
    this->ui->codeApplyPushButton, SIGNAL( clicked() ),
    this, SLOT( slotCodeApply() ) );

  QRegExp grx("\\S+");
  QValidator* gv = new QRegExpValidator( grx, this );
  this->ui->groupLineEdit->setValidator( gv );

  QRegExp crx("[A-Z,a-z]{1,3}");
  QValidator* cv = new QRegExpValidator( crx, this );
  this->ui->codeLineEdit->setValidator( cv );

  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QCodeDialog::~QCodeDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotClose()
{
  this->accept();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotHeaderClicked( int index )
{
  QHeaderView* header = qobject_cast<QHeaderView*>(sender());
  if( !header ) return;
  QTableWidget* table = qobject_cast<QTableWidget*>(header->parentWidget());
  std::string name = table == this->ui->codeTableWidget ? "CodeType" : "CodeGroup";

  Qt::SortOrder order = this->sortOrder[name][index];
  order = ( Qt::AscendingOrder == order ) ? Qt::DescendingOrder : Qt::AscendingOrder;
  table->sortByColumn( index, order );
  this->sortOrder[name][index] = order;
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
  this->ui->codeTableWidget->setSortingEnabled( false );
  this->ui->codeTableWidget->setRowCount( 0 );

  QStringList columns;
  std::string name = "CodeType";
  columns << "Type" << "Code" << "Value" << "Usage" << "Group" << "Group Value" << "Active";
  for( int i = 0; i < columns.size(); ++i )
  {
    this->sortOrder[name][i] = Qt::AscendingOrder;
    this->columnIndex[name][ columns.at( i ).toStdString() ] = i;
  }
  this->ui->codeTableWidget->setHorizontalHeaderLabels( columns );

  // prototype for cloning center aligned cells
  QTableWidgetItem* proto = new QTableWidgetItem();
  proto->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  proto->setTextAlignment( Qt::AlignCenter );
  QTableWidgetItem *item;

  vtkSmartPointer< Alder::CodeGroup > codeGroup;

  vtkSmartPointer< Alder::QueryModifier > modifier = vtkSmartPointer< Alder::QueryModifier >::New();
  modifier->Join( "ScanTypeHasCodeType", "ScanType.Id", "ScanTypeHasCodeType.ScanTypeId" );
  modifier->Group( "ScanType.Id" );

  std::vector< vtkSmartPointer< Alder::ScanType > > scanTypeList;
  Alder::ScanType::GetAll( &scanTypeList, modifier );

  modifier->Reset();
  modifier->Join( "ScanType", "ScanType.Id", "ScanTypeHasCodeType.ScanTypeId" );
  std::string override = "ScanTypeHasCodeType";

  for( auto it = scanTypeList.begin(); it != scanTypeList.end(); ++it )
  { // for every scanType, add a new row
    Alder::ScanType *scanType = (*it);

    QVariant scanTypeId = scanType->Get( "Id" ).ToInt();
    QString type = scanType->Get( "Type" ).ToString().c_str();

    // get all the code types associated with this scan type
    std::vector< vtkSmartPointer< Alder::CodeType > > codeTypeList;
    (*it)->GetList( &codeTypeList, modifier, override );
    for( auto cit = codeTypeList.begin(); cit != codeTypeList.end(); ++cit )
    {
      Alder::CodeType *codeType = (*cit);
      QVariant codeTypeId = codeType->Get( "Id" ).ToInt();
      QString code = codeType->Get( "Code" ).ToString().c_str();
      int value = codeType->Get( "Value" ).ToInt();
      QString active = 0 < codeType->Get( "Active" ).ToInt() ? "Yes" : "No";
      int usage = codeType->GetUsage();
      QString groupName;
      int groupValue;
      QVariant groupId;
      bool hasGroup = codeType->GetRecord( codeGroup );
      if( hasGroup )
      {
        groupId = codeGroup->Get( "Id" ).ToInt();
        groupName = codeGroup->Get( "Name" ).ToString().c_str();
        groupValue = codeGroup->Get( "Value" ).ToInt();
      }

      this->ui->codeTableWidget->insertRow( 0 );

      // add Type to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( type );
      item->setData( Qt::UserRole, scanTypeId );
      this->ui->codeTableWidget->setItem( 0, this->columnIndex[name]["Type"], item );

      // add Code to row
      item = proto->clone();
      item->setText( code );
      item->setData( Qt::UserRole, codeTypeId );
      this->ui->codeTableWidget->setItem( 0, this->columnIndex[name]["Code"], item );

      // add Value to row
      item = proto->clone();
      item->setData( Qt::DisplayRole, value );
      this->ui->codeTableWidget->setItem( 0, this->columnIndex[name]["Value"], item );

      // add Active to row
      item = proto->clone();
      item->setText( active );
      this->ui->codeTableWidget->setItem( 0, this->columnIndex[name]["Active"], item );

      // add Usage to row
      item = proto->clone();
      item->setData( Qt::DisplayRole, usage );
      this->ui->codeTableWidget->setItem( 0, this->columnIndex[name]["Usage"], item );

      if( hasGroup )
      {
        // add Group to row
        item = new QTableWidgetItem;
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        item->setText( groupName );
        item->setData( Qt::UserRole, groupId );
        this->ui->codeTableWidget->setItem( 0, this->columnIndex[name]["Group"], item );

        // add Group Value to row
        item = proto->clone();
        item->setData( Qt::DisplayRole, groupValue );
        this->ui->codeTableWidget->setItem( 0, this->columnIndex[name]["Group Value"], item );
      }
      else
      {
        // add Group to row
        item = new QTableWidgetItem;
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        this->ui->codeTableWidget->setItem( 0, this->columnIndex[name]["Group"], item );

        // add Group Value to row
        item = proto->clone();
        this->ui->codeTableWidget->setItem( 0, this->columnIndex[name]["Group Value"], item );
      }
    }
  }

  this->ui->codeAddPushButton->setEnabled( true );
  this->ui->codeEditPushButton->setEnabled( false );
  this->ui->codeRemovePushButton->setEnabled( false );
  this->ui->codeApplyPushButton->setEnabled( false );
  this->ui->codeLineEdit->clear();
  this->ui->codeValueSpinBox->setValue( 0 );
  this->ui->codeActiveCheckBox->setCheckState( Qt::Unchecked );
  this->ui->scanTypeComboBox->setCurrentIndex( -1 );
  this->ui->codeGroupComboBox->setCurrentIndex( -1 );

  this->ui->codeTableWidget->setSortingEnabled( true );
  this->ui->codeTableWidget->blockSignals( false );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::updateGroupTable()
{
  this->ui->groupTableWidget->blockSignals( true );
  this->ui->groupTableWidget->setSortingEnabled( false );
  this->ui->groupTableWidget->setRowCount( 0 );

  QStringList columns;
  columns << "Group" << "Group Value" << "Usage";
  std::string name = "CodeGroup";
  for( int i = 0; i < columns.size(); ++i )
  {
    this->sortOrder[name][i] = Qt::AscendingOrder;
    this->columnIndex[name][ columns.at( i ).toStdString() ] = i;
  }

  // the last column indexes a dummy column of empty cells to stretch
  // the table on
  int lastColumn = columns.size();

  this->ui->groupTableWidget->setHorizontalHeaderLabels( columns );
  this->ui->codeGroupComboBox->clear();
  this->ui->codeGroupComboBox->addItem( "", QVariant( -1 ) );
  this->ui->codeGroupComboBox->setCurrentIndex( 0 );

  // prototype for cloning center aligned cells
  QTableWidgetItem* proto = new QTableWidgetItem();
  proto->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  proto->setTextAlignment( Qt::AlignCenter );
  QTableWidgetItem *item;

  // list of all groups
  std::vector< vtkSmartPointer< Alder::CodeGroup > > codeGroupList;
  Alder::CodeGroup::GetAll( &codeGroupList );
  for( auto it = codeGroupList.begin(); it != codeGroupList.end(); ++it )
  {
    Alder::CodeGroup *codeGroup = (*it);
    QString groupName = codeGroup->Get( "Name" ).ToString().c_str();
    this->ui->groupTableWidget->insertRow( 0 );
    QVariant id = QVariant( codeGroup->Get( "Id" ).ToInt() );

    // add Group to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    item->setText( groupName );
    item->setData( Qt::UserRole, id );
    this->ui->groupTableWidget->setItem( 0, this->columnIndex[name]["Group"], item );

    // add Group Value to row
    item = proto->clone();
    item->setData( Qt::DisplayRole, codeGroup->Get( "Value" ).ToInt() );
    this->ui->groupTableWidget->setItem( 0, this->columnIndex[name]["Group Value"], item );

    // add Usage to row
    item = proto->clone();
    item->setData( Qt::DisplayRole, codeGroup->GetUsage() );
    this->ui->groupTableWidget->setItem( 0, this->columnIndex[name]["Usage"], item );

    // add a dummy column to expand out on
    item = new QTableWidgetItem;
    this->ui->groupTableWidget->setItem( 0, lastColumn, item );

    this->ui->codeGroupComboBox->addItem( groupName, id );
  }

  this->ui->groupAddPushButton->setEnabled( true );
  this->ui->groupEditPushButton->setEnabled( false );
  this->ui->groupRemovePushButton->setEnabled( false );
  this->ui->groupApplyPushButton->setEnabled( false );
  this->ui->groupLineEdit->clear();
  this->ui->groupValueSpinBox->setValue( 0 );

  this->ui->groupTableWidget->setSortingEnabled( true );
  this->ui->groupTableWidget->blockSignals( false );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotTabChanged()
{
  this->ui->groupTableWidget->clearSelection();
  this->ui->codeTableWidget->clearSelection();

  this->ui->codeAddPushButton->setEnabled( true );
  this->ui->codeEditPushButton->setEnabled( false );
  this->ui->codeRemovePushButton->setEnabled( false );
  this->ui->codeApplyPushButton->setEnabled( false );

  this->ui->groupAddPushButton->setEnabled( true );
  this->ui->groupEditPushButton->setEnabled( false );
  this->ui->groupRemovePushButton->setEnabled( false );
  this->ui->groupApplyPushButton->setEnabled( false );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotTableDoubleClicked()
{
  QTableWidget* widget = qobject_cast<QTableWidget*>( sender() );
  if( widget && widget == this->ui->codeTableWidget )
  {
    this->ui->codeTableWidget->clearSelection();
    this->ui->codeAddPushButton->setEnabled( true );
    this->ui->codeEditPushButton->setEnabled( false );
    this->ui->codeRemovePushButton->setEnabled( false );
    this->ui->codeApplyPushButton->setEnabled( false );
  }
  else
  {
    this->ui->groupTableWidget->clearSelection();
    this->ui->groupAddPushButton->setEnabled( true );
    this->ui->groupEditPushButton->setEnabled( false );
    this->ui->groupRemovePushButton->setEnabled( false );
    this->ui->groupApplyPushButton->setEnabled( false );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotGroupAdd()
{
  // add if group name is filled
  int value = this->ui->groupValueSpinBox->value();
  std::string groupName = this->ui->groupLineEdit->text().trimmed().toStdString();
  if( groupName.empty() ) return;

  // ensure proposed CodeGroup Name and Value are unique
  bool unique = Alder::CodeGroup::IsUnique( groupName, value );
  if( unique )
  {
    vtkNew<Alder::CodeGroup> codeGroup;
    codeGroup->Set( "Name", groupName );
    codeGroup->Set( "Value", value );
    codeGroup->Save();
    this->updateGroupTable();
  }
  else
  {
    QString title = "Invalid Code Group";
    QString text = "Name and value are not unique";
    QMessageBox::warning( this, title, text );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotGroupRemove()
{
  // a group cannot be removed if it has a non-zero usage count
  QList< QTableWidgetItem* > items = this->ui->groupTableWidget->selectedItems();
  bool selected = !items.empty();
  if( selected )
  {
    int id = items.first()->data( Qt::UserRole ).toInt();
    vtkNew<Alder::CodeGroup> codeGroup;
    codeGroup->Load( "Id", id );

    QString title = "Confirm Group Removal";
    QString text = "Remove ";
    text += codeGroup->Get( "Name" ).ToString().c_str();
    text += " group?";
    QMessageBox::StandardButton reply = QMessageBox::question( this,
      title, text, QMessageBox::Yes|QMessageBox::No );
    if( QMessageBox::Yes == reply )
    {
      codeGroup->Remove();
      this->updateGroupTable();
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotGroupApply()
{
  // get the selected item
  QList< QTableWidgetItem* > items = this->ui->groupTableWidget->selectedItems();
  bool selected = !items.empty();
  QString groupName = this->ui->groupLineEdit->text();

  if( selected && !groupName.isEmpty() )
  {
    int value = this->ui->groupValueSpinBox->value();

    // ensure proposed CodeGroup Name and Value are unique
    bool unique = Alder::CodeGroup::IsUnique( groupName.toStdString(), value );
    if( unique )
    {
      std::string name = "CodeGroup";
      QTableWidgetItem* first = items.first();
      int id = first->data( Qt::UserRole ).toInt();
      vtkNew<Alder::CodeGroup> codeGroup;
      codeGroup->Load( "Id", id );

      QString lastName = first->text();
      int lastValue = items.at( this->columnIndex[name]["Group Value"] )->data( Qt::DisplayRole ).toInt();

      bool modified = false;

      if( lastName != groupName )
      {
        codeGroup->Set( "Name",  groupName.toStdString() );
        first->setText( groupName );
        modified = true;
      }
      if( lastValue != value )
      {
        codeGroup->Set( "Value", value );
        items.at( this->columnIndex[name]["Group Value"] )->setData( Qt::DisplayRole, value );
        modified = true;
      }

      if( modified )
      {
        codeGroup->Save();

        if( lastValue != value )
          codeGroup->UpdateRatings();

        int usage = codeGroup->GetUsage();
        items.at( this->columnIndex[name]["Usage"] )->setData( Qt::DisplayRole, usage );

        this->updateCodeTable();
      }
    }
    else
    {
      QString title = "Invalid Code Group";
      QString text = "Name and value are not unique";
      QMessageBox::warning( this, title, text );
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotGroupEdit()
{
  // get the selected item
  QList< QTableWidgetItem* > items = this->ui->groupTableWidget->selectedItems();
  bool selected = !items.empty();
  if( selected )
  {
    std::string name = "CodeGroup";
    this->ui->groupLineEdit->setText( items.at( this->columnIndex[name]["Group"] )->text() );
    this->ui->groupValueSpinBox->setValue( items.at( this->columnIndex[name]["Group Value"] )->data( Qt::DisplayRole ).toInt() );
    this->ui->groupApplyPushButton->setEnabled( true );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotGroupSelectionChanged()
{
  QList< QTableWidgetItem* > items = this->ui->groupTableWidget->selectedItems();
  bool selected = !items.empty();
  this->ui->groupAddPushButton->setEnabled( !selected );
  this->ui->groupEditPushButton->setEnabled( selected );
  this->ui->groupApplyPushButton->setEnabled( false );
  this->ui->groupLineEdit->clear();
  this->ui->groupValueSpinBox->setValue( 0 );

  if( selected )
  {
    int usage = items.at( this->columnIndex["CodeGroup"]["Usage"] )->data( Qt::DisplayRole ).toInt();
    bool enabled = 0 == usage;
    this->ui->groupRemovePushButton->setEnabled( enabled );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotCodeAdd()
{
  // add if code name is filled
  std::string code = this->ui->codeLineEdit->text().trimmed().toStdString();
  if( code.empty() ) return;

  int index = this->ui->scanTypeComboBox->currentIndex();
  int scanTypeId = -1;
  if( -1 != index )
    scanTypeId = this->ui->scanTypeComboBox->itemData( index ).toInt();

  if( -1 == scanTypeId )
  {
    QString title = "Invalid Code";
    QString text = "A Code must be associated with a Type";
    QMessageBox::warning( this, title, text );
    return;
  }

  int value = this->ui->codeValueSpinBox->value();

  index = this->ui->codeGroupComboBox->currentIndex();
  int groupId = -1;
  if( -1 != index )
    groupId = this->ui->codeGroupComboBox->itemData( index ).toInt();

  // ensure proposed CodeType Name, Value and GroupId are unique
  bool unique = Alder::CodeType::IsUnique( code, value, groupId );
  if( unique )
  {
    vtkNew<Alder::CodeType> codeType;
    codeType->Set( "Code", code );
    codeType->Set( "Value", value );
    if( -1 != groupId )
      codeType->Set( "CodeGroupId", groupId );
    codeType->Save();

    // get the ScanType and add the CodeType to the ScanTypeHasCodeType table
    vtkNew<Alder::ScanType> scanType;
    scanType->Load( "Id", scanTypeId );
    vtkSmartPointer<Alder::CodeType> ptr = codeType.GetPointer();
    scanType->AddRecord( ptr );

    this->updateCodeTable();
  }
  else
  {
    QString title = "Invalid Code";
    QString text = "Code, value and group are not unique";
    QMessageBox::warning( this, title, text );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotCodeRemove()
{
  // a code cannot be removed if it has a non-zero usage count
  QList< QTableWidgetItem* > items = this->ui->codeTableWidget->selectedItems();
  bool selected = !items.empty();
  if( selected )
  {
    std::string name = "CodeType";
    int id = items.at( this->columnIndex[name]["Code"] )->data( Qt::UserRole ).toInt();
    vtkNew<Alder::CodeType> codeType;
    codeType->Load( "Id", id );

    QString title = "Confirm Code Removal";
    QString text = "Remove ";
    text += codeType->Get( "Code" ).ToString().c_str();
    text += " code?";
    QMessageBox::StandardButton reply = QMessageBox::question( this,
      title, text, QMessageBox::Yes|QMessageBox::No );
    if( QMessageBox::Yes == reply )
    {
      int scanTypeId = items.at( this->columnIndex[name]["Type"] )->data( Qt::UserRole ).toInt();
      vtkNew<Alder::ScanType> scanType;
      scanType->Load( "Id", scanTypeId );
      vtkSmartPointer<Alder::CodeType> ptr = codeType.GetPointer();
      scanType->RemoveRecord( ptr );

      codeType->Remove();
      this->updateCodeTable();
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotCodeApply()
{
  // get the selected item
  QList< QTableWidgetItem* > items = this->ui->codeTableWidget->selectedItems();
  bool selected = !items.empty();
  QString code = this->ui->codeLineEdit->text();

  if( selected && !code.isEmpty() )
  {
    int value = this->ui->codeValueSpinBox->value();
    int index = this->ui->codeGroupComboBox->currentIndex();
    int groupId = -1;
    if( -1 != index )
      groupId = this->ui->codeGroupComboBox->itemData( index ).toInt();
    int groupValue = 0;
    int active = Qt::Checked == this->ui->codeActiveCheckBox->checkState() ? 1 : 0;

    // ensure proposed Code, Value, and CodeGroupId are unique
    bool unique = Alder::CodeType::IsUnique( code.toStdString(), value, groupId );
    if( unique )
    {
      std::string name = "CodeType";
      int id = items.at( this->columnIndex[name]["Code"] )->data( Qt::UserRole ).toInt();
      vtkNew<Alder::CodeType> codeType;
      codeType->Load( "Id", id );

      QString lastCode = items.at( this->columnIndex[name]["Code"] )->text();
      int lastValue = items.at( this->columnIndex[name]["Value"] )->data( Qt::DisplayRole ).toInt();
      int lastGroupId = items.at( this->columnIndex[name]["Group"] )->data( Qt::UserRole ).toInt();
      int lastGroupValue = items.at( this->columnIndex[name]["Group Value"] )->data( Qt::DisplayRole ).toInt();
      int lastActive = "Yes" == items.at( this->columnIndex[name]["Active"] )->text() ? 1 : 0;
      bool modified = false;

      if( lastCode != code )
      {
        codeType->Set( "Code",  code.toStdString() );
        items.at( this->columnIndex[name]["Code"] )->setText( code );
        modified = true;
      }
      if( lastValue != value )
      {
        codeType->Set( "Value", value );
        items.at( this->columnIndex[name]["Value"] )->setData( Qt::DisplayRole, value );
        modified = true;
      }
      if( lastActive != active )
      {
        codeType->Set( "Active", active );
        items.at( this->columnIndex[name]["Active"] )->setText( 1 == active ? "Yes" : "No" );
        modified = true;
      }
      if( lastGroupId != groupId )
      {
        if( -1 == groupId )
        {
          codeType->SetNull( "CodeGroupId" );
          items.at( this->columnIndex[name]["Group"] )->setText( "" );
          items.at( this->columnIndex[name]["Group Value"] )->setData( Qt::DisplayRole, "" );
        }
        else
        {
          codeType->Set( "CodeGroupId", groupId );
          vtkNew<Alder::CodeGroup> group;
          group->Load( "Id", groupId );
          groupValue = group->Get( "Value" ).ToInt();
          items.at( this->columnIndex[name]["Group"] )->setText( group->Get( "Name" ).ToString().c_str() );
          items.at( this->columnIndex[name]["Group Value"] )->setData( Qt::DisplayRole, groupValue );
        }
        modified = true;
      }

      if( modified )
      {
        codeType->Save();
        if( lastValue != value || lastGroupValue != groupValue )
          codeType->UpdateRatings();

        int usage = codeType->GetUsage();
        items.at( this->columnIndex[name]["Usage"] )->setData( Qt::DisplayRole, usage );

        this->updateCodeTable();
      }
    }
    else
    {
      QString title = "Invalid Code";
      QString text = "Code, Value and Group are not unique";
      QMessageBox::warning( this, title, text );
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotCodeEdit()
{
  // get the selected item
  QList< QTableWidgetItem* > items = this->ui->codeTableWidget->selectedItems();
  bool selected = !items.empty();
  if( selected )
  {
    std::string name = "CodeType";
    this->ui->codeLineEdit->setText( items.at( this->columnIndex[name]["Code"] )->text() );
    this->ui->codeValueSpinBox->setValue( items.at( this->columnIndex[name]["Value"] )->data( Qt::DisplayRole ).toInt() );
    QString group = items.at( this->columnIndex[name]["Group"] )->text();
    if( !group.isEmpty() )
    {
      int index = this->ui->codeGroupComboBox->findText( group );
      this->ui->codeGroupComboBox->setCurrentIndex( index );
    }
    QString scanType = items.at( this->columnIndex[name]["Type"] )->text();
    int index = this->ui->scanTypeComboBox->findText( scanType );
    this->ui->scanTypeComboBox->setCurrentIndex( index );
    Qt::CheckState checked = "Yes" == items.at( this->columnIndex[name]["Active"] )->text() ? Qt::Checked : Qt::Unchecked;
    this->ui->codeActiveCheckBox->setCheckState( checked );
    this->ui->codeApplyPushButton->setEnabled( true );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::slotCodeSelectionChanged()
{
  QList< QTableWidgetItem* > items = this->ui->codeTableWidget->selectedItems();
  bool selected = !items.empty();
  this->ui->codeAddPushButton->setEnabled( !selected );
  this->ui->codeEditPushButton->setEnabled( selected );
  this->ui->codeApplyPushButton->setEnabled( false );
  this->ui->codeLineEdit->clear();
  this->ui->codeValueSpinBox->setValue( 0 );
  this->ui->scanTypeComboBox->setCurrentIndex( -1 );
  this->ui->codeGroupComboBox->setCurrentIndex( -1 );
  this->ui->codeActiveCheckBox->setCheckState( Qt::Unchecked );

  if( selected )
  {
    int usage = items.at( this->columnIndex["CodeType"]["Usage"] )->data( Qt::DisplayRole ).toInt();
    bool enabled = 0 == usage;
    this->ui->codeRemovePushButton->setEnabled( enabled );
  }
}
