#pragma once

//The header file for the renderer interface
#include "../Renderer/Renderer.h"
#include <Windows.h>
#include <D3D10.h>
#include <D3DX10.h>

#define _XM_NO_INTRINSICS_
#include <xnamath.h>

//forward declarations of the D3D10 interfaces
struct ID3D10Device;
struct IDXGISwapChain;
struct ID3D10RenderTargetView;
struct ID3D10DepthStencilView;
struct ID3D10Texture2D;

struct ID3D10Effect;
struct ID3D10Buffer;
struct ID3D10InputLayout;
struct ID3D10EffectTechnique;

//D3D10Renderer implements the Renderer interface
class D3D10Renderer:public IRenderer
{
public:
	D3D10Renderer();
	~D3D10Renderer();
	//notice these have the same signature as the pure methods
	//in the interface
	bool init(void *pWindowHandle,bool fullScreen);
	void clear(float r,float g,float b,float a);
	void present();
	void render();

private:
	bool createDevice(HWND pWindowHandle,int windowWidth, int windowHeight,
	bool fullScreen);
	bool createInitialRenderTarget(int windowWidth, int windowHeight);
	bool loadEffectFromMemory(const char* pMem);
	bool loadEffectFromFile(const char* pFilename);
	bool createBuffer();
	bool createVertexLayout();
	void createCamera(XMVECTOR &position, XMVECTOR &focus, XMVECTOR &up, float fov, float aspectRatio, float nearClip, float farClip);
	void positionObject(float x, float y, float z);
	void rotateObject(float p, float y, float r);
	bool loadBaseTexture(char * pFilename);

private:
	//D3D10 stuff
	ID3D10Device * m_pD3D10Device;
	IDXGISwapChain * m_pSwapChain;
	ID3D10RenderTargetView * m_pRenderTargetView;
	ID3D10DepthStencilView * m_pDepthStencelView;
	ID3D10Texture2D *m_pDepthStencilTexture;
	ID3D10Effect * m_pTempEffect;
	ID3D10EffectTechnique * m_pTempTechnique;
	ID3D10Buffer * m_pTempBuffer;
	ID3D10Buffer * m_pTempIndexBuffer;
	ID3D10InputLayout * m_pTempVertexLayout;
	ID3D10ShaderResourceView * m_pBaseTextureMap;
	ID3D10EffectShaderResourceVariable * m_pBaseTextureEffectVariable;
	
	//XNA mathematics matrices
	XMMATRIX m_View;
	XMMATRIX m_Projection;
	XMMATRIX m_World;

	//Matrix Variables
	ID3D10EffectMatrixVariable * m_pViewEffectVariable;
	ID3D10EffectMatrixVariable * m_pProjectionEffectVariable;
	ID3D10EffectMatrixVariable * m_pWorldEffectVariable;

	//Colours
	XMFLOAT4 m_AmbientMatColour;
	XMFLOAT4 m_AmbientLightColour;

	//Colour variables
	ID3D10EffectVectorVariable * m_pAmbientMatColourVariable;
	ID3D10EffectVectorVariable * m_pAmbientLightColourVariable;
};