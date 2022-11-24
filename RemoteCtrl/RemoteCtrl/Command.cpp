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
	/*std::pair��Ҫ�������ǽ�����������ϳ�һ�����ݣ��������ݿ�����ͬһ���ͻ��߲�ͬ���͡�
	����std::pair<int,float> ���� std����pair<double,double>�ȡ�pairʵ������һ���ṹ�壬
	����Ҫ��������Ա������first��second����������������ֱ��ʹ��*/
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
