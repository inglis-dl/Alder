/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QCodeDialog.cxx
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QCodeDialog.h>
#include <QCodeDialog_p.h>

// Qt includes
#include <QMessageBox>

// VTK includes
#include <vtkSmartPointer.h>

// Alder includes
#include <CodeGroup.h>
#include <CodeType.h>
#include <ScanType.h>

// C++ includes
#include <string>
#include <vector>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QCodeDialogPrivate methods
//
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QCodeDialogPrivate::QCodeDialogPrivate(QCodeDialog& object)
  : QObject(&object), q_ptr(&object)
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QCodeDialogPrivate::~QCodeDialogPrivate()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialogPrivate::setupUi(QDialog* widget)
{
  Q_Q(QCodeDialog);

  this->Ui_QCodeDialog::setupUi(widget);

  QStringList tables;
  tables << "CodeType" << "CodeGroup";
  for (int i = 0; i < tables.size(); ++i)
  {
    this->sortOrder[tables[i]];
    this->columnIndex[tables[i]];
  }

  std::vector<vtkSmartPointer<Alder::ScanType>> scanTypeList;
  Alder::ScanType::GetAll(&scanTypeList);
  this->scanTypeMap.clear();
  for (auto it = scanTypeList.begin(); it != scanTypeList.end(); ++it)
  {
    Alder::ScanType *scanType = (*it);
    QString type = scanType->Get("Type").ToString().c_str();
    QVariant id = scanType->Get("Id").ToInt();
    QMap<QString, QList<QVariant>>::iterator m = this->scanTypeMap.find(type);
    QList<QVariant> list;
    if (m == this->scanTypeMap.end())
    {
      list.push_back(id);
    }
    else
    {
      list = m.value();
      if (!list.contains(id))
      {
        list.push_back(id);
      }
    }
    if (!list.isEmpty())
    {
       this->scanTypeMap[type] = list;
    }
  }

  // list of all scan types that can have a code
  for (auto it = this->scanTypeMap.begin(); it != this->scanTypeMap.end(); ++it)
  {
    this->scanTypeComboBox->addItem(it.key(), it.value());
  }

  this->codeGroupComboBox->addItem("", QVariant(-1));
  this->codeGroupComboBox->setCurrentIndex(0);

  this->groupValueSpinBox->setMinimum(-5);
  this->groupValueSpinBox->setMaximum(0);
  this->groupValueSpinBox->setSingleStep(-1);

  this->codeValueSpinBox->setMinimum(-5);
  this->codeValueSpinBox->setMaximum(0);
  this->codeValueSpinBox->setSingleStep(-1);

  this->codeTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  this->codeTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

  this->groupTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  this->groupTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

  QHeaderView* header = this->codeTableWidget->horizontalHeader();
  header->setClickable(true);
  header->setResizeMode(QHeaderView::ResizeToContents);
  header->setStretchLastSection(true);

  QObject::connect(
    header, SIGNAL(sectionDoubleClicked(int)),
    this, SLOT(sort(int)));

  header = this->groupTableWidget->horizontalHeader();

  header->setClickable(true);
  header->setResizeMode(QHeaderView::ResizeToContents);
  header->setStretchLastSection(true);

  QObject::connect(
    header, SIGNAL(sectionDoubleClicked(int)),
    this, SLOT(sort(int)));

  QObject::connect(
    this->closePushButton, SIGNAL(clicked(bool)),
    q, SLOT(close()));

  QObject::connect(
    this->groupTableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
    this, SLOT(tableDoubleClicked(QTableWidgetItem*)));

  QObject::connect(
    this->codeTableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
    this, SLOT(tableDoubleClicked(QTableWidgetItem*)));

  QObject::connect(
    this->tabWidget, SIGNAL(currentChanged(int)),
    this, SLOT(tabChanged()));

  QObject::connect(
    this->groupTableWidget, SIGNAL(itemSelectionChanged()),
    this, SLOT(selectedGroupChanged()));

  QObject::connect(
    this->groupAddPushButton, SIGNAL(clicked()),
    this, SLOT(addGroup()));

  QObject::connect(
    this->groupEditPushButton, SIGNAL(clicked()),
    this, SLOT(editGroup()));

  QObject::connect(
    this->groupRemovePushButton, SIGNAL(clicked()),
    this, SLOT(removeGroup()));

  QObject::connect(
    this->groupApplyPushButton, SIGNAL(clicked()),
    this, SLOT(applyGroup()));

  QObject::connect(
    this->codeTableWidget, SIGNAL(itemSelectionChanged()),
    this, SLOT(selectedCodeChanged()));

  QObject::connect(
    this->codeAddPushButton, SIGNAL(clicked()),
    this, SLOT(addCode()));

  QObject::connect(
    this->codeEditPushButton, SIGNAL(clicked()),
    this, SLOT(editCode()));

  QObject::connect(
    this->codeRemovePushButton, SIGNAL(clicked()),
    this, SLOT(removeCode()));

  QObject::connect(
    this->codeApplyPushButton, SIGNAL(clicked()),
    this, SLOT(applyCode()));

  QRegExp grx("\\S+");
  QValidator* gv = new QRegExpValidator(grx, this);
  this->groupLineEdit->setValidator(gv);

  QRegExp crx("[A-Z,a-z]{1,3}");
  QValidator* cv = new QRegExpValidator(crx, this);
  this->codeLineEdit->setValidator(cv);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialogPrivate::sort(int col)
{
  QHeaderView* header = qobject_cast<QHeaderView*>(sender());
  if (!header) return;
  QTableWidget* table = qobject_cast<QTableWidget*>(header->parentWidget());
  QString name = table == this->codeTableWidget ? "CodeType" : "CodeGroup";

  Qt::SortOrder order = this->sortOrder[name][col];
  order =
    Qt::AscendingOrder == order ? Qt::DescendingOrder : Qt::AscendingOrder;
  table->sortByColumn(col, order);
  this->sortOrder[name][col] = order;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialogPrivate::selectedGroupChanged()
{
  QList<QTableWidgetItem*> items = this->groupTableWidget->selectedItems();
  bool selected = !items.empty();
  this->groupAddPushButton->setEnabled(!selected);
  this->groupEditPushButton->setEnabled(selected);
  this->groupApplyPushButton->setEnabled(false);
  this->groupLineEdit->clear();
  this->groupValueSpinBox->setValue(0);

  if (selected)
  {
    int usage = items.at(
      this->columnIndex["CodeGroup"]["Usage"])->data(Qt::DisplayRole).toInt();
    bool enabled = 0 == usage;
    this->groupRemovePushButton->setEnabled(enabled);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialogPrivate::tableDoubleClicked(QTableWidgetItem* item)
{
  QTableWidget* widget = item->tableWidget();
  widget->clearSelection();
  if (widget == this->codeTableWidget)
  {
    this->codeAddPushButton->setEnabled(true);
    this->codeEditPushButton->setEnabled(false);
    this->codeRemovePushButton->setEnabled(false);
    this->codeApplyPushButton->setEnabled(false);
  }
  else
  {
    this->groupAddPushButton->setEnabled(true);
    this->groupEditPushButton->setEnabled(false);
    this->groupRemovePushButton->setEnabled(false);
    this->groupApplyPushButton->setEnabled(false);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialogPrivate::updateUi()
{
  this->updateCodeUi();
  this->updateGroupUi();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialogPrivate::updateCodeUi()
{
  this->codeTableWidget->blockSignals(true);
  this->codeTableWidget->setSortingEnabled(false);
  this->codeTableWidget->setRowCount(0);

  QStringList columns;
  QString name = "CodeType";
  columns << "Type" << "Code" << "Value" << "Usage" << "Group"
          << "Group Value" << "Active";
  for (int i = 0; i < columns.size(); ++i)
  {
    this->sortOrder[name][i] = Qt::AscendingOrder;
    this->columnIndex[name][columns[i]] = i;
  }
  this->codeTableWidget->setHorizontalHeaderLabels(columns);

  // prototype for cloning center aligned cells
  QTableWidgetItem* proto = new QTableWidgetItem();
  proto->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  proto->setTextAlignment(Qt::AlignCenter);
  QTableWidgetItem* item;

  vtkSmartPointer<Alder::CodeGroup> codeGroup;

  vtkSmartPointer<Alder::QueryModifier> modifier =
    vtkSmartPointer<Alder::QueryModifier>::New();
  modifier->Join("ScanType", "ScanType.Id", "ScanTypeHasCodeType.ScanTypeId");
  std::string override = "ScanTypeHasCodeType";

  for (auto it = this->scanTypeMap.begin(); it != this->scanTypeMap.end(); ++it)
  {
    // for every scanType, add a new row
    QList<QVariant> scanTypeIdList = it.value();
    QString type = it.key();
    // get one of the scantype records associated with this type
    vtkNew<Alder::ScanType> scanType;
    if (!scanType->Load("Id", scanTypeIdList.first().toInt()))
      continue;

    // get all the code types associated with this scan type
    std::vector<vtkSmartPointer<Alder::CodeType>> codeTypeList;
    scanType->GetList(&codeTypeList, modifier, override);
    for (auto cit = codeTypeList.begin(); cit != codeTypeList.end(); ++cit)
    {
      Alder::CodeType* codeType = (*cit);
      QVariant codeTypeId = codeType->Get("Id").ToInt();
      QString code = codeType->Get("Code").ToString().c_str();
      int value = codeType->Get("Value").ToInt();
      QString active = 0 < codeType->Get("Active").ToInt() ? "Yes" : "No";
      int usage = codeType->GetUsage();
      QString groupName;
      int groupValue;
      QVariant groupId;
      bool hasGroup = codeType->GetRecord(codeGroup);
      if (hasGroup)
      {
        groupId = codeGroup->Get("Id").ToInt();
        groupName = codeGroup->Get("Name").ToString().c_str();
        groupValue = codeGroup->Get("Value").ToInt();
      }

      this->codeTableWidget->insertRow(0);

      // add Type to row
      item = new QTableWidgetItem;
      item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
      item->setText(type);
      item->setData(Qt::UserRole, scanTypeIdList);
      this->codeTableWidget->setItem(
        0, this->columnIndex[name]["Type"], item);

      // add Code to row
      item = proto->clone();
      item->setText(code);
      item->setData(Qt::UserRole, codeTypeId);
      this->codeTableWidget->setItem(
        0, this->columnIndex[name]["Code"], item);

      // add Value to row
      item = proto->clone();
      item->setData(Qt::DisplayRole, value);
      this->codeTableWidget->setItem(
        0, this->columnIndex[name]["Value"], item);

      // add Active to row
      item = proto->clone();
      item->setText(active);
      this->codeTableWidget->setItem(
        0, this->columnIndex[name]["Active"], item);

      // add Usage to row
      item = proto->clone();
      item->setData(Qt::DisplayRole, usage);
      this->codeTableWidget->setItem(
        0, this->columnIndex[name]["Usage"], item);

      if (hasGroup)
      {
        // add Group to row
        item = new QTableWidgetItem;
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        item->setText(groupName);
        item->setData(Qt::UserRole, groupId);
        this->codeTableWidget->setItem(
          0, this->columnIndex[name]["Group"], item);

        // add Group Value to row
        item = proto->clone();
        item->setData(Qt::DisplayRole, groupValue);
        this->codeTableWidget->setItem(
          0, this->columnIndex[name]["Group Value"], item);
      }
      else
      {
        // add Group to row
        item = new QTableWidgetItem;
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        this->codeTableWidget->setItem(
          0, this->columnIndex[name]["Group"], item);

        // add Group Value to row
        item = proto->clone();
        this->codeTableWidget->setItem(
          0, this->columnIndex[name]["Group Value"], item);
      }
    }
  }

  this->codeAddPushButton->setEnabled(true);
  this->codeEditPushButton->setEnabled(false);
  this->codeRemovePushButton->setEnabled(false);
  this->codeApplyPushButton->setEnabled(false);
  this->codeLineEdit->clear();
  this->codeValueSpinBox->setValue(0);
  this->codeActiveCheckBox->setCheckState(Qt::Unchecked);
  this->scanTypeComboBox->setCurrentIndex(-1);
  this->codeGroupComboBox->setCurrentIndex(-1);

  this->codeTableWidget->setSortingEnabled(true);
  this->codeTableWidget->blockSignals(false);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialogPrivate::updateGroupUi()
{
  this->groupTableWidget->blockSignals(true);
  this->groupTableWidget->setSortingEnabled(false);
  this->groupTableWidget->setRowCount(0);

  QStringList columns;
  columns << "Group" << "Group Value" << "Usage";
  QString name = "CodeGroup";
  for (int i = 0; i < columns.size(); ++i)
  {
    this->sortOrder[name][i] = Qt::AscendingOrder;
    this->columnIndex[name][columns[i]] = i;
  }

  // the last column indexes a dummy column of empty cells to stretch
  // the table on
  int lastColumn = columns.size();

  this->groupTableWidget->setHorizontalHeaderLabels(columns);
  this->codeGroupComboBox->clear();
  this->codeGroupComboBox->addItem("", QVariant(-1));
  this->codeGroupComboBox->setCurrentIndex(0);

  // prototype for cloning center aligned cells
  QTableWidgetItem* proto = new QTableWidgetItem();
  proto->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  proto->setTextAlignment(Qt::AlignCenter);
  QTableWidgetItem* item;

  // list of all groups
  std::vector<vtkSmartPointer<Alder::CodeGroup>> codeGroupList;
  Alder::CodeGroup::GetAll(&codeGroupList);
  for (auto it = codeGroupList.begin(); it != codeGroupList.end(); ++it)
  {
    Alder::CodeGroup* codeGroup = (*it);
    QString groupName = codeGroup->Get("Name").ToString().c_str();
    this->groupTableWidget->insertRow(0);
    QVariant id = QVariant(codeGroup->Get("Id").ToInt());

    // add Group to row
    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setText(groupName);
    item->setData(Qt::UserRole, id);
    this->groupTableWidget->setItem(
      0, this->columnIndex[name]["Group"], item);

    // add Group Value to row
    item = proto->clone();
    item->setData(Qt::DisplayRole, codeGroup->Get("Value").ToInt());
    this->groupTableWidget->setItem(
      0, this->columnIndex[name]["Group Value"], item);

    // add Usage to row
    item = proto->clone();
    item->setData(Qt::DisplayRole, codeGroup->GetUsage());
    this->groupTableWidget->setItem(
      0, this->columnIndex[name]["Usage"], item);

    // add a dummy column to expand out on
    item = new QTableWidgetItem;
    this->groupTableWidget->setItem(0, lastColumn, item);

    this->codeGroupComboBox->addItem(groupName, id);
  }

  this->groupAddPushButton->setEnabled(true);
  this->groupEditPushButton->setEnabled(false);
  this->groupRemovePushButton->setEnabled(false);
  this->groupApplyPushButton->setEnabled(false);
  this->groupLineEdit->clear();
  this->groupValueSpinBox->setValue(0);

  this->groupTableWidget->setSortingEnabled(true);
  this->groupTableWidget->blockSignals(false);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialogPrivate::tabChanged()
{
  this->groupTableWidget->clearSelection();
  this->codeTableWidget->clearSelection();

  this->codeAddPushButton->setEnabled(true);
  this->codeEditPushButton->setEnabled(false);
  this->codeRemovePushButton->setEnabled(false);
  this->codeApplyPushButton->setEnabled(false);

  this->groupAddPushButton->setEnabled(true);
  this->groupEditPushButton->setEnabled(false);
  this->groupRemovePushButton->setEnabled(false);
  this->groupApplyPushButton->setEnabled(false);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialogPrivate::addGroup()
{
  Q_Q(QCodeDialog);
  // add if group name is filled
  int value = this->groupValueSpinBox->value();
  std::string groupName = this->groupLineEdit->text().trimmed().toStdString();
  if (groupName.empty()) return;

  // ensure proposed CodeGroup Name and Value are unique
  bool unique = Alder::CodeGroup::IsUnique(groupName, value);
  if (unique)
  {
    vtkNew<Alder::CodeGroup> codeGroup;
    codeGroup->Set("Name", groupName);
    codeGroup->Set("Value", value);
    codeGroup->Save();
    this->updateGroupUi();
  }
  else
  {
    QString title = "Invalid Code Group";
    QString text = "Name and value are not unique";
    QMessageBox::warning(q, title, text);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialogPrivate::removeGroup()
{
  Q_Q(QCodeDialog);
  // a group cannot be removed if it has a non-zero usage count
  QList<QTableWidgetItem*> items = this->groupTableWidget->selectedItems();
  bool selected = !items.empty();
  if (selected)
  {
    int id = items.first()->data(Qt::UserRole).toInt();
    vtkNew<Alder::CodeGroup> codeGroup;
    codeGroup->Load("Id", id);

    QString title = "Confirm Group Removal";
    QString text = "Remove ";
    text += codeGroup->Get("Name").ToString().c_str();
    text += " group?";
    QMessageBox::StandardButton reply = QMessageBox::question(q,
      title, text, QMessageBox::Yes|QMessageBox::No);
    if (QMessageBox::Yes == reply)
    {
      codeGroup->Remove();
      this->updateGroupUi();
    }
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialogPrivate::applyGroup()
{
  Q_Q(QCodeDialog);
  // get the selected item
  QList<QTableWidgetItem*> items = this->groupTableWidget->selectedItems();
  bool selected = !items.empty();
  QString groupName = this->groupLineEdit->text();

  if (selected && !groupName.isEmpty())
  {
    int value = this->groupValueSpinBox->value();

    // ensure proposed CodeGroup Name and Value are unique
    bool unique = Alder::CodeGroup::IsUnique(groupName.toStdString(), value);
    if (unique)
    {
      QString name = "CodeGroup";
      QTableWidgetItem* first = items.first();
      int id = first->data(Qt::UserRole).toInt();
      vtkNew<Alder::CodeGroup> codeGroup;
      codeGroup->Load("Id", id);

      QString lastName = first->text();
      int lastValue = items.at(
        this->columnIndex[name]["Group Value"])->data(Qt::DisplayRole).toInt();

      bool modified = false;

      if (lastName != groupName)
      {
        codeGroup->Set("Name",  groupName.toStdString());
        first->setText(groupName);
        modified = true;
      }
      if (lastValue != value)
      {
        codeGroup->Set("Value", value);
        items.at(
          this->columnIndex[name]["Group Value"])->setData(
            Qt::DisplayRole, value);
        modified = true;
      }

      if (modified)
      {
        codeGroup->Save();

        if (lastValue != value)
          codeGroup->UpdateRatings();

        int usage = codeGroup->GetUsage();
        items.at(
          this->columnIndex[name]["Usage"])->setData(Qt::DisplayRole, usage);

        this->updateCodeUi();
      }
    }
    else
    {
      QString title = "Invalid Code Group";
      QString text = "Name and value are not unique";
      QMessageBox::warning(q, title, text);
    }
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialogPrivate::editGroup()
{
  // get the selected item
  QList<QTableWidgetItem*> items = this->groupTableWidget->selectedItems();
  bool selected = !items.empty();
  if (selected)
  {
    QString name = "CodeGroup";
    this->groupLineEdit->setText(
      items.at(this->columnIndex[name]["Group"])->text());
    this->groupValueSpinBox->setValue(
      items.at(
        this->columnIndex[name]["Group Value"])->data(
          Qt::DisplayRole).toInt());
    this->groupApplyPushButton->setEnabled(true);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialogPrivate::addCode()
{
  Q_Q(QCodeDialog);
  // add if code name is filled
  std::string code = this->codeLineEdit->text().trimmed().toStdString();
  if (code.empty()) return;

  int index = this->scanTypeComboBox->currentIndex();
  QList<QVariant> scanTypeIdList;
  if (-1 != index)
    scanTypeIdList = this->scanTypeComboBox->itemData(index).toList();

  if (scanTypeIdList.empty())
  {
    QString title = "Invalid Code";
    QString text = "A Code must be associated with a Type";
    QMessageBox::warning(q, title, text);
    return;
  }

  int value = this->codeValueSpinBox->value();

  index = this->codeGroupComboBox->currentIndex();
  int groupId = -1;
  if (-1 != index)
    groupId = this->codeGroupComboBox->itemData(index).toInt();

  // ensure proposed CodeType Name, Value and GroupId are unique
  bool unique = Alder::CodeType::IsUnique(code, value, groupId);
  if (unique)
  {
    vtkNew<Alder::CodeType> codeType;
    codeType->Set("Code", code);
    codeType->Set("Value", value);
    if (-1 != groupId)
      codeType->Set("CodeGroupId", groupId);
    codeType->Save();

    // get the ScanType and add the CodeType to the ScanTypeHasCodeType table
    for (auto it = scanTypeIdList.begin(); it != scanTypeIdList.end(); ++it)
    {
      vtkNew<Alder::ScanType> scanType;
      scanType->Load("Id", (*it).toInt());
      vtkSmartPointer<Alder::CodeType> ptr = codeType.GetPointer();
      scanType->AddRecord(ptr);
    }

    this->updateCodeUi();
  }
  else
  {
    QString title = "Invalid Code";
    QString text = "Code, value and group are not unique";
    QMessageBox::warning(q, title, text);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialogPrivate::removeCode()
{
  Q_Q(QCodeDialog);
  // a code cannot be removed if it has a non-zero usage count
  QList<QTableWidgetItem*> items = this->codeTableWidget->selectedItems();
  bool selected = !items.empty();
  if (selected)
  {
    QString name = "CodeType";
    int id = items.at(
      this->columnIndex[name]["Code"])->data(Qt::UserRole).toInt();
    vtkNew<Alder::CodeType> codeType;
    codeType->Load("Id", id);

    QString title = "Confirm Code Removal";
    QString text = "Remove ";
    text += codeType->Get("Code").ToString().c_str();
    text += " code?";
    QMessageBox::StandardButton reply = QMessageBox::question(q,
      title, text, QMessageBox::Yes|QMessageBox::No);
    if (QMessageBox::Yes == reply)
    {
      QList<QVariant> scanTypeIdList =
        items.at(this->columnIndex[name]["Type"])->data(Qt::UserRole).toList();
      for (auto it = scanTypeIdList.begin(); it != scanTypeIdList.end(); ++it)
      {
        vtkNew<Alder::ScanType> scanType;
        if (scanType->Load("Id", (*it).toInt()))
        {
          vtkSmartPointer<Alder::CodeType> ptr = codeType.GetPointer();
          scanType->RemoveRecord(ptr);
        }
      }
      codeType->Remove();
      this->updateCodeUi();
    }
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialogPrivate::applyCode()
{
  Q_Q(QCodeDialog);
  // get the selected item
  QList<QTableWidgetItem*> items = this->codeTableWidget->selectedItems();
  bool selected = !items.empty();
  QString code = this->codeLineEdit->text();

  if (selected && !code.isEmpty())
  {
    int value = this->codeValueSpinBox->value();
    int index = this->codeGroupComboBox->currentIndex();
    int groupId = -1;
    if (-1 != index)
      groupId = this->codeGroupComboBox->itemData(index).toInt();
    int groupValue = 0;
    int active = Qt::Checked == this->codeActiveCheckBox->checkState() ? 1 : 0;

    // ensure proposed Code, Value, and CodeGroupId are unique
    bool unique = Alder::CodeType::IsUnique(code.toStdString(), value, groupId);
    if (unique)
    {
      QString name = "CodeType";
      int id = items.at(
        this->columnIndex[name]["Code"])->data(Qt::UserRole).toInt();
      vtkNew<Alder::CodeType> codeType;
      codeType->Load("Id", id);

      QString lastCode = items.at(this->columnIndex[name]["Code"])->text();
      int lastValue = items.at(
        this->columnIndex[name]["Value"])->data(Qt::DisplayRole).toInt();
      int lastGroupId = items.at(
        this->columnIndex[name]["Group"])->data(Qt::UserRole).toInt();
      int lastGroupValue = items.at(
        this->columnIndex[name]["Group Value"])->data(Qt::DisplayRole).toInt();
      int lastActive = "Yes" == items.at(
        this->columnIndex[name]["Active"])->text() ? 1 : 0;
      bool modified = false;

      if (lastCode != code)
      {
        codeType->Set("Code",  code.toStdString());
        items.at(
          this->columnIndex[name]["Code"])->setText(code);
        modified = true;
      }
      if (lastValue != value)
      {
        codeType->Set("Value", value);
        items.at(
          this->columnIndex[name]["Value"])->setData(Qt::DisplayRole, value);
        modified = true;
      }
      if (lastActive != active)
      {
        codeType->Set("Active", active);
        items.at(
          this->columnIndex[name]["Active"])->setText(
            1 == active ? "Yes" : "No");
        modified = true;
      }
      if (lastGroupId != groupId)
      {
        if (-1 == groupId)
        {
          codeType->SetNull("CodeGroupId");
          items.at(
            this->columnIndex[name]["Group"])->setText("");
          items.at(
            this->columnIndex[name]["Group Value"])->setData(
              Qt::DisplayRole, "");
        }
        else
        {
          codeType->Set("CodeGroupId", groupId);
          vtkNew<Alder::CodeGroup> group;
          group->Load("Id", groupId);
          groupValue = group->Get("Value").ToInt();
          items.at(
            this->columnIndex[name]["Group"])->setText(
              group->Get("Name").ToString().c_str());
          items.at(
            this->columnIndex[name]["Group Value"])->setData(
              Qt::DisplayRole, groupValue);
        }
        modified = true;
      }

      if (modified)
      {
        codeType->Save();
        if (lastValue != value || lastGroupValue != groupValue)
          codeType->UpdateRatings();

        int usage = codeType->GetUsage();
        items.at(
          this->columnIndex[name]["Usage"])->setData(Qt::DisplayRole, usage);

        this->updateCodeUi();
      }
    }
    else
    {
      QString title = "Invalid Code";
      QString text = "Code, Value and Group are not unique";
      QMessageBox::warning(q, title, text);
    }
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialogPrivate::editCode()
{
  // get the selected item
  QList<QTableWidgetItem*> items = this->codeTableWidget->selectedItems();
  bool selected = !items.empty();
  if (selected)
  {
    QString name = "CodeType";
    this->codeLineEdit->setText(
      items.at(
        this->columnIndex[name]["Code"])->text());
    this->codeValueSpinBox->setValue(
      items.at(
        this->columnIndex[name]["Value"])->data(Qt::DisplayRole).toInt());
    QString group = items.at(this->columnIndex[name]["Group"])->text();
    if (!group.isEmpty())
    {
      int index = this->codeGroupComboBox->findText(group);
      this->codeGroupComboBox->setCurrentIndex(index);
    }
    QString scanType = items.at(this->columnIndex[name]["Type"])->text();
    int index = this->scanTypeComboBox->findText(scanType);
    this->scanTypeComboBox->setCurrentIndex(index);
    Qt::CheckState checked = "Yes" == items.at(
      this->columnIndex[name]["Active"])->text() ? Qt::Checked : Qt::Unchecked;
    this->codeActiveCheckBox->setCheckState(checked);
    this->codeApplyPushButton->setEnabled(true);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialogPrivate::selectedCodeChanged()
{
  QList<QTableWidgetItem*> items = this->codeTableWidget->selectedItems();
  bool selected = !items.empty();
  this->codeAddPushButton->setEnabled(!selected);
  this->codeEditPushButton->setEnabled(selected);
  this->codeApplyPushButton->setEnabled(false);
  this->codeLineEdit->clear();
  this->codeValueSpinBox->setValue(0);
  this->scanTypeComboBox->setCurrentIndex(-1);
  this->codeGroupComboBox->setCurrentIndex(-1);
  this->codeActiveCheckBox->setCheckState(Qt::Unchecked);

  if (selected)
  {
    int usage = items.at(
      this->columnIndex["CodeType"]["Usage"])->data(Qt::DisplayRole).toInt();
    bool enabled = 0 == usage;
    this->codeRemovePushButton->setEnabled(enabled);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QCodeDialog::QCodeDialog(QWidget* parent)
  : Superclass(parent)
  , d_ptr(new QCodeDialogPrivate(*this))
{
  Q_D(QCodeDialog);
  d->setupUi(this);
  d->updateUi();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QCodeDialog::~QCodeDialog()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QCodeDialog::close()
{
  this->accept();
}
