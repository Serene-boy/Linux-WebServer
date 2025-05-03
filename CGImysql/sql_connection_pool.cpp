#include <mysql/mysql.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <iostream>
#include "sql_connection_pool.h"

using namespace std;

connection_pool::connection_pool()
{
	m_CurConn = 0;
	m_FreeConn = 0;
}

connection_pool *connection_pool::GetInstance()//static函数，创建一个静态局部数据库连接池实例
{
	static connection_pool connPool;//静态局部变量
	return &connPool;
}

//构造初始化
void connection_pool::init(string url, string User, string PassWord, string DBName, int Port, int MaxConn, int close_log)
{
	m_url = url;
	m_Port = Port;
	m_User = User;
	m_PassWord = PassWord;
	m_DatabaseName = DBName;
	m_close_log = close_log;//这里暂时没有使用到close_log这个参数

	for (int i = 0; i < MaxConn; i++)
	{
		MYSQL *con = NULL;
		con = mysql_init(con);

		if (con == NULL)
		{
			LOG_ERROR("MySQL Error");
			exit(1);//exit退出整个进程，而不是单个线程
		}

		//c_str() 是 std::string类的一个成员函数，作用是：把 C++ 的 string 类型，转换成 C语言风格的 "char*" 字符串指针（以 \0 结尾）
		//Always assign the return value of mysql_real_connect() back to the MYSQL* pointer.
		con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);

		if (con == NULL)
		{
			LOG_ERROR("MySQL Error");
			exit(1);
		}
		connList.push_back(con);//list<MYSQL *> connList;
		++m_FreeConn;
	}

	reserve = sem(m_FreeConn);

	m_MaxConn = m_FreeConn;
}


//当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
MYSQL *connection_pool::GetConnection()
{
	MYSQL *con = NULL;

	if (0 == connList.size())
		return NULL;

	reserve.wait();//信号量wait操作一般在互斥锁lock之前，这样可以避免死锁
	
	lock.lock();

	con = connList.front();
	connList.pop_front();

	--m_FreeConn;//数据库连接池可用连接数量实际上就是信号量的值，对连接池访问的控制，并没有用到m_FreeConn和m_CurConn.
	++m_CurConn;

	lock.unlock();
	return con;
}

//释放当前使用的连接
bool connection_pool::ReleaseConnection(MYSQL *con)
{
	if (NULL == con)
		return false;

	lock.lock();

	connList.push_back(con);
	++m_FreeConn;
	--m_CurConn;

	lock.unlock();

	reserve.post();
	return true;
}

//销毁数据库连接池
void connection_pool::DestroyPool()
{

	lock.lock();
	if (connList.size() > 0)
	{
		list<MYSQL *>::iterator it;
		for (it = connList.begin(); it != connList.end(); ++it)
		{
			MYSQL *con = *it;
			mysql_close(con);//关闭数据库连接
		}
		m_CurConn = 0;
		m_FreeConn = 0;
		connList.clear();//把里面存的所有数据库连接指针都移除掉. 注意：clear() 本身不会释放元素指向的内存. 所以在clear之前需要手动管理数据库连接，释放内存.
	}

	lock.unlock();
}

//当前空闲的连接数
int connection_pool::GetFreeConn()
{
	return this->m_FreeConn;//这个地方，实际上加锁比较好，因为别的线程可能在操作连接池，导致m_FreeConn一直在变化，可能引发竞态条件
}

connection_pool::~connection_pool()
{
	DestroyPool();
}

connectionRAII::connectionRAII(MYSQL **SQL, connection_pool *connPool){
	*SQL = connPool->GetConnection();
	
	conRAII = *SQL;
	poolRAII = connPool;
}

connectionRAII::~connectionRAII(){
	poolRAII->ReleaseConnection(conRAII);
}