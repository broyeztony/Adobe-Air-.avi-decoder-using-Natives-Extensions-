#include "stdafx.h"


ABCBridge::ABCBridge(void)
{
}


ABCBridge::~ABCBridge(void)
{
}


FREObject ABCBridge::CreateFlashString(const char* msg)
{
	const uint8_t* u8msg = (const uint8_t*)msg;
	FREObject retObj;
	FRENewObjectFromUTF8(strlen((const char*)u8msg)+1, u8msg, &retObj);
	return retObj;
}

FREObject ABCBridge::CreateFlashArray(void)
{
	FREObject uArray	= 0;
	FRENewObject((const uint8_t*)"Array", 0, NULL, &uArray, 0);
	return uArray;
}

FREObject ABCBridge::CreateFlashInt(int n)
{
	FREObject retObj;
	FRENewObjectFromInt32( n, &retObj);
	return retObj;
}