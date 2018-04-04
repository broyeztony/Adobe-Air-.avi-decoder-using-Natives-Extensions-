#pragma once
class ABCBridge
{
public:
	ABCBridge(void);
	~ABCBridge(void);



	// Return an UTF-8 String
	static FREObject							CreateFlashString(const char*);	
	static FREObject							CreateFlashArray(void);	
	static FREObject							CreateFlashInt(int);


};

