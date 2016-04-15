#pragma once
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <iostream>



class Serial {
  public:
    Serial();
    virtual ~Serial();
    void close();
    bool setup(std::string portName, int baudrate);
    int readBytes(unsigned char * buffer, int length);
    int writeBytes(char * buffer, int length);
    bool writeByte(unsigned char singleByte);
    int readByte();  // returns -1 on no read or error...
    void flush(bool flushIn = true, bool flushOut = true);
    int available();
	
  protected:
    bool isSerialInit;
    HANDLE hComm;		        // the handle to the serial port pc
    COMMTIMEOUTS oldTimeout;	// we alter this, so keep a record
};

