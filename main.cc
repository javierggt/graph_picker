/**
   \file

   \author Javier Gonzalez
   \version $Id: TEMPLATE.cc.tpl,v 1.4 2003/09/25 14:38:19 lukas Exp $
   \date 29 Aug 2011
*/

static const char CVSId[] =
"$Id$";



#include <QApplication>
#include "GraphPicker.h"
#include <string>
#include <sstream>
#include <iostream>

#include <QGenericMatrix>

using namespace picker;
using namespace std;


int main(int argc, char *argv[])
{
  string file;

  if (argc == 2)
    file = argv[1];

  // QGenericMatrix<2,2,qreal> m1;
  // m1(0,0) = 1;
  // m1(0,1) = 2;
  // m1(1,0) = 3;
  // m1(1,1) = 4;
  // QGenericMatrix<2,2,qreal> m2;
  // m2(0,0) = 1;
  // m2(0,1) = 2;
  // m2(1,0) = -2;
  // m2(1,1) = -1;
  // qreal d[2] = {1,2};
  // QGenericMatrix<1,2,qreal> v1(d);
  // v1(0,0) = 1.;
  // v1(0,1) = 2.;
  // cout << "m1:\n" << m1 << endl
  //      << "m2:\n" << m2 << endl
  //      << "m1*m2:\n" << m1*m2 << endl
  //      << "m2*m1:\n" << m2*m1 << endl
  //      << "v1:\n" << v1 << endl
  //      << "m1*v1:\n" << m1*v1 << endl
  //      << "m2*v1:\n" << m2*v1 << endl
  //      << "m1*m2*v1:\n" << m1*m2*v1 << endl;
  // return 0;

  QApplication app(argc, argv);
  GraphPicker graphPicker(file);
  graphPicker.show();
  return app.exec();
}


// Configure (x)emacs for this file ...
// Local Variables:
// mode:c++
// compile-command: "make -C .. -k"
// End:
