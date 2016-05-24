/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QUserListDialog_p.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QUserListDialog_p_h
#define __QUserListDialog_p_h

#include <QUserListDialog.h>
#include <ui_QUserListDialog.h>

class QTableWidgetItem;

class QUserListDialogPrivate : public QObject, public Ui_QUserListDialog
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(QUserListDialog);
  protected:
    QUserListDialog* const q_ptr;

  public:
    explicit QUserListDialogPrivate(QUserListDialog& object);
    virtual ~QUserListDialogPrivate();

    void setupUi(QWidget* widget);
    void updateUi();

  public Q_SLOTS:
    void sort(int column);
    void userSelectionChanged();
    void modalitySelectionChanged(QTableWidgetItem* item);

  private:
    int sortColumn;
    Qt::SortOrder sortOrder;
    QMap<QString, int> columnIndex;
};

#endif
