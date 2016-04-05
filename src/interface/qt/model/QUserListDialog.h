/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QUserListDialog.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QUserListDialog_h
#define __QUserListDialog_h

// Qt includes
#include <QDialog>

class QTableWidgetItem;
class QUserListDialogPrivate;

class QUserListDialog : public QDialog
{
  Q_OBJECT

public:
  typedef QDialog Superclass;
  //constructor
  explicit QUserListDialog( QWidget* parent = 0 );
  //destructor
  virtual ~QUserListDialog();

signals:
  void permissionChanged();

public slots:
  virtual void close();
  virtual void add();
  virtual void remove();
  virtual void resetPassword();

protected:
  QScopedPointer<QUserListDialogPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(QUserListDialog);
  Q_DISABLE_COPY(QUserListDialog);
};

#endif
