/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QSelectWaveDialog.cxx
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QSelectWaveDialog.h>
#include <QSelectWaveDialog_p.h>

// Qt includes
#include <QTableWidgetItem>

// Alder includes
#include <Application.h>
#include <Wave.h>
#include <Utilities.h>

// VTK includes
#include <vtkSmartPointer.h>

// C++ includes
#include <string>
#include <utility>
#include <vector>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QSelectWaveDialogPrivate methods
//
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSelectWaveDialogPrivate::QSelectWaveDialogPrivate(QSelectWaveDialog& object)
  : QObject(&object), q_ptr(&object)
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSelectWaveDialogPrivate::~QSelectWaveDialogPrivate()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectWaveDialogPrivate::setupUi(QDialog* widget)
{
  Q_Q(QSelectWaveDialog);

  this->Ui_QSelectWaveDialog::setupUi(widget);

  QObject::connect(
    this->buttonBox, SIGNAL(accepted()),
    q, SLOT(close()));

  int index = 0;
  QStringList labels;
  labels << "Name";
  this->columnIndex[labels.last()] = index++;
  labels << "Rank";
  this->columnIndex[labels.last()] = index++;
  labels << "Identifiers";
  this->columnIndex[labels.last()] = index++;
  labels << "Select";
  this->columnIndex[labels.last()] = index++;
  labels << "Refresh";
  this->columnIndex[labels.last()] = index++;
  labels << "Updated";
  this->columnIndex[labels.last()] = index++;

  this->waveTableWidget->setHorizontalHeaderLabels(labels);
  QHeaderView* header = this->waveTableWidget->horizontalHeader();
  header->setSectionResizeMode(QHeaderView::Stretch);
  header->setSectionsClickable(true);
  header->setSectionResizeMode(QHeaderView::ResizeToContents);
  header->setStretchLastSection(true);

  this->waveTableWidget->verticalHeader()->setVisible(false);
  this->waveTableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
  this->waveTableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

  this->sortColumn = 1;
  this->sortOrder = Qt::AscendingOrder;

  QObject::connect(
    this->waveTableWidget->horizontalHeader(), SIGNAL(sectionClicked(int)),
    this, SLOT(sort(int)));

  std::vector<vtkSmartPointer<Alder::Wave>> waveList;
  Alder::Wave::GetAll(&waveList);
  this->waveTableWidget->setRowCount(0);
  QTableWidgetItem* item;
  for (auto it = waveList.begin(); it != waveList.end(); ++it)
  {
    Alder::Wave* wave = *it;
    QString rankStr = wave->Get("Rank").ToString().c_str();
    QString nameStr = wave->Get("Name").ToString().c_str();
    std::string s = vtkVariant(wave->GetCount("Interview")).ToString();
    s += "/";
    s += vtkVariant(wave->GetIdentifierCount()).ToString();
    QString identStr = s.c_str();

    QVariant vId = wave->Get("Id").ToInt();
    this->waveTableWidget->insertRow(0);

    // add name to row
    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setText(nameStr);
    this->waveTableWidget->setItem(0, this->columnIndex["Name"], item);

    // add rank to row
    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setText(rankStr);
    this->waveTableWidget->setItem(0, this->columnIndex["Rank"], item);

    // add identifier counts to row
    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setText(identStr);
    this->waveTableWidget->setItem(0, this->columnIndex["Identifiers"], item);

    // add select checkbox to row
    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Unchecked);
    item->setData(Qt::UserRole, vId);
    this->waveTableWidget->setItem(0, this->columnIndex["Select"], item);

    // add update checkbox to row
    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Unchecked);
    item->setData(Qt::UserRole, vId);
    this->waveTableWidget->setItem(0, this->columnIndex["Refresh"], item);

    // add most recent interview update timestamp to row
    s = wave->GetMaximumInterviewUpdateTimestamp();
    Alder::Utilities::left(s, 10);
    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsEnabled);
    item->setText(s.c_str());
    this->waveTableWidget->setItem(0, this->columnIndex["Updated"], item);
  }

  QObject::connect(
    this->waveTableWidget, SIGNAL(itemPressed(QTableWidgetItem*)),
    this, SLOT(itemPressed(QTableWidgetItem*)));
  QObject::connect(
    this->waveTableWidget, SIGNAL(itemClicked(QTableWidgetItem*)),
    this, SLOT(itemClicked(QTableWidgetItem*)));

  this->lastItemPressedState = Qt::Unchecked;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectWaveDialogPrivate::itemPressed(QTableWidgetItem* item)
{
  // only check selected column items
  int column = this->waveTableWidget->column(item);
  if (column == this->columnIndex["Select"])
    this->lastItemPressedState = item->checkState();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectWaveDialogPrivate::itemClicked(QTableWidgetItem* item)
{
  // only check selected column items
  int column = this->waveTableWidget->column(item);
  if (column == this->columnIndex["Select"])
  {
    int row = this->waveTableWidget->row(item);
    if (this->lastItemPressedState != item->checkState())
    {
      QTableWidgetItem* companion =
        this->waveTableWidget->item(row, this->columnIndex["Refresh"]);
      Qt::ItemFlags flags;
      if (Qt::Checked == item->checkState())
        flags |= (Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
      else
        flags |= Qt::ItemIsUserCheckable;
      companion->setFlags(flags);
      companion->setCheckState(Qt::Unchecked);
      this->lastItemPressedState = item->checkState();
    }
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectWaveDialogPrivate::sort(int col)
{
  // reverse order if already sorted
  if (this->sortColumn == col)
    this->sortOrder =
      Qt::AscendingOrder == this->sortOrder ?
        Qt::DescendingOrder : Qt::AscendingOrder;

  this->sortColumn = col;
  this->waveTableWidget->sortItems(this->sortColumn, this->sortOrder);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectWaveDialogPrivate::buildSelection()
{
  this->selection.clear();
  for (int i = 0; i < this->waveTableWidget->rowCount(); ++i)
  {
    QTableWidgetItem* item =
      this->waveTableWidget->item(i, this->columnIndex["Select"]);
    if (Qt::Checked == item->checkState())
    {
      QTableWidgetItem* companion =
        this->waveTableWidget->item(i, this->columnIndex["Refresh"]);
      this->selection.push_back(
        std::make_pair(
          item->data(Qt::UserRole).toInt(),
          Qt::Checked == companion->checkState()));
    }
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSelectWaveDialog::QSelectWaveDialog(QWidget* parent)
  : Superclass(parent)
  , d_ptr(new QSelectWaveDialogPrivate(*this))
{
  Q_D(QSelectWaveDialog);
  d->setupUi(this);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSelectWaveDialog::~QSelectWaveDialog()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
std::vector<std::pair<int, bool>> QSelectWaveDialog::selection()
{
  Q_D(QSelectWaveDialog);
  d->buildSelection();
  return d->selection;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectWaveDialog::close()
{
  this->accept();
}
