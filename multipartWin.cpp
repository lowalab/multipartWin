// multipartWin.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <map>
#include <direct.h>		//mkdir

class HeaderItem {
public:
	std::string name;
	std::string value;
	std::map<std::string, std::string> attributes;
	// 여러개의 조각으로 나눈다.
	std::vector<std::string> split(std::string& input, char delimiter) {
		std::vector<std::string> answer;
		std::stringstream ss(input);
		std::string temp;
		std::string t = " \t\n\r\f\v";
		while (getline(ss, temp, delimiter)) {
			temp.erase(0, temp.find_first_not_of(t));
			temp.erase(temp.find_last_not_of(t) + 1);
			answer.push_back(temp);
		}
		return answer;
	}
	// 2개로 나눈다.
	std::vector<std::string> divide(std::string& input, char delimiter) {
		std::vector<std::string> answer;
		size_t pos = input.find(delimiter);
		if (pos < 1 || pos >= input.size() - 1)
		{
			answer.push_back(input);
			return answer;
		}
		std::string temp;
		temp = input.substr(0, pos);
		std::string t = " \t\n\r\f\v";
		temp.erase(0, temp.find_first_not_of(t));
		temp.erase(temp.find_last_not_of(t) + 1);
		if (!temp.empty()) answer.push_back(temp);
		temp = input.substr(pos + 1, input.size() - pos - 1);
		temp.erase(0, temp.find_first_not_of(t));
		temp.erase(temp.find_last_not_of(t) + 1);
		if (!temp.empty()) answer.push_back(temp);
		return answer;
	}

	bool parse(std::string data)
	{
		std::vector<std::string> splitData = divide(data, ':');
		if (splitData.size() != 2) return false;
		name = splitData[0];
		// 앞 뒤 따옴표 제거
		name.erase(0, name.find_first_not_of('"'));
		name.erase(name.find_last_not_of('"') + 1);
		std::vector<std::string> attributeData = split(splitData[1], ';');
		if (attributeData.size() >= 1) value = attributeData[0];
		for (int i = 1; i < attributeData.size(); i++)
		{
			std::vector<std::string> attributeItem = divide(attributeData[i], '=');
			if (attributeItem.size() == 2)
			{
				// 앞 뒤 따옴표 제거
				attributeItem[1].erase(0, attributeItem[1].find_first_not_of('"'));
				attributeItem[1].erase(attributeItem[1].find_last_not_of('"') + 1);
				attributes.insert({ attributeItem[0], attributeItem[1] });
			}
		}
		return true;
	}
};

class MultiPartParser {
public:
	std::string header;
	std::string body;
	std::vector<HeaderItem> headerItems;
	std::vector<MultiPartParser> sub;

	void replaceAll(const std::string& pattern, const std::string& replace) {
		std::string::size_type pos = 0;
		while ((pos = header.find(pattern)) != std::string::npos)
		{
			header.replace(pos, pattern.size(), replace);
		}
	}

	std::vector<std::string> splitHeader(std::string& input, char delimiter) {
		std::vector<std::string> answer;
		std::stringstream ss(input);
		std::string temp;

		while (getline(ss, temp, delimiter)) {
			answer.push_back(temp);
		}

		return answer;
	}

	MultiPartParser(const std::string& data)
	{
		//std::cout << "MultiPartParser constructor\n";
		// 줄바꿈 문자를 찾는다.
		size_t pos_LF = data.find('\n');
		if (pos_LF == std::string::npos)
		{
			std::cout << "LF not find" << std::endl;
			return;
		}
		// 줄바꿈 문자의 종류 결정
		std::string strNewLine = "\n";
		if (data.substr(pos_LF - 1, 1) == "\r") strNewLine = "\r\n";
		//std::cout << "New Line String : " << ((strNewLine.size() == 1) ? "LF" : "CRLF") << std::endl;
		// 헤더정보 분리
		size_t pos_HeadEnd = data.find(strNewLine + strNewLine);
		if (pos_HeadEnd == std::string::npos)
		{
			std::cout << "Header not find" << std::endl;
			return;
		}
		header = data.substr(0, pos_HeadEnd);
		body = data.substr(pos_HeadEnd + strNewLine.size() * 2, data.length() - strNewLine.size() * 2 - pos_HeadEnd);

		// 헤더 줄 별로 trim 해서 재조립
		std::vector<std::string> lines = splitHeader(header, '\n');
		header = "";
		for (const auto& line : lines)
		{
			std::string t = " \t\n\r\f\v";
			std::string temp = line;
			temp.erase(0, line.find_first_not_of(t));
			temp.erase(temp.find_last_not_of(t) + 1);
			if (header.length() > 0) header += "\n";
			header += temp;
		}
		// 헤더에서 ";\n" 을 ";" 로 교체
		replaceAll(";\n", ";");
		// 줄단위로 자르기
		lines = splitHeader(header, '\n');
		for (const auto& line : lines)
		{
			HeaderItem item;
			if (item.parse(line) == true)
			{
				headerItems.push_back(item);
				//item.print();
			}
		}
		header = "";

		for (const auto& headerItem : headerItems)
		{
			if (headerItem.name != "Content-Type") continue;
			// 비교하기 위해 소문자료 변환 Multipart or multipart
			std::string strContentType = headerItem.value;
			std::transform(strContentType.begin(), strContentType.end(), strContentType.begin(), [](unsigned char c) { return std::tolower(c); });
			if (strContentType == "multipart/related")
			{
				auto boun = headerItem.attributes.find("boundary");
				if (boun == headerItem.attributes.end()) continue;
				std::string strBoundary = boun->second;
				processMultipartRelated("--" + strBoundary, strNewLine);
				break;
			}
		}
	}
	~MultiPartParser()
	{
		//std::cout << "MultiPartParser destructor\n";
	}

	void processMultipartRelated(std::string strBoundary, std::string strNewLine)
	{
		size_t start_pos = 0;
		size_t end_pos = 0;
		int nCount = 0;

		while (true)
		{
			end_pos = body.find(strBoundary, start_pos);
			if (end_pos == std::string::npos) break;
			nCount++;
			if (start_pos > 0)
			{
				MultiPartParser multi(body.substr(start_pos, end_pos - start_pos));
				sub.emplace_back(multi);
			}
			start_pos = end_pos + strBoundary.size() + strNewLine.size();
		}
		body = "";
	}
};


void createFolder(std::string &strFilename)
{
	size_t pos = 0;
	while (true)
	{
		pos = strFilename.find("/", pos + 1);
		if (pos == std::string::npos) break;
		//std::cout << strFilename.substr(0, pos) << std::endl;
		int ret = _mkdir(strFilename.substr(0, pos).c_str());

		//std::cout << strFilename.substr(0, pos) <<  ret << std::endl;
	}
}

int main()
{
	std::string strPackageName = "sls.pkg"; // run3tv_app.multipart   // App.pkg
	std::ifstream file(strPackageName, std::ios::binary);
	std::stringstream ss;
	ss << file.rdbuf();

	MultiPartParser parser(ss.str());

	std::cout << std::endl << "----------------result----------------" << std::endl << std::endl;


	for (const auto& item : parser.headerItems) {
		std::cout << item.name << ":" << item.value << std::endl;
		for (const auto& attribute : item.attributes) {
			std::cout << "\t" << attribute.first << "=" << attribute.second << std::endl;
		}
	}
	for (const auto& s : parser.sub) {
		for (const auto& item : s.headerItems) {
			std::cout << item.name << ":" << item.value << std::endl;
			for (const auto& attribute : item.attributes) {
				std::cout << "\t" << attribute.first << "=" << attribute.second << std::endl;
			}
			if (item.name == "Content-Location")
			{
				std::string strFilename = "cache/" + strPackageName + "/" + item.value;
				// 폴더 검사 및 생성
				createFolder(strFilename);
				std::ofstream file(strFilename, std::ios::binary);
				file.write(s.body.c_str(), s.body.size());
				file.close();
			}
		}
	}
}
