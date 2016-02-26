/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAboutDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QAboutDialog.h>
#include <ui_QAboutDialog.h>

// Qt includes

class QAboutDialogPrivate : public Ui_QAboutDialog
{
  Q_DECLARE_PUBLIC(QAboutDialog);
protected:
  QAboutDialog* const q_ptr;

public:
  explicit QAboutDialogPrivate(QAboutDialog& object);
  virtual ~QAboutDialogPrivate();

  void init();
  void setupUi(QDialog*);
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QAboutDialogPrivate methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAboutDialogPrivate::QAboutDialogPrivate(QAboutDialog& object)
  : q_ptr(&object)
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAboutDialogPrivate::~QAboutDialogPrivate()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAboutDialogPrivate::init()
{
  Q_Q(QAboutDialog);
  this->setupUi(q);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAboutDialogPrivate::setupUi( QDialog* widget )
{
  this->Ui_QAboutDialog::setupUi( widget );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAboutDialog::QAboutDialog( QWidget* parent )
  : Superclass( parent )
  , d_ptr(new QAboutDialogPrivate(*this))
{
  Q_D(QAboutDialog);
  d->init();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAboutDialog::~QAboutDialog()
{
}
