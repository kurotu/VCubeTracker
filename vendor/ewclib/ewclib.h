// EWCLIB [Easy Web Camera LIBrary]
//           version 2.5
// Copyright (C) 2018 I.N.  All rights reserved.

// OS:Windows XP/Vista/7/8/8.1/10
// Compiler:Visual C++ 2008, 2010~2017+[qedit.h]

// 2010/04/15 ver.1.9 EWC_GetLastMessage():Unicode/マルチバイトの両対応化
// 2010/05/04         開始
// 2010/05/06         EWC_Open()/EWC_Close()仕様変更．
// 2010/05/06         EWC_Run()/EWC_Stop()追加．
// 2010/05/08         EWC_CloseAll()追加．
// 2010/05/13 ver.2.0 完成
// 2012/02/28 ver.2.1 メモリ解放時のメモリリークを修正
// 2014/01            最大カメラ数変更(8->10), @device:swも対象とする
// 2014/01            物理デバイス使用済みフラグ,EWC_GetFormat()追加
// 2014/01            EWC_Open():FriendlyName指定,デフォルト値追加
// 2014/01/17 ver.2.2 EWC_Open()のバリエーション追加,EWC_GetDeviceName()追加
// 2014/01/27         デバイス出力ピンのフォーマット指定の機能を追加
// 2014/01/27         フォーマット一覧を取得するサンプルの修正
// 2014/01/27         EWC_Open()引数追加,ewc_type,ewc_device_type(EWC_DEVICE_TYPE)追加
// 2014/01/27 ver.2.3 EWC_GetDeviceSubtype(),EWC_GetSubtype(),EWC_GUIDtoTEXT()を追加
// 2014/04/08         ewc_s[].count追加,EWC_Pause()追加,EWC_OneShot()追加
// 2014/04/22 ver.2.4 EWC_SaveProperty(),EWC_LoadProperty(),EWC_SetManual()追加
// 2018/05/29 ver.2.5 Visual C++ 2017対応

#pragma once
#define EWCLIB_H

#include <dshow.h>
#pragma include_alias( "dxtrans.h", "qedit.h" )
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__
#include <qedit.h>
#pragma comment(lib,"strmiids.lib")

#include <math.h>	//for floor()

//画像フォーマット（出力部分）
#ifndef EWC_TYPE
	#ifdef _CV_H_
		#define EWC_TYPE MEDIASUBTYPE_RGB24
	#else
		#ifdef __OPENCV_CV_H__
			#define EWC_TYPE MEDIASUBTYPE_RGB24
		#else
			#define EWC_TYPE MEDIASUBTYPE_RGB32
		#endif
	#endif
#endif
//デフォルト値
GUID ewc_type= EWC_TYPE;

//画像フォーマット（デバイス出力ピン部分）
#ifndef EWC_DEVICE_TYPE
	#define EWC_DEVICE_TYPE GUID_NULL
#endif
//デフォルト値
GUID ewc_device_type= EWC_DEVICE_TYPE;

#ifndef EWC_WX
	#define EWC_WX 640
#endif
#ifndef EWC_WY
	#define EWC_WY 480
#endif
#ifndef EWC_FPS
	#define EWC_FPS 30
#endif
//デフォルト値
int ewc_wx= EWC_WX;
int ewc_wy= EWC_WY;
double ewc_fps= EWC_FPS;

#ifndef min
	#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
	#define max(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef EWC_NCAMMAX
#define EWC_NCAMMAX 10	//カメラの最大認識数v2.2
#endif

int ewc_cominit= 0;			//COM初期化フラグ(1なら終了処理を行う)
int ewc_ncam= -1;			//カメラ接続数
int ewc_order[EWC_NCAMMAX];	//初期化した番号の順番
int ewc_ordercnt= 0;		//ewc_order[]のためのカウンタ
HRESULT ewc_hr;
int ewc_used[EWC_NCAMMAX];	//物理デバイス使用済みフラグ

#define EWC_RUN_TIMEOUT		3000
#define EWC_STOP_TIMEOUT	3000
#define EWC_RETRYTIMES		3

#define EWC_VPAMPMAX	10
#define EWC_CAMCTLMAX	7
#define EWC_ITEMMAX		(EWC_VPAMPMAX+EWC_CAMCTLMAX)

#define ewc_release(x) {if(x)x->Release();x=0;}

//構造体の定義
struct ewc_struct{
	int wx;					//画像の幅
	int wy;					//画像の高さ
	double fps;				//フレームレート（引数指定値）
	int device;				//物理デバイス番号（引数指定値）
	int devn;				//割り当てられた物理デバイス番号v2.2
	char *pdname;			//FriendlyName比較用v2.2
	char dname[256];		//取得したFriendlyName v2.2
	GUID mstype;			//出力画像のフォーマット
	char mstype_t[80];		//mstypeに対応する文字列v2.3
	GUID dev_mstype;		//デバイス出力ピンのフォーマットv2.3
	char dev_mstype_t[80];	//dev_mstypeに対応する文字列v2.3
	volatile int init;
	volatile int stop;
	volatile int errcode;
	int *pbuf;				//画像の保存先
	int *buffer;			//内部で確保したフレームバッファ
	volatile long bufsize;	//得られた画像データのバイト数
	volatile double stime;	//サンプル時刻(s)
	volatile double ftime;	//フレーム周期(s)の実測値
	volatile int count;		//フレーム取得数v2.4
	int vflag[EWC_ITEMMAX];
	IGraphBuilder *pGraph;
	IBaseFilter *pF;
	ISampleGrabber *pGrab;
	ICaptureGraphBuilder2 *pBuilder;
	IBaseFilter *pCap;
	IAMVideoProcAmp *pVPAmp;
	IAMCameraControl *pCamCtl;
	IMediaControl *pMediaControl;
	IAMStreamConfig *pConfig;
	IMoniker *pMoniker;
	IEnumMoniker *pEnum;
	ICreateDevEnum *pDevEnum;
	AM_MEDIA_TYPE *pmt;
	AM_MEDIA_TYPE mt;
	IPin *pSrcOut;
	IPin *pSGrabIn;
	IMediaEvent *pMediaEvent;
};

ewc_struct ewc_s[EWC_NCAMMAX];

//フォーマット格納のための構造体v2.3
struct ewc_format{
	int width;
	int height;
	int bit;
	REFERENCE_TIME AvgTimePerFrame;
	double fps;
	GUID subtype;
	char subtype_t[80];	//subtypeに対応する文字列v2.3
};

//コールバック関数の定義
class ewc_SampleGrabberCB :public ISampleGrabberCB
{
public:
	STDMETHODIMP_(ULONG) AddRef()
	{
		return 2;
	}
	STDMETHODIMP_(ULONG) Release()
	{
		return 1;
	}
	STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
	{
		if(riid==IID_ISampleGrabberCB || riid==IID_IUnknown){
			*ppv= (void *)static_cast<ISampleGrabberCB*>(this);
			return NOERROR;
		}
		return E_NOINTERFACE;
	}
	STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample)
	{
		return S_OK;
	}
	//フレーム毎に呼ばれる関数
	STDMETHODIMP BufferCB(double dblSampleTime, BYTE *pBuffer, long lBufferSize)
	{
		ewc_s[i].bufsize= lBufferSize;
		int wx= ewc_s[i].wx;
		int wy= ewc_s[i].wy;
		int byte= lBufferSize/wy;
		//画像の上下を逆にしてコピー
		for(int y=0; y<wy; y++){
			memcpy((unsigned char *)ewc_s[i].pbuf+(wy-1-y)*byte, pBuffer+y*byte,byte);
		}
		ewc_s[i].ftime= dblSampleTime - ewc_s[i].stime;
		ewc_s[i].stime= dblSampleTime;
		ewc_s[i].count++;	//フレーム取得数の更新v2.4
		return S_OK;
	}
	//コンストラクタ	
	ewc_SampleGrabberCB(int num)
	{
		i=num;
		ewc_s[i].pbuf=ewc_s[i].buffer;
		ewc_s[i].bufsize=0;
		tm=ewc_s[i].stime=0.;
	}
	//デストラクタ
	~ewc_SampleGrabberCB()
	{
	}
	void TimeSet(double *t)
	{
		*t= tm= ewc_s[i].stime;
	}
	int IsCaptured(void)
	{
		if(tm!=ewc_s[i].stime) return 1;
		else return 0;
	}
private:
	int i;
	double tm;
};

//コールバック関数
ewc_SampleGrabberCB *ewc_pSampleGrabberCB[EWC_NCAMMAX];

//IAMVideoProcAmp
#define EWC_BRIGHTNESS				0
#define EWC_CONTRAST				1
#define EWC_HUE						2
#define EWC_SATURATION				3
#define EWC_SHARPNESS				4
#define EWC_GAMMA					5
#define EWC_COLORENABLE				6
#define EWC_WHITEBALANCE			7
#define EWC_BACKLIGHTCOMPENSATION	8
#define EWC_GAIN					9
//IAMCameraControl
#define EWC_PAN						10
#define EWC_TILT					11
#define EWC_ROLL					12
#define EWC_ZOOM					13
#define EWC_EXPOSURE				14
#define EWC_IRIS					15
#define EWC_FOCUS					16

//カメラ番号のチェック(戻り値：0ならカメラnumは有効)
int numCheck(int num)
{
	if(num<0 || num>=EWC_NCAMMAX) return 1;
	if(!ewc_s[num].init) return 2;
	return 0;
}

//カメラ台数を返す
int EWC_GetCamera(void)
{
	if(ewc_ncam==-1) return 0;
	return ewc_ncam;
}

//カメラ(番号:num)のフレームバッファサイズ(単位:バイト)を返す
int EWC_GetBufferSize(int num)
{
	if(numCheck(num)) return 0;
	return ewc_s[num].bufsize;
}

//フィルタのピンを取得する
IPin *ewc_GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir)
{
	IEnumPins *pEnum;
	IPin *pPin= 0;

	ewc_hr= pFilter->EnumPins(&pEnum);
	if(ewc_hr!=S_OK) return NULL;

	while(pEnum->Next(1,&pPin,0)==S_OK){
		PIN_DIRECTION PinDirThis;
		pPin->QueryDirection(&PinDirThis);
		if(PinDir==PinDirThis) break;
		pPin->Release();
	}
	pEnum->Release();
	return pPin;
}

//カメラ(番号:num)の画像取得
int EWC_GetImage(int num, void *buffer)
{
	if(numCheck(num)) return 1;
	memcpy(buffer,ewc_s[num].pbuf,ewc_s[num].bufsize);
	return 0;
}

//バッファアドレスを変更
int EWC_SetBuffer(int num, void *buffer)
{
	if(numCheck(num)) return 1;
	ewc_s[num].pbuf= (int *)buffer;
	return 0;
}

//現在のバッファアドレスを取得
int EWC_GetBuffer(int num, void **buffer)
{
	if(numCheck(num)) return 1;
	*buffer=ewc_s[num].pbuf;
	return 0;
}

//設定値を読んだり書いたりする関数 v2.4
int ewc_propfunc(int func, int num, int prop, double *value=NULL, int *mode=NULL)
{
	if(numCheck(num)) return 1;
	if(prop<0 || prop>=EWC_ITEMMAX) return 2;
	if(!ewc_s[num].vflag[prop]) return 3;

	long Min, Max, Step, Default, CapsFlags, Flags, Val;   

	if(prop<EWC_VPAMPMAX){
		//IAMVideoProcAmpの取得 pVPAmp
		ewc_s[num].pCap->QueryInterface(IID_IAMVideoProcAmp,(void **)&ewc_s[num].pVPAmp);
		//IAMVideoProcAmp
		ewc_hr= ewc_s[num].pVPAmp->GetRange(prop,&Min,&Max,&Step,&Default,&CapsFlags);	//範囲の取得
		if(ewc_hr==S_OK){
			ewc_hr= ewc_s[num].pVPAmp->Get(prop,&Val,&Flags);	//現在値の取得

			switch(func){
				case 0:	//初期値に戻す
					ewc_hr= ewc_s[num].pVPAmp->Set(prop,Default,Flags);
					break;
				case 1:	//値を設定，手動モードへ切替
					Val= (long)((*value*(Max-Min)/100.0)+Min);
					Val= min(max(Val,Min),Max);
					Val= (Val/Step)*Step;
					ewc_hr= ewc_s[num].pVPAmp->Set(prop,Val,VideoProcAmp_Flags_Manual);
					break;
				case 2:	//値の読み出し
					*value= (Val-Min)*100.0/(double)(Max-Min);
					if(mode) if(Flags & VideoProcAmp_Flags_Auto) *mode=1; else *mode=0;
					break;
				case 3:	//手動モード切替のみ
					ewc_hr= ewc_s[num].pVPAmp->Set(prop,Val,VideoProcAmp_Flags_Manual);
					break;
				case 4:	//自動モード切替のみ
					ewc_hr= ewc_s[num].pVPAmp->Set(prop,Val,VideoProcAmp_Flags_Auto);
					break;
			}
			ewc_release(ewc_s[num].pVPAmp);
			if(ewc_hr!=S_OK) return 5;
		}else{
			ewc_release(ewc_s[num].pVPAmp);
			return 4;
		}
	}else{
		//IAMCameraControlの取得 pCamCtl
		ewc_s[num].pCap->QueryInterface(IID_IAMCameraControl,(void **)&ewc_s[num].pCamCtl);
		//IAMCameraControl
		prop -= EWC_VPAMPMAX;
		ewc_hr= ewc_s[num].pCamCtl->GetRange(prop,&Min,&Max,&Step,&Default,&CapsFlags);	//範囲の取得
		if(ewc_hr==S_OK){
			ewc_hr= ewc_s[num].pCamCtl->Get(prop,&Val,&Flags);	//現在値の取得

			switch(func){
				case 0:	//初期値に戻す
					ewc_hr= ewc_s[num].pCamCtl->Set(prop,Default,Flags);
					break;
				case 1:	//値を設定，手動モードへ切替
					Val= (long)((*value*(Max-Min)/100.0)+Min);
					Val= min(max(Val,Min),Max);
					Val= (Val/Step)*Step;
					ewc_hr= ewc_s[num].pCamCtl->Set(prop,Val,CameraControl_Flags_Manual);
					break;
				case 2:	//値の読み出し
					*value= (Val-Min)*100.0/(double)(Max-Min);
					if(mode) if(Flags & CameraControl_Flags_Auto) *mode=1; else *mode=0;
					break;
				case 3:	//手動モード切替のみ
					ewc_hr= ewc_s[num].pCamCtl->Set(prop,Val,CameraControl_Flags_Manual);
					break;
				case 4:	//自動モード切替のみ
					ewc_hr= ewc_s[num].pCamCtl->Set(prop,Val,CameraControl_Flags_Auto);
					break;
			}
			ewc_release(ewc_s[num].pCamCtl);
			if(ewc_hr!=S_OK) return 5;
		}else{
			ewc_release(ewc_s[num].pCamCtl);
			return 4;
		}
	}
	return 0;
}

//設定値を読み出す
//  オプション：modeには0(manual)または1(auto)が格納される
double EWC_GetValue(int num, int prop, int *mode=NULL)
{
	double v;
	if(mode) *mode=0;
	int r= ewc_propfunc(2, num, prop, &v, mode);
	if(r) return -1.0;
	else return v;
}

//制御を手動モードに切り替える v2.4
int EWC_SetManual(int num, int prop)
{
	return ewc_propfunc(3, num, prop);
}

//設定値を変更する(v2.4からewc_propfunc()を呼び出す形に変更)
int EWC_SetValue(int num, int prop, double value)
{
	return ewc_propfunc(1, num, prop, &value);
}

//設定を初期値に戻す(v2.4からewc_propfunc()を呼び出す形に変更)
int EWC_SetDefault(int num, int prop)
{
	return ewc_propfunc(0, num, prop);
}

//制御を自動モードにする(v2.4からewc_propfunc()を呼び出す形に変更)
int EWC_SetAuto(int num, int prop)
{
	return ewc_propfunc(4, num, prop);
}

#pragma comment(lib,"Quartz.lib")

//最後のエラーメッセージを取得する
//s:文字列格納先  size:領域sのサイズ
void EWC_GetLastMessage(char *s, int size)
{
	wchar_t w[MAX_ERROR_TEXT_LEN];
	AMGetErrorTextW(ewc_hr,w,MAX_ERROR_TEXT_LEN);	//1.9
	WideCharToMultiByte(CP_ACP,0,w,-1,s,size,NULL,NULL);
}

//キャプチャの停止
int EWC_Stop(int num)
{
	if(numCheck(num)) return 1;

	//stop
	ewc_hr= ewc_s[num].pGraph->QueryInterface(IID_IMediaControl,(void **)&ewc_s[num].pMediaControl);
	if(ewc_hr==S_OK) ewc_s[num].pMediaControl->Stop(); else return 2;
	ewc_release(ewc_s[num].pMediaControl);

	long evCode;
	ewc_hr= ewc_s[num].pMediaEvent->WaitForCompletion(EWC_STOP_TIMEOUT,&evCode);
	if(ewc_hr!=S_OK){
		if(ewc_hr==E_ABORT) return 3;
	}
	return 0;
}

//キャプチャの再開
int EWC_Run(int num)
{
	if(numCheck(num)) return 1;

	//run
	ewc_hr= ewc_s[num].pGraph->QueryInterface(IID_IMediaControl,(void **)&ewc_s[num].pMediaControl);
	if(ewc_hr==S_OK) ewc_hr= ewc_s[num].pMediaControl->Run(); else return 2;
	ewc_release(ewc_s[num].pMediaControl);

	long evCode;
	ewc_hr= ewc_s[num].pMediaEvent->WaitForCompletion(EWC_RUN_TIMEOUT,&evCode);
	if(ewc_hr!=S_OK){
		if(ewc_hr==E_ABORT) return 3;
	}
	return 0;
}

//キャプチャのPause v2.4
int EWC_Pause(int num)
{
	if(numCheck(num)) return 1;

	//pause
	ewc_hr= ewc_s[num].pGraph->QueryInterface(IID_IMediaControl,(void **)&ewc_s[num].pMediaControl);
	if(ewc_hr==S_OK) ewc_hr= ewc_s[num].pMediaControl->Pause(); else return 2;
	ewc_release(ewc_s[num].pMediaControl);

	long evCode;
	ewc_hr= ewc_s[num].pMediaEvent->WaitForCompletion(EWC_RUN_TIMEOUT,&evCode);
	if(ewc_hr!=S_OK){
		if(ewc_hr==E_ABORT) return 3;
	}
	return 0;
}

//ワンショット v2.4
int EWC_OneShot(int num)
{
	if(numCheck(num)) return 1;

	int c1,c2,c3;

	c1= ewc_s[num].count;
	EWC_Run(num);

	while((c2= ewc_s[num].count) == c1) Sleep(1);	//１フレーム捨てる
	while((c3= ewc_s[num].count) == c2) Sleep(1);
	EWC_Pause(num);

	return 0;
}

//プロパティページを表示させる
int EWC_PropertyPage(int num)
{
	if(numCheck(num)) return 1;

	ISpecifyPropertyPages *pProp=0;

	ewc_hr= ewc_s[num].pCap->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
	if(ewc_hr!=S_OK) return 2;

	CAUUID caGUID;
	pProp->GetPages(&caGUID);
	ewc_release(pProp);

	FILTER_INFO FilterInfo;
	ewc_hr= ewc_s[num].pCap->QueryFilterInfo(&FilterInfo); 
	if(ewc_hr!=S_OK) return 3;

	IUnknown *pFilterUnk=0;
	ewc_hr= ewc_s[num].pCap->QueryInterface(IID_IUnknown,(void **)&pFilterUnk);
	if(ewc_hr!=S_OK){
		ewc_release(FilterInfo.pGraph);
		return 4;
	}

	OleCreatePropertyFrame(
		HWND_DESKTOP,		// Parent window
		0,0,				// Reserved
		FilterInfo.achName,	// Caption for the dialog box
		1,					// Number of objects (just the filter)
		&pFilterUnk,		// Array of object pointers. 
		caGUID.cElems,		// Number of property pages
		caGUID.pElems,		// Array of property page CLSIDs
		0,					// Locale identifier
		0, NULL				// Reserved
	);

	ewc_release(pFilterUnk);
	ewc_release(FilterInfo.pGraph);
	CoTaskMemFree(caGUID.pElems);
	return 0;
}

//新しい画像が到着したかどうか
//  num:カメラ番号  t:取得時刻(秒)
int EWC_IsCaptured(int num, double *t=NULL)
{
	if(numCheck(num)) return 0;

	if(ewc_pSampleGrabberCB[num]->IsCaptured()){
		double tt;
		if(!t) t= &tt;
		ewc_pSampleGrabberCB[num]->TimeSet(t);
		return 1;
	}
	return 0;
}

//画像変換(32ビット->24ビット)
void EWC_Cnv32to24(unsigned char *dst, unsigned int *src, int pxl)
{
	unsigned char R,G,B;
	unsigned int ui;

	for(int n=0; n<pxl; n++){
		ui= *src++;
		B= ui;
		G= ui>>8;
		R= ui>>16;
		*(dst+0)= B;
		*(dst+1)= G;
		*(dst+2)= R;
		dst += 3;
	}
}

//画像変換(24ビット->32ビット)
void EWC_Cnv24to32(unsigned int *dst, unsigned char *src, int pxl)
{
	unsigned char R,G,B;

	for(int n=0; n<pxl; n++){
		B = *(src+0);
		G = *(src+1);
		R = *(src+2);
		src += 3;
		*dst++ = (R<<16) | (G<<8) | B;
	}
}

//メディアサブタイプのGUID値に対応した文字列を得る v2.3
#include <wmsdkidl.h>
int EWC_GUIDtoTEXT(GUID guid, char *s, int size)
{
		 if(guid == MEDIASUBTYPE_YUY2) strcpy_s(s,size,"YUY2");
	else if(guid == MEDIASUBTYPE_MJPG) strcpy_s(s,size,"MJPG");
	else if(guid == MEDIASUBTYPE_H264) strcpy_s(s,size,"H264");
	else if(guid == MEDIASUBTYPE_IYUV) strcpy_s(s,size,"IYUV");
	else if(guid == MEDIASUBTYPE_RGB24) strcpy_s(s,size,"RGB24");
	else if(guid == MEDIASUBTYPE_RGB32) strcpy_s(s,size,"RGB32");
	else if(guid == WMMEDIASUBTYPE_I420) strcpy_s(s,size,"I420");
	else if(guid == MEDIASUBTYPE_RGB555) strcpy_s(s,size,"RGB555");
	else{
		char ss[80];
		sprintf_s(ss,sizeof(ss),"%X",guid.Data1);
		strcpy_s(s,size,ss);
		return 1;
	}
	return 0;
}

//メモリ解放 v2.2
void ewc_freememory(int num)
{
	if(ewc_s[num].devn!=-1) ewc_used[ewc_s[num].devn]=0;	//v2.2
	ewc_release(ewc_s[num].pMediaEvent);
	ewc_release(ewc_s[num].pMediaControl);
	ewc_release(ewc_s[num].pCamCtl);
	ewc_release(ewc_s[num].pVPAmp);
	ewc_release(ewc_s[num].pGrab);
	ewc_release(ewc_s[num].pF);
	ewc_release(ewc_s[num].pBuilder);
	ewc_release(ewc_s[num].pCap);
	if(ewc_s[num].buffer){
		delete[] ewc_s[num].buffer;
		ewc_s[num].buffer= 0;
	}
	ewc_release(ewc_s[num].pGraph);
	if(ewc_pSampleGrabberCB[num]){	//v2.1
		delete ewc_pSampleGrabberCB[num];
		ewc_pSampleGrabberCB[num]= 0;
	}
}

//使用開始(EWC_Open()から呼ばれる) v2.3
void ewc_Open(int num)
{
	int errcode, retryflag, retrytime= EWC_RETRYTIMES;
	ewc_s[num].devn= -1;
	int t0= 0;
	VIDEOINFOHEADER *vh= NULL;
	int n= 0;		//デバイス数のカウント用
	int regflag= 0;	//登録したか

cont:
	retryflag= 0;
	errcode= 0;

	//フィルタグラフマネージャ作成 pGraph
	ewc_hr= CoCreateInstance(CLSID_FilterGraph,0,CLSCTX_INPROC_SERVER,IID_IGraphBuilder,(void **)&ewc_s[num].pGraph);
	if(ewc_hr!=S_OK){errcode=3; goto fin;}

	//システムデバイス列挙子の作成
	ewc_hr= CoCreateInstance(CLSID_SystemDeviceEnum,0,CLSCTX_INPROC_SERVER,IID_ICreateDevEnum,(void **)&ewc_s[num].pDevEnum);
	if(ewc_hr!=S_OK){errcode=4; goto fin;}

	//列挙子の取得
	ewc_hr= ewc_s[num].pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&ewc_s[num].pEnum,0);
	if(ewc_hr!=S_OK){
		//ESP_Printf("No driver\n");
		errcode=5; goto fin;
	}

	//モニカの取得
	ULONG cFetched;
	wchar_t SrcName[32];

	for(int i=0; i<EWC_NCAMMAX; i++){
		if(ewc_s[num].pEnum->Next(1,&ewc_s[num].pMoniker,&cFetched)==S_OK){
			
			//DisplayNameの取得
			LPOLESTR strMonikerName=0;
			ewc_hr= ewc_s[num].pMoniker->GetDisplayName(NULL,NULL,&strMonikerName);
			if(ewc_hr!=S_OK){errcode=6; goto fin;}

			//char displayname[1024];
			//WideCharToMultiByte(CP_ACP,0,strMonikerName,-1,displayname,sizeof(displayname),0,0);
			//ESP_Printf("displayname(%d):%s\n",i,displayname);

			int cntflag= 0;		//デバイスとしてカウントすべきか
			if(wcsstr(strMonikerName,L"@device:pnp")!=NULL) cntflag= 1;	//DisplayNameに'@device:pnp'がある
			if(wcsstr(strMonikerName,L"@device:sw" )!=NULL) cntflag= 1;	//DisplayNameに'@device:sw'がある

			if(cntflag){
				char devname[256];	//FriendlyName格納用

				//FriendlyNameの取得
				IPropertyBag *pBag= 0;
				ewc_s[num].pMoniker->BindToStorage(0,0,IID_IPropertyBag,(void **)&pBag);
				VARIANT var;
				VariantInit(&var);
				var.vt= VT_BSTR;
				pBag->Read(L"FriendlyName",&var,0);
				WideCharToMultiByte(CP_ACP,0,var.bstrVal,-1,devname,sizeof(devname),0,0);
				VariantClear(&var);
				ewc_release(pBag);

				int match= 0;	//登録条件に合致したか
				if(ewc_s[num].pdname){
					//デイバス名指定の場合
					if(strstr(devname,ewc_s[num].pdname)) match=1;		//FriendlyName照合
				}else{
					//デイバス番号指定または省略時
					if((ewc_s[num].device==-1) || (ewc_s[num].device!=-1 && n==ewc_s[num].device)) match=1;
				}
				//条件が合えば登録
				if(!regflag && !ewc_used[n] && match){
					//オブジェクト初期化 pCap
					ewc_s[num].pMoniker->BindToObject(0,0,IID_IBaseFilter,(void **)&ewc_s[num].pCap);
					//グラフにフィルタを追加
					swprintf_s(SrcName,32,L"Video Capture %d",num);
					ewc_hr= ewc_s[num].pGraph->AddFilter(ewc_s[num].pCap, SrcName);
					if(ewc_hr!=S_OK){errcode=7; goto fin;}
					regflag++;
					ewc_s[num].devn= n;
					strcpy_s(ewc_s[num].dname, sizeof(ewc_s[num].dname), devname);	//FriendlyName保存
					ewc_used[n]= 1;
				}
				n++;
			}
			ewc_release(ewc_s[num].pMoniker);
		}
	}

	if(ewc_ncam==-1) ewc_ncam= n;	//カメラ数の登録

	ewc_release(ewc_s[num].pEnum);
	ewc_release(ewc_s[num].pDevEnum);

	if(!ewc_ncam){errcode=8; goto fin;}	//カメラがない
	if(!regflag){errcode=9; goto fin;}	//登録するものがなかった

	//ESP_Printf("camera=%d\n",ewc_ncam);

	//キャプチャビルダの作成 pBuilder
	CoCreateInstance(CLSID_CaptureGraphBuilder2,0,CLSCTX_INPROC_SERVER,
		IID_ICaptureGraphBuilder2,(void **)&ewc_s[num].pBuilder);
	ewc_hr= ewc_s[num].pBuilder->SetFiltergraph(ewc_s[num].pGraph);
	if(ewc_hr!=S_OK){errcode=10; goto fin;}
	
	//IAMStreamConfigインタフェースの取得
	ewc_hr= ewc_s[num].pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,&MEDIATYPE_Video,
		ewc_s[num].pCap,IID_IAMStreamConfig,(void**)&ewc_s[num].pConfig);
	if(ewc_hr!=S_OK){errcode=11; goto fin;}

	//画像サイズ，フレームレートの設定
	ewc_hr= ewc_s[num].pConfig->GetFormat(&ewc_s[num].pmt);
	vh= (VIDEOINFOHEADER*)ewc_s[num].pmt->pbFormat;
	if(ewc_s[num].wx==0) ewc_s[num].wx= ewc_wx;		//デフォルト値の適用
	if(ewc_s[num].wy==0) ewc_s[num].wy= ewc_wy;		//デフォルト値の適用
	if(ewc_s[num].fps==0) ewc_s[num].fps= ewc_fps;	//デフォルト値の適用
	ewc_wx= ewc_s[num].wx;		//デフォルト値の更新
	ewc_wy= ewc_s[num].wy;		//デフォルト値の更新
	ewc_fps= ewc_s[num].fps;	//デフォルト値の更新
	vh->bmiHeader.biWidth = ewc_s[num].wx;
	vh->bmiHeader.biHeight= ewc_s[num].wy; 
	vh->AvgTimePerFrame= (LONGLONG)floor((10000000.0/ewc_s[num].fps+0.5));

	//デバイス出力ピンのフォーマット指定v2.3
	if(ewc_s[num].dev_mstype!=GUID_NULL){
		ewc_s[num].pmt->subtype= ewc_s[num].dev_mstype;
	}
	ewc_s[num].dev_mstype= ewc_s[num].pmt->subtype;
	//フォーマットに対応する文字列の設定v2.3
	EWC_GUIDtoTEXT(ewc_s[num].mstype, ewc_s[num].mstype_t, sizeof(ewc_s[num].mstype_t));
	EWC_GUIDtoTEXT(ewc_s[num].dev_mstype, ewc_s[num].dev_mstype_t, sizeof(ewc_s[num].dev_mstype_t));

	ewc_hr= ewc_s[num].pConfig->SetFormat(ewc_s[num].pmt);
	if(ewc_hr!=S_OK){errcode=12; goto fin;}
	ewc_release(ewc_s[num].pConfig);

	//サンプルグラバの生成 pF,pGrab
	CoCreateInstance(CLSID_SampleGrabber,0,CLSCTX_INPROC_SERVER,IID_IBaseFilter,(LPVOID *)&ewc_s[num].pF);
	ewc_hr= ewc_s[num].pF->QueryInterface(IID_ISampleGrabber,(void **)&ewc_s[num].pGrab);
	if(ewc_hr!=S_OK){errcode=13; goto fin;}

	//メディアタイプの設定
	ZeroMemory(&ewc_s[num].mt,sizeof(AM_MEDIA_TYPE));
	ewc_s[num].mt.majortype= MEDIATYPE_Video;
	ewc_s[num].mt.subtype= ewc_s[num].mstype;
	ewc_type= ewc_s[num].mstype;	//デフォルト値の更新
	ewc_s[num].mt.formattype= FORMAT_VideoInfo;
	ewc_hr= ewc_s[num].pGrab->SetMediaType(&ewc_s[num].mt);
	if(ewc_hr!=S_OK){errcode=14; goto fin;}
	//フィルタグラフへの追加
	wchar_t GrabName[32];
	swprintf_s(GrabName,32,L"Grabber %d",num);
	ewc_hr= ewc_s[num].pGraph->AddFilter(ewc_s[num].pF, GrabName);
	if(ewc_hr!=S_OK){errcode=15; goto fin;}

	//サンプルグラバの接続
	// ピンの取得
	ewc_s[num].pSrcOut= ewc_GetPin(ewc_s[num].pCap,PINDIR_OUTPUT);
	ewc_s[num].pSGrabIn= ewc_GetPin(ewc_s[num].pF,PINDIR_INPUT);
	// ピンの接続
	ewc_hr= ewc_s[num].pGraph->Connect(ewc_s[num].pSrcOut, ewc_s[num].pSGrabIn);
	if(ewc_hr!=S_OK){errcode=16; goto fin;}

	ewc_release(ewc_s[num].pSrcOut);
	ewc_release(ewc_s[num].pSGrabIn);

	//グラバのモード設定
	ewc_hr= ewc_s[num].pGrab->SetBufferSamples(FALSE);
	if(ewc_hr!=S_OK){errcode=17; goto fin;}
	ewc_hr= ewc_s[num].pGrab->SetOneShot(FALSE);
	if(ewc_hr!=S_OK){errcode=18; goto fin;}

	//バッファの確保，コールバック関数の登録 buffer,ewc_pSampleGrabberCB[]
	ewc_s[num].buffer= (int *)new int[ewc_s[num].wx*ewc_s[num].wy];
	ewc_pSampleGrabberCB[num]= new ewc_SampleGrabberCB(num);
	ewc_hr= ewc_s[num].pGrab->SetCallback(ewc_pSampleGrabberCB[num],1);
	if(ewc_hr!=S_OK){errcode=19; goto fin;}

	//IAMVideoProcAmpの取得 pVPAmp
	ewc_hr= ewc_s[num].pCap->QueryInterface(IID_IAMVideoProcAmp,(void **)&ewc_s[num].pVPAmp);
	if(ewc_hr!=S_OK){
		//IAMVideoProcAmpが取得できなければ，サポートしてないとみなす．
		for(int j=0;j<EWC_VPAMPMAX;j++){
			//not supported
			ewc_s[num].vflag[j]= 0;
		}
	}else{
		for(int j=0;j<EWC_VPAMPMAX;j++){
			long Min, Max, Step, Default, Flags;   
			ewc_hr= ewc_s[num].pVPAmp->GetRange(j,&Min,&Max,&Step,&Default,&Flags);
			if(ewc_hr==S_OK){
				ewc_s[num].vflag[j]= 1;
			}else{
				//not supported
				ewc_s[num].vflag[j]= 0;
			}
		}
	}
	ewc_release(ewc_s[num].pVPAmp);

	//IAMCameraControlの取得 pCamCtl
	ewc_hr= ewc_s[num].pCap->QueryInterface(IID_IAMCameraControl,(void **)&ewc_s[num].pCamCtl);
	if(ewc_hr!=S_OK){
		//IAMCameraControlが取得できなければ，サポートしてないとみなす．
		for(int j=0;j<EWC_CAMCTLMAX;j++){
			//not supported
			ewc_s[num].vflag[j+EWC_VPAMPMAX]= 0;
		}
	}else{
		for(int j=0;j<EWC_CAMCTLMAX;j++){
			long Min, Max, Step, Default, Flags;   
			ewc_hr= ewc_s[num].pCamCtl->GetRange(j,&Min,&Max,&Step,&Default,&Flags);
			if(ewc_hr==S_OK){
				ewc_s[num].vflag[j+EWC_VPAMPMAX]= 1;
			}else{
				//not supported
				ewc_s[num].vflag[j+EWC_VPAMPMAX]= 0;
			}
		}
	}
	ewc_release(ewc_s[num].pCamCtl);

	//IMediaEventの取得 pMediaEvent
	ewc_hr= ewc_s[num].pGraph->QueryInterface(IID_IMediaEvent,(LPVOID *)&ewc_s[num].pMediaEvent);
	if(ewc_hr!=S_OK){errcode=20; goto fin;}

	//キャプチャ開始 pMediaControl
	ewc_hr= ewc_s[num].pGraph->QueryInterface(IID_IMediaControl,(void **)&ewc_s[num].pMediaControl);
	if(ewc_hr!=S_OK){errcode=21; goto fin;}
	ewc_hr= ewc_s[num].pMediaControl->Run();
	if(ewc_hr!=S_OK){errcode=22; retryflag=1; goto fin;}
	ewc_release(ewc_s[num].pMediaControl);

	//１回以上サンプルされるまで待機
	long evCode;
	ewc_s[num].pMediaEvent->WaitForCompletion(EWC_RUN_TIMEOUT,&evCode);
	if(ewc_hr!=S_OK){
		if(ewc_hr==E_ABORT){errcode=23; retryflag=1; goto fin;}
	}
	t0= GetTickCount();
	do{
		if((GetTickCount()-t0)>EWC_RUN_TIMEOUT){errcode=24; retryflag=1; goto fin;}
	}while(ewc_s[num].bufsize==0);

fin:
	if(errcode){
		//ESP_Printf("errcode=%d\n",errcode);
		ewc_release(ewc_s[num].pDevEnum);
		ewc_release(ewc_s[num].pEnum);
		ewc_release(ewc_s[num].pMoniker);
		ewc_release(ewc_s[num].pConfig);
		ewc_freememory(num);

		//正常に接続されるまでリトライ
		if(retryflag) if(--retrytime) goto cont;
	}else{
		ewc_s[num].init= 1;
		ewc_order[ewc_ordercnt++]= num;
	}

	ewc_s[num].errcode= errcode;
}

//EWC_Open()で最初に呼ばれるサブ関数
void _ewc_open_pre(void)
{
	//COMの初期化
	if(!ewc_cominit && ewc_ncam==-1){
		ewc_hr= CoInitializeEx(NULL,COINIT_MULTITHREADED);
		if(ewc_hr==S_OK) ewc_cominit=1;	//成功したらフラグを立てる
	}
	//構造体の初期化
	if(ewc_ncam==-1){
		for(int i=0; i<EWC_NCAMMAX; i++){
			ZeroMemory(&ewc_s[i],sizeof(ewc_struct));
		}
	}
}

//使用開始[1]（デバイス番号指定）v2.3
int EWC_Open(int num, int wx=0, int wy=0, double fps=0, int device=-1, GUID mstype=ewc_type, GUID dev_mstype=ewc_device_type)
{
	if(num<0 || num>=EWC_NCAMMAX) return 1;

	_ewc_open_pre();

	if(ewc_s[num].init) return 2;

	ewc_s[num].wx= wx;
	ewc_s[num].wy= wy;
	ewc_s[num].fps= fps;
	ewc_s[num].device= device;
	ewc_s[num].pdname= NULL;
	ewc_s[num].mstype= mstype;
	ewc_s[num].dev_mstype= dev_mstype;
	ewc_s[num].errcode= -1;

	ewc_Open(num);

	return ewc_s[num].errcode;
}

//使用開始[2]（デバイス名指定）v2.3
int EWC_Open(int num, int wx, int wy, double fps, char *devicename, GUID mstype=ewc_type, GUID dev_mstype=ewc_device_type)
{
	if(num<0 || num>=EWC_NCAMMAX) return 1;

	_ewc_open_pre();

	if(ewc_s[num].init) return 2;

	ewc_s[num].wx= wx;
	ewc_s[num].wy= wy;
	ewc_s[num].fps= fps;
	ewc_s[num].device= -1;
	ewc_s[num].pdname= devicename;
	ewc_s[num].mstype= mstype;
	ewc_s[num].dev_mstype= dev_mstype;
	ewc_s[num].errcode= -1;

	ewc_Open(num);

	return ewc_s[num].errcode;
}

//使用開始[3]（デバイス名指定，wx/wy/fps省略）v2.2
int EWC_Open(int num, char *devicename, GUID mstype=ewc_type, GUID dev_mstype=ewc_device_type)
{
	if(num<0 || num>=EWC_NCAMMAX) return 1;

	_ewc_open_pre();

	if(ewc_s[num].init) return 2;

	ewc_s[num].wx= 0;
	ewc_s[num].wy= 0;
	ewc_s[num].fps= 0;
	ewc_s[num].pdname= devicename;
	ewc_s[num].mstype= mstype;
	ewc_s[num].dev_mstype= dev_mstype;
	ewc_s[num].errcode= -1;

	ewc_Open(num);

	return ewc_s[num].errcode;
}

//終了処理
int EWC_Close(int num)
{
	if(numCheck(num)) return 1;

	//キャプチャ停止
	int r= EWC_Stop(num);
	if(r) return 2;

	//メモリ解放
	ewc_freememory(num);
	ewc_s[num].init= 0;

	if(ewc_ordercnt){
		if(ewc_order[ewc_ordercnt-1]==num){
			ewc_ordercnt--;
		}
	}

	//すべて終了ならCOM終了
	int c=0;
	for(int i=0; i<EWC_NCAMMAX; i++){
		c += ewc_s[i].init;
	}
	if(c==0){
		if(ewc_cominit){
			CoUninitialize();
			ewc_cominit= 0;
		}
		ewc_ncam= -1;
	}
	return 0;
}

//終了処理（すべて）
int EWC_CloseAll(void)
{
	int r=0;
	while(ewc_ordercnt>0){
		r += EWC_Close(ewc_order[--ewc_ordercnt]);
	}
	return r;
}

void ewc_FreeMediaType(AM_MEDIA_TYPE& mt)
{
	if(mt.cbFormat){
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat= 0;
		mt.pbFormat= NULL;
	}
	if(mt.pUnk){
		ewc_release(mt.pUnk);
	}
}

void ewc_DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
	if(pmt){
        ewc_FreeMediaType(*pmt); 
        CoTaskMemFree(pmt);
    }
}

//EWC_GetFormat()から呼ばれる
int ewc_GetFormat(int device, char *pdname, ewc_format *fmt, int *nmax)
{
	IGraphBuilder *pGraph= 0;
	ICreateDevEnum *pDevEnum= 0;
	IEnumMoniker *pEnum= 0;
	ICaptureGraphBuilder2 *pBuilder= 0;
	IBaseFilter *pCap= 0;
	IAMStreamConfig *pConfig= 0;
	IMoniker *pMoniker= 0;

	int ewc_cominit_gf= 0;
	int errcode= 0;

	int count= 0, size= 0;
	int m= 0;	//最終的なフォーマット数
				
	int n= 0;		//デバイス数のカウント用
	int regflag= 0;	//登録したか
	
	//COMの初期化
	if(!ewc_cominit){
		ewc_hr= CoInitializeEx(NULL,COINIT_MULTITHREADED);
		if(ewc_hr==S_OK) ewc_cominit_gf= 1;	//成功したらフラグを立てる
	}

	//フィルタグラフマネージャ作成 pGraph
	ewc_hr= CoCreateInstance(CLSID_FilterGraph,0,CLSCTX_INPROC_SERVER,IID_IGraphBuilder,(void **)&pGraph);
	if(ewc_hr!=S_OK){errcode=3; goto fin_gf;}

	//システムデバイス列挙子の作成
	ewc_hr= CoCreateInstance(CLSID_SystemDeviceEnum,0,CLSCTX_INPROC_SERVER,IID_ICreateDevEnum,(void **)&pDevEnum);
	if(ewc_hr!=S_OK){errcode=4; goto fin_gf;}

	//列挙子の取得
	ewc_hr= pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pEnum,0);
	if(ewc_hr!=S_OK){errcode=5; goto fin_gf;}

	//モニカの取得
	ULONG cFetched;
	wchar_t SrcName[32];

	for(int i=0; i<EWC_NCAMMAX; i++){
		if(pEnum->Next(1,&pMoniker,&cFetched)==S_OK){
			
			//DisplayNameの取得
			LPOLESTR strMonikerName=0;
			ewc_hr= pMoniker->GetDisplayName(NULL,NULL,&strMonikerName);
			if(ewc_hr!=S_OK){errcode=6; goto fin_gf;}

			int cntflag=0;		//デバイスとしてカウントすべきか
			if(wcsstr(strMonikerName,L"@device:pnp")!=NULL) cntflag=1;	//DisplayNameに'@device:pnp'がある
			if(wcsstr(strMonikerName,L"@device:sw" )!=NULL) cntflag=1;	//DisplayNameに'@device:sw'がある

			if(cntflag){
				char devname[256];	//FriendlyName格納用

				//FriendlyNameの取得
				IPropertyBag *pBag= 0;
				pMoniker->BindToStorage(0,0,IID_IPropertyBag,(void **)&pBag);
				VARIANT var;
				VariantInit(&var);
				var.vt= VT_BSTR;
				pBag->Read(L"FriendlyName",&var,0);
				WideCharToMultiByte(CP_ACP,0,var.bstrVal,-1,devname,sizeof(devname),0,0);
				VariantClear(&var);
				ewc_release(pBag);

				int match= 0;	//登録条件に合致したか
				if(pdname){
					//デイバス名指定の場合
					if(strstr(devname,pdname)) match= 1;		//FriendlyName照合
				}else{
					//デイバス番号指定または省略時
					if(n==device) match= 1;
				}
				//条件が合えば登録
				if(!regflag && match){
					//オブジェクト初期化 pCap
					pMoniker->BindToObject(0,0,IID_IBaseFilter,(void **)&pCap);
					//グラフにフィルタを追加
					swprintf_s(SrcName,32,L"Video Capture %d",i);
					ewc_hr= pGraph->AddFilter(pCap, SrcName);
					if(ewc_hr!=S_OK){errcode=7; goto fin_gf;}
					regflag++;
				}
				n++;
			}
			ewc_release(pMoniker);
		}
	}

	ewc_release(pEnum);
	ewc_release(pDevEnum);

	if(!regflag){errcode=9; goto fin_gf;}	//登録するものがなかった

	//キャプチャビルダの作成 pBuilder
	CoCreateInstance(CLSID_CaptureGraphBuilder2,0,CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2,(void **)&pBuilder);
	ewc_hr= pBuilder->SetFiltergraph(pGraph);
	if(ewc_hr!=S_OK){errcode=10; goto fin_gf;}
	
	//IAMStreamConfigインタフェースの取得
	ewc_hr= pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,&MEDIATYPE_Video, pCap,IID_IAMStreamConfig,(void**)&pConfig);
	if(ewc_hr!=S_OK){errcode=11; goto fin_gf;}

	//-----数の取得-----
	ewc_hr= pConfig->GetNumberOfCapabilities(&count,&size);
	if(ewc_hr!=S_OK){errcode=12; goto fin_gf;}

	//ESP_Printf("count=%d\n",count);

	//-----フォーマットの取得-----
	if(size==sizeof(VIDEO_STREAM_CONFIG_CAPS)){
		for(int i=0; i<count; i++){
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE *pmt;
			ewc_hr= pConfig->GetStreamCaps(i,&pmt,reinterpret_cast<BYTE*>(&scc)); 
			if(ewc_hr==S_OK && pmt->formattype==FORMAT_VideoInfo){
				VIDEOINFOHEADER *vh= reinterpret_cast<VIDEOINFOHEADER*>(pmt->pbFormat);
				fmt[m].width= vh->bmiHeader.biWidth;
				fmt[m].height= vh->bmiHeader.biHeight;
				fmt[m].bit= vh->bmiHeader.biBitCount;
				fmt[m].AvgTimePerFrame= vh->AvgTimePerFrame;
				fmt[m].fps= 10000000./vh->AvgTimePerFrame;
				fmt[m].subtype= pmt->subtype;
				EWC_GUIDtoTEXT(fmt[m].subtype, fmt[m].subtype_t, sizeof(fmt[m].subtype_t));	//v2.3
				ewc_DeleteMediaType(pmt);
				m++;
				if(m >= *nmax) break;
			} 
		}
	}
	//数の更新
	*nmax= m;

fin_gf:
	ewc_release(pConfig);
	ewc_release(pBuilder);
	ewc_release(pCap);
	ewc_release(pGraph);

	if(ewc_cominit_gf){
		CoUninitialize();
		ewc_cominit_gf= 0;
	}
	return errcode;
}

//対応フォーマットを取得する（デバイス番号）v2.2
int EWC_GetFormat(int devn, ewc_format *fmt, int *nmax)
{
	if(devn<0 || devn>=EWC_NCAMMAX) return 1;

	return ewc_GetFormat(devn, 0, fmt, nmax);
}

//対応フォーマットを取得する（デバイス名）v2.2
int EWC_GetFormat(char *devicename, ewc_format *fmt, int *nmax)
{
	return ewc_GetFormat(0, devicename, fmt, nmax);
}

//使用開始されたカメラのデバイス名を取得 v2.2
char *EWC_GetDeviceName(int num)
{
	if(numCheck(num)) return NULL;
	
	return ewc_s[num].dname;
}

//使用開始されたカメラの出力ピンのメディアサブタイプ文字列を取得 v2.3
char *EWC_GetDeviceSubtype(int num)
{
	if(numCheck(num)) return NULL;
	
	return ewc_s[num].dev_mstype_t;
}

//使用開始されたカメラの出力画像のメディアサブタイプ文字列を取得 v2.3
char *EWC_GetSubtype(int num)
{
	if(numCheck(num)) return NULL;
	
	return ewc_s[num].mstype_t;
}

const char *ewc_propstr[EWC_ITEMMAX]={
	"BRIGHTNESS","CONTRAST","HUE","SATURATION","SHARPNESS","GAMMA","COLORENABLE","WHITEBALANCE","BACKLIGHTCOMPENSATION","GAIN",
	"PAN","TILT","ROLL","ZOOM","EXPOSURE","IRIS","FOCUS"
};

//設定をファイルへ保存する v2.4
int EWC_SaveProperty(int num, char *filename=NULL)
{
	if(numCheck(num)) return 1;

	FILE *fp;
	double val;
	int mode;
	char buf[256];

	char fname[MAX_PATH];
	if(!filename){
		//ファイル名省略時はデバイス名.txtを使う
		strcpy_s(fname,sizeof(fname),EWC_GetDeviceName(num));
		strcat_s(fname,sizeof(fname),".txt");
	}else{
		strcpy_s(fname,sizeof(fname),filename);
	}

	fopen_s(&fp,fname,"w");
	if(fp==NULL) return 2;

	for(int i=0;i<EWC_ITEMMAX;i++){
		val= EWC_GetValue(num, i, &mode);
		char ss[64];
		_gcvt_s(ss,sizeof(ss),val,18);
		if(mode){
			sprintf_s(buf,sizeof(buf),"%-21s = %s, AUTO\n",ewc_propstr[i],ss);
		}else{
			sprintf_s(buf,sizeof(buf),"%-21s = %s\n",ewc_propstr[i],ss);
		}
		fputs(buf,fp);		
	}

	fclose(fp);

	return 0;
}

//文字列sからprop文字列を検索し，番号[0-(EWC_ITEMMAX-1)]を返す
//なければ-1を返す
//また，AUTOがあれば，mode=1にする．','は終端にする．
int find_propstr(char *s, int size, int *mode)
{
	for(int i=0;i<EWC_ITEMMAX;i++){
		_strupr_s(s,size);
		if(strstr(s,ewc_propstr[i])){
			char *p1,*p2;
			p1=strstr(s,",");
			p2=strstr(s,"AUTO");
			if(p1 && p2){
				*mode=1;
				*p1='\0';
			}else{
				*mode=0;
			}
			return i;
		}
	}
	return -1;
}

//設定をファイルから読み込む v2.4
int EWC_LoadProperty(int num, char *filename=NULL)
{
	if(numCheck(num)) return 1;

	FILE *fp;

	char fname[MAX_PATH];
	if(!filename){
		//ファイル名省略時はデバイス名.txtを使う
		strcpy_s(fname,sizeof(fname),EWC_GetDeviceName(num));
		strcat_s(fname,sizeof(fname),".txt");
	}else{
		strcpy_s(fname,sizeof(fname),filename);
	}

	fopen_s(&fp,fname,"r");
	if(fp==NULL) return 2;

	for(;;){
		char buf[256];
		if(fgets(buf,sizeof(buf),fp)==NULL) break;

		int mode=0;
		int i=find_propstr(buf,sizeof(buf),&mode);
		if(i<0) continue;
		
		char *p;
		p=strstr(buf,"=");
		if(!p) continue;
		p++;
		double val=atof(p);
		
		EWC_SetValue(num, i, val);
		if(mode) EWC_SetAuto(num, i);
	}

	fclose(fp);

	return 0;
}

//End of ewclib.h
