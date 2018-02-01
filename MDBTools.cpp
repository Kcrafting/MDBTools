#include <qpushbutton.h>
#include <qtablewidget.h>
#include <qtoolbar.h>
#include <qmessagebox.h>
#include <algorithm> //for_each引用
#include <iterator>//迭代器
#include <functional>//mem_fun_ref 引出
#include <qprogressbar.h>
#include "MDBTools.h"
#include <memory>
#include "MDBClass.h"
#define ProGressMax 10000
#define ProGressMin 0

using std::shared_ptr;
struct MapWord
{
	_bstr_t ColumnName;
	_bstr_t DefaultValue;
	int LengthRequire;//判断长度要求
};
MDBClass *MDB;
MDBCombine *MDBCM;//目标文件
_bstr_t g_TarFileName;
vector<bstr_t> g_SourceFileNames;
using std::for_each;
long EveryProGressMax = 0;//每个进度条的最大值
long ProGressIndex = -1;//每个进度条下标
long GetMDBFileMaxRow(_RecordsetPtr pRs, _ConnectionPtr pCon, _RecordsetPtr pConRECORD);
void RecordSetDisplayInTableWidget(_RecordsetPtr pRSP, QTableWidget* QWD);//将记录集显示到tablewidget
bool GetRecordMatchTar(_RecordsetPtr pRecotd, _bstr_t TableName, _bstr_t ColumnName, _bstr_t DataType);//遍历查找列存在
int GetTableRowCount(_ConnectionPtr pRecord, _bstr_t ColumnName, _bstr_t TableName);//获取某个表格的行数/输出的表格中只含有一个参数/输入的字段中不能包含单引号
_bstr_t GetDefaultValue(int Code);//字段为非空时的默认字段
bool IfSrouceTableInTarget(_RecordsetPtr Source, _RecordsetPtr Target);//判断源文件中的表在目标中是否有同名表
MDBTools::MDBTools(QWidget *parent)
	: QMainWindow(parent),m_fileCount(0), m_TarSchema(nullptr)
{
	ui.setupUi(this);
	g_TarFileName = (_bstr_t)T2W(L"");
	if (FAILED(::CoInitialize(NULL)))
	{
		QMessageBox msgBox;
		msgBox.setText("初始化COM组件失败！");
		msgBox.exec();
		
		ui.label->setText(QString::fromStdWString(L"初始化COM组件失败！"));
		return;
	}
	
	//ui.setWindowIcon(QIcon(":/UploadClient/UploadClient.ico"));
	//this->setFixedHeight(this->height());
	//设置文件列表框高度
	//ui.tabWidget->addTab(this->ui.tabWidget, "thires");
	//ui.tabWidget->setCurrentIndex(0);//设置显示的页签
	//ui.tableWidget->setFixedHeight(this->height() / 5);//锁定控件高度
	ui.tableWidget->setShowGrid(true);//显示网格
	ui.tableWidget->setColumnWidth(0, this->width() / 4);//设置初始的单元格宽度
	ui.tableWidget->setColumnWidth(1, this->width() / 4);
	ui.tableWidget->setColumnWidth(2, this->width() / 4);
	ui.tableWidget->setColumnWidth(3, this->width() / 4);
	//ui.listWidget->setCellWidget(row, column, new QLineEdit);
	/*
	ui.tableWidget->setRowCount(2);//设置表格可用的最大行数
	QTableWidgetItem *TItem;//建立一个插入项
	TItem = new QTableWidgetItem(QString::fromStdWString(L"合并全部完成！"));//构造函数
	ui.tableWidget->setItem(0,0, TItem);//设置显示出来
	*/
	ui.mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);//设置动作条的显示模式
	//ui.mainToolBar->toolButtonStyle(Qt::ToolButtonTextBesideIcon);
	QToolBar* pToolBar = ui.mainToolBar;//获取动作条指针
	QAction* ActButton = new QAction(QIcon(QPixmap(":/MDBTools/100.png")), QString::fromStdWString(L"目标文件"));//构造一个按钮
	pToolBar->addAction(ActButton);//插入到动作条
	QObject::connect(ActButton, SIGNAL(triggered(bool)), this, SLOT(slots_OnTargetFileSelect()));//绑定槽
	ActButton = new QAction(QIcon(QPixmap(":/MDBTools/101.png")), QString::fromStdWString(L"来源文件"));
	pToolBar->addAction(ActButton);
	QObject::connect(ActButton, SIGNAL(triggered(bool)), this, SLOT(slots_OnSourceFileSelect()));
	ActButton = new QAction(QIcon(QPixmap(":/MDBTools/102.png")), QString::fromStdWString(L"开始合并"));
	pToolBar->addAction(ActButton);
	QObject::connect(ActButton, SIGNAL(triggered(bool)), this, SLOT(slots_OnStartCombineClick()));
	ActButton = new QAction(QIcon(QPixmap(":/MDBTools/103.png")), QString::fromStdWString(L"使用说明"));
	pToolBar->addAction(ActButton);
	QObject::connect(ActButton, SIGNAL(triggered(bool)), this, SLOT(slots_OnHelpClick()));
	QCoreApplication::processEvents();//类似于VBA DoEvents
	MDB = new MDBClass();
	MDBCM = new MDBCombine();
	ui.progressBar->setRange(ProGressMin, ProGressMax);
	ui.progressBar->setValue(ProGressMin);
	ui.statusBar->setToolTip(QString::fromStdWString(L"目标文件"));
	ui.statusBar->showMessage(QString::fromStdWString(L"就绪！"));
}

void MDBTools::slots_OnTargetFileSelect()
{
	if (nullptr != m_TarSchema && ui.tabWidget_2->count() >= 2) ui.tabWidget_2->removeTab(1);
	ui.label->setText(QString::fromStdWString(L"目标文件   "));
	g_TarFileName = MDB->GetSingleFileName();//先获取文件名称
	if (g_TarFileName == (_bstr_t)T2W(L"")) return;
	std::wstring ws = (wchar_t *)g_TarFileName;//若要正常显示则要转为qstring
	ui.label->setText(QString::fromStdWString(L"目标文件   ")+ QString::fromStdWString(ws));//设置显示
	MDBCM->SetConnectInfo(g_TarFileName);
	if (!MDBCM->Connect()) return;
	//if (!MDBCM->GetFileSchema()) return;
	if (!MDBCM->GetTableSchema()) return;
	//MDBCM->GetTableConstraint();	
	m_TarSchema = new QTableWidget();//创建一个新的TabWidget实例
	ui.tabWidget_2->insertTab(1, m_TarSchema, QString::fromStdWString(L"目标文件详情"));//在Tab中新建Tab,插入这个控件
	//_RecordsetPtr TT1 = MDBCM->get_TableSchema();
	RecordSetDisplayInTableWidget(MDBCM->get_TableSchema(), m_TarSchema);
	ui.tabWidget_2->setCurrentIndex(1);
}
void RecordSetDisplayInTableWidget(_RecordsetPtr pRSP,QTableWidget* QWD)
{
	QStringList headers;//一个用于记录表头的文本数组
	QWD->setColumnCount(pRSP->Fields->Count);//设置最大列
	int i = 0;
	while (i < pRSP->Fields->Count)
	{
		headers << QString(pRSP->Fields->Item[long(i)]->Name);
		OutputDebugString(LPCWSTR(pRSP->Fields->Item[long(i)]->Name) + (_bstr_t)T2W(_T("\n")));
		_variant_t dd = pRSP->Fields->Item[long(i)]->Name;
		i++;
	}
	QWD->setHorizontalHeaderLabels(headers);//写入表头
	int j = -1;
	while (!(pRSP->EndOfFile))//循环录入数据
	{
		j++;
		QWD->setRowCount(j + 1);//设置最大行
		for (int i = 0; i < pRSP->Fields->Count; i++)
		{
			QTableWidgetItem *TItem;
			_bstr_t temp = MDBCM->ConvertVariantV2(pRSP->Fields->GetItem(long(i))->Value);
			TItem = new QTableWidgetItem(QString::fromStdWString((std::wstring)MDBCM->ConvertVariantV2(pRSP->Fields->GetItem(long(i))->Value)));
			//QWD->setRowCount(j);
			//QWD->setColumnCount(i);
			QWD->setItem(j, i, TItem);
		}
		pRSP->MoveNext();
	}
}

_bstr_t ReplaceDYH(_bstr_t Words)//替换数据库中带有单引号的名称 替换单引号
{
	CString Temp_4 = CString((LPCSTR)Words);//初始化一个CString
	Temp_4.Replace(L"'", L"");//替换掉单引号
	return _bstr_t(Temp_4);
}
_bstr_t AddDYH(_bstr_t Words)
{
	CString Temp_4 = CString((LPCSTR)Words);//初始化一个CString
	return _bstr_t(L"'" + Temp_4 + L"'");
}
long GetMDBFileMaxRow(_RecordsetPtr pRs,_ConnectionPtr pCon, _RecordsetPtr pTar)//获取到文件的最大行数//有问题需改动
{									//源文件
	long RowCount = 0;//总行技术
	long FirstRowCount = 0;//首行取值
	//_RecordsetPtr pConRcrd;//
	//pConRcrd = pCon->Open();
	try {
		_bstr_t tablename = ReplaceDYH(MDBCM->ConvertVariantV2(pRs->Fields->GetItem("TABLE_NAME")->Value));//初始化表//tablename保留上一个表的名字 以便和当前表对比
			while (!pRs->EndOfFile)//遍历记录
			{
				
				if ((CString(W2T(ReplaceDYH(MDBCM->ConvertVariantV2(pRs->Fields->GetItem("TABLE_NAME")->Value))))).Left(2) != "MS")//如果以MS开头则是系统表格，系统表格不进行合并
				{
					//if (IfSrouceTableInTarget(pRs, pTar))
					{
						//第一次运行，获取第一个表的行数
						OutputDebugString(_bstr_t(">>>>>>>>>>")  + ReplaceDYH(MDBCM->ConvertVariantV2(pRs->Fields->GetItem("TABLE_NAME")->Value)) + _bstr_t("<<<<<<<<<<\n"));
						if (FirstRowCount == 0) FirstRowCount = GetTableRowCount(pCon, ReplaceDYH(MDBCM->ConvertVariantV2(pRs->Fields->GetItem("COLUMN_NAME")->Value)), ReplaceDYH(MDBCM->ConvertVariantV2(pRs->Fields->GetItem("TABLE_NAME")->Value)));
						_RecordsetPtr pRRs;
						_bstr_t Temp10 = MDBCM->ConvertVariantV2(pRs->Fields->GetItem("TABLE_NAME")->Value);//获取到当前的表明
						if (ReplaceDYH(tablename) != ReplaceDYH(Temp10))//如果这个是新表，计算行数（这个方法会导致第一个表不计算的问题 所有第一个表要单独计数）
						{
							//_variant_t re;
							//_bstr_t Temp = bstr_t(L"select count(") + ReplaceDYH(MDBCM->ConvertVariantV2(pRs->Fields->GetItem("COLUMN_NAME")->Value)) +
								//_bstr_t(L") as rowcount from ") + ReplaceDYH(MDBCM->ConvertVariantV2(pRs->Fields->GetItem("TABLE_NAME")->Value));
							RowCount += GetTableRowCount(pCon, ReplaceDYH(MDBCM->ConvertVariantV2(pRs->Fields->GetItem("COLUMN_NAME")->Value)), ReplaceDYH(MDBCM->ConvertVariantV2(pRs->Fields->GetItem("TABLE_NAME")->Value)));
							OutputDebugString(_bstr_t("表行计数     :") + ReplaceDYH(MDBCM->ConvertVariantV2(pRs->Fields->GetItem("TABLE_NAME")->Value)) + _bstr_t("  :   ") +
								_bstr_t(GetTableRowCount(pCon, ReplaceDYH(MDBCM->ConvertVariantV2(pRs->Fields->GetItem("COLUMN_NAME")->Value)), ReplaceDYH(MDBCM->ConvertVariantV2(pRs->Fields->GetItem("TABLE_NAME")->Value)))) + _bstr_t("\n"));
							OutputDebugString(_bstr_t(CString(W2T(tablename))) + _bstr_t(L"   查找结果   ") + _bstr_t((CString(W2T(tablename))).Left(2) != "MS") + _bstr_t(L"\n"));
						}
					}
					//pRRs->open
				}
				tablename = ReplaceDYH(MDBCM->ConvertVariantV2(pRs->Fields->GetItem("TABLE_NAME")->Value));
				pRs->MoveNext();

			}
	}
	catch (_com_error &e)
	{
		MessageBox(NULL,e.Description(),L"",NULL);
	}
	OutputDebugString(_bstr_t(L"所有记录共计   ") + _bstr_t(RowCount + FirstRowCount) + _bstr_t(L"行！"));
	return RowCount + FirstRowCount;
}
bool IfSrouceTableInTarget(_RecordsetPtr Source, _RecordsetPtr Target)//判断源文件中的表在目标中是否有同名表
{
	Source->MoveFirst();
	Target->MoveFirst();
	while (!Target->EndOfFile)
	{
		while (!Source->EndOfFile)
		{
			OutputDebugString(ReplaceDYH(MDBCM->ConvertVariantV2(Source->Fields->GetItem("TABLE_NAME")->Value)) + _bstr_t(L"    对比    ") + ReplaceDYH(MDBCM->ConvertVariantV2(Target->Fields->GetItem("TABLE_NAME")->Value)) + _bstr_t(L"    \n"));
			if (ReplaceDYH(MDBCM->ConvertVariantV2(Source->Fields->GetItem("TABLE_NAME")->Value)) == ReplaceDYH(MDBCM->ConvertVariantV2(Target->Fields->GetItem("TABLE_NAME")->Value)))
			{
				OutputDebugString(ReplaceDYH(MDBCM->ConvertVariantV2(Source->Fields->GetItem("TABLE_NAME")->Value)) + _bstr_t(L"    对比    ") + ReplaceDYH(MDBCM->ConvertVariantV2(Target->Fields->GetItem("TABLE_NAME")->Value)) + _bstr_t(L"    \n"));
				return true;
			}
			Source->MoveNext();
		}
		Target->MoveNext();
	}
	return false;
}
int GetTableRowCount(_ConnectionPtr pRecord,_bstr_t ColumnName,_bstr_t TableName=L"")//获取某个表格的行数/输出的表格中只含有一个参数/输入的字段中不能包含单引号
{
	_RecordsetPtr pRRs;
	_variant_t re;
	if (TableName == _bstr_t(L""))
	{
		_bstr_t Temp1 = bstr_t(L"select * from ") + ColumnName + bstr_t(L" ;");
		try {
			pRRs = pRecord->Execute(Temp1, &re, adCmdText);
		}
		catch (...)
		{
			return 0;
		}
		return 1;
	}
	int RowCount = 0;

	_bstr_t Temp = bstr_t(L"select count(") + ColumnName +
		_bstr_t(L") as rowcount from ") + TableName;
	try {
		pRRs = pRecord->Execute(Temp, &re, adCmdText);
	}
	catch (...)
	{
		return 0;
	}
	if (!(pRRs->EndOfFile && pRRs->BOF))
	{
		pRRs->MoveFirst();
		RowCount += _ttoi((CString(W2T(MDBCM->ConvertVariantV2(pRRs->Fields->GetItem("rowcount")->Value)))));
	}
	return RowCount;
}
bool MDBTools::CombineFile(_RecordsetPtr Tar, MDBCombine* MDBobj)//合并文件主过程
{
	//此处位合并的主要过程
	//所有 
	long TableWidgetRowCount = 1;
	long CurrentRow = 0;
	long MaxRow = 0;
	//MaxRow=MDBobj->get_TableSchema()->PageCount;
	MaxRow = GetMDBFileMaxRow(MDBobj->get_TableSchema(), MDBobj->get_ConnectPtr(),Tar);
	(*m_ControlsDocker.at(ProGressIndex)).setRange(0, MaxRow);
	/***************************************************
	1.判断表名相同，列名相同，数据类型相同，约束相同
	****************************************************/
	//MDBobj->get_TableSchema()->Open("SELECT * FROM CBDKXX"/*MDBobj->get_ConnectionString()*/,
		//MDBCM->get_ConnectPtr().GetInterfacePtr(), adOpenStatic, adLockOptimistic, adCmdTable);
	//long MaxRow = MDBobj->get_TableSchema()->GetRecordCount();
	_RecordsetPtr Sou = MDBobj->get_TableSchema();
	_bstr_t LastTableName = T2W(L"");//临时置空
	_bstr_t ColumnName = L"";
	try {
		Tar->MoveFirst();
		//开始初始化
		LastTableName = MDBCM->ConvertVariantV2(Tar->Fields->GetItem("TABLE_NAME")->Value);
		//Tar->Fields->GetItem("TABLE_NAME")->Value;
		//Tar->Open(MDBCM->get_ConnectionString(), _variant_t(MDBCM->get_ConnectPtr()), adOpenStatic, adLockReadOnly, adCmdText);
		//bool aa = Tar->GetState() == adStateOpen;
			//Tar->MoveFirst();
		HRESULT Hr = S_OK;
		_bstr_t NeceColumnValue = L"";//非空列
		_bstr_t UnNeceColumnValue = L"";//可空存在列
		_bstr_t INSERTTEXT = L"";//
		vector<_bstr_t> ColumnNamesOfTable_Nece;//必须列
		vector<_bstr_t> ColumnNamesOfTable_NeceNotExist;//必须但在原表中不存在的列
		vector<_bstr_t> ColumnNamesOfTable_UnNece;//非必需列
		vector<_bstr_t> ColumnNamesOfTable;//汇总
		vector<MapWord> ColumnDataMap;//汇总
		//初始化结束！
		while (!Tar->EndOfFile)//开始读取
		{
			//if ((CString(W2T((_bstr_t)(Tar->Fields->GetItem("TABLE_NAME")->Value)))).Left(2) != "MS")//以MS打头的大多时系统表格，忽略
			
				HRESULT HRS = S_OK;
				_bstr_t SQL = L"";
				//_bstr_t ColumnName = L"";
				//if (Sou->Supports(adSeek) == VARIANT_FALSE) return false;//查询是否支持seek
				if (MDBCM->ConvertVariantV2(Tar->Fields->GetItem("TABLE_NAME")->Value) == LastTableName)//先判断是否需要提交
					//当表格发生变化的时候提交上一个表格的改动
				{
					if (MDBCM->ConvertVariantV2(Tar->Fields->GetItem("IS_NULLABLE")->Value) == _bstr_t(L"FALSE"))
					{
						//如果列时非必需的那么在插入的时候进行查找，找不到则不插入改列
						//_bstr_t SQLText = _bstr_t(L"TABLE_NAME = '") + MDBCM->ConvertVariantV2(Tar->Fields->GetItem("TABLE_NAME")->Value) +bstr_t(L"' and ");
						//Hr=Tar->Seek(_variant_t("dkxx,fbf"), adSeekFirstEQ);不支持
						//HRS = Tar->Find(_bstr_t(L"TABLE_NAME like '%")+ MDBCM->ConvertVariantV2(Tar->Fields->GetItem("TABLE_NAME")->Value)+_bstr_t(L"%'"),1,adSearchForward);
						_bstr_t Temp_0 = MDBCM->ConvertVariantV2(Tar->Fields->GetItem("COLUMN_NAME")->Value);//获取到列名
						if (GetRecordMatchTar(Sou, MDBCM->ConvertVariantV2(Tar->Fields->GetItem("TABLE_NAME")->Value),
							MDBCM->ConvertVariantV2(Tar->Fields->GetItem("COLUMN_NAME")->Value),
							MDBCM->ConvertVariantV2(Tar->Fields->GetItem("DATA_TYPE")->Value)))//判断该列名在源数据库中是否存在，如果存在则
						{
							MapWord ThisColumnReqiuire;
							ThisColumnReqiuire.ColumnName = ReplaceDYH( MDBCM->ConvertVariantV2(Tar->Fields->GetItem("COLUMN_NAME")->Value));
							ThisColumnReqiuire.DefaultValue = GetDefaultValue(atoi((const char*)MDBCM->ConvertVariantV2(Tar->Fields->GetItem("DATA_TYPE")->Value)));
							ThisColumnReqiuire.LengthRequire = atoi((const char*)MDBCM->ConvertVariantV2(Tar->Fields->GetItem("CHARACTER_MAXIMUM_LENGTH")->Value));
							OutputDebugString(_bstr_t(L"非空字段:")+ ThisColumnReqiuire.ColumnName + _bstr_t(L"   默认值为:")+ ThisColumnReqiuire.DefaultValue  + _bstr_t(L"   要求长度:") + _bstr_t(ThisColumnReqiuire.LengthRequire) + _bstr_t(L"\n"));
							m_TabDocker.at(ProGressIndex)->setRowCount(TableWidgetRowCount++);
							m_TabDocker.at(ProGressIndex)->setItem(TableWidgetRowCount - 2, 0, new QTableWidgetItem(QString::fromStdWString((wchar_t *)(_bstr_t(L"非空字段:") + ThisColumnReqiuire.ColumnName + _bstr_t(L"   默认值为:") + ThisColumnReqiuire.DefaultValue + _bstr_t(L"   要求长度:") + _bstr_t(ThisColumnReqiuire.LengthRequire) + _bstr_t(L"\n")))));
							ColumnDataMap.push_back(ThisColumnReqiuire);
							_bstr_t Temp_3 = MDBCM->ConvertVariantV2(Tar->Fields->GetItem("COLUMN_NAME")->Value);//获取到列名
							CString Temp_4 = CString((LPCSTR)Temp_3);//初始化一个CString
							Temp_4.Replace(L"'", L"");//替换掉单引号
							//_bstr_t Temp_2 = _bstr_t(CString((LPCSTR)MDBCM->ConvertVariantV2(Tar->Fields->GetItem("COLUMN_NAME")->Value)).Replace(L"'", L""));
							if (NeceColumnValue == _bstr_t(L""))//必须字段
								NeceColumnValue += _bstr_t(Temp_4);//判断累加
							else
								NeceColumnValue += L"," + _bstr_t(Temp_4);//需要加逗号的累加

							ColumnNamesOfTable_Nece.push_back(MDBCM->ConvertVariantV2(Tar->Fields->GetItem("COLUMN_NAME")->Value));//将列名加入vector
						}
						else//判断该列名在源数据库中是否存在，如果不存在则
						{
							CString Temp_5 = CString((LPCSTR)MDBCM->ConvertVariantV2(Tar->Fields->GetItem("COLUMN_NAME")->Value));//去单引号
							Temp_5.Replace(L"'", L"");//去单引号
							if (INSERTTEXT == _bstr_t(L""))//判断累加方式
								INSERTTEXT = GetDefaultValue(atoi((const char*)MDBCM->ConvertVariantV2(Tar->Fields->GetItem("DATA_TYPE")->Value))) + L" as " +
								_bstr_t(Temp_5);//为空复制
							else//判断累加方式
								INSERTTEXT += _bstr_t(L",") + GetDefaultValue(atoi((const char*)MDBCM->ConvertVariantV2(Tar->Fields->GetItem("DATA_TYPE")->Value))) + L" as " +
								_bstr_t(Temp_5);//有值累加
							ColumnNamesOfTable_NeceNotExist.push_back(MDBCM->ConvertVariantV2(Tar->Fields->GetItem("COLUMN_NAME")->Value));//将列名加入vector
						}
						//else 如果原表中不存在该列，则忽略
					}
					else//如果列可以为空
					{
						//_bstr_t Temp_1 = MDBCM->ConvertVariantV2(Tar->Fields->GetItem("COLUMN_NAME")->Value);//测试用
						if (GetRecordMatchTar(Sou, MDBCM->ConvertVariantV2(Tar->Fields->GetItem("TABLE_NAME")->Value),
							MDBCM->ConvertVariantV2(Tar->Fields->GetItem("COLUMN_NAME")->Value),
							MDBCM->ConvertVariantV2(Tar->Fields->GetItem("DATA_TYPE")->Value)))//判断该列是否在原表中存在
						{
							//列位必须时，查找该列，找不到则使用默认值进行填充
							_bstr_t Temp_3 = MDBCM->ConvertVariantV2(Tar->Fields->GetItem("COLUMN_NAME")->Value);//获取到列名
							CString Temp_4 = CString((LPCSTR)Temp_3);//初始化一个CString
							Temp_4.Replace(L"'", L"");//替换掉单引号
							if (UnNeceColumnValue == _bstr_t(L""))//可空字段
								UnNeceColumnValue += _bstr_t(Temp_4);//累加方式
							else
								UnNeceColumnValue += L"," + _bstr_t(Temp_4);//累加方式
							ColumnNamesOfTable_UnNece.push_back(MDBCM->ConvertVariantV2(Tar->Fields->GetItem("COLUMN_NAME")->Value));//加入vector
						}
						//else//原表中不存在该列 因为可空 所以放弃
						//{
							//_bstr_t x;
							//int i = atoi((const char*)x);
							//非必需字段查不到，结束
						//}
					}
				}
				else//不同时则进行提交动作
				{
					vector<_bstr_t>::iterator iter;
					//MDB->GetTableList();
					for (iter = ColumnNamesOfTable_Nece.begin(); iter != ColumnNamesOfTable_Nece.end(); iter++)
					{
						CString T1 = CString((LPCSTR)*iter);
						T1.Replace(L"'", L"");
						ColumnNamesOfTable.push_back(_bstr_t(T1));
						OutputDebugString(_bstr_t(L"输出必须字段:")+ _bstr_t(T1)+_bstr_t(L"\n"));
						m_TabDocker.at(ProGressIndex)->setRowCount(TableWidgetRowCount++);
						m_TabDocker.at(ProGressIndex)->setItem(TableWidgetRowCount - 2, 0, new QTableWidgetItem(QString::fromStdWString((wchar_t *)(_bstr_t(L"输出必须字段:") + _bstr_t(T1) + _bstr_t(L"\n")))));
					}
					for (iter = ColumnNamesOfTable_UnNece.begin(); iter != ColumnNamesOfTable_UnNece.end(); iter++)
					{
						CString T2 = CString((LPCSTR)*iter);
						T2.Replace(L"'", L"");
						ColumnNamesOfTable.push_back(_bstr_t(T2));
					}
					for (iter = ColumnNamesOfTable_NeceNotExist.begin(); iter != ColumnNamesOfTable_NeceNotExist.end(); iter++)
					{
						CString T3 = CString((LPCSTR)*iter);
						T3.Replace(L"'", L"");
						ColumnNamesOfTable.push_back(_bstr_t(T3));
					}
					ColumnNamesOfTable_Nece.clear();
					ColumnNamesOfTable_UnNece.clear();
					ColumnNamesOfTable_NeceNotExist.clear();
					_bstr_t InsertText;
					for (iter = ColumnNamesOfTable.begin(); iter != ColumnNamesOfTable.end(); iter++)
					{
						if (InsertText == _bstr_t(L""))
						{
							//CString T1= CString((LPCSTR)*iter);
							//T1.Replace(L"'", L"");
							InsertText = *iter;//_bstr_t(T1);
						}
						else
						{
							//CString T2 = CString((LPCSTR)*iter);
							//T2.Replace(L"'", L"");
							InsertText += *iter; //_bstr_t(L",") + _bstr_t(T2);
						}
					}
					CString TableName = CString((LPCSTR)LastTableName);
					TableName.Replace(L"'", L"");
					LastTableName = _bstr_t(TableName);//去掉表明中的单引号
					_bstr_t Temp_0;//= _bstr_t(L"select ");//查询语句 根据列名的存在与否 组合语句
					if (NeceColumnValue != _bstr_t(L""))
						Temp_0 += NeceColumnValue;
					if (UnNeceColumnValue != _bstr_t(L""))
						Temp_0 += _bstr_t(L",") + UnNeceColumnValue;
					if (INSERTTEXT != _bstr_t(L""))
						Temp_0 += _bstr_t(L",") + INSERTTEXT;
					Temp_0 = _bstr_t(L"select ") + Temp_0 + _bstr_t(L" from ") + _bstr_t(TableName);
					//OutputDebugString(Temp_0+L"\n");//输出选择的sql语句
					//OutputDebugString((_bstr_t)T2W(_T("\n")) + UnNeceColumnValue + NeceColumnValue);
					_variant_t ExeResultStatus;
					_RecordsetPtr Result = MDBobj->get_ConnectPtr()->Execute(Temp_0, &ExeResultStatus, adCmdText);//把所有的信息存入Recordset再遍历插入
					m_TabDocker.at(ProGressIndex)->setRowCount(TableWidgetRowCount++);
					m_TabDocker.at(ProGressIndex)->setItem(TableWidgetRowCount - 2, 0, new QTableWidgetItem(QString::fromStdWString((wchar_t *)Temp_0)));
					//long NowTheTableWidgetRowCount = m_TabDocker.at(ProGressIndex)->rowCount();
					while (!Result->EndOfFile)
					{
						_bstr_t InsertValue;
						_bstr_t InsertVolumns;
						//RecordSetDisplayInTableWidget(Result,ui.tableWidget);
						for (long i = 0; i < Result->Fields->Count; i++)
						{
							if (InsertValue == _bstr_t(L""))
							{

								for (long j = 0; j < Result->Fields->Count; j++)
								{
									//OutputDebugString(L"\n" + Result->Fields->Item[j]->Name);
								}
								//_bstr_t Temp = ColumnNamesOfTable.at(i);
								_bstr_t Value_Temp= MDBCM->ConvertVariantV2(Result->Fields->GetItem(_variant_t(ColumnNamesOfTable.at(i)))->Value);
								//**********************************
								vector<MapWord>::iterator iter;//检测非空字段必须不为空
								//MDB->GetTableList();
								for (iter = ColumnDataMap.begin(); iter != ColumnDataMap.end(); iter++)
								{
									if (iter->ColumnName == ColumnNamesOfTable.at(i))//必须字段
									{
										if (MDBCM->ConvertVariantV2(Result->Fields->GetItem(_variant_t(ColumnNamesOfTable.at(i)))->Value) == _bstr_t(L"")
											|| MDBCM->ConvertVariantV2(Result->Fields->GetItem(_variant_t(ColumnNamesOfTable.at(i)))->Value) == _bstr_t(L"NULL")
											|| MDBCM->ConvertVariantV2(Result->Fields->GetItem(_variant_t(ColumnNamesOfTable.at(i)))->Value) == _bstr_t(L"''"))
										{
											Value_Temp = iter->DefaultValue;
											int TLen = CString(W2T(ReplaceDYH(iter->DefaultValue))).GetLength();
											if (!(TLen <= iter->LengthRequire || iter->LengthRequire == 0))//必须大于要求长度，要求长度必须不为0
											{
												Value_Temp = (CString(W2T(ReplaceDYH(iter->DefaultValue)))).Left(iter->LengthRequire);
												Value_Temp = AddDYH(Value_Temp);
											}
											break;
										}
									}
								}
								//**********************************
								InsertValue = Value_Temp;
								InsertVolumns = ColumnNamesOfTable.at(i);
							}
							else
							{
								_bstr_t Value_Temp = MDBCM->ConvertVariantV2(Result->Fields->GetItem(_variant_t(ColumnNamesOfTable.at(i)))->Value);
								//**********************************
								vector<MapWord>::iterator iter;//检测非空字段必须不为空
															   //MDB->GetTableList();
								for (iter = ColumnDataMap.begin(); iter != ColumnDataMap.end(); iter++)
								{
									if (iter->ColumnName == ColumnNamesOfTable.at(i))//必须字段
									{
										if (LastTableName==_bstr_t(L"CBF_JTCY"))//本if测试用
										{
											_bstr_t Temp = MDBCM->ConvertVariantV2(Result->Fields->GetItem(_variant_t(ColumnNamesOfTable.at(i)))->Value);
											//OutputDebugString(Temp+L"\n");
											
										}
										if (MDBCM->ConvertVariantV2(Result->Fields->GetItem(_variant_t(ColumnNamesOfTable.at(i)))->Value) == _bstr_t(L"")
											|| MDBCM->ConvertVariantV2(Result->Fields->GetItem(_variant_t(ColumnNamesOfTable.at(i)))->Value) == _bstr_t(L"NULL")
											|| MDBCM->ConvertVariantV2(Result->Fields->GetItem(_variant_t(ColumnNamesOfTable.at(i)))->Value) == _bstr_t(L"''"))
										{
											Value_Temp = iter->DefaultValue;
											int TLen = CString(W2T(ReplaceDYH(iter->DefaultValue))).GetLength();
											if (!(TLen <= iter->LengthRequire || iter->LengthRequire == 0))//
											{
												Value_Temp = (CString(W2T(ReplaceDYH(iter->DefaultValue)))).Left(iter->LengthRequire);
												Value_Temp = AddDYH(Value_Temp);
											}
											break;
										}
									}
								}
								//**********************************
								InsertValue += L"," + Value_Temp;
								//InsertValue += L"," + MDBCM->ConvertVariantV2(Result->Fields->GetItem(_variant_t(ColumnNamesOfTable.at(i)))->Value);
								InsertVolumns += L"," + ColumnNamesOfTable.at(i);
							}
						}
						_variant_t ExeResult;
						if ((CString(W2T(LastTableName))).Left(2) != "MS")//过滤系统表
						{
							_bstr_t Temp = L"insert into " + LastTableName + L"(" + InsertVolumns + L")Values(" + InsertValue + ")";
							OutputDebugString(Temp + L"\n");//输出调试语句
							CurrentRow++;
							MDBCM->get_ConnectPtr()->Execute(Temp, &ExeResult, adCmdText);
							(*m_ControlsDocker.at(ProGressIndex)).setValue(CurrentRow);//设置进度条
							QTableWidgetItem *TItem;//建立一个插入项
							TItem = new QTableWidgetItem(_bstr_t(CurrentRow) + _bstr_t(L"/") + QString::fromStdWString((wchar_t *)( _bstr_t((*m_ControlsDocker.at(ProGressIndex)).maximum())  + _bstr_t(L"  已完成/总数目")  )));//构造函数
							//long ProGressValue = ui.progressBar->value() + EveryProGressMax * (float(CurrentRow / float((*m_ControlsDocker.at(ProGressIndex)).maximum())));
							long ProGressValue = EveryProGressMax*(float(CurrentRow) /(*m_ControlsDocker.at(ProGressIndex)).maximum());
							//m_TabDocker.at(CurrentRow)->setColumnCount()
							//m_TabDocker.at(ProGressIndex)->setRowCount(TableWidgetRowCount++);
							//m_TabDocker.at(ProGressIndex)->setItem(TableWidgetRowCount, 1, new QTableWidgetItem(_bstr_t(L"输出必须字段:") + _bstr_t(T1) + _bstr_t(L"\n")));
							m_TabDocker.at(ProGressIndex)->setRowCount(TableWidgetRowCount++);
							QTableWidgetItem *TabTItem = new QTableWidgetItem(QString::fromStdWString((wchar_t *)Temp));//建立一个插入项 
							m_TabDocker.at(ProGressIndex)->setItem(TableWidgetRowCount - 2,0, TabTItem);
							ui.progressBar->setValue(ProGressIndex*EveryProGressMax + ProGressValue);
							ui.tableWidget->setItem(ProGressIndex, 2, TItem);//显示文字
							QApplication::processEvents();
							//delete TItem;
							//delete TabTItem;
						}
						Result->MoveNext();
						//MDBCM->get_ConnectPtr()->Execute();
					}
					ColumnNamesOfTable.clear();
					NeceColumnValue = L"";
					UnNeceColumnValue = L"";
					INSERTTEXT = L"";
					//MessageBox(NULL, UnNeceColumnValue + NeceColumnValue,L"",MB_OK);
				}
				bool IfNewTable = MDBCM->ConvertVariantV2(Tar->Fields->GetItem("TABLE_NAME")->Value) == LastTableName;//若本次判定和上表不同则本次没有执行表格的合并流程，所以不进行游标下移动作
				LastTableName = MDBCM->ConvertVariantV2(Tar->Fields->GetItem("TABLE_NAME")->Value);
				if (IfNewTable)
					Tar->MoveNext();
				//else
					//Tar->MovePrevious();
			
		}
		//合并完成
		
	}
	catch (_com_error &e)
	{
		
		_bstr_t bb = e.Description();
		//MessageBox(NULL, L"发生错误！\n"+e.Description() + L"\n" + _bstr_t((CString(W2T((_bstr_t)(Tar->Fields->GetItem("TABLE_NAME")->Value)))).Left(2)), L"错误提示！", NULL);
		//return false;
		m_TabDocker.at(ProGressIndex)->setRowCount(TableWidgetRowCount++);
		m_TabDocker.at(ProGressIndex)->setItem(TableWidgetRowCount - 2, 0, new QTableWidgetItem(QString::fromStdWString((wchar_t *)(L"发生错误！" + e.Description() + _bstr_t((CString(W2T((_bstr_t)(Tar->Fields->GetItem("TABLE_NAME")->Value)))).Left(2)) + L"错误提示！"))));
	
	}
	return true;
}
bool GetRecordMatchTar(_RecordsetPtr pRecord, _bstr_t TableName, _bstr_t ColumnName, _bstr_t DataType)//获取到与源文件记录相同的咧
{
	pRecord->MoveFirst();
	while (!pRecord->EndOfFile)
	{
		_bstr_t TABLE_NAME = MDBCM->ConvertVariantV2(pRecord->Fields->GetItem("TABLE_NAME")->Value);
		_bstr_t COLUMN_NAME = MDBCM->ConvertVariantV2(pRecord->Fields->GetItem("COLUMN_NAME")->Value);
		_bstr_t DATA_TYPE = MDBCM->ConvertVariantV2(pRecord->Fields->GetItem("DATA_TYPE")->Value);
		if (MDBCM->ConvertVariantV2(pRecord->Fields->GetItem("TABLE_NAME")->Value) == TableName &&
			MDBCM->ConvertVariantV2(pRecord->Fields->GetItem("COLUMN_NAME")->Value) == ColumnName &&
			MDBCM->ConvertVariantV2(pRecord->Fields->GetItem("DATA_TYPE")->Value) == DataType)
		{
			return true;
		}
		pRecord->MoveNext();
	}
	return false;
}
_bstr_t GetDefaultValue(int Code)//获取到某种类型的默认值
{
	if (Code == 0x2000) return _bstr_t(L"'AdArray'");
	if (Code == 20) return _bstr_t(L"0");//_bstr_t(L"adBigInt");
	if (Code == 128) return _bstr_t(L"'adBinary'");
	if (Code == 11) return _bstr_t(L"'FALSE'");//_bstr_t(L"adBoolean");
	if (Code == 8) return _bstr_t(L"'adBSTR'");
	if (Code == 136) return _bstr_t(L"'adChapter'");
	if (Code == 129) return _bstr_t(L"'adChar'");
	if (Code == 6) return _bstr_t(L"'adCurrency'");
	if (Code == 7) return _bstr_t(L"'2000/01/01'");//_bstr_t(L"adDate");
	if (Code == 133) return _bstr_t(L"'2000/01/01'");//_bstr_t(L"adDBDate");
	if (Code == 134) return _bstr_t(L"'12:12:12'");//_bstr_t(L"adDBTime");
	if (Code == 135) return _bstr_t(L"'2000/01/01 12:12:12'");//_bstr_t(L"adDBTimeStamp");
	if (Code == 14) return _bstr_t(L"0.0");//_bstr_t(L"adDecimal");
	if (Code == 5) return _bstr_t(L"0.0");//_bstr_t(L"adDouble");
	if (Code == 0) return _bstr_t(L"'adEmpty'");
	if (Code == 10) return _bstr_t(L"'adError'");
	if (Code == 64) return _bstr_t(L"'2000/01/01'");//_bstr_t(L"adFileTime");
	if (Code == 72) return _bstr_t(L"'adGUID'");
	if (Code == 9) return _bstr_t(L"'adIDispatch'");
	if (Code == 3) return  _bstr_t(L"0");//_bstr_t(L"adInteger");
	if (Code == 13) return _bstr_t(L"'adIUnknown'");
	if (Code == 205) return _bstr_t(L"'adLongVarBinary'");
	if (Code == 201) return _bstr_t(L"'adLongVarChar'");
	if (Code == 203) return _bstr_t(L"'adLongVarWChar'");
	if (Code == 131) return _bstr_t(L"0");//_bstr_t(L"adNumeric");
	if (Code == 138) return _bstr_t(L"'adPropVariant'");
	if (Code == 4) return _bstr_t(L"0.0");//_bstr_t(L"adSingle");
	if (Code == 2) return _bstr_t(L"0");//_bstr_t(L"adSmallInt");
	if (Code == 16) return _bstr_t(L"0");//_bstr_t(L"adTinyInt");
	if (Code == 21) return _bstr_t(L"0");//_bstr_t(L"adUnsignedBigInt");
	if (Code == 19) return _bstr_t(L"0");//_bstr_t(L"adUnsignedInt");
	if (Code == 18) return _bstr_t(L"0");//_bstr_t(L"adUnsignedSmallInt");
	if (Code == 17) return _bstr_t(L"0");//_bstr_t(L"adUnsignedTinyInt");
	if (Code == 132) return _bstr_t(L"'adUserDefined'");
	if (Code == 204) return _bstr_t(L"'adVarBinary'");
	if (Code == 200) return _bstr_t(L"'adVarChar'");
	if (Code == 12) return _bstr_t(L"'adVariant'");
	if (Code == 139) return _bstr_t(L"0"); //_bstr_t(L"adVarNumeric");
	if (Code == 202) return _bstr_t(L"'adVarWChar'");
	if (Code == 130) return _bstr_t(L"'adWChar'");
	return _bstr_t(L"'NULL'");

}

void MDBTools::SetGlobleProGressValue(int Value)//设置主界面上的进度条
{
	if (Value >= ProGressMin && Value <= ProGressMax)
		ui.progressBar->setValue(Value);
}

void MDBTools::ShowSourceFilesWeSelected(_bstr_t fileName,_bstr_t Percent)//显示选择的源文件到界面
{
	ui.statusBar->showMessage(QString::fromStdWString(L"正在载入文件信息！   ") + Percent, 2000);
	MDBCombine TempClass(fileName);
	ui.tableWidget->setRowCount(ui.tableWidget->rowCount() + 1);//更新最大行/最大行是下一行/非当前行
	QTableWidgetItem *TItem;//建立一个插入项
	TItem = new QTableWidgetItem(QString::fromStdWString((wchar_t *)fileName));//构造函数
	/*****************测试**************
	QMessageBox msgBox;
	msgBox.setText(QString::number(ui.tableWidget->rowCount(),10));
	msgBox.exec();
	*************************************/
	ui.tableWidget->setItem((ui.tableWidget->rowCount()-1), 0, TItem);//设置显示出来
	QProgressBar *EveryProgress = new QProgressBar();//新建一个进度条实例
	EveryProgress->setTextVisible(false);//设置进度条文字不可见
	ui.tableWidget->setCellWidget((ui.tableWidget->rowCount() - 1),1, EveryProgress);//设置单个进度条进tablewidget
	m_ControlsDocker.push_back(EveryProgress);//将进度条加入容器
	//动态添加Tab的方法
	QTableWidget *EveryNewTab = new QTableWidget();//创建一个新的Tab实例
	ui.tabWidget->insertTab(0, EveryNewTab, QString::number(m_fileCount,10));//在tab中新建tab,插入这个控件
	m_TabDocker.push_back(EveryNewTab);
	QApplication::processEvents();
	if (!TempClass.Connect())
	{
		//delete TempClass;
		return;
	}
		
	if (!TempClass.GetTableSchema())
	{
		//delete TempClass;
		return;
	}
	//RecordSetDisplayInTableWidget(TempClass.get_TableSchema(), EveryNewTab);
	ui.tabWidget_2->setCurrentIndex(0);
	//delete TempClass;
}

void MDBTools::slots_OnSourceFileSelect()//选择源文件
{
	m_ControlsDocker.clear();
	if (m_ControlsDocker.size() != 0)
	{
		vector<QProgressBar *>::iterator iter;
		for (iter = m_ControlsDocker.begin(); iter != m_ControlsDocker.end(); iter++)
		{
			delete *iter;
		}

	}
	if (m_TabDocker.size() != 0)
	{
		vector<QTableWidget *>::iterator iter;
		for (iter = m_TabDocker.begin(); iter != m_TabDocker.end(); iter++)
		{
			delete *iter;
		}
	}
	m_fileCount = 0;
	ui.tableWidget->clearContents();
	ui.tabWidget->clear();
	ui.tableWidget->setRowCount(0);
	g_SourceFileNames = MDB->GetMultiFileName();//获取选中的文件名称
	if (0 == g_SourceFileNames.size()) return;
	//for_each(g_SourceFileNames.begin(), g_SourceFileNames.end(), std::mem_fun_ref(&MDBTools::ShowSourceFilesWeSelected));//遍历显示
	vector<_bstr_t>::iterator iter;
	//MDB->GetTableList();
	for (iter = g_SourceFileNames.begin(); iter != g_SourceFileNames.end(); iter++)
	{
		if (*iter != g_TarFileName)
		{
			m_fileCount++;
			ShowSourceFilesWeSelected(*iter, _bstr_t(m_fileCount) + _bstr_t("/") + _bstr_t(g_SourceFileNames.size()));
		}
	}
	ui.statusBar->showMessage(QString::fromStdWString(L"文件信息已经全部载入完成！！！"));
}

void MDBTools::slots_OnStartCombineClick()
{
	if (!MDBCM->get_ConnectPtr())
	{
		MessageBox(NULL,L"还没有选择目标文件！",L"错误提示",MB_OK);
		return;
	}
  	if (g_SourceFileNames.size() == (size_t)0)
	{
		MessageBox(NULL, L"还没有选择要合并的源文件！", L"错误提示", MB_OK);
		return;
	}
	vector<_bstr_t>::iterator iter;
	EveryProGressMax = ProGressMax / g_SourceFileNames.size();
	for (iter = g_SourceFileNames.begin(); iter != g_SourceFileNames.end(); iter++)
	{
		if (*iter != g_TarFileName)//目标和源文件不能是同一文件
		{
			ProGressIndex++;
			MDBCombine  pCombine(*iter);
			pCombine.Connect();
			pCombine.GetTableSchema();
			m_TabDocker.at(ProGressIndex)->setColumnCount(1);
			m_TabDocker.at(ProGressIndex)->setHorizontalHeaderLabels(QStringList()<<QString::fromStdWString(L"合并信息"));
			m_TabDocker.at(ProGressIndex)->setColumnWidth(0,this->width());
			//m_TabDocker.at(ProGressIndex)->setRowCount(1000);
			if (!CombineFile(MDBCM->get_TableSchema(), &pCombine))
				ui.tableWidget->setItem(ProGressIndex,2,new QTableWidgetItem(QString::fromStdWString(L"合并失败！")));
				//MessageBox(NULL, *iter + L"\n合并失败！", L"提示", NULL);
			//delete pCombine;
		}
	}
	delete MDBCM;
	MessageBox(NULL, L"合并完成！", L"提示", MB_OK);
	//delete MDBCM;
}

void MDBTools::slots_OnHelpClick()
{
	QMessageBox msgBox;
	QString msg =
		QString::fromStdWString(L"请注意，本工具适用于MDB文件合并，有以下需要注意的事项\n1.所有合并的文件的表，列，数据类型，") +
		QString::fromStdWString(L"约束，全部都以目标文件为主\n2.主机上需要安装x64的MDB数据引擎\n3.同时需要安装64位的Office，否则不能使用") +
		QString::fromStdWString(L"\n4.当原表中的数据与目标表格中的约束不符时，将对原数据进行修整以进行合并，如截断，或加长\n5.当原表中的非空列在原表中没有时，将插入默认数据！")+
		QString::fromStdWString(L"\n6.不同的数据类型对应的默认值不同，具体情况见下表：");
	msgBox.setText(msg);
	msgBox.exec();
}
