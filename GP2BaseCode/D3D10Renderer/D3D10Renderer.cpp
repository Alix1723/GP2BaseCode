#include "D3D10Renderer.h"

//#include <D3D10.h>
//#include <D3DX10.h>
struct Vertex {
	//Position co-ordinates
	float x;
	float y;
	float z;

	//Texture co-ordinates
	float nu;
	float nv;
	float nw;
};

const D3D10_INPUT_ELEMENT_DESC VertexLayout[] = 
{
	{"POSITION",
	0,
	DXGI_FORMAT_R32G32B32_FLOAT,
	0,
	0,
	D3D10_INPUT_PER_VERTEX_DATA,
	0},
	
	{"NORMAL",
	0,
	DXGI_FORMAT_R32G32B32_FLOAT, //Important!0
	0,
	12,	//Also important!
	D3D10_INPUT_PER_VERTEX_DATA,
	0}
};

//Constructor for D3D10Renderer
D3D10Renderer::D3D10Renderer()
{	
	//Initializes all required objects
	m_pD3D10Device=NULL;
	m_pRenderTargetView=NULL;
	m_pSwapChain=NULL;
	m_pDepthStencelView=NULL;
	m_pDepthStencilTexture=NULL;

	m_pTempEffect = NULL;
	m_pTempTechnique = NULL;
	m_pTempBuffer = NULL;
	m_pTempVertexLayout = NULL;

	//Initialize matrices
	m_View = XMMatrixIdentity();
	m_Projection = XMMatrixIdentity();
	m_World = XMMatrixIdentity();
}

//Deconstructor
D3D10Renderer::~D3D10Renderer()
{
	//If an object exists, clear it up/release it
	if (m_pD3D10Device)
		m_pD3D10Device->ClearState();
	if (m_pRenderTargetView)
		m_pRenderTargetView->Release();
	if (m_pDepthStencelView)
		m_pDepthStencelView->Release();
	if (m_pDepthStencilTexture)
		m_pDepthStencilTexture->Release();
	if (m_pSwapChain)
		m_pSwapChain->Release();
	if (m_pD3D10Device)
		m_pD3D10Device->Release();
	if (m_pTempEffect)
		m_pTempEffect->Release();
	if (m_pTempVertexLayout)
		m_pTempVertexLayout->Release();	
	if (m_pBaseTextureMap)
		m_pBaseTextureMap->Release();
	if (m_pLightTextureMap)
		m_pLightTextureMap->Release();
	if(m_pTempIndexBuffer)
		m_pTempIndexBuffer->Release();
}

//Initialize the window 
bool D3D10Renderer::init(void *pWindowHandle,bool fullScreen)
{
	HWND window=(HWND)pWindowHandle;			//Window Handle, the object referring to the current window
	RECT windowRect;							//Window dimensions (X/Y)
	GetClientRect(window,&windowRect);			//Relative window position for the client
	UINT width=windowRect.right-windowRect.left;		//Window width and height
	UINT height=windowRect.bottom-windowRect.top;

		//Create and initiate functions/objects
	if (!createDevice(window,width,height,fullScreen))
		return false;
	if (!createInitialRenderTarget(width,height))
		return false;
	if (!createBuffer())
		return false;
	if(!loadEffectFromFile("Effects/Specular_Effect_Map.fx"))
		return false;
	if(!createVertexLayout())
		return false;
	if(!loadTextures("Textures/Base_Texture.png","Textures/Lightmap_Texture_A.png"))
		return false;

	//Creating a view camera
	XMFLOAT3 cameraPos = XMFLOAT3(0.0f,0.0f,-10.0f);
	XMFLOAT3 focusPos = XMFLOAT3(0.0f,0.0f,0.0f);
	XMFLOAT3 up = XMFLOAT3(0.0f,1.0f,0.0f);

	createCamera(XMLoadFloat3(&cameraPos),
		XMLoadFloat3(&focusPos),
		XMLoadFloat3(&up),
		XM_PI/4,
		(float)width/(float)height,
		0.1f,
		100.0f);

	//Ambient
	m_AmbientMatColour = XMFLOAT4(0.5f,0.5f,0.5f,1.0f);
	m_AmbientLightColour = XMFLOAT4(0.5f,0.5f,0.5f,1.0f);
	//Diffuse
	m_DiffuseMatColour = XMFLOAT4(1.0f,1.0f,1.0f,1.0f);
	m_DiffuseLightColour = XMFLOAT4(1.0f,1.0f,1.0f,1.0f);
	m_DiffuseLightDirection = XMFLOAT3(1.0f,-1.0f,-1.0f);
	//Specular
	m_SpecularMatColour = XMFLOAT4(1.0f,1.0f,1.0f,1.0f);
	m_SpecularLightColour = XMFLOAT4(1.0f,1.0f,1.0f,1.0f);
	m_SpecularCameraPosition = cameraPos;
	m_SpecularPower = 3;
	
	//Moving the object
	positionObject(0.0f,10.0f,0.0f);
	rotateObject(70.0f,40.0f,0.0f);
	return true;
}

//Create the D3D10 Device object, taking the window handle, the width and height and wether the program is fullscreen
bool D3D10Renderer::createDevice(HWND window,int windowWidth, int windowHeight,bool fullScreen)
{
	UINT createDeviceFlags=0;		//Extra options for creating the device

#ifdef _DEBUG
	createDeviceFlags|=D3D10_CREATE_DEVICE_DEBUG;		//Print out extra information in debugmode
#endif

	DXGI_SWAP_CHAIN_DESC sd;							//Object with description of the swap chain
       ZeroMemory( &sd, sizeof( sd ) );					//Clears an area of memory the size of the swap chain, filling it with 0s
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//What to use the buffer/swap chain as: In this case set it to output the rendered surface
	if (fullScreen)
		sd.BufferCount = 2;								//How many surfaces/buffers to use: Two when fullscreen, one when windowed
	else 
		sd.BufferCount=1;
	sd.OutputWindow = window;							//Set the render output to the current window
	sd.Windowed = (BOOL)(!fullScreen);						//Check if the program's fullscreen
       sd.SampleDesc.Count = 1;								//How many samples to take per pixel
       sd.SampleDesc.Quality = 0;							//Quality of sampling, from 0 to ID3D10Device::CheckMultiSampleQualityLevels - 1
       sd.BufferDesc.Width = windowWidth;					//Width and height of the window
       sd.BufferDesc.Height = windowHeight;
       sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//An enumerated type describing the colour depth and other rendering specifics
       sd.BufferDesc.RefreshRate.Numerator = 60;			//Image update rate, in this case 60FPS = 16.6666... ms per frame
       sd.BufferDesc.RefreshRate.Denominator = 1;

	if (FAILED(D3D10CreateDeviceAndSwapChain(NULL,			//Create the device and the swap chain.
		D3D10_DRIVER_TYPE_HARDWARE,
		NULL, 
		createDeviceFlags,
		D3D10_SDK_VERSION,		
              &sd,
		&m_pSwapChain, 
		&m_pD3D10Device)))                       
		return false;

	return true;
}

//Creates the initial buffer/swap chain/render surface
bool D3D10Renderer::createInitialRenderTarget(int windowWidth, int windowHeight)
{
	ID3D10Texture2D *pBackBuffer;				//Reference to the 2D surface we're drawing everything to
	
	if (FAILED(m_pSwapChain->GetBuffer(0,		
		__uuidof(ID3D10Texture2D),
		(void**)&pBackBuffer))) 
		return false;

	D3D10_TEXTURE2D_DESC descDepth;				//Object describing the 2D surface's properties
	descDepth.Width=windowWidth;				//Width and height of the window, and of the surface
	descDepth.Height=windowHeight;
	descDepth.MipLevels=1;						//Mip-mapping levels, for texture filtering purposes
	descDepth.ArraySize=1;						//How many textures to load
	descDepth.Format=DXGI_FORMAT_D32_FLOAT;		//What image format the texture or textures are in
	descDepth.SampleDesc.Count=1;				//How many samples to take per pixel during the pixel shader stage
	descDepth.SampleDesc.Quality=0;				//Quality of sampling/filtering
	descDepth.Usage=D3D10_USAGE_DEFAULT;		//Determines read/write access for the texture
	descDepth.BindFlags=D3D10_BIND_DEPTH_STENCIL;	//Flag or flags used to bind this buffer to pipeline stages and buffers
	descDepth.CPUAccessFlags=0;					//Determines CPU access to the surface, if necessary. 0 means no access required.
	descDepth.MiscFlags=0;						//Miscellaneous flags for special cases such as cubemap textures. 0 means no extra flags apply.

	if (FAILED(m_pD3D10Device->CreateTexture2D(&descDepth,NULL,	//Create the 2D texture for our render surface
			&m_pDepthStencilTexture)))
		return false;

	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;					//Describing the properties of the depth stencil view
	descDSV.Format=descDepth.Format;						//Match the format of the previous texture
	descDSV.ViewDimension=D3D10_DSV_DIMENSION_TEXTURE2D;	//Determines how a depth-stencil should be accessed
	descDSV.Texture2D.MipSlice=0;							//The first mip-map level to use

	if (FAILED(m_pD3D10Device->CreateDepthStencilView(m_pDepthStencilTexture,	//Create the depth stencil resource
                   &descDSV,&m_pDepthStencelView)))
		return false;

	if (FAILED(m_pD3D10Device->CreateRenderTargetView( pBackBuffer,		//Create the render target
		NULL, 
		&m_pRenderTargetView ))){
             pBackBuffer->Release();
		return  false;
	}
       pBackBuffer->Release();	//Release the buffer resource

	m_pD3D10Device->OMSetRenderTargets(1,	//Set the device's render target
		&m_pRenderTargetView,		
		m_pDepthStencelView);
	
	D3D10_VIEWPORT vp;					//Specifies a viewport with width, height, X and Y origins and depth information.
   	vp.Width = windowWidth;
    vp.Height = windowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    
	m_pD3D10Device->RSSetViewports( 1 
		, &vp );	//Sets the device's viewport to our vp structure
	return true;
}

//Clear the renderer
void D3D10Renderer::clear(float r,float g,float b,float a)
{
    // Just clear the backbuffer, colours start at 0.0 to 1.0
	// Red, Green , Blue, Alpha - BMD
    const float ClearColor[4] = { r, g, b, a}; 
	//Clear the Render Target
	//http://msdn.microsoft.com/en-us/library/bb173539%28v=vs.85%29.aspx - BMD
    m_pD3D10Device->ClearRenderTargetView( m_pRenderTargetView, ClearColor );
	m_pD3D10Device->ClearDepthStencilView(m_pDepthStencelView,D3D10_CLEAR_DEPTH,1.0f,0);	
}

//Present the result of a rendered frame
void D3D10Renderer::present()
{
	//Swaps the buffers in the chain, the back buffer to the front(screen)
	//http://msdn.microsoft.com/en-us/library/bb174576%28v=vs.85%29.aspx - BMD
    m_pSwapChain->Present( 0, 0 );
}

//Render a frame from the given information in the device
void D3D10Renderer::render()
{
	//Send matrices
	m_pViewEffectVariable->SetMatrix((float*)&m_View);
	m_pProjectionEffectVariable->SetMatrix((float*)&m_Projection);
	m_pWorldEffectVariable->SetMatrix((float*)&m_World);
	//Send texture
	//m_pBaseTextureEffectVariable->SetResource(m_pBaseTextureMap);
	//Colours
	//AMBIENT
	m_pAmbientMatColourVariable->SetFloatVector((float*)&m_AmbientMatColour);
	m_pAmbientLightColourVariable->SetFloatVector((float*)&m_AmbientLightColour);
	//DIFFUSE
	m_pDiffuseMatColourVariable->SetFloatVector((float*)&m_DiffuseMatColour);
	m_pDiffuseLightColourVariable->SetFloatVector((float*)&m_DiffuseLightColour);
	m_pDiffuseLightDirectionVariable->SetFloatVector((float*)&m_DiffuseLightDirection);
	//SPECULAR
	m_pSpecularMatColourVariable->SetFloatVector((float*)&m_SpecularMatColour);
	m_pSpecularLightColourVariable->SetFloatVector((float*)&m_SpecularLightColour);
	m_pSpecularCameraPositionVariable->SetFloatVector((float*)&m_SpecularCameraPosition);
	m_pSpecularPowerVariable->SetFloat(m_SpecularPower);

	m_pD3D10Device->IASetPrimitiveTopology(
		D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );				//What kind of information we're giving the renderer, E.G. LINELIST, TRIANGLELIST, POINTLIST
	m_pD3D10Device->IASetInputLayout(m_pTempVertexLayout);		//Passes our temporary vertex layout information into the device, binding it to the input assembler

	UINT stride = sizeof( Vertex );
	UINT offset = 0;

	m_pD3D10Device->IASetVertexBuffers(						//Binds an array of vertex buffers to the input assembler
		0,
		1,
		&m_pTempBuffer,
		&stride,
		&offset);

	m_pD3D10Device->IASetIndexBuffer(						//Binds an array of indices
		m_pTempIndexBuffer,
		DXGI_FORMAT_R32_UINT,
		0);

	D3D10_TECHNIQUE_DESC techniqueDesc;						//Description of a shader 'technique'
	m_pTempTechnique->GetDesc(&techniqueDesc);				//Sets the Temp technique description to &techniqueDesc

	for(unsigned int i = 0; i < techniqueDesc.Passes;++i)			//For each pass in this technique:
	{
		ID3D10EffectPass *pCurrentPass = m_pTempTechnique->GetPassByIndex(i);		//Retrieve the pass information from the technique
		pCurrentPass->Apply(0);														//Apply it
		m_pD3D10Device->DrawIndexed(36,0,0);													//And draw it to the device's surface
	}
}

//Load a shader effect from an external file
bool D3D10Renderer::loadEffectFromFile(const char* pFilename)
{	
	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;			//Do not allow legacy/old syntax in the shader compilation
	#if defined( DEBUG ) || defined( _DEBUG )
		dwShaderFlags |= D3D10_SHADER_DEBUG;					//If in debugmode, output verbose debugging information
	#endif
		ID3D10Blob * pErrorBuffer = NULL;					//Returns errors with arbitrary size/length
		
		if(FAILED(D3DX10CreateEffectFromFileA(pFilename,
			NULL,
			NULL,
			"fx_4_0",
			dwShaderFlags,
			0,
			m_pD3D10Device,
			NULL,
			NULL,
			&m_pTempEffect,
			&pErrorBuffer,NULL)))
		{
			OutputDebugStringA((char*)pErrorBuffer->GetBufferPointer());
			return false;
		}
		//Matrices
		m_pViewEffectVariable = m_pTempEffect->GetVariableByName("matView")->AsMatrix();
		m_pProjectionEffectVariable = m_pTempEffect->GetVariableByName("matProjection")->AsMatrix();
		m_pWorldEffectVariable = m_pTempEffect->GetVariableByName("matWorld")->AsMatrix();
		
		//Technique
		//Amb. colours
		m_pAmbientMatColourVariable = m_pTempEffect->GetVariableByName("ambientMaterial")->AsVector();
		m_pAmbientLightColourVariable = m_pTempEffect->GetVariableByName("ambientLightColour")->AsVector();
		//Diff. colours and direction
		m_pDiffuseMatColourVariable = m_pTempEffect->GetVariableByName("diffuseMaterial")->AsVector();
		m_pDiffuseLightColourVariable = m_pTempEffect->GetVariableByName("diffuseLightColour")->AsVector();
		m_pDiffuseLightDirectionVariable = m_pTempEffect->GetVariableByName("lightDirection")->AsVector();
		//Specular colours, camera pos and power scalar
		m_pSpecularMatColourVariable = m_pTempEffect->GetVariableByName("specMaterial")->AsVector();
		m_pSpecularLightColourVariable = m_pTempEffect->GetVariableByName("specLightColour")->AsVector();
		m_pSpecularCameraPositionVariable = m_pTempEffect->GetVariableByName("cameraPosition")->AsVector();
		m_pSpecularPowerVariable = m_pTempEffect->GetVariableByName("specPower")->AsScalar();
		//Textures
		m_pBaseMapTextureVariable = m_pTempEffect->GetVariableByName("baseMapTexture")->AsShaderResource();
		m_pLightMapTextureVariable = m_pTempEffect->GetVariableByName("lightMapTexture")->AsShaderResource();

		m_pTempTechnique = m_pTempEffect->GetTechniqueByName("Render");	//Set the temporary technique to the "Render" effect within m_pTempEffect
		return true;
}

//Create the draw buffer
bool D3D10Renderer::createBuffer()
{
	//What vertices to give the renderer to draw
	Vertex verts[] = {
		//Front
		{-1.0f,-1.0f,1.0f,	0.0f,0.5f,0.5f}, 
		{-1.0f,1.0f,1.0f,	0.0f,0.5f,0.5f}, 
		{1.0f,-1.0f,1.0f,	0.0f,-0.5f,0.5f}, 
		{1.0f,1.0f,1.0f,	0.0f,-0.5f,0.5f}, 
		//Back
		{-1.0f,-1.0f,-1.0f,	0.0f,0.5f,-0.5f}, 
		{-1.0f,1.0f,-1.0f,	0.0f,0.5f,-0.5f}, 
		{1.0f,-1.0f,-1.0f,	0.0f,-0.5f,-0.5f}, 
		{1.0f,1.0f,-1.0f,	0.0f,-0.5f,-0.5f} 
	};

	//Vertex buffer
	D3D10_BUFFER_DESC bd;					//Object holding information about the buffer
	bd.Usage = D3D10_USAGE_DEFAULT;			//Access and usage permissions
	bd.ByteWidth = sizeof( Vertex ) * 8;		//Size of the buffer, in bytes
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;	//How the buffer will be bound to the pipeline, in this case to the vertex shader stage
	bd.CPUAccessFlags = 0;					//Access permission for the CPU. 0 means no access required.
	bd.MiscFlags = 0;						//Extra operations required

	D3D10_SUBRESOURCE_DATA InitVxData;		//Initialize a subresource
	InitVxData.pSysMem = &verts;				//Data to take into the resource

	if(FAILED(m_pD3D10Device->CreateBuffer(			//Create the buffer on the device
		&bd,
		&InitVxData,
		&m_pTempBuffer)))
	{
		OutputDebugStringA("Can't create Vertex buffer!");
		return false;
	}
	
	//Index buffer
	int indices[36] = {
		//Front face
		0,1,2,
		1,3,2,
		//Right face
		2,3,6,
		3,7,6,
		//Back face
		6,7,5,
		5,4,6,
		//Left face
		4,5,1,
		1,0,4,
		//Top face
		3,7,5,
		5,1,3,
		//Bottom face
		2,6,4,
		4,0,2
	};

	D3D10_BUFFER_DESC indexBD;
	indexBD.Usage = D3D10_USAGE_DEFAULT;
	indexBD.ByteWidth = sizeof(int)*36;
	indexBD.BindFlags = D3D10_BIND_INDEX_BUFFER;
	indexBD.CPUAccessFlags = 0;
	indexBD.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA InitIBData;
	InitIBData.pSysMem = &indices;

	if(FAILED(m_pD3D10Device->CreateBuffer(			//Create the buffer on the device
		&indexBD,
		&InitIBData,
		&m_pTempIndexBuffer)))
	{
		OutputDebugStringA("Can't create Index buffer!");
		return false;}

	return true;
	
}

//Creates the vertex layout
bool D3D10Renderer::createVertexLayout()
{
	UINT numElements = sizeof( VertexLayout ) / sizeof(D3D10_INPUT_ELEMENT_DESC);		//How many vertices in the layout
	D3D10_PASS_DESC PassDesc;										//Object holding info about the current render pass
	m_pTempTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );	//Retrieve the first pass's info

	if(FAILED(m_pD3D10Device->CreateInputLayout( VertexLayout,		//Create the layout on the device
		numElements,
		PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize,
		&m_pTempVertexLayout )))
	{
		OutputDebugStringA("Can't create layout!");
		return false;
	}
	return true;
}

//Creates a view camera
void D3D10Renderer::createCamera(XMVECTOR &position, XMVECTOR &focus, XMVECTOR &up, float fov, float aspectRatio, float nearClip, float farClip)
{
	m_View = XMMatrixLookAtLH(position,focus,up);
	m_Projection = XMMatrixPerspectiveFovLH(fov,aspectRatio,nearClip,farClip);
}

//Repositions the current object
void D3D10Renderer::positionObject(float x, float y, float z)
{
	m_World = XMMatrixTranslation(x,y,z);
}

//Rotates the current object
void D3D10Renderer::rotateObject(float p, float y, float r)
{
	m_World = XMMatrixRotationRollPitchYaw(p,y,r);
}

//Loads a texture (view) from a file
bool D3D10Renderer::loadTextures(char * pBaseFilename, char* pLightFilename)
{
	if(FAILED(D3DX10CreateShaderResourceViewFromFileA(
		m_pD3D10Device,
		pBaseFilename,
		NULL,
		NULL,
		&m_pBaseTextureMap,
		NULL)))
		{
			return false;
		}

	if(FAILED(D3DX10CreateShaderResourceViewFromFileA(
		m_pD3D10Device,
		pLightFilename,
		NULL,
		NULL,
		&m_pLightTextureMap,
		NULL)))
		{
			return false;
		}

	return true;
}
