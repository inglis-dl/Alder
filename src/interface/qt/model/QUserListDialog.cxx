/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QUserListDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QUserListDialog.h>
#include <ui_QUserListDialog.h>

#include <Modality.h>
#include <User.h>

#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <QErrorMessage>
#include <QInputDialog>
#include <QMessageBox>
#include <QList>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <stdexcept>
#include <vector>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QUserListDialog::QUserListDialog( QWidget* parent )
  : QDialog( parent )
{
  int index = 0;
  this->ui = new Ui_QUserListDialog;
  this->ui->setupUi( this );
  QStringList labels;
  QHeaderView* header = this->ui->userTableWidget->horizontalHeader();
  header->setStretchLastSection( true );

  labels << "Name";
  this->columnIndex["Name"] = index++;
  header->setResizeMode(
    this->columnIndex["Name"], QHeaderView::Stretch );

  labels << "Last Login";
  this->columnIndex["LastLogin"] = index++;
  header->setResizeMode(
    this->columnIndex["LastLogin"], QHeaderView::Stretch );

  labels << "Expert";
  this->columnIndex["Expert"] = index++;
  header->setResizeMode(
    this->columnIndex["Expert"], QHeaderView::ResizeToContents );

  // list all modalities (to see if user rates them)
  std::vector< vtkSmartPointer< Alder::Modality > > modalityList;
  Alder::Modality::GetAll( &modalityList );

  this->ui->userTableWidget->setColumnCount( index + modalityList.size() );
  for( auto modalityListIt = modalityList.begin(); modalityListIt != modalityList.end(); ++modalityListIt )
  {
    std::string modalityName = (*modalityListIt)->Get( "Name" ).ToString();
    labels << modalityName.c_str();
    header->setResizeMode( index, QHeaderView::ResizeToContents );
    this->columnIndex[modalityName] = index++;
  }

  this->ui->userTableWidget->setHorizontalHeaderLabels( labels );
  this->ui->userTableWidget->horizontalHeader()->setClickable( true );
  this->ui->userTableWidget->verticalHeader()->setVisible( false );
  this->ui->userTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  this->ui->userTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );

  this->sortColumn = 0;
  this->sortOrder = Qt::AscendingOrder;

  QObject::connect(
    this->ui->addPushButton, SIGNAL( clicked( bool ) ),
    this, SLOT( slotAdd() ) );

  QObject::connect(
    this->ui->removePushButton, SIGNAL( clicked( bool ) ),
    this, SLOT( slotRemove() ) );

  QObject::connect(
    this->ui->resetPasswordPushButton, SIGNAL( clicked( bool ) ),
    this, SLOT( slotResetPassword() ) );

  QObject::connect(
    this->ui->closePushButton, SIGNAL( clicked( bool ) ),
    this, SLOT( slotClose() ) );

  QObject::connect(
    this->ui->userTableWidget, SIGNAL( itemSelectionChanged() ),
    this, SLOT( slotSelectionChanged() ) );

  QObject::connect(
    this->ui->userTableWidget->horizontalHeader(), SIGNAL( sectionClicked( int ) ),
    this, SLOT( slotHeaderClicked( int ) ) );

  QObject::connect(
    this->ui->userTableWidget, SIGNAL( itemChanged( QTableWidgetItem* ) ),
    this, SLOT( slotItemChanged( QTableWidgetItem* ) ) );

  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QUserListDialog::~QUserListDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::slotAdd()
{
  // get the new user's name
  std::string name = QInputDialog::getText(
    this,
    QObject::tr( "Create User" ),
    QObject::tr( "New user's name:" ),
    QLineEdit::Normal ).toStdString();

  if( 0 < name.size() )
  {
    // make sure the user name doesn't already exist
    vtkNew< Alder::User > user;
    if( user->Load( "Name", name ) )
    {
      std::stringstream stream;
      stream << "Unable to create new user \"" << name << "\", name already in use.";
      QErrorMessage *dialog = new QErrorMessage( this );
      dialog->setModal( true );
      dialog->showMessage( tr( stream.str().c_str() ) );
    }
    else
    {
      user->Set( "Name", name );
      user->ResetPassword();
      user->Save();
      this->updateInterface();
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::slotRemove()
{
  QList< QTableWidgetItem* > items = this->ui->userTableWidget->selectedItems();
  if( items.empty() ) return;

  vtkNew< Alder::User > user;
  user->Load( "Id", items.at( this->columnIndex["Name"] )->data( Qt::UserRole ).toInt() );

  // show a warning to the user before continuing
  std::stringstream stream;
  stream << "Are you sure you wish to remove user \"" << user->Get( "Name" ).ToString() << "\"?  "
         << "All of this user's ratings will also be permanantely removed.  "
         << "This operation cannot be undone.";
  if( QMessageBox::Yes == QMessageBox::question(
     this, "Remove User", stream.str().c_str(), QMessageBox::Yes | QMessageBox::No ) )
  {
    user->Remove();
    this->updateInterface();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::slotResetPassword()
{
  QList< QTableWidgetItem* > items = this->ui->userTableWidget->selectedItems();
  if( items.empty() ) return;

  int id = items.at( this->columnIndex["Name"] )->data( Qt::UserRole ).toInt();
  vtkNew< Alder::User > user;
  user->Load( "Id", id );
  user->ResetPassword();
  user->Save();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::slotClose()
{
  this->accept();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::slotSelectionChanged()
{
  QList< QTableWidgetItem* > items = this->ui->userTableWidget->selectedItems();
  bool selected = !items.empty();
  this->ui->removePushButton->setEnabled( selected );
  this->ui->resetPasswordPushButton->setEnabled( selected );

  // do not allow resetting the password to the admin account
  if( selected )
  {
    QTableWidgetItem* item = items.at( this->columnIndex["Name"] );
    if( "administrator" == item->text() )
      this->ui->resetPasswordPushButton->setEnabled( false );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::slotHeaderClicked( int index )
{
  // NOTE: currently the columns with checkboxes cannot be sorted.  In order to do this we would need
  // to either override QSortFilterProxyModel::lessThan() or QAbstractTableModel::sort()
  // For now we'll just ignore requests to sort by these columns

  if( this->columnIndex["Name"] == index ||
      this->columnIndex["LastLogin"] == index )
  {
    // reverse order if already sorted
    if( this->sortColumn == index )
      this->sortOrder = Qt::AscendingOrder == this->sortOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
    this->sortColumn = index;
    this->ui->userTableWidget->sortItems( this->sortColumn, this->sortOrder );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::slotItemChanged( QTableWidgetItem *item )
{
  int row = item->row();
  int column = item->column();
  Qt::CheckState state = item->checkState();

  // get the user
  vtkNew< Alder::User > user;
  user->Load( "Id",
    this->ui->userTableWidget->item( row, this->columnIndex["Name"] )->data( Qt::UserRole ).toInt() );

  // update the user's settings
  bool modified = false;
  if( this->columnIndex["Expert"] == column )
  {
    user->Set( "Expert", Qt::Checked == state ? 1 : 0 );
    modified = true;
  }
  else
  {
    std::vector< vtkSmartPointer< Alder::Modality > > modalityList;
    Alder::Modality::GetAll( &modalityList );
    for( auto it = modalityList.begin(); it != modalityList.end(); ++it )
    {
      std::string modalityName = (*it)->Get( "Name" ).ToString();
      if( this->columnIndex[ modalityName ] == column )
      {
        if( Qt::Checked == state )
          user->AddRecord( *it );
        else
          user->RemoveRecord( *it );

        modified = true;
        break;
      }
    }
  }
  if( modified )
  {
    user->Save();
    emit this->userModalityChanged();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::updateInterface()
{
  this->ui->userTableWidget->blockSignals( true );
  this->ui->userTableWidget->setRowCount( 0 );
  QTableWidgetItem *item;

  std::vector< vtkSmartPointer< Alder::User > > userList;
  Alder::User::GetAll( &userList );
  for( auto it = userList.begin(); it != userList.end(); ++it )
  { // for every user, add a new row
    Alder::User *user = (*it);
    this->ui->userTableWidget->insertRow( 0 );

    // add name to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    item->setText( QString( user->Get( "Name" ).ToString().c_str() ) );
    item->setData( Qt::UserRole, user->Get( "Id" ).ToInt() );
    this->ui->userTableWidget->setItem( 0, this->columnIndex["Name"], item );

    // add last login to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    item->setText( QString( user->Get( "LastLogin" ).ToString().c_str() ) );
    this->ui->userTableWidget->setItem( 0, this->columnIndex["LastLogin"], item );

    // add the expert row
    Qt::CheckState check =  0 < user->Get( "Expert" ).ToInt() ? Qt::Checked : Qt::Unchecked;
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
    item->setCheckState( check );
    this->ui->userTableWidget->setItem( 0, this->columnIndex["Expert"], item );

    // add all modalities (one per column)
    std::vector< vtkSmartPointer< Alder::Modality > > modalityList;
    Alder::Modality::GetAll( &modalityList );
    for( auto modalityListIt = modalityList.begin(); modalityListIt != modalityList.end(); ++modalityListIt )
    {
      std::string modalityName = (*modalityListIt)->Get( "Name" ).ToString();
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
      item->setCheckState( 0 < user->Has( *modalityListIt ) ? Qt::Checked : Qt::Unchecked );
      this->ui->userTableWidget->setItem( 0, this->columnIndex[modalityName], item );
    }
  }

  this->ui->userTableWidget->sortItems( this->sortColumn, this->sortOrder );
  this->ui->userTableWidget->blockSignals( false );
}
