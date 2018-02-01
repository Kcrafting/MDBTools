#include "MDBCombine.h"




MDBCombine::MDBCombine()
	:m_pConnection(nullptr), m_pTableSchema(nullptr), m_pFileSchema(nullptr), m_FilePathName(""),
	m_ConnectionString("")
{
	m_ClassCount++;
	OutputDebugString(L"\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>类正在被无参数构造！  " + _bstr_t(m_ClassCount) + _bstr_t(L"次  \n"));
}


MDBCombine::MDBCombine(_bstr_t FileName)
	:m_FilePathName(FileName), m_pConnection(nullptr), m_pTableSchema(nullptr), m_pFileSchema(nullptr),
	m_ConnectionString("")
{
	m_ClassCount++;
	OutputDebugString(L"\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>类正在含有文件名的构造！  " + _bstr_t(m_ClassCount) + _bstr_t(L"次  \n"));
}

MDBCombine::~MDBCombine()
{
	try {
		m_ClassCount--;
		OutputDebugString(L"\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>类正在析构！  " + _bstr_t(m_ClassCount) + _bstr_t(L"次  \n"));
		if (m_pConnection)
			if (m_pConnection->State == adStateOpen)
				m_pConnection->Close();
		if (m_pTableSchema)
			if (m_pTableSchema->State == adStateOpen)
				m_pTableSchema->Close();
		if (m_pFileSchema)
			if (m_pFileSchema->State == adStateOpen)
				m_pFileSchema->Close();
		if (m_pTableConstraint)
			if (m_pTableConstraint->State == adStateOpen)
				m_pTableConstraint->Close();
		//delete this;
	}
	catch (std::exception &e)
	{
		const char * aaa = e.what();
	}
}

_bstr_t MDBCombine::getFileName()
{
	return m_FilePathName;
}

_ConnectionPtr MDBCombine::get_ConnectPtr()
{
	if (m_pConnection) 
		return m_pConnection;
	else
	{
		if (Connect())
		{
				return m_pConnection;
		}
	}
	return nullptr;
}

_RecordsetPtr MDBCombine::get_FileSchema()
{
	if (m_pFileSchema) return m_pFileSchema;
	if (!m_pConnection)
	{
		if (Connect())
		{
			if (GetFileSchema())
			{
				return m_pFileSchema;
			}
		}

	}
	else
	{
		if (GetFileSchema())
		{
			return m_pFileSchema;
		}
	}
	return nullptr;
}

_RecordsetPtr MDBCombine::get_TableSchema()
{
	if (m_pTableSchema) return m_pTableSchema;
	if (!m_pConnection) 
	{
		if (Connect())
		{
			if (GetTableSchema())
			{
				return m_pTableSchema;
			}
		}
		
	}
	else {
		if (GetTableSchema())
		{
			return m_pTableSchema;
		}
	}
	return nullptr;
}

_RecordsetPtr MDBCombine::get_TableConstraint()
{
	return m_pTableConstraint;
}

_bstr_t MDBCombine::get_ConnectionString()
{
	if (m_ConnectionString == _bstr_t(L""))
		Connect();
	return m_ConnectionString;
}

void MDBCombine::SetConnectInfo(_bstr_t FilePathName)
{
	m_FilePathName = FilePathName;
}

bool MDBCombine::Connect()
{
	//if (nullptr != m_pConnection) return;//若不注释则连接后必须手动释放，否则每次执行都将刷新连接
	if (m_FilePathName == (_bstr_t)T2W(L""))
	{
		MessageBox(NULL, L"连接还未初始化！", L"提示", NULL);
		return false;
	}
		
	if (FAILED(::CoInitialize(NULL)))
	{
		MessageBox(NULL, L"初始化COM环境失败！", L"提示", NULL);
		return false;
	}
	_bstr_t Provider("Provider = Microsoft.ACE.OLEDB.12.0; Data Source = ");
	_bstr_t Semicolon(";");
	m_ConnectionString = Provider + m_FilePathName + Semicolon;
	HRESULT hr = S_OK;
	try
	{
		hr = m_pConnection.CreateInstance(__uuidof(Connection));
	}
	catch (_com_error &err)
	{
		MessageBox(NULL,err.Description(), L"\n发生错误！", MB_OK);
		return false;
	}
	if (hr != S_OK)
	{
		MessageBox(NULL, L"\n创建连接实例时发生错误！\n (1) 系统上安装着不同的ADO版本！\n(2) 您安装的不是64位的office!\n(3) 您的计算机上没有安装64位的MDB引擎!\n(4) 您可以在以下地址下载\nhttps://www.microsoft.com/zh-cn/download/details.aspx?id=13255", L"发生错误！", MB_OK);
	}
	try {
		m_pConnection->Open(m_ConnectionString, "admin", "", adConnectUnspecified);
	}
	catch (_com_error &e) {
		MessageBox(NULL, (_bstr_t)T2W(L"打开文件的过程中发生错误！\n")+e.Description(),L"\n连接出错 ！ 原因有可能: <1> 系统上安装着不同的ADO版本！\n<2> 您安装的不是64位的office!\n<3> 您的计算机上没有安装64位的MDB引擎！\n<4> 您可以在以下地址下载\nhttps://www.microsoft.com/zh-cn/download/details.aspx?id=13255",MB_OK);
		return false;
	}
	return true;
}

bool MDBCombine::DisConnect()
{
	try {
		if (m_pConnection)
		{
			if (m_pConnection->State == adStateOpen)
				m_pConnection->Close();
		}
		m_pConnection = nullptr;
		if (m_pTableSchema)
		{
			if (m_pTableSchema->State == adStateOpen)
				m_pTableSchema->Close();
		}
		m_pTableSchema = nullptr;
		if (m_pFileSchema)
		{
			if (m_pFileSchema->State == adStateOpen)
				m_pFileSchema->Close();
		}
		m_pFileSchema = nullptr;
		if (m_pTableConstraint)
		{
			if (m_pTableConstraint->State == adStateOpen)
				m_pTableConstraint->Close();
		}
		m_pTableConstraint = nullptr;
	}
	catch (std::exception &err)
	{
		MessageBox(NULL,T2W(LPTSTR(err.what())), L"发生错误！", MB_OK);
		return false;
	}
	return true;
}

bool MDBCombine::GetTableSchema()
{
	if (!m_pConnection)
	{
		if (!Connect())
		{
			return false;
		}
	}
	try {
		m_pTableSchema = m_pConnection->OpenSchema(adSchemaColumns);//返回列信息
		//m_pTableSchema = m_pConnection->OpenSchema(adSchemaTableConstraints);
	}
	catch (_com_error &err)
	{
		MessageBox(NULL, (_bstr_t)T2W(L"获取表格结构时发生错误！\n") + err.Description(), L"发生错误！", MB_OK);
		return false;
	}
	return true;
}

bool MDBCombine::GetFileSchema()
{
	if (!m_pConnection)
	{
		if (!Connect())
		{
			return false;
		}
	}
	try {
		m_pFileSchema = m_pConnection->OpenSchema(adSchemaTables);
	}
	catch (_com_error &err)
	{
		MessageBox(NULL, (_bstr_t)T2W(L"获取文件结构时发生错误！\n") + err.Description(), L"发生错误！", MB_OK);
		return false;
	}
	return true;
}

bool MDBCombine::GetTableConstraint()
{
	if (!m_pConnection)
	{
		if (!Connect())
		{
			return false;
		}
	}
	try {
		m_pTableConstraint=m_pConnection->OpenSchema(adSchemaConstraintColumnUsage);
	}
	catch (_com_error &err)
	{
		MessageBox(NULL, (_bstr_t)T2W(L"获取文件约束时发生错误！\n") + err.Description(), L"发生错误！", MB_OK);
		return false;
	}
	return true;
}

//转换Variant第二版
_bstr_t MDBCombine::ConvertVariantV2(_variant_t var)
{
	CString str;
	switch (var.vt)
	{
	case	VT_EMPTY:
	{
		break;
	}
	case	VT_NULL:
	{
		str = "NULL";
		//str = W2T(_bstr_t(var));
		break;
	}
	case	VT_I2:
	{
		str.Format(_T("%d"), (int)var.iVal);
		str.Format(_T("%d"), (int)var.iVal);
		break;
	}
	case 	VT_I4:
	{
		str.Format(_T("%d"), var.lVal);
		break;
	}
	case 	VT_R4:
	{
		str.Format(_T("%10.6f"), (double)var.fltVal);
		break;
	}
	case 	VT_R8:
	{
		str.Format(_T("%10.6f"), var.dblVal);
		break;
	}
	case 	VT_CY:
	{
		str = COleCurrency(var).Format();
		break;
	}
	case 	VT_DATE:
	{
		str = COleDateTime(var).Format();
		str = "'" + str + "'";
		//str = W2T(_bstr_t(var));
		break;
	}
	case 	VT_BSTR:
	{
		str = var.bstrVal;
		str = "'" + str + "'";
		break;
	}
	case 	VT_DISPATCH:
	{
		break;
	}
	case 	VT_ERROR:
	{
		break;
	}
	case 	VT_BOOL:
	{
		str = (var.boolVal == 0) ? "FALSE" : "TRUE";
		break;
	}
	case 	VT_VARIANT:
	{
		break;
	}
	case 	VT_UNKNOWN:
	{
		break;
	}
	case 	VT_DECIMAL:
	{
		//str.Format(_T("%15.6d"),var.dblVal);
		str = W2T(_bstr_t(var));
		break;
	}
	case 	VT_I1:
	{
		break;
	}
	case 	VT_UI1:
	{
		break;
	}
	case 	VT_UI2:
	{
		break;
	}
	case 	VT_UI4:
	{
		break;
	}
	case 	VT_I8:
	{
		break;
	}
	case 	VT_UI8:
	{
		break;
	}
	case 	VT_INT:
	{
		break;
	}
	case 	VT_UINT:
	{
		break;
	}
	case 	VT_VOID:
	{
		break;
	}
	case 	VT_HRESULT:
	{
		break;
	}
	case 	VT_PTR:
	{
		break;
	}
	case 	VT_SAFEARRAY:
	{
		break;
	}
	case 	VT_CARRAY:
	{
		break;
	}
	case 	VT_USERDEFINED:
	{
		break;
	}
	case 	VT_LPSTR:
	{
		break;
	}
	case 	VT_LPWSTR:
	{
		break;
	}
	case 	VT_RECORD:
	{
		break;
	}
	case 	VT_INT_PTR:
	{
		break;
	}
	case 	VT_UINT_PTR:
	{
		break;
	}
	case 	VT_FILETIME:
	{
		break;
	}
	case 	VT_BLOB:
	{
		break;
	}
	case 	VT_STREAM:
	{
		break;
	}
	case 	VT_STORAGE:
	{
		break;
	}
	case 	VT_STREAMED_OBJECT:
	{
		break;
	}
	case 	VT_STORED_OBJECT:
	{
		break;
	}
	case 	VT_BLOB_OBJECT:
	{
		break;
	}
	case 	VT_CF:
	{
		break;
	}
	case 	VT_CLSID:
	{
		break;
	}
	case 	VT_VERSIONED_STREAM:
	{
		break;
	}
	case 	VT_BSTR_BLOB:
	{
		break;
	}
	case 	VT_VECTOR:
	{
		break;
	}
	case 	VT_ARRAY:
	{
		break;
	}
	case 	VT_BYREF:
	{
		break;
	}
	case 	VT_RESERVED:
	{
		break;
	}
	case 	VT_ILLEGAL:
	{
		break;
	}
	//case 	VT_ILLEGALMASKED = 0xfff,
	//case 	VT_TYPEMASK = 0xfff
	default:
	{

	}
	}
	return str.AllocSysString();
}
