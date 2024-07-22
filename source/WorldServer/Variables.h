/*  
    EQ2Emulator:  Everquest II Server Emulator
    Copyright (C) 2007  EQ2EMulator Development Team (http://www.eq2emulator.net)

    This file is part of EQ2Emulator.

    EQ2Emulator is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    EQ2Emulator is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with EQ2Emulator.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef EQ2_VARIABLES_H
#define EQ2_VARIABLES_H
#include <vector>
#include <string>

class Variable{
public:
	Variable (const char* name, const char* value, const char* comment){
		variableName = string(name);
		variableValue = string(value);
		if(comment)
			variableComment = string(comment);
	}

	const char* GetName() { return variableName.c_str(); }
	const char* GetValue() { return variableValue.c_str(); }
	const char* GetComment() { return variableComment.c_str(); }
	string GetNameValuePair(){ return string(variableName).append(" ").append(variableValue); }
	void SetValue(const char* value){
		if(value)
			variableValue = string(value);
	}
private:
	string variableName;
	string variableValue;
	string variableComment;
};

class Variables
{
public:
	~Variables(){
		ClearVariables();
	}
	void AddVariable ( Variable* var )
	{
		variables[string(var->GetName())] = var;
	}

	void ClearVariables()
	{
		if(variables.size() == 0)
			return;

		map<string,Variable*>::iterator map_list;
		for( map_list = variables.begin(); map_list != variables.end(); map_list++ ) {
			safe_delete(map_list->second);
		}
		variables.clear();
	}

	Variable* FindVariable ( string name )
	{
		if(variables.count(name) > 0)
			return variables[name];
		return 0;
	}

	vector<Variable*>* GetVariables(string partial_name){
		vector<Variable*>* ret = new vector<Variable*>();
		map<string,Variable*>::iterator itr;
		for(itr = variables.begin(); itr != variables.end(); itr++){
			if(itr->first.find(partial_name) < 0xFFFFFFFF)
				ret->push_back(itr->second);
		}
		return ret;
	}

private:
	map<string,Variable*> variables;

};
#endif
