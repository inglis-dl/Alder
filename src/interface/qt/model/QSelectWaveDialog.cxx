/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QSelectWaveDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QSelectWaveDialog.h>
#include <ui_QSelectWaveDialog.h>

#include <QTableWidgetItem>

#include <Application.h>
#include <Wave.h>

#include <vtkSmartPointer.h>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSelectWaveDialog::QSelectWaveDialog( QWidget* parent )
  : QDialog( parent )
{
  this->ui = new Ui_QSelectWaveDialog;
  this->ui->setupUi( this );

  QObject::connect(
    this->ui->buttonBox, SIGNAL( accepted() ),
    this, SLOT( slotAccepted() ) );

  int index = 0;
  QStringList labels;
  labels << "Name";
  this->columnIndex[labels.last().toStdString()] = index++;
  labels << "Rank";
  this->columnIndex[labels.last().toStdString()] = index++;
  labels << "Selected";
  this->columnIndex[labels.last().toStdString()] = index++;
  labels << "Update";
  this->columnIndex[labels.last().toStdString()] = index++;

  this->ui->waveTableWidget->setHorizontalHeaderLabels( labels );
  QHeaderView* header = this->ui->waveTableWidget->horizontalHeader();
  header->setResizeMode( QHeaderView::Stretch );
  header->setClickable( true );
  header->setResizeMode( QHeaderView::ResizeToContents );
  header->setStretchLastSection( true );

  this->ui->waveTableWidget->verticalHeader()->setVisible( false );
  this->ui->waveTableWidget->setSelectionBehavior( QAbstractItemView::SelectItems );
  this->ui->waveTableWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );

  this->sortColumn = 1;
  this->sortOrder = Qt::AscendingOrder;

  QObject::connect(
    this->ui->waveTableWidget->horizontalHeader(), SIGNAL( sectionClicked( int ) ),
    this, SLOT( slotHeaderClicked( int ) ) );

  std::vector< vtkSmartPointer< Alder::Wave > > waveList;
  Alder::Wave::GetAll( &waveList );
  this->ui->waveTableWidget->setRowCount( 0 );
  QTableWidgetItem *item;
  for( auto it = waveList.begin(); it != waveList.end(); ++it )
  {
    Alder::Wave *wave = *it;
    QString rankStr = wave->Get( "Rank" ).ToString().c_str();
    QString nameStr = wave->Get( "Name" ).ToString().c_str();
    QVariant vId = wave->Get( "Id" ).ToInt();
    this->ui->waveTableWidget->insertRow( 0 );

    // add name to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    item->setText( nameStr );
    this->ui->waveTableWidget->setItem( 0, this->columnIndex["Name"], item );

    // add rank to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    item->setText( rankStr );
    this->ui->waveTableWidget->setItem( 0, this->columnIndex["Rank"], item );

    // add select checkbox to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
    item->setCheckState( Qt::Unchecked );
    item->setData( Qt::UserRole, vId );
    this->ui->waveTableWidget->setItem( 0, this->columnIndex["Selected"], item );

    // add update checkbox to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsUserCheckable );
    item->setCheckState( Qt::Unchecked );
    item->setData( Qt::UserRole, vId );
    this->ui->waveTableWidget->setItem( 0, this->columnIndex["Update"], item );
  }

  QObject::connect(
    this->ui->waveTableWidget, SIGNAL( itemPressed( QTableWidgetItem *) ),
    this, SLOT( slotItemPressed( QTableWidgetItem *) ) );
  QObject::connect(
    this->ui->waveTableWidget, SIGNAL( itemClicked( QTableWidgetItem *) ),
    this, SLOT( slotItemClicked( QTableWidgetItem *) ) );

  this->lastItemPressedState = Qt::Unchecked;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSelectWaveDialog::~QSelectWaveDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectWaveDialog::slotItemPressed( QTableWidgetItem *item )
{
  // only check selected column items
  int column = this->ui->waveTableWidget->column( item );
  if( column == this->columnIndex["Selected"] )
    this->lastItemPressedState = item->checkState();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectWaveDialog::slotItemClicked( QTableWidgetItem *item )
{
  // only check selected column items
  int column = this->ui->waveTableWidget->column( item );
  if( column == this->columnIndex["Selected"] )
  {
    int row = this->ui->waveTableWidget->row( item );
    if( this->lastItemPressedState != item->checkState() )
    {
      QTableWidgetItem *companion = this->ui->waveTableWidget->item( row, this->columnIndex["Update"] );
      Qt::ItemFlags flags;
      if( Qt::Checked == item->checkState() )
        flags |= ( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
      else
        flags |= Qt::ItemIsUserCheckable;
      companion->setFlags( flags );
      companion->setCheckState( Qt::Unchecked );
      this->lastItemPressedState = item->checkState();
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectWaveDialog::slotHeaderClicked( int index )
{
  // reverse order if already sorted
  if( this->sortColumn == index )
    this->sortOrder = Qt::AscendingOrder == this->sortOrder ? Qt::DescendingOrder : Qt::AscendingOrder;

  this->sortColumn = index;
  this->ui->waveTableWidget->sortItems( this->sortColumn, this->sortOrder );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectWaveDialog::slotAccepted()
{
  this->selection.clear();
  for( int i = 0; i < this->ui->waveTableWidget->rowCount(); ++i )
  {
    QTableWidgetItem* item = this->ui->waveTableWidget->item( i, this->columnIndex["Selected"] );
    if( Qt::Checked == item->checkState() )
    {
      QTableWidgetItem *companion = this->ui->waveTableWidget->item( i, this->columnIndex["Update"] );
      this->selection.push_back(
        std::make_pair(
          item->data( Qt::UserRole ).toInt(),
          Qt::Checked == companion->checkState()
          )
        );
    }
  }
}
