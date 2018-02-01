#pragma once
/*
#define _AFXDLL
#include <afxwin.h>
#include <comdef.h>
#include <tchar.h>
#include <atlconv.h>
#include <exception>
*/

#define _AFXDLL 
//#include <afxwin.h>
#include <afxdisp.h>//COleCurrency class
#include <vector>//vector
#include <comutil.h>//_bstr_t
#include <typeinfo>
#include <iostream>
#include <iomanip>//er jin zhi 
#include <comdef.h>
//注意本类只能被new创建，因为类内的析构函数中使用了delete，因此不用再外部delete

#import "C:\\Program Files (x86)\\Common Files\\system\\ado\\msado15.dll" no_namespace rename("EOF", "EndOfFile")  
#import "C:\\Program Files (x86)\\Common Files\\system\\ado\\msadox.dll" 

class MDBCombine
{
private:
	_bstr_t m_FilePathName;//文件名
	_ConnectionPtr m_pConnection;//连接指针
	_RecordsetPtr m_pFileSchema;//文件记录集
	_RecordsetPtr m_pTableSchema;//表格记录集
	_RecordsetPtr m_pTableConstraint;//获取表格的约束
	_bstr_t m_ConnectionString;//连接字符串
	static int m_ClassCount;
public:
	MDBCombine(_bstr_t FilePathName);
	MDBCombine();
	~MDBCombine();
	_bstr_t getFileName();//获取到文件名
	_ConnectionPtr get_ConnectPtr();//获取到文件数据库的连接
	_RecordsetPtr get_FileSchema();//获取文件结构|表名|
	_RecordsetPtr get_TableSchema();//获取表格结构
	_RecordsetPtr get_TableConstraint();//获取表格的约束
	_bstr_t get_ConnectionString();//
	void SetConnectInfo(_bstr_t FilePathName);//设置连接的文件名
	bool Connect();//连接到文件
	bool DisConnect();//断开文件连接
	bool GetTableSchema();//获取文件结构
	bool GetFileSchema();//获取表格结构
	bool GetTableConstraint();//获取表格约束
	_bstr_t ConvertVariantV2(_variant_t var);//variant转换函数
};

