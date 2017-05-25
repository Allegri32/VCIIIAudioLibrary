// Allegri Log System v.0.1
// Licensed under MIT.
#include <iostream>
#include <fstream>

#pragma once
#ifndef ALLOG_H
#define ALLOG_H

using namespace std;

class allog{
private:
	ofstream logger;

public:
	void start()
	{
		logger.open ("AudioLibrary.log");
		logger << "AudioLibrary 2.0 by Allegri32 started.\n";
		logger.close();
	}

	void write(const char* write)
	{
		logger.open("AudioLibrary.log", ios_base::app);
		logger << write << endl;
		logger.close();
	}

	void writeint(int inter)
	{
		logger.open("AudioLibrary.log", ios_base::app);
		logger << inter << endl;
		logger.close();
	}
};

#endif