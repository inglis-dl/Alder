/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QUserListDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QUserListDialog.h>
#include <ui_QUserListDialog.h>

// Alder includes
#include <Modality.h>
#include <User.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

// Qt includes
#include <QErrorMessage>
#include <QInputDialog>
#include <QMessageBox>
#include <QList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMap>

#include <stdexcept>
#include <vector>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class QUserListDialogPrivate : public Ui_QUserListDialog
{
  Q_DECLARE_PUBLIC(QUserListDialog);
protected:
  QUserListDialog* const q_ptr;

public:
  QUserListDialogPrivate(QUserListDialog& object);
  virtual ~QUserListDialogPrivate();

  virtual void setupUi(QWidget*);
  virtual void updateUi();
  virtual void updateUserUi();
  virtual void sort(int);

  QMap<QString, int> columnIndex;

private:
  int sortColumn;
  Qt::SortOrder sortOrder;
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QUserListDialogPrivate methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QUserListDialogPrivate::QUserListDialogPrivate
(QUserListDialog& object)
  : q_ptr(&object)
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QUserListDialogPrivate::~QUserListDialogPrivate()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialogPrivate::setupUi( QWidget* widget )
{
  Q_Q(QUserListDialog);

  this->Ui_QUserListDialog::setupUi( widget );

  int index = 0;
  QStringList labels;
  QHeaderView* header = this->userTableWidget->horizontalHeader();
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
  std::vector< vtkSmartPointer< Alder::Modality > > vecModality;
  Alder::Modality::GetAll( &vecModality );

  this->userTableWidget->setColumnCount( index + vecModality.size() );
  for( auto it = vecModality.cbegin(); it != vecModality.cend(); ++it )
  {
    QString name = (*it)->Get( "Name" ).ToString().c_str();
    labels << name;
    header->setResizeMode( index, QHeaderView::ResizeToContents );
    this->columnIndex[name] = index++;
  }

  this->userTableWidget->setHorizontalHeaderLabels( labels );
  this->userTableWidget->horizontalHeader()->setClickable( true );
  this->userTableWidget->verticalHeader()->setVisible( false );
  this->userTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  this->userTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );

  this->sortColumn = 0;
  this->sortOrder = Qt::AscendingOrder;

  QObject::connect(
    this->addPushButton, SIGNAL( clicked( bool ) ),
    q, SLOT( add() ) );

  QObject::connect(
    this->removePushButton, SIGNAL( clicked( bool ) ),
    q, SLOT( remove() ) );

  QObject::connect(
    this->resetPasswordPushButton, SIGNAL( clicked( bool ) ),
    q, SLOT( resetPassword() ) );

  QObject::connect(
    this->closePushButton, SIGNAL( clicked( bool ) ),
    q, SLOT( close() ) );

  QObject::connect(
    this->userTableWidget, SIGNAL( itemSelectionChanged() ),
    q, SLOT( userSelectionChanged() ) );

  QObject::connect(
    this->userTableWidget->horizontalHeader(), SIGNAL( sectionClicked( int ) ),
    q, SLOT( sortColumn( int ) ) );

  QObject::connect(
    this->userTableWidget, SIGNAL( itemChanged( QTableWidgetItem* ) ),
    q, SLOT( modalitySelectionChanged( QTableWidgetItem* ) ) );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialogPrivate::updateUi()
{
  this->userTableWidget->blockSignals( true );
  this->userTableWidget->setRowCount( 0 );

  QTableWidgetItem *item;
  std::vector< vtkSmartPointer< Alder::User > > vecUser;
  Alder::User::GetAll( &vecUser );
  for( auto it = vecUser.cbegin(); it != vecUser.cend(); ++it )
  { // for every user, add a new row
    Alder::User *user = *it;
    this->userTableWidget->insertRow( 0 );

    // add name to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    item->setText( QString( user->Get( "Name" ).ToString().c_str() ) );
    item->setData( Qt::UserRole, user->Get( "Id" ).ToInt() );
    this->userTableWidget->setItem( 0, this->columnIndex["Name"], item );

    // add last login to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    item->setText( QString( user->Get( "LastLogin" ).ToString().c_str() ) );
    this->userTableWidget->setItem( 0, this->columnIndex["LastLogin"], item );

    // add the expert row
    Qt::CheckState check =  0 < user->Get( "Expert" ).ToInt() ? Qt::Checked : Qt::Unchecked;
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
    item->setCheckState( check );
    this->userTableWidget->setItem( 0, this->columnIndex["Expert"], item );

    // add all modalities (one per column)
    std::vector< vtkSmartPointer< Alder::Modality > > vecModality;
    Alder::Modality::GetAll( &vecModality );
    for( auto vit = vecModality.begin(); vit != vecModality.end(); ++vit )
    {
      QString name = (*vit)->Get( "Name" ).ToString().c_str();
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
      item->setCheckState( 0 < user->Has( *vit ) ? Qt::Checked : Qt::Unchecked );
      this->userTableWidget->setItem( 0, this->columnIndex[name], item );
    }
  }

  this->userTableWidget->sortItems( this->sortColumn, this->sortOrder );
  this->userTableWidget->blockSignals( false );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialogPrivate::updateUserUi()
{
  QList< QTableWidgetItem* > items = this->userTableWidget->selectedItems();
  bool selected = !items.empty();
  this->removePushButton->setEnabled( selected );
  this->resetPasswordPushButton->setEnabled( selected );

  // do not allow resetting the password to the admin account
  if( selected )
  {
    QTableWidgetItem* item = items.at( this->columnIndex["Name"] );
    if( "administrator" == item->text() )
      this->resetPasswordPushButton->setEnabled( false );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialogPrivate::sort( int index )
{
  // NOTE: currently the columns with checkboxes cannot be sorted.  In order to do this we would need
  // to either override QSortFilterProxyModel::lessThan() or QAbstractTableModel::sort()
  // For now we'll just ignore requests to sort by these columns

  if( index == this->columnIndex["Name"] ||
      index == this->columnIndex["LastLogin"] )
  {
    // reverse order if already sorted
    if( index == this->sortColumn )
    {
      this->sortOrder =
        Qt::AscendingOrder == this->sortOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
    }
    this->sortColumn = index;
    this->userTableWidget->sortItems( this->sortColumn, this->sortOrder );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QUserListDialog methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QUserListDialog::QUserListDialog( QWidget* parent )
  : Superclass( parent )
  , d_ptr(new QUserListDialogPrivate(*this))
{
  Q_D(QUserListDialog);
  d->setupUi(this);
  d->updateUi();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QUserListDialog::~QUserListDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::userSelectionChanged()
{
  Q_D(QUserListDialog);
  d->updateUserUi();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::sortColumn( int index )
{
  Q_D(QUserListDialog);
  d->sort( index );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::remove()
{
  Q_D(QUserListDialog);

  QList< QTableWidgetItem* > items = d->userTableWidget->selectedItems();
  if( items.empty() ) return;

  vtkNew< Alder::User > user;
  user->Load( "Id", items.at( d->columnIndex["Name"] )->data( Qt::UserRole ).toInt() );

  // show a warning to the user before continuing
  std::stringstream stream;
  stream << "Are you sure you wish to remove user \"" << user->Get( "Name" ).ToString() << "\"?  "
         << "All of this user's ratings will also be permanantely removed.  "
         << "This operation cannot be undone.";
  if( QMessageBox::Yes == QMessageBox::question(
     this, "Remove User", stream.str().c_str(), QMessageBox::Yes | QMessageBox::No ) )
  {
    user->Remove();
    d->updateUi();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::resetPassword()
{
  Q_D(QUserListDialog);

  QList< QTableWidgetItem* > items = d->userTableWidget->selectedItems();
  if( items.empty() ) return;

  int id = items.at( d->columnIndex["Name"] )->data( Qt::UserRole ).toInt();
  vtkNew< Alder::User > user;
  user->Load( "Id", id );
  user->ResetPassword();
  user->Save();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::add()
{
  Q_D(QUserListDialog);

  // get the new user's name
  QString name =QInputDialog::getText(
    this, QObject::tr( "Create User" ), QObject::tr( "New user's name:" ), QLineEdit::Normal );

  if( !name.isEmpty() )
  {
    // make sure the user name doesn't already exist
    vtkNew< Alder::User > user;
    std::string s = name.toStdString();
    if( user->Load( "Name", s ) )
    {
      std::stringstream stream;
      stream << "Unable to create new user \"" << s << "\", name already in use.";
      QErrorMessage *dialog = new QErrorMessage( this );
      dialog->setModal( true );
      dialog->showMessage( tr( stream.str().c_str() ) );
    }
    else
    {
      user->Set( "Name", s );
      user->ResetPassword();
      user->Save();
      d->updateUi();
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::modalitySelectionChanged( QTableWidgetItem *item )
{
  Q_D(QUserListDialog);

  int row = item->row();
  int column = item->column();
  Qt::CheckState state = item->checkState();

  // get the user
  vtkNew< Alder::User > user;
  user->Load( "Id",
    d->userTableWidget->item( row, d->columnIndex["Name"] )->data( Qt::UserRole ).toInt() );

  // update the user's settings
  bool modified = false;
  if( d->columnIndex["Expert"] == column )
  {
    user->Set( "Expert", Qt::Checked == state ? 1 : 0 );
    modified = true;
  }
  else
  {
    std::vector< vtkSmartPointer< Alder::Modality > > vecModality;
    Alder::Modality::GetAll( &vecModality );
    for( auto it = vecModality.begin(); it != vecModality.end(); ++it )
    {
      QString name = (*it)->Get( "Name" ).ToString().c_str();
      if( d->columnIndex[ name ] == column )
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
     emit userModalityChanged();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::close()
{
  this->accept();
}
