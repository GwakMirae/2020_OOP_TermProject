#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <string>

using namespace std;

// 수정

class FileMonitor final {
private:
	static FileMonitor *fileMonitor;
	vector<string> buffer;
	int curLine;
	int startLine;
	string consoleMsg;
	string file;
	FileMonitor() {
		startLine = 0;
		curLine = 0;
		consoleMsg = "";
		file = "";
	}
public:
	int getStartLine() {
		return startLine;
	}
	void setStartLine(int newLine) {
		this->startLine = newLine;
	}
	int getCurLine() {
		return curLine;
	}
	void setCurLine(int newLine) {
		this->curLine = newLine;
	}
	vector<string> getFileBuffer() {
		return this->buffer;
	}
	void setFileBuffer(vector<string> buffer) {
		this->buffer = buffer;
	}
	void setConsoleMsg(string newMsg) {
		this->consoleMsg = newMsg;
	}
	void setFile(string file) {
		this->file = file;
	}
	static FileMonitor *instance() {
		if (!fileMonitor) {
			fileMonitor = new FileMonitor;
		}
		return fileMonitor;
	}
	void fileSave() {
		ofstream outStream;
		outStream.open(file, ios::out);
		if (outStream.fail()) {
			cout << "output file opennig failed" << endl;
			exit(1);
		}

		int size=0;
		while (size < buffer.size()) {
			outStream << buffer.at(size);
			size++;
		}

		outStream.close();
	}
	vector<string> fileContentSave() {
		buffer = {};
		ifstream inStream;
		inStream.open(file);
		if (inStream.fail()) {
			cout << "input file opennig failed" << endl;
			exit(1);
		}
		
		string charbuffer = "";
		int charCount = 0;
		int wordStartIndex = 0;
		char achar;
		string word = "";
		while (!inStream.eof()) {
			charbuffer = "";
			charCount = 0;
			word = "";
			while (charCount <= 76) {
				inStream.get(achar);
				if (!word.empty() && inStream.eof()) { //파일 끝인데 저장안된 word남으면 저장후 종료
					charbuffer += word;
					word = "";
					break;
				}
				if (achar == ' ') {
					charbuffer += word;
					charbuffer += achar;
					word = "";
				}
				else {
					word += achar;
				}
				charCount++;
			}
			buffer.push_back(charbuffer);
			
			if (!word.empty()) {
				inStream.seekg(-((signed)word.size()), ios::cur);
			}
		}
		inStream.close();
		return buffer;
	}
	void resortText(vector<string> &buffer, int lineNum) {
		int i = lineNum;
		while (i<buffer.size()) {
			int charCount = 0;
			int wordStart = 0;
			string line = buffer.at(i);
			if (line.size() <= 75) {
				i++;
				continue;
			}
			while (charCount < 75) {
				if (line.at(charCount) == ' ') {
					wordStart= charCount+1;
				}
				charCount++;
			}
			if (i == (buffer.size()-1)) {
				string originLine = line.substr(0, wordStart);
				buffer.erase(buffer.begin() + i);
				buffer.insert(buffer.begin() + i, originLine);
				string rest = line.substr(wordStart);
				buffer.push_back(rest);
			}
			else {
				string originLine = line.substr(0, wordStart);
				buffer.erase(buffer.begin() + i);
				buffer.insert(buffer.begin() + i, originLine);
				string rest = line.substr(wordStart);
				string newLine = buffer.at(i + 1);
				newLine.insert(0, rest);
				buffer.erase(buffer.begin() + i + 1);
				buffer.insert(buffer.begin() + i + 1, newLine);
			}
			i++;
		}
		setFileBuffer(buffer);
	}
	string printText() {
		this->setStartLine(curLine);
		int line = 1;
		while (line <= 20) {
			cout << setw(2) << line++;
			if (curLine >= buffer.size()) {
				cout << " | " << endl;
			}
			else {
				cout << " | " << buffer.at(curLine)<< endl;
			}
			curLine++;
			
		}
		// 문구 출력
		string input;
		cout << "----------------------------------------------------------------------------------------------" << endl;
		cout << "n:다음페이지, p:이전페이지, i:삽입, d:삭제, c:변경, s:찾기, t:저장후종료" << endl;
		cout << "----------------------------------------------------------------------------------------------" << endl;
		cout << "(콘솔메시지) " << consoleMsg << endl;
		cout << "----------------------------------------------------------------------------------------------" << endl;
		cout << "입력:";
		getline(cin,input);
		cout << "----------------------------------------------------------------------------------------------" << endl;
		return input;
	}
};

FileMonitor *FileMonitor::fileMonitor = 0 ;

class Action {
private:
	vector<string> buffer;
public:
	Action(vector<string> buffer) {
		this->buffer = buffer;
	}
	vector<string> &getBuffer() {
		return this->buffer;
	}
	static Action* inputAction(string choice, string input, vector<string> buffer);
	virtual void run() = 0;
};

class InputN : public Action{
public:
	InputN(vector<string> buffer) : Action(buffer) {
		
	}
	~InputN() {
		delete this;
	}
	void run() {
		int curLine = FileMonitor::instance()->getCurLine();
		int strLine = FileMonitor::instance()->getStartLine();
		if (curLine >= getBuffer().size()) {
			FileMonitor::instance()->setConsoleMsg("This is last page!");
			FileMonitor::instance()->setCurLine(strLine);
		}
		else if (curLine + 20 > getBuffer().size()) {
			int gab = getBuffer().size()-20;
			FileMonitor::instance()->setCurLine(gab);
		}

	}
};
class InputP : public Action {
public:
	InputP(vector<string> buffer) : Action(buffer) {

	}
	~InputP() {
		delete this;
	}
	void run() {
		int curLine = FileMonitor::instance()->getCurLine();
		int strLine = FileMonitor::instance()->getStartLine();
		if (strLine == 0) {
			FileMonitor::instance()->setConsoleMsg("This is first page!");
			FileMonitor::instance()->setCurLine(0);
		}
		else if (strLine - 20 < 0) {
			FileMonitor::instance()->setCurLine(0);
		}
		else {
			FileMonitor::instance()->setCurLine(curLine - 40);
		}
	}
};
class InputT : public Action {
public:
	InputT(vector<string> buffer) : Action(buffer) {

	}
	void run() {
		FileMonitor::instance()->fileSave();
		exit(0);
	}
};
class InputI : public Action{
private:
	string input;
	int lineNum;
	int wordNum;
	string word;
public:
	InputI(string input, vector<string> buffer) : Action(buffer) {
		this->input = input;
		this->lineNum = 0;
		this->wordNum = 0;
		this->word = "";
	}
	~InputI() {
		delete this;
	}
	void run() {
		if (!subInput()) {
			return;
		}
		int strLine = FileMonitor::instance()->getStartLine();
		int index = strLine + lineNum -1;
		string line = getBuffer().at(index);//출력된 라인의 몇번째인지 계산후 해당 string반환
		int i = 0;
		int wordCount = 0;
		string newLine;
		while (i < line.size()) {
			if (wordCount == wordNum) {
				break;
			}
			if (i == line.size() - 1) {
				if ((wordCount == wordNum - 1) && (line.at(i)!=' ')) {
					newLine = line + " " + word;
					break;
				}
			}
			if (line.at(i) == ' ') {
				wordCount++;
			}
			i++;
		}
		if (wordCount == wordNum) {
			newLine = line.insert(i, word + " ");
		}
		if ((wordCount <= wordNum-1)&&(i==line.size())) {
			FileMonitor::instance()->setConsoleMsg("please enter smaller word number");
			FileMonitor::instance()->setCurLine(strLine);
			return;
		}
		getBuffer().insert(getBuffer().begin()+index, newLine);
		getBuffer().erase(getBuffer().begin()+index+1);
		FileMonitor::instance()->resortText(getBuffer(), index);
		FileMonitor::instance()->setCurLine(strLine);
	}
	vector<string> separateInput() {
		string next = input;
		int current = next.find(",");
		vector<string> data;
		// 콤마 단위로 분리
		while (current != string::npos) {
			string substring = next.substr(0, current);
			data.push_back(substring);
			next = next.erase(0, current + 1);
			current = next.find(",");
		}
		data.push_back(next);
		return data;
	}
	bool subInput() {
		vector<string> data = separateInput();

		// 예외 처리
		if (data.size() != 3) {
			int strLine = FileMonitor::instance()->getStartLine();
			FileMonitor::instance()->setConsoleMsg("i have three pharamiter!");
			FileMonitor::instance()->setCurLine(strLine);
			return false;
		}
		try
		{
			lineNum = stoi(data.at(0));
			wordNum = stoi(data.at(1));
		}
		catch (const std::invalid_argument&)
		{
			int strLine = FileMonitor::instance()->getStartLine();
			FileMonitor::instance()->setConsoleMsg("please enter number");
			FileMonitor::instance()->setCurLine(strLine);
			return false;
		}
		if ((lineNum < 1) && (lineNum > 20)) {
			int strLine = FileMonitor::instance()->getStartLine();
			FileMonitor::instance()->setConsoleMsg("please enter (1 ~ 20) line number");
			FileMonitor::instance()->setCurLine(strLine);
		}

		word = data.at(2);
		if (word.size() >= 76) {
			int strLine = FileMonitor::instance()->getStartLine();
			FileMonitor::instance()->setConsoleMsg("Please enter no more than 75 characters");
			FileMonitor::instance()->setCurLine(strLine);
			return false;
		}
		
		return true;
	}
};
class InputD : public Action {
private:
	string input;
	int lineNum;
	int wordNum;
public:
	InputD(string input, vector<string> buffer) : Action(buffer) {
		this->input = input;
		this->lineNum = 0;
		this->wordNum = 0;
	}
	~InputD() {
		delete this;
	}
	void run() {
		if (!subInput()) {
			return;
		}
		int strLine = FileMonitor::instance()->getStartLine();
		int index = strLine + lineNum-1;
		string line = getBuffer().at(index);//출력된 라인의 몇번째인지 계산후 해당 string반환
		int i = 0;
		int wordCount = 0;
		int wordStart = 0;
		string newLine;
		while (i < line.size()) {
			if (wordCount == wordNum-1) {
				while ((i < line.size()) ) {
					if ((line.at(i) == ' ')) {
						line.erase(i, 1);
						break;
					}
					line.erase(i, 1);
				}
				break;
			}
			if (line.at(i) == ' ') {
				wordCount++;
			}
			i++;
		}
		if (wordCount < wordNum-1) {
			FileMonitor::instance()->setConsoleMsg("please enter smaller word number");
			FileMonitor::instance()->setCurLine(strLine);
			return;
		}
		if (index == getBuffer().size()-1) { //마지막 줄 인경우
			getBuffer().pop_back(); // 마지막 줄 삭제
			getBuffer().push_back(line);
			FileMonitor::instance()->setFileBuffer(getBuffer());
		}
		else if (index+1 == getBuffer().size()-1) {
			newLine = line + " " + getBuffer().at(index + 1);
			getBuffer().pop_back(); // index 줄 삭제
			getBuffer().pop_back(); // 마지막 줄 삭제
			getBuffer().push_back(newLine);
			FileMonitor::instance()->resortText(getBuffer(), index);
		}
		else {
			newLine = line + " " + getBuffer().at(index + 1);
			getBuffer().erase(getBuffer().begin() + index); // 해당 줄 삭제
			getBuffer().erase(getBuffer().begin() + index); // 해당 다음 줄 삭제
			getBuffer().insert(getBuffer().begin() + index, newLine); // 두 줄을 합쳐서 삽입
			FileMonitor::instance()->resortText(getBuffer(), index);
		}
		FileMonitor::instance()->setCurLine(strLine);
	}

	vector<string> separateInput() {
		string next = input;
		int current = next.find(",");
		vector<string> data;
		// 콤마 단위로 분리
		while (current != string::npos) {
			string substring = next.substr(0, current);
			data.push_back(substring);
			next = next.erase(0, current + 1);
			current = next.find(",");
		}
		data.push_back(next);
		return data;
	}

	bool subInput() {
		vector<string> data = separateInput();

		// 예외 처리
		if (data.size() != 2) {
			int strLine = FileMonitor::instance()->getStartLine();
			FileMonitor::instance()->setConsoleMsg("d have two pharamiter!");
			FileMonitor::instance()->setCurLine(strLine);
			return false;
		}
		try
		{
			lineNum = stoi(data.at(0));
			wordNum = stoi(data.at(1));
		}
		catch (const std::invalid_argument&)
		{
			int strLine = FileMonitor::instance()->getStartLine();
			FileMonitor::instance()->setConsoleMsg("please enter number");
			FileMonitor::instance()->setCurLine(strLine);
			return false;
		}
		if ((lineNum < 1) | (lineNum > 20)) {
			int strLine = FileMonitor::instance()->getStartLine();
			FileMonitor::instance()->setConsoleMsg("please enter (1 ~ 20) line number");
			FileMonitor::instance()->setCurLine(strLine);
		}
		return true;
	}
};

class InputC : public Action {
private:
	string input;
	string oldword;
	string newword;
public:
	InputC(string input, vector<string> buffer) : Action(buffer) {
		this->input = input;
		this->oldword = "";
		this->newword = "";
	}
	~InputC() {
		delete this;
	}
	void run() {
		if (!subInput()) {
			return;
		}
		bool found = false;
		int lineCount = 0;
		while (lineCount < getBuffer().size()) {
			string line = getBuffer().at(lineCount);
			int index =0;
			while (index < line.size()) {
				if ((index = line.find(oldword,index)) != string::npos) {
					if ((index != 0)&&(line.at(index - 1) != ' ')) {
						index++;
						continue;
					}
					int size = 0;
					int start = index;
					while (size < oldword.size()) {
						index++;
						size++;
					}
					if ((index == line.size()) || (line.at(index) == ' ')) {
						line.replace(start, oldword.size(), newword);
						found = true;
					}
				}
			}
			getBuffer().insert(getBuffer().begin() + lineCount, line);
			getBuffer().erase(getBuffer().begin() + lineCount + 1);
			lineCount++;
		}
		if (!found) {
			FileMonitor::instance()->setConsoleMsg("not found [" + oldword + "]");
		}
		FileMonitor::instance()->setFileBuffer(getBuffer());
		int strLine = FileMonitor::instance()->getStartLine();
		FileMonitor::instance()->setCurLine(strLine);
	}
	vector<string> separateInput() {
		string next = input;
		int current = next.find(",");
		vector<string> data;
		// 콤마 단위로 분리
		while (current != string::npos) {
			string substring = next.substr(0, current);
			data.push_back(substring);
			next = next.erase(0, current + 1);
			current = next.find(",");
		}
		data.push_back(next);
		return data;
	}
	bool subInput() {
		vector<string> data = separateInput();

		// 예외 처리
		if (data.size() != 2) {
			int strLine = FileMonitor::instance()->getStartLine();
			FileMonitor::instance()->setConsoleMsg("c have two pharamiter!");
			FileMonitor::instance()->setCurLine(strLine);
			return false;
		}
		oldword = data.at(0);
		newword = data.at(1);
		if (oldword.size() >= 76 || newword.size()>=76) {
			int strLine = FileMonitor::instance()->getStartLine();
			FileMonitor::instance()->setConsoleMsg("Please enter no more than 75 characters");
			FileMonitor::instance()->setCurLine(strLine);
			return false;
		}
		return true;
	}
};

class InputS : public Action {
private:
	string input;
public:
	InputS(string input, vector<string> buffer) : Action(buffer) {
		this->input = input;
	}
	~InputS() {
		delete this;
	}
	void run() {
		int lineCount = 0;
		while (lineCount < getBuffer().size()) {
			string line = getBuffer().at(lineCount);
			int index = 0;
			while (index < line.size()) {
				if ((index = line.find(input,index)) != string::npos) {
					if ((index != 0) && (line.at(index - 1) != ' ')) {
						index++;
						continue;
					}
					int size = 0;
					while (size < input.size()) {
						index++;
						size++;
					}
					if ((index == line.size()) || (line.at(index) == ' ')) {						
						FileMonitor::instance()->setCurLine(lineCount);
						return;
					}
				}
			}
			lineCount++;
		}
		int strLine = FileMonitor::instance()->getStartLine();
		FileMonitor::instance()->setCurLine(strLine);
		FileMonitor::instance()->setConsoleMsg("not found [" + input + "]");
	}
};

Action* Action::inputAction(string choice, string input, vector<string> buffer) {

	if (choice == "n") {
		return new InputN(buffer);
	}
	else if (choice == "p") {
		return new InputP(buffer);
	}
	else if (choice == "i") {
		return new InputI(input, buffer);
	}
	else if (choice == "d") {
		return new InputD(input, buffer);
	}
	else if (choice == "s") {
		return new InputS(input, buffer);
	}
	else if (choice == "c") {
		return new InputC(input, buffer);
	}
	else {
		return new InputT(buffer);
	}
}



class Controller {
private:
	string file;
	FileMonitor* monitor;
	vector<string> buffer;
public:
	Controller(string file) {
		this->file = file;
		monitor = FileMonitor::instance();
		monitor->setFile(file);
		buffer = monitor->fileContentSave();
	}

	void controller() {
		while (true) {
			string input = repeat();
			string actionlist = "n/p/i/d/c/s/t";
			string action = input.substr(0, 1);
			char delimiter1 = '(';
			char delimiter2 = ')';
			string consoleMsg = "";
			
			if (input.empty()) {
				consoleMsg = "작업을 입력하세요";
			}
			else if ((input.size() > 1) && (input.at(1) != '(')) {
				consoleMsg = "해당 기능은 존재하지 않습니다";
			}
			else if (input.size() == 1 ) {
				if (actionlist.find(action) == string::npos) {
					consoleMsg = "해당 기능은 존재하지 않습니다";
				}
				else {
					Action::inputAction(action, "", buffer)->run();
					continue;
				}
			}
			else if( input.find(delimiter2) == string::npos) {
				consoleMsg ="괄호가 필요합니다";
			}
			else {
				string line = input.substr(input.find(delimiter1) + 1);
				line = line.substr(0, line.find(delimiter2));
				if (line.find(' ') != string::npos) { // 보완 필요함
					consoleMsg = "괄호 안에 공백 입력은 불가능합니다";
				}
				else {
					Action::inputAction(action, line, buffer)->run();
					continue;
				}
			}
			setFileMonitor(consoleMsg);
		}
	}
	string repeat() {
		buffer = monitor->getFileBuffer();
		string input = monitor->printText();
		monitor->setConsoleMsg("");
		return input;
	}
	
	void setFileMonitor(string msg) {
		int strLine = monitor->getStartLine();
		monitor->setConsoleMsg(msg);
		monitor->setCurLine(strLine);
	}
};

int main() {
	string file = "test.txt";
	Controller control(file);
	control.controller();
}

