#include "pch.h"
#include "Command.h"
CCommand::CCommand():m_threadid(0){
	struct {
		int nCmd;
		FUNCCMD func;
	}data[]{
		{1,&CCommand::MakeDirverInfo},
		{2,&CCommand::MakeDirectoryInfo},
		{3,&CCommand::RunFile},
		{4,&CCommand::DownFile},
		{5,&CCommand::MoueEvent},
		{6,&CCommand::SendScreen},
		{7,&CCommand::LockMachine},
		{8,&CCommand::UnLockMachine},
		{9,&CCommand::DeleteLocalFile},
		{100,&CCommand::TestLink},
		{-1,NULL}
	};
	/*std::pair主要的作用是将两个数据组合成一个数据，两个数据可以是同一类型或者不同类型。
	例如std::pair<int,float> 或者 std：：pair<double,double>等。pair实质上是一个结构体，
	其主要的两个成员变量是first和second，这两个变量可以直接使用*/
	for (int i = 0; data[i].nCmd != -1; i++) {
		m_mapFunc.insert(std::pair<int, FUNCCMD>(data[i].nCmd, data[i].func));
	}
}
int CCommand::ExcuteCommand(int nCmd)
{
	std::map<int, FUNCCMD>::iterator it = m_mapFunc.find(nCmd);
	if (it == m_mapFunc.end()) {
		return -1;
	}
	return (this->*it->second)();
}
