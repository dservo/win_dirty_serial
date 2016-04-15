#include "Serial.h"

///////////////////////////////////////////////////////////
// serial  setup and close 
Serial::Serial(){
  isSerialInit = false;
}

bool Serial::setup(std::string portName, int baud){

  isSerialInit = false;
  char sizePortName[sizeof(portName)];
  int num;
  if (sscanf(portName.c_str(), "COM%d", &num) == 1) {
    // Microsoft KB115831 a.k.a if COM > COM9 you have to use a different syntax
    sprintf(sizePortName, "\\\\.\\COM%d", num);
  } else {
    strncpy(sizePortName, (const char *)portName.c_str(), sizeof(portName)-1);
  }
  
  // open the serial port:
  hComm=CreateFileA(sizePortName,GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,0,0);
  if(hComm==INVALID_HANDLE_VALUE){
    printf("setup(): unable to open %s\n", portName.c_str());
    return false;
  }

  // now try the settings:
  COMMCONFIG cfg;
  DWORD cfgSize;
  char  buf[80];
  cfgSize=sizeof(cfg);
  GetCommConfig(hComm,&cfg,&cfgSize);
  int bps = baud;
  sprintf(buf,"baud=%d parity=N data=8 stop=1",bps);
  // msvc doesn't like BuildCommDCB,need to use BuildCommDCBA
  if(!BuildCommDCBA(buf,&cfg.dcb)){
    printf("setup(): unable to build comm dcb, ( %s)", buf );
  }

  // Set baudrate and bits etc.
  // Note that BuildCommDCB() clears XON/XOFF and hardware control by default

  if(!SetCommState(hComm,&cfg.dcb)){
    printf( "setup(): couldn't set comm state: " );
  }
  // Set communication timeouts (NT)
  COMMTIMEOUTS tOut;
  GetCommTimeouts(hComm,&oldTimeout);
  tOut = oldTimeout;
  // Make timeout so that - return immediately with buffered characters
  tOut.ReadIntervalTimeout=MAXDWORD;
  tOut.ReadTotalTimeoutMultiplier=100;
  tOut.ReadTotalTimeoutConstant=100;
  SetCommTimeouts(hComm,&tOut);

  isSerialInit = true;
  return true;
}

Serial::~Serial(){
  close();
  isSerialInit = false;
}

void Serial::close(){
  if (isSerialInit){
    SetCommTimeouts(hComm,&oldTimeout);
    CloseHandle(hComm);
    hComm = INVALID_HANDLE_VALUE;
    isSerialInit = false;
  }
}
// end serial  setup and close 
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// serial  data send/recive
int Serial::readBytes(unsigned char * buffer, int length){
  if (isSerialInit == false){
    printf("readBytes(): serial not inited");
    return 0;
  }
  DWORD nRead = 0;
    if (!ReadFile(hComm,buffer,length,&nRead,0)){
    printf("readBytes(): couldn't read from port");
    return 0;
  }
  return (int)nRead;
}

int Serial::readByte(){
  if (isSerialInit == false){
    printf("readByte(): serial not inited");
    return 0;
  }
  unsigned char tmpByte[1];
  memset(tmpByte, 0, 1);
  DWORD nRead;
  if (!ReadFile(hComm, tmpByte, 1, &nRead, 0)){
    printf("readByte(): couldn't read from port");
  return 0;
  }
  return (int)(tmpByte[0]);
}

int Serial::writeBytes(char * buffer, int length){
  if (isSerialInit == false){
    printf("writeBytes(): serial not inited");
  }
  DWORD written;
  if(!WriteFile(hComm, buffer, length, &written,0)){
    // printf("writeBytes(): couldn't write to port");
    return 0;
  }
  //printf("wrote %i  bytes" , (int) written);
  return (int)written;
}

bool Serial::writeByte(unsigned char singleByte){
  if (isSerialInit == false){
    printf("writeByte(): serial not inited");
    return false;
  }
  unsigned char tmpByte[1];
  tmpByte[0] = singleByte;
  DWORD written = 0;
  if(!WriteFile(hComm, tmpByte, 1, &written,0)){
    printf( "writeByte(): couldn't write to port");
    return false;
  }
  //printf("verbose wrote byte");
  return ((int)written > 0 ? true : false);
}
// end serial  data send/recive
///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
// serial utilities


void Serial::flush(bool flushIn, bool flushOut){
  if (isSerialInit == false){
    printf("flush(): serial not inited");
    return;
  }

  int flushType = 0;
  if( flushIn && flushOut){
    flushType = PURGE_TXCLEAR | PURGE_RXCLEAR;
  }
  else if(flushIn){
    flushType = PURGE_RXCLEAR;
  }
  else if(flushOut){
    flushType = PURGE_TXCLEAR;
  }
  else return;

  PurgeComm(hComm, flushType);
}

int Serial::available(){
  if (isSerialInit == false){
    printf("available(): serial not inited");
    return 0;
  }
  int numBytes = 0;
  COMSTAT stat;
  DWORD err;
  if(hComm!=INVALID_HANDLE_VALUE){
    if(!ClearCommError(hComm, &err, &stat)){
      numBytes = 0;
    } else {
      numBytes = stat.cbInQue;
    }
  } else {
    numBytes = 0;
  }
  return numBytes;
}
// end serial utilities
///////////////////////////////////////////////////////////
