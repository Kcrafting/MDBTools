#pragma once
#define _AFXDLL 
#include <afxwin.h>
#include <afxdisp.h>//COleCurrency class
#include <QtWidgets/QMainWindow>
#include "ui_MDBTools.h"
#include <comdef.h>
#include <vector>
#include "MDBCombine.h"
using std::vector;
class MDBTools : public QMainWindow
{
	Q_OBJECT

public:
	MDBTools(QWidget *parent = Q_NULLPTR);
	bool CombineFile(_RecordsetPtr Org, MDBCombine* MDBobj);
	void SetGlobleProGressValue(int Value);
private:
	QTableWidget * m_TarSchema;
	vector<QProgressBar*> m_ControlsDocker;//进度条容器
	vector<QTableWidget*> m_TabDocker;//Tab的容器
	int m_fileCount;//文件计数
private:
	void ShowSourceFilesWeSelected(_bstr_t fileName,_bstr_t Percent);//显示我们选择的需要进行合并的文件
	Ui::MDBToolsClass ui;
public slots://下面为自定义的槽函数
void slots_OnTargetFileSelect();//单击选择目标文件事件
void slots_OnSourceFileSelect();//单击选择源文件
void slots_OnStartCombineClick();//单击合并按钮
void slots_OnHelpClick();//单击帮助
};
