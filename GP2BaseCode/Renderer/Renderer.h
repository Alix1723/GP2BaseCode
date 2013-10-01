#pragma once

class IRenderer
{
public:
	virtual ~IRenderer(){};
	virtual bool init(void *pWindowHandle,bool fullScreen)=0;
	virtual void clear(float r,float g,float b,float a)=0;
	virtual void present()=0;
	virtual void render()=0;
};