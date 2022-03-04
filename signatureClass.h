#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <filesystem>

#include "safeQueue.h"
#include "sha.h"

class signatureClass
{
private:
	std::string inFile;
	std::string outFile;
	unsigned int blockSize = 1024;
	unsigned int lastBlockSize;

	std::ifstream ifstream;
	std::ofstream ofstream;

	safeQueue<std::shared_ptr<char>> readBuffer;
	safeQueue<std::shared_ptr<uint8_t>> resultHash;

	std::atomic<bool> endReadFlag;
	std::atomic<bool> dataFlag;

public:
	signatureClass();
	~signatureClass();

	bool getInputData();

	void readFile();
	void writeFile();
	void calcSign();
	void work();
};

