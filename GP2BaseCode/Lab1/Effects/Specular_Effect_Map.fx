//Specular effect + map texture

float4x4 matWorld:WORLD<string UIWidget = "None";>;
float4x4 matView:VIEW<string UIWidget = "None";>;
float4x4 matProjection:PROJECTION<string UIWidget = "None";>;

Texture2D specMapTexture;

float4 ambientMaterial
<
	string UIName="Ambient Material";
	string UIWidget="Color";
>;

float4 ambientLightColour:COLOR
<
	string UIName="Ambient Light Colour";
	string UIWidget="Color";
	string Object="DirectionalLight";
>;

float4 diffuseLightColour:DIFFUSE
<
	string UIName="Diffuse Light Colour";
	string UIWidget="None";
	string Object = "DirectionalLight";
>;

float3 lightDirection:DIRECTION
<
	string Object="DirectionalLight";
>;

float4 specLightColour:COLOR
<
	string UIName="Specular Light Colour";
	string UIWidget="Color";
	string Object="DirectionalLight";
>;

float4 cameraPosition:POSITION
<
	string Object = "Perspective";
>;

float specPower
<
	string UIName="Specular Power";
	string UIWidget="Slider";
	float UIMin = 1.0;
	float UIMax = 30.0;
	float UIStep = 0.01;
	> = 25;

struct VS_INPUT
{
	float4 position:POSITION;
	float3 normal:NORMAL;
	float2 texCoord:TEXCOORD0;
};

struct PS_INPUT
{
	float4 cameraDirection:VIEWDIR;
	float4 position:SV_POSITION;
	float3 normal:NORMAL;
	float2 texCoord:TEXCOORD0;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output=(PS_INPUT)0;
	
	float4x4 matViewProjection=mul(matView,matProjection);
	float4x4 matWorldViewProjection=mul(matWorld,matViewProjection);
	
	output.position=mul(input.position,matWorldViewProjection);
	output.normal=mul(input.normal,matWorld);
	float4 worldPos=mul(input.position,matWorld);
	output.cameraDirection=normalize(cameraPosition-worldPos);
	output.texCoord = input.texCoord;
	
	return output;
}

SamplerState WrapPointSampler
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};

float4 PS(PS_INPUT input):SV_TARGET
{
	float3 normal = normalize(input.normal);
	float3 lightDir = normalize(lightDirection);
	float diffuseHighlight = saturate(dot(normal,lightDir));
	
	float3 halfVec = normalize(lightDir+input.cameraDirection);
	
	float specular = pow(saturate(dot(normal,halfVec)),specPower);
	
	float lMap = specMapTexture.Sample(WrapPointSampler,input.texCoord);
	
	return ((ambientMaterial*ambientLightColour) 
	+(lMap*diffuseLightColour*diffuseHighlight)
	+(lMap*specLightColour*specular*lMap)
	);
}

RasterizerState DisableCulling
{
    CullMode = NONE;
};

technique10 Render
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader(CompileShader(ps_4_0, PS() ) );
		SetRasterizerState(DisableCulling); 
	}
}