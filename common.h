
#ifndef __COMMON_H__
#define __COMMON_H__

#include "ReadFromIni.h"
#include "Logger.h"
#include <boost/format.hpp>
#include <boost/thread/mutex.hpp>

#include <exception>
#include <boost/exception/exception.hpp>
#include <boost/foreach.hpp>
#include <algorithm> 

#include <bitset>

#include "Config.h"

#define LOG_LEVEL sl::TRACE

#define logger GlobalLogger<LOG_LEVEL>::Get()

#define VICS_DLLEXPORT __declspec(dllexport)  
#define VICS_DLLIMPORT __declspec(dllimport)  

#define MAX_TRY_PUSH 1000000000
#define MAX_CACHED_IMAGE  500

#define MAX_RATIO_TYPE0 2.0
#define IMG_NORM_HEIGHT_TYPE0  320
#define IMG_NORM_WIDTH_TYPE0 IMG_NORM_HEIGHT_TYPE0*MAX_RATIO_TYPE0

#define MAX_RATIO_TYPE1 2.0
#define IMG_NORM_HEIGHT_TYPE1  640
#define IMG_NORM_WIDTH_TYPE1 IMG_NORM_HEIGHT_TYPE1*MAX_RATIO_TYPE1

#define IMG_BATCH_IMAGE 4
#define IMG_BATCH_VECHILE 4

#define SENCECLASSINDEX 25

#define SENCETYPE_KAKOU		0
#define SENCETYPE_DIANJING	1

#define SCENETYPE_DAY		0
#define SCENETIME_NIGHT		1

#define MaxGPUimgNum 20
#define MaxThreadTracking 16
//#define VISUAL_DEBUG_WINDOW
//#define VISUAL_DEBUG_VEHICLE

#define	SAFEBALT 251

#define FRONTWINDOW_QUEUE 

#define OBJ_CACHE_FRAME 5
const int CacheWidth = 352;
const int CacheHeight = 288;


#include<stdio.h>   
#include<tchar.h>
#include <Windows.h>

// Use to convert bytes to MB
#define DIV 1048576
#define WRITRXML
#ifdef WRITRXML
#define XMLSAVE 1
#else XMLSAVE 0
#endif

#include "opencv2/opencv.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#pragma comment (lib, "opencv_world300.lib")
//#pragma comment (lib, "opencv_core310.lib")
//#pragma comment (lib, "opencv_photo310.lib")
//#pragma comment (lib, "opencv_highgui310.lib")
//#pragma comment (lib, "opencv_imgproc310.lib")
//#pragma comment (lib, "opencv_imgcodecs310.lib")
//#pragma comment (lib, "opencv_video310.lib")
//#pragma comment (lib, "opencv_features2d310.lib")


#define THROW_RUNTIME_EXCEPTION(x) BOOST_THROW_EXCEPTION(std::runtime_error(std::string(x).c_str()));

#define SAFE_DELETE(x) if(NULL != x) delete x

#define SAFE_DELETE_ARR(x) if(NULL !=x) delete[] x

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/exception/exception.hpp>
#include <boost/exception/diagnostic_information.hpp>

int roll_die(boost::random::mt19937& gen);

class GlobalVariable
{
public:
	static std::string xml_path;
};

std::string GetDllPath();

long long CreateGlobalVehicleID(long long image_id, long long vehicle_id);

void show_image(uchar* buf, int width, int height, int step, int channel, int timeout=0);

//typedef (unsigned char) uchar;
typedef std::vector<uchar> MemObj;


#define CHECK(call) \
{ \
	const cudaError_t error = call; \
	if (error != cudaSuccess) \
			{ \
		printf("Error: %s:%d, ", __FILE__, __LINE__); \
		printf("code:%d, reason: %s\n", error, cudaGetErrorString(error)); \
		char temp_char[200];\
		sprintf_s(temp_char,"Error: %s:%d, code:%d, reason: %s\n", __FILE__, __LINE__,error, cudaGetErrorString(error));\
		OutputDebugStringW((LPCWSTR)temp_char);\
		exit(1); \
			} \
}

#endif