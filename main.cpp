#include "MDBTools.h"
#include <QtWidgets/QApplication>
int MDBCombine::m_ClassCount = 0;
int main(int argc, char *argv[])
{
	
	QApplication a(argc, argv);
	a.setWindowIcon(QIcon(QPixmap(":/MDBTools/APPICON.ico")));
	MDBTools w;
	w.show();
	return a.exec();
}
