// FRENativeDecoderEx.cpp : définit les fonctions exportées pour l'application DLL.
//

#include "stdafx.h"


extern "C"
{
	IGraphBuilder* g_Graph			= 0;
	IBaseFilter* g_GrabberFilter	= 0;
	ISampleGrabber* g_Grabber		= 0;
	IMediaControl *pControl			= 0;
	
	long pBufferSize				= 0;
	unsigned char* pBuffer			= 0;

	unsigned int gWidth = 0, gHeight = 0, gPixelChannels = 0;
	
	/* ActionScript Objects */
	FREObject objectBitmapData;
	FREBitmapData2 bitmapData;
	int firstTime					= 1;
	int VERBOSE						= 0;

	/* Headers */
	FREObject RenderingThreadFunction(void);




	void Debug(HRESULT hr, LPCSTR pText)
	{
		if (VERBOSE == 0) return;

		if (SUCCEEDED(hr)) MessageBox(0, pText, "Success", MB_OK);
		else MessageBox(0, pText, "Failed", MB_OK);
	}

	void DebugFRE(FREResult re, LPCSTR pText)
	{
		if (VERBOSE == 0) return;

		if (re == FRE_OK) MessageBox(0, pText, "FRE_OK", MB_OK);
		else if (re == FRE_TYPE_MISMATCH) MessageBox(0, pText, "FRE_TYPE_MISMATCH", MB_OK);
		else if (re == FRE_INVALID_OBJECT) MessageBox(0, pText, "FRE_INVALID_OBJECT", MB_OK);
		else if (re == FRE_INVALID_ARGUMENT) MessageBox(0, pText, "FRE_INVALID_ARGUMENT", MB_OK);
		else if (re == FRE_WRONG_THREAD) MessageBox(0, pText, "FRE_WRONG_THREAD", MB_OK);
		else if (re == FRE_ILLEGAL_STATE) MessageBox(0, pText, "FRE_ILLEGAL_STATE", MB_OK);     
	}

	HRESULT SetupFrameGrabber(void)
	{
		CoInitialize( 0 );
		HRESULT hr		= CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&g_Graph));
		Debug(hr, "CLSID_FilterGraph");

		hr				= g_Graph->QueryInterface(IID_PPV_ARGS(&pControl));
		Debug(hr, "QueryInterface(IID_PPV_ARGS(&pControl))");

		hr				= CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_GrabberFilter));
		Debug(hr, "CLSID_SampleGrabber");

		hr				= g_Graph->AddFilter(g_GrabberFilter, L"Sample Grabber");
		Debug(hr, "g_GrabberFilter");

	    hr				= g_GrabberFilter->QueryInterface(IID_PPV_ARGS(&g_Grabber));
		Debug(hr, "QueryInterface");


		AM_MEDIA_TYPE mt;
        ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
        mt.majortype	= MEDIATYPE_Video;
        mt.subtype		= MEDIASUBTYPE_RGB24;
        hr				= g_Grabber->SetMediaType(&mt);
		Debug(hr, "SetMediaType");

		hr				= g_Grabber->SetOneShot(FALSE);
		Debug(hr, "SetOneShot");

        hr				= g_Grabber->SetBufferSamples(TRUE);
		Debug(hr, "SetBufferSamples");

		return hr;
	}

	HRESULT GetMetaData()
	{
		AM_MEDIA_TYPE mt;
		HRESULT hr		= g_Grabber->GetConnectedMediaType(&mt);

		VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER *)mt.pbFormat;
		gPixelChannels	= pVih->bmiHeader.biBitCount / 8;
		gWidth			= pVih->bmiHeader.biWidth;
		gHeight			= pVih->bmiHeader.biHeight;

		if (mt.cbFormat != 0)
		{
			CoTaskMemFree((PVOID)mt.pbFormat);
			mt.cbFormat = 0;
			mt.pbFormat = NULL;
		}
		if (mt.pUnk != NULL)// Unecessary because pUnk should not be used, but safest.
		{
			mt.pUnk->Release();
			mt.pUnk = NULL;
		}
		return hr;
	}

	unsigned char* GetSampleData()
	{
		HRESULT hr;

		long Size = 0;

		hr = g_Grabber->GetCurrentBuffer(&Size, NULL);


		if (FAILED(hr)) return 0;
		else if (Size != pBufferSize)
		{
			pBufferSize = Size;
			if (pBuffer != 0) delete[] pBuffer;
			pBuffer = new unsigned char[pBufferSize];
		}

		hr = g_Grabber->GetCurrentBuffer(&pBufferSize, (long*)pBuffer);
		if (FAILED(hr)) return 0;
		else return pBuffer;                
	}

	FREObject PreloadFile(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
	{
		// Init DirectShow
		HRESULT hr = SetupFrameGrabber();

		// Get filename
		const uint8_t* str	= 0;
		uint32_t len		= -1;
		FREObject filename			= argv[0 ];
		FREGetObjectAsUTF8( filename, &len, &str );

		char* filepath = (char*)str;
		
		wchar_t *wFilepath = new wchar_t[len + 1];
		memset(wFilepath, 0, len);
		MultiByteToWideChar(  CP_ACP, NULL, filepath, -1, wFilepath, len );

		
		// BSTR filepathBSTR;
		// ASCtoBSTR(filepath, &filepathBSTR);

		// Start Rendering
		hr			= g_Graph->RenderFile( L"C:\\9.avi", NULL );

		hr			= GetMetaData();

		// Return metadata
		char metadata[40];
		sprintf(metadata, "%i:%i", gWidth, gHeight);
		return ABCBridge::CreateFlashString( metadata );
	}

	FREObject Render(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
	{
		FREResult re;
		HRESULT hr;


		if (firstTime > 0) 
		{
			firstTime = 0;
			objectBitmapData	= argv[0];
			
			hr = pControl->Run();
			Debug(hr, "pControl->Run");
		}

		re = FREAcquireBitmapData2(objectBitmapData, &bitmapData);
		// DebugFRE(re, "FREAcquireBitmapData2");


		return RenderingThreadFunction();
	}

	// unsigned __stdcall RenderingThreadFunction(void* args)
	FREObject RenderingThreadFunction()
	{
		FREResult re;
		unsigned char* pData = GetSampleData();


		if (pData != 0) 
		{
			// MessageBox(0, "pData != 0", "Inside Thread", MB_OK);


			BITMAPINFOHEADER *bmih  = (BITMAPINFOHEADER*)pBuffer;
			BYTE *pixels			= (BYTE *)bmih + sizeof(bmih);
			// pixels				   += 36; // 12 * 3 -> but i don't know why !!!


			int x			= 0;
			int y			= 0;
			int nLineBytes	= 0;

			BYTE A, R, G, B;

			BYTE* pLine		= NULL;
			BYTE* pixel32	= NULL;

			unsigned int pixelARGB;
			int k	= 0;

			nLineBytes				= (gWidth * 24 + 31) / 32 * 4;
				

			for (y = 0 ; y < gHeight ; ++y, pixels += nLineBytes)
			{
				pLine					= pixels;
				//	pLine				= pixels + (videoHeight - y - 1) * nLineBytes; // For Y Inverted pictures

				for (x = 0 ; x < gWidth ; ++x, pLine += 3)
				{
					pixel32					= pLine;
					A						= 0;
					R						= *pixel32;
					G						= *(pixel32+1);
					B						= *(pixel32+2);
					pixelARGB				= A << 24 | R << 16 | G << 8 | B;

					bitmapData.bits32[ k ]	= pixelARGB;
					k++;
				}
			}


		}


		re = FREInvalidateBitmapDataRect(objectBitmapData, 0, 0, gWidth, gHeight);
		// DebugFRE(re, "FREInvalidateBitmapDataRect" );

		FREReleaseBitmapData(objectBitmapData);
		// DebugFRE(re, "FREReleaseBitmapData" );

		return 0;
	}















	// ABC Bridge
	void contextInitializer(void* extData, const uint8_t* ctxType, FREContext ctx, uint32_t* numFunctions, const FRENamedFunction** functions)
	{
		static FRENamedFunction classMethods[] =
		{
			{(const uint8_t *)"PreloadFile", NULL, PreloadFile},
			{(const uint8_t *)"Render", NULL, Render},
		};

		const int methodCount	= sizeof(classMethods) / sizeof(FRENamedFunction);
		*numFunctions			= methodCount;
		*functions				= classMethods;
	}

	void contextFinalizer(FREContext ctx)
	{
		return;
	}

	__declspec(dllexport) void initializer(void** extData, FREContextInitializer* ctxInitializer, FREContextFinalizer* ctxFinalizer)
	{
		*ctxInitializer = &contextInitializer;
		*ctxFinalizer = &contextFinalizer;
	}

	__declspec(dllexport) void finalizer(void* extData)
	{
		return;
	}
}