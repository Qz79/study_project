#pragma once
class CCommand
{
public:
	CCommand();
	~CCommand(){}
	int ExcuteCommand(int nCmd);
private:
	int (CCommand::* FUNCCMD)();
};

