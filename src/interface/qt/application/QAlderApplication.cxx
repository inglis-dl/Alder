/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderApplication.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QAlderApplication.h>

// Qt includes
#include <QErrorMessage>

#include <stdexcept>
#include <iostream>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderApplication::notify( QObject *pObject, QEvent *pEvent )
{
  try
  {
    return QApplication::notify( pObject, pEvent );
  }
  catch( std::exception &e )
  {
    // catch any exception and display it to the user
    QErrorMessage *dialog = new QErrorMessage( this->activeWindow() );
    dialog->setModal( true );
    dialog->showMessage( tr( e.what() ) );
  }

  return false;
}
