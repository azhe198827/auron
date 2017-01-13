
#ifndef __GLOBAL_POOL_H__
#define __GLOBAL_POOL_H__

#include "VehicleBuffer.h"


//this class is used to get global object pools
//
//class Global
//{
//
//public:
//	//get ImagePool (object pool of ImageInfo)
//	static ImagePool* GetImagePool()
//	{
//		//global_pool_mtx_img.lock();
//		static ImagePool* p = new ImagePool();		
//		//global_pool_mtx_img.unlock();
//
//		return p;
//	}
//
//	//get VehiclePool (object pool of VehicleInfo)
//	static VehiclePool* GetVehiclePool()
//	{
//		//global_pool_mtx_veh.lock();
//		static VehiclePool* p = new VehiclePool();
//		//global_pool_mtx_veh.unlock();
//		return p;
//	}
//
//	//get whatever pool you want
//	template<class T>
//	static ObjectPool<T> * GetPool()
//	{
//		//global_pool_mtx_other.lock();
//		static ObjectPool<T>* p = new ObjectPool<T>();
//		//global_pool_mtx_other.unlock();
//		return p;
//	}
//};

#endif
