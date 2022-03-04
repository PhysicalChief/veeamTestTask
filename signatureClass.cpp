#include "signatureClass.h"

signatureClass::signatureClass() {
	endReadFlag.store(false);
	dataFlag.store(false);
	lastBlockSize = 0;
}

signatureClass::~signatureClass() {
	if (ifstream.is_open())
		ifstream.close();
	if (ofstream.is_open())
		ofstream.close();
}

bool signatureClass::getInputData() {
	try {
		std::cout << "Enter inner file: " << std::endl;
		std::cin >> inFile;
		if (!std::filesystem::exists(inFile)) {
			throw std::exception("Inner file doesn't exists");
		}
		ifstream.open(inFile.c_str(), std::ifstream::in | std::ifstream::binary);

		std::cout << "Enter output file: " << std::endl;
		std::cin >> outFile;
		ofstream.open(outFile, std::ifstream::out | std::ifstream::binary | std::ifstream::app);

		std::cout << "Enter size of block(Kb): " << std::endl;
		std::cin >> blockSize;

		if (blockSize == 0) {
			blockSize = 1024;
			std::cout << "Will be use block of default size." << std::endl;
		}
		blockSize *= 1024;
		return true;
	}
	catch (std::exception& error) {
		std::cout << error.what() << std::endl;
		return false;
	}
}



void signatureClass::readFile()
{
	uint64_t fileSize = 0;
	unsigned int block = blockSize;
	if (ifstream.is_open()) {
		try {
			ifstream.seekg(0, std::ifstream::end);
			fileSize = ifstream.tellg();
			ifstream.seekg(0, std::ifstream::beg);

			while (fileSize > 0) {
				if (fileSize < block) {
					lastBlockSize = fileSize;
					block = fileSize;
				}
				std::shared_ptr<char> tmpBlock(new char[block], std::default_delete<char[]>());
				ifstream.read(tmpBlock.get(), block);
				readBuffer.push(tmpBlock);
				fileSize -= block;
				if (!dataFlag.load())
					dataFlag.store(true);
			}
			endReadFlag.store(true);
			ifstream.close();
		}
		catch (std::exception& error) {
			std::cout << error.what() << std::endl;
		}
		return;
	}
	else {
		throw (std::exception("Input filedoesn't open"));
		return;
	}
}

void signatureClass::writeFile()
{
	if (ofstream.is_open()) {
		try {
			ofstream.clear();
			while (1)
			{
				if (endReadFlag.load() && readBuffer.empty() && resultHash.empty()) {
					ofstream.close();
					break;
				}
				while (resultHash.empty())
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
				}
				std::shared_ptr<uint8_t> tmpBlock (new uint8_t[32], std::default_delete<uint8_t[]>());
				resultHash.pop(tmpBlock);
				ofstream.write(reinterpret_cast<char*>(tmpBlock.get()), 32);
			}
		}
		catch (std::exception& error) {
			std::cout << error.what() << std::endl;
			ofstream.close();
			return;
		}
	}
	else {
		throw std::exception("Output file doesn't open");
		return;
	}
	return;
}

void signatureClass::calcSign()
{
	;

	try {
		while (!dataFlag.load()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		while (true) {
			if (readBuffer.empty()) {
				if (endReadFlag.load()) {
					break;
				}
				else {
					std::this_thread::sleep_for(std::chrono::milliseconds(20));
				}
			}
			unsigned int blockLen;
			if (readBuffer.size() == 1 && endReadFlag.load()) {
				blockLen = lastBlockSize;
			}
			else {
				blockLen = blockSize;
			}
			SHA256 sha256;
			std::shared_ptr<char> tmpReadBlock(new char[blockLen], std::default_delete<char[]>());
			readBuffer.pop(tmpReadBlock);
			std::shared_ptr<uint8_t> tmpHash(new uint8_t[32], std::default_delete<uint8_t[]>());
			sha256.update(tmpReadBlock.get());
			sha256.digest(tmpHash.get());
			resultHash.push(tmpHash);
		}
		return;
	}
	catch (std::exception& error) {
		std::cout << error.what() << std::endl;
		return;
	}
  }

void signatureClass::work()
{
	if (getInputData()) {
		try {
			std::thread readingThread(&signatureClass::readFile, this);
			if (readingThread.joinable())
				readingThread.detach();

			std::thread calculateHash(&signatureClass::calcSign, this);
			if (calculateHash.joinable())
				calculateHash.detach();

			std::thread writeThread(&signatureClass::writeFile, this);
			if (writeThread.joinable())
				writeThread.join();

		}
		catch (std::exception& error) {
			std::cout << error.what() << std::endl;
			return;
		}
	}
}