/*
 * TI Voxel Lib component.
 *
 * Copyright (c) 2014 Texas Instruments Inc.
 */

#ifndef VOXEL_DEPTHCAMERA_H
#define VOXEL_DEPTHCAMERA_H

#include <Device.h>
#include <Parameter.h>
#include <Frame.h>
#include "VideoMode.h"
#include <FrameBuffer.h>

#define MAX_FRAME_BUFFERS 2

namespace Voxel
{
  
class DepthCamera
{
public:
  enum FrameCallBackType
  {
      CALLBACK_RAW_FRAME_UNPROCESSED,
      CALLBACK_RAW_FRAME_PROCESSED,
      CALLBACK_DEPTH_FRAME,
      CALLBACK_XYZ_POINT_CLOUD_FRAME
  };
  
  typedef Function<void (DepthCamera &camera, Frame &frame, FrameCallBackType callBackType)> CallbackType;
  
protected:
  DevicePtr _device;
  
  String _name, _id;
  
  Map<String, ParameterPtr> _parameters;
  
  FrameBuffer<RawFrame> _rawFrameBuffers;
  FrameBuffer<DepthFrame> _depthFrameBuffers;
  FrameBuffer<PointCloudFrame> _pointCloudBuffers;
  
  bool _addParameters(const Vector<ParameterPtr> &params);
  
  CallbackType _callback;
  
  FrameCallBackType _callBackType;
  
  ThreadPtr _captureThread;
  
  virtual bool _start() = 0;
  virtual bool _stop() = 0;
  
  virtual bool _captureRawFrame(RawFramePtr &rawFrame) = 0;
  virtual bool _captureDepthFrame(RawFramePtr &rawFrame, DepthFramePtr &depthFrame) = 0;
  
  virtual void _captureLoop(); // the main capture loop
  
  void _captureThreadWrapper(); // this is non-virtual and simply calls _captureLoop
  
  bool _running; // is capture running?
  
public:
  DepthCamera(const String &name, DevicePtr device): _device(device), _name(name), _id(name + "(" + device->id() + ")"),
  _rawFrameBuffers(MAX_FRAME_BUFFERS), _depthFrameBuffers(MAX_FRAME_BUFFERS), _pointCloudBuffers(MAX_FRAME_BUFFERS) {}
  
  virtual bool isInitialized() = 0;
  
  inline const String &name() const { return _name; }
  
  inline const String &id() const { return _id; }
  
  inline bool isRunning() { return _running; }
  
  template <typename T>
  bool get(const String &name, T &value, bool refresh = false);
  
  template <typename T>
  bool set(const String &name, const T &value);
  
  virtual bool setFrameRate(const FrameRate &r) = 0;
  virtual bool getFrameRate(FrameRate &r) = 0;
  
  virtual bool setFrameSize(const FrameSize &s) = 0;
  virtual bool getFrameSize(FrameSize &s) = 0;
  
  virtual bool registerCallback(FrameCallBackType type, CallbackType f);
  
  virtual bool clearCallback();
  
  virtual bool start();
  virtual bool stop();
  
  virtual void wait();
  
  virtual ~DepthCamera();
};

template <typename T>
bool DepthCamera::get(const String &name, T &value, bool refresh)
{
  auto p = _parameters.find(name);
  
  if(p != _parameters.end())
  {
    ParameterTemplate<T> *param = dynamic_cast<ParameterTemplate<T> *>(p->second.get());
    
    if(param == 0)
    {
      logger(ERROR) << "DepthCamera: Invalid value type '" << typeid(value).name() << "' used to set parameter " << _id << "." << name << std::endl;
      return false;
    }
    
    if(!param->get(value, refresh))
    {
      logger(ERROR) << "DepthCamera:Could not get value for parameter " << _id << "." << name << std::endl;
      return false;
    }
    
    return true;
  }
  else
  {
    logger(ERROR) << "DepthCamera: Unknown parameter " << _id << "." << name << std::endl;
    return false;
  }
}

template <typename T>
bool DepthCamera::set(const String &name, const T &value)
{
  auto p = _parameters.find(name);
  
  if(p != _parameters.end())
  {
    ParameterTemplate<T> *param = dynamic_cast<ParameterTemplate<T> *>(p->second.get());
    
    if(param == 0)
    {
      logger(ERROR) << "Invalid value type '" << typeid(value).name() << "' used to set parameter " << this->name() << "(" << _device->id() << ")." << name << std::endl;
      return false;
    }
    
    if(!param->set(value))
    {
      logger(ERROR) << "Could not set value " << value << " for parameter " << this->name() << "(" << _device->id() << ")." << name << std::endl;
      return false;
    }
    
    return true;
  }
  else
    return false;
}

typedef Ptr<DepthCamera> DepthCameraPtr;

}

#endif // DEPTHCAMERA_H