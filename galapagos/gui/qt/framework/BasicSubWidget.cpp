#include "BasicSubWidget.h"
#include "BasicGui.h";
#include <QDir>


namespace gapg {

BasicSubWidget::BasicSubWidget (QWidget* parent) :
    QWidget (parent), fSetup(0)
{
  fLastFileDir= QDir::currentPath();


}

BasicSubWidget::~BasicSubWidget ()
{
}

void BasicSubWidget::SetBasicParent(gapg::BasicGui* parent)
   {
     fParent=parent;
     fSetup=fParent->GetSetup();
     fImpName=fParent->fImplementationName;
   }

bool BasicSubWidget::IsAutoApply()
    {
      return fParent->IsAutoApply();
    }

} // namespace

