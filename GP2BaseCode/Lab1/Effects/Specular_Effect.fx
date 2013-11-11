//Specular effect 
float4x4 matWorld:WORLD<string UIWidget = "None";>;
float4x4 matView:VIEW<string UIWidget = "None";>;
float4x4 matProjection:PROJECTION<string UIWidget = "None";>;

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

float4 diffuseMaterial
<
	string UIName="Diffuse Material";
	string UIWidget="Color";
	
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

float4 specMaterial
<
	string UIName="Specular Material";
	string UIWidget="Color";
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
};

struct PS_INPUT
{
	float4 cameraDirection:VIEWDIR;
	float4 position:SV_POSITION;
	float3 normal:NORMAL;
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
	
	return output;
}

float4 PS(PS_INPUT input):SV_TARGET
{
	float3 normal = normalize(input.normal);
	float3 lightDir = normalize(lightDirection);
	float diffuseHighlight = saturate(dot(normal,lightDir));
	
	float3 halfVec = normalize(lightDir+input.cameraDirection);
	
	float specular = pow(saturate(dot(normal,halfVec)),specPower);
	
	return ((ambientMaterial*ambientLightColour) 
	+(diffuseMaterial*diffuseLightColour*diffuseHighlight)
	+(specMaterial*specLightColour*specular)
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