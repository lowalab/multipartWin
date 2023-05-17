#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <map>
#include <filesystem>  // C++17부터 표준 라이브러리에 포함됨

class HeaderItem {
public:
	std::string name;
	std::string value;
	std::map<std::string, std::string> attributes;

	bool parse(const std::string& data) {
		size_t colonPos = data.find(':');
		if (colonPos == std::string::npos)
			return false;

		name = trimString(data.substr(0, colonPos));
		value = trimString(data.substr(colonPos + 1));

		std::istringstream attributeStream(value);
		std::string attribute;
		while (std::getline(attributeStream, attribute, ';')) {
			size_t equalsPos = attribute.find('=');
			if (equalsPos != std::string::npos) {
				std::string attrName = trimString(attribute.substr(0, equalsPos));
				std::string attrValue = trimString(attribute.substr(equalsPos + 1));

				if (!attrName.empty() && !attrValue.empty()) {
					removeQuotes(attrValue);
					attributes.insert({ attrName, attrValue });
				}
			}
		}
		removeSemicolon(value);
		return true;
	}
private:
	std::string trimString(const std::string& str) {
		const std::string whitespace = " \t\n\r\f\v";
		size_t start = str.find_first_not_of(whitespace);
		size_t end = str.find_last_not_of(whitespace);
		return (start != std::string::npos && end != std::string::npos) ? str.substr(start, end - start + 1) : "";
	}

	void removeQuotes(std::string& str) {
		str.erase(std::remove(str.begin(), str.end(), '\"'), str.end());
	}

	void removeSemicolon(std::string& str) {
		size_t semicolonPos = str.find(";");
		if (semicolonPos != std::string::npos)
			str.erase(semicolonPos);
	}
};

class MultiPartParser {
public:
	std::string header;
	std::string body;
	std::vector<HeaderItem> headerItems;
	std::vector<MultiPartParser> subParts;

	MultiPartParser(const std::string& data) {
		size_t headerEndPos = data.find("\r\n\r\n");
		if (headerEndPos != std::string::npos)
		{
			header = data.substr(0, headerEndPos);
			body = data.substr(headerEndPos + 4);
		}
		else
		{
			headerEndPos = data.find("\n\n");
			if (headerEndPos != std::string::npos)
			{
				header = data.substr(0, headerEndPos);
				body = data.substr(headerEndPos + 2);
			}
			else
			{
				return;
			}
		}
		// 헤더 줄 별로 trim 해서 재조립
		std::vector<std::string> lines = splitStringToLines(header);
		header = "";
		for (const auto& line : lines)
		{
			std::string temp = trimString(line);
			if (temp.empty()) continue;
			header += temp;
			if (temp.back() != ';') header += "\n";
		}

		lines = splitStringToLines(header);
		for (const auto& line : lines) {
			HeaderItem item;
			if (item.parse(line))
				headerItems.push_back(item);
		}

		for (const auto& item : headerItems) {
			if (findStringIgnoreCase(item.name, "Content-Type") && findStringIgnoreCase(item.value, "multipart")) {
				auto boundaryAttr = item.attributes.find("boundary");
				if (boundaryAttr != item.attributes.end()) {
					std::string boundary = "--" + boundaryAttr->second;
					processMultipart(boundary);
				}
			}
		}
	}

	bool findStringIgnoreCase(const std::string& input, const std::string& searchString) {
		std::string inputCopy = input;
		std::transform(inputCopy.begin(), inputCopy.end(), inputCopy.begin(), ::tolower);
		std::string searchStringCopy = searchString;
		std::transform(searchStringCopy.begin(), searchStringCopy.end(), searchStringCopy.begin(), ::tolower);

		return inputCopy.find(searchStringCopy) != std::string::npos;
	}

	void processMultipart(const std::string& boundary) {
		size_t startPos = 0;
		size_t endPos = 0;
		while ((endPos = body.find(boundary, startPos)) != std::string::npos) {
			MultiPartParser subPart(body.substr(startPos, endPos - startPos));
			subParts.push_back(subPart);
			startPos = endPos + boundary.length();
		}
	}

	std::vector<std::string> splitStringToLines(const std::string& input) {
		std::vector<std::string> lines;
		std::istringstream iss(input);

		std::string line;
		while (std::getline(iss, line)) {
			lines.push_back(line);
		}

		return lines;
	}

	std::string trimString(const std::string& str) {
		std::string t = " \t\n\r\f\v";
		std::string temp = str;
		temp.erase(0, str.find_first_not_of(t));
		temp.erase(temp.find_last_not_of(t) + 1);
		return temp;
	}

};


void createFolder(const std::string& strFilename)
{
	size_t pos = strFilename.find_last_of("/");
	if (pos == std::string::npos) return;
	std::string strFolder = strFilename.substr(0, pos);
	std::filesystem::create_directories(strFilename.substr(0, pos));
}


int main() {

	std::string strPackageName = "App.pkg"; // run3tv_app.multipart    App.pkg    sls.pkg
	std::ifstream file(strPackageName, std::ios::binary); 
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string multipartData = buffer.str();

	MultiPartParser parser(multipartData);

	// Access parsed data
	for (const auto& item : parser.headerItems) {
		std::cout << "Name: " << item.name << std::endl;
		std::cout << "Value: " << item.value << std::endl;
		for (const auto& attr : item.attributes) {
			std::cout << "Attribute: " << attr.first << ", Value: " << attr.second << std::endl;
		}
		std::cout << std::endl;
	}

	for (const auto& s : parser.subParts) {
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

	return 0;
}