#include "NyxorWidget.h"

/*
 *  Constructs a NyxorGui which is a child of 'parent', with the
 *  name 'name'.'
 */
NyxorWidget::NyxorWidget (QWidget* parent) :
    QWidget (parent)
{
  setupUi (this);
}

NyxorWidget::~NyxorWidget ()
{
}

