/*********************************************************************NVMH3****
$Revision$

Copyright NVIDIA Corporation 2007
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY
LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF
NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


To learn more about shading, shaders, and to bounce ideas off other shader
    authors and users, visit the NVIDIA Shader Library Forums at:

    http://developer.nvidia.com/forums/

******************************************************************************/
string ParamID = "0x003";

float Script : STANDARDSGLOBAL <
    string UIWidget = "none";
    string ScriptClass = "object";
    string ScriptOrder = "standard";
    string ScriptOutput = "color";
    string Script = "Technique=Main;";
> = 0.8;

//// UN-TWEAKABLES - AUTOMATICALLY-TRACKED TRANSFORMS ////////////////

float4x4 WorldITXf : WorldInverseTranspose < string UIWidget="None"; >;
float4x4 WvpXf : WorldViewProjection < string UIWidget="None"; >;
float4x4 WorldXf : World < string UIWidget="None"; >;
float4x4 ViewIXf : ViewInverse < string UIWidget="None"; >;

#ifdef _MAX_
int texcoord1 : Texcoord
<
	int Texcoord = 1;
	int MapChannel = 0;
	string UIWidget = "None";
>;

int texcoord2 : Texcoord
<
	int Texcoord = 2;
	int MapChannel = -2;
	string UIWidget = "None";
>;

int texcoord3 : Texcoord
<
	int Texcoord = 3;
	int MapChannel = -1;
	string UIWidget = "None";
>;
#endif

//// TWEAKABLE PARAMETERS ////////////////////

/// Point Lamp 0 ////////////
float3 Lamp0Pos : POSITION <
    string Object = "PointLight0";
    string UIName =  "Light Position";
    string Space = "World";
	int refID = 0;
> = {-0.5f,2.0f,1.25f};


float3 Lamp0Color : COLOR <
    string UIName =  "Light Color";
    string Object = "Pointlight0";
    string UIWidget = "Color";
> = {1.0f,1.0f,1.0f};

float4 k_s  <
	string UIName = "Specular";
	string UIWidget = "Color";
> = float4( 1.0f, 1.0f, 1.0f, 1.0f );    // diffuse    // specular

float g_roughness <
	string UIName = "roughness";
	string UIWidget = "slider";
	float UIMin = 0.0f;
	float UIMax = 1.0f;
	float UIStep = 0.1f;
>   = 0.5f;
									
float g_metallic <
	string UIName = "metallic";
	string UIWidget = "slider";
	float UIMin = 0.0f;
	float UIMax = 1.0f;
	float UIStep = 0.1f;
>   = 0.5f;

TextureCube g_ReflectionTexture < 
	string UIName = "Reflection";
	string ResourceType = "CUBE";
>;

Texture2D <float4> g_BaseTexture : DiffuseMap< 
	string UIName = "Base";
	string ResourceType = "2D";
	int Texcoord = 0;
	int MapChannel = 1;
>;

Texture2D <float4> g_NormalTexture : NormalMap< 
	string UIName = "Normal";
	string ResourceType = "2D";
	int Texcoord = 0;
	int MapChannel = 1;
>;

Texture2D <float4> g_PreIntegratedGFTexture : PreIntegratedGFTexture< 
	string UIName = "PreIntegratedGF";
	string ResourceType = "2D";
	int Texcoord = 0;
	int MapChannel = 1;
>;

SamplerState g_BaseSampler
{
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU = Wrap;
    	AddressV = Wrap;
};

SamplerState g_NormalSampler
{
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU = Wrap;
    	AddressV = Wrap;
};

SamplerState g_PreIntegratedGFSampler
{
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU = Wrap;
    	AddressV = Wrap;
};

SamplerState g_ReflectionSampler
{
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU = Clamp;
        AddressV = Clamp;	
	AddressW = Clamp;	
};

/* data from application vertex buffer */
struct appdata {
	float4 Position		: POSITION;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Binormal		: BINORMAL;
	float2 UV0		: TEXCOORD0;	
	float3 Colour		: TEXCOORD1;
	float3 Alpha		: TEXCOORD2;
	float3 Illum		: TEXCOORD3;
	float3 UV1		: TEXCOORD4;
	float3 UV2		: TEXCOORD5;
	float3 UV3		: TEXCOORD6;
	float3 UV4		: TEXCOORD7;
};

/* data passed from vertex shader to pixel shader */
struct vertexOutput {
    float4 HPosition	: SV_Position;
    float4 UV0		: TEXCOORD0;
    // The following values are passed in "World" coordinates since
    //   it tends to be the most flexible and easy for handling
    //   reflections, sky lighting, and other "global" effects.
    float3 LightVec	: TEXCOORD1;
    float3 WorldNormal	: TEXCOORD2;
    float3 WorldTangent	: TEXCOORD3;
    float3 WorldBinormal : TEXCOORD4;
    float3 WorldView	: TEXCOORD5;
	float4 UV1		: TEXCOORD6;
	float4 UV2		: TEXCOORD7;
	float4 wPos		: TEXCOORD8;
};
 
///////// VERTEX SHADING /////////////////////

/*********** Generic Vertex Shader ******/

vertexOutput std_VS(appdata IN) {
    vertexOutput OUT = (vertexOutput)0;
    OUT.WorldNormal = mul(IN.Normal,WorldITXf).xyz;
    OUT.WorldTangent = mul(IN.Tangent,WorldITXf).xyz;
    OUT.WorldBinormal = mul(IN.Binormal,WorldITXf).xyz;
    float4 Po = float4(IN.Position.xyz,1);
    float3 Pw = mul(Po,WorldXf).xyz;
    OUT.LightVec = (Lamp0Pos - Pw);
    OUT.WorldView = normalize(ViewIXf[3].xyz - Pw);
    OUT.HPosition = mul(Po,WvpXf);
	OUT.wPos = mul(IN.Position, WorldXf);
	
// UV bindings
// Encode the color data
 	float4 colour;
   	colour.rgb = IN.Colour * IN.Illum;
   	colour.a = IN.Alpha.x;
   	OUT.UV0.z = colour.r;
   	OUT.UV0.a = colour.g;
  	OUT.UV1.z = colour.b;
   	OUT.UV1.a = colour.a;

// Pass through the UVs
	OUT.UV0.xy = IN.UV0.xy;
   	OUT.UV1.xy = IN.UV1.xy;
   	OUT.UV2.xyz = IN.UV2.xyz;
// 	OUT.UV3 = OUT.UV3;
// 	OUT.UV4 = OUT.UV4;
    return OUT;
}

///////// PIXEL SHADING //////////////////////

// Utility function for phong shading

void phong_shading(vertexOutput IN,
		    float3 LightColor,
		    float3 Nn,
		    float3 Ln,
		    float3 Vn,
		    out float3 DiffuseContrib,
		    out float3 SpecularContrib)
{
}

#define PIE 3.1415926535
#define UNITY_PI 3.1415926535
#define EPSILON 0.00000001

float chiGGX(float v)
{
    return v > 0 ? 1 : 0;
}

float GGX_Distribution(float3 n, float3 h, float alpha)
{
    float NoH = dot(n,h);
    float alpha2 = alpha * alpha;
    float NoH2 = NoH * NoH;
    float den = NoH2 * alpha2 + (1 - NoH2);
    return (chiGGX(NoH) * alpha2) / ( PIE * den * den );
}

float GGX_PartialGeometryTerm(float3 v, float3 n, float3 h, float alpha)
{
    float VoH2 = saturate(dot(v,h));
    float chi = chiGGX( VoH2 / saturate(dot(v,n)) );
    VoH2 = VoH2 * VoH2;
    float tan2 = ( 1 - VoH2 ) / VoH2;
    return (chi * 2) / ( 1 + sqrt( 1 + alpha * alpha * tan2 ) );
}

float4 PBR_XXX_PS_PL(vertexOutput IN) : SV_Target {

    float roughness = g_roughness;
    float metallic = g_metallic;

    float3 LightColor = float3(1.0f,1.0f,1.0f);
    float3 SpecularColor = k_s.xyz;

    float3 L = normalize(IN.LightVec);
    float3 V = normalize(IN.WorldView);
    float3 N = normalize(IN.WorldNormal);
    float3 H = normalize(L + V);

    float NoV = saturate( dot( N, V ) );
    float NoL = saturate( dot( N, L ) );
    float NoH = saturate( dot( N, H ) );
    float VoH = saturate( dot( V, H ) );

    //float G = G_Smith( roughness , NoV, NoL );
    float G = GGX_PartialGeometryTerm(V, N, H, roughness) * GGX_PartialGeometryTerm(L, N, H, roughness);
    //float Fc = pow( 1 - VoH, 5 );
    //float3 F = (1 - Fc) * SpecularColor + Fc;
    float Fc = pow((1 - saturate(dot(H, V))	), 5);
    float3 F = SpecularColor + (1 - SpecularColor)*Fc;
    float D = saturate( 4 * (NoV * saturate(dot(H, N)) + 0.05) );
    // Incident light = SampleColor * NoL
    // Microfacet specular = D*G*F / (4*NoL*NoV)
    // pdf = D * NoH / (4 * VoH)
    float3 radiance = LightColor * D*G*F / (4*NoL*NoV);
    //float3 radiance = LightColor * F * G * VoH / (NoH * NoV);

    float3 kd = (1 - F) * (1 - metallic);

    return float4(radiance,1.0f);
    //return float4(kd * NoL * LightColor + radiance,1.0f);

    float3 result = kd * NoL * LightColor + radiance;
    return float4(pow(result,2.2f),1.0f);

}

inline half DotClamped (half3 a, half3 b)
{
	//#if (SHADER_TARGET < 30 || defined(SHADER_API_PS3))
	//	return saturate(dot(a, b));
	//#else
		return max(0.0h, dot(a, b));
	//#endif
}

inline half BlinnTerm (half3 normal, half3 halfDir)
{
	return DotClamped (normal, halfDir);
}


inline half SmithJointGGXVisibilityTerm (half NdotL, half NdotV, half roughness)
{
#if 0
	// Original formulation:
	//	lambda_v	= (-1 + sqrt(a2 * (1 - NdotL2) / NdotL2 + 1)) * 0.5f;
	//	lambda_l	= (-1 + sqrt(a2 * (1 - NdotV2) / NdotV2 + 1)) * 0.5f;
	//	G			= 1 / (1 + lambda_v + lambda_l);

	// Reorder code to be more optimal
	half a		= roughness * roughness; // from unity roughness to true roughness
	half a2		= a * a;

	half lambdaV = NdotL * sqrt((-NdotV * a2 + NdotV) * NdotV + a2);
	half lambdaL = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);

	// Unity BRDF code expect already simplified data by (NdotL * NdotV)
	// return (2.0f * NdotL * NdotV) / (lambda_v + lambda_l + 1e-5f);
	return 2.0f / (lambdaV + lambdaL + 1e-5f);
#else
    // Approximation of the above formulation (simplify the sqrt, not mathematically correct but close enough)
	half a = roughness * roughness;
	half lambdaV = NdotL * (NdotV * (1 - a) + a);
	half lambdaL = NdotV * (NdotL * (1 - a) + a);
	return 2.0f / (lambdaV + lambdaL + 1e-5f);	// This function is not intended to be running on Mobile,
												// therefore epsilon is smaller than can be represented by half
#endif
}

inline half GGXTerm (half NdotH, half roughness)
{
	half a = roughness * roughness;
	half a2 = a * a;
	half d = NdotH * NdotH * (a2 - 1.f) + 1.f;
	return a2 / (UNITY_PI * d * d + 1e-7f); // This function is not intended to be running on Mobile,
											// therefore epsilon is smaller than what can be represented by half
}

inline bool IsGammaSpace()
{
	return false;
}

inline half3 Unity_SafeNormalize(half3 inVec)
{
	half dp3 = max(0.001f, dot(inVec, inVec));
	return inVec * rsqrt(dp3);
}


inline half3 Pow5 (half3 x)
{
	return x*x * x*x * x;
}
inline half3 Pow4 (half3 x)
{
	return x*x*x*x;
}

inline half3 FresnelTerm (half3 F0, half cosA)
{
	half t = Pow5 (1 - cosA);	// ala Schlick interpoliation
	return F0 + (1-F0) * t;
}
inline half3 FresnelLerp (half3 F0, half3 F90, half cosA)
{
	half t = Pow5 (1 - cosA);	// ala Schlick interpoliation
	return lerp (F0, F90, t);
}
// approximage Schlick with ^4 instead of ^5
inline half3 FresnelLerpFast (half3 F0, half3 F90, half cosA)
{
	half t = Pow4 (1 - cosA);
	return lerp (F0, F90, t);
}

//#define GAMMA_IN	(2.2f)
//#define GAMMA_OUT	(1.0f/2.2f)
#define GAMMA_IN	(1.0f)
#define GAMMA_OUT	(1.0f)


half4 PBR_Unity_PL(half3 diffColor, half3 specColor, half oneMinusReflectivity, half oneMinusRoughness,
	half3 normal, half3 viewDir,
	half3 lightdir,half3 lightcolor)
{
	half roughness = 1-oneMinusRoughness;
	half3 halfDir = Unity_SafeNormalize (lightdir + viewDir);

//#if UNITY_BRDF_GGX 
	// NdotV should not be negative for visible pixels, but it can happen due to perspective projection and normal mapping
	// In this case we will modify the normal so it become valid and not cause weird artifact (other game try to clamp or abs the NdotV to prevent this trouble).
	// The amount we shift the normal toward the view vector is define by the dot product.
	// This correction is only apply with smithJoint visibility function because artifact are more visible in this case due to highlight edge of rough surface
	half shiftAmount = dot(normal, viewDir);
	normal = shiftAmount < 0.0f ? normal + viewDir * (-shiftAmount + EPSILON) : normal;
	// A re-normalization should be apply here but as the shift is small we don't do it to save ALU.
	//normal = normalize(normal);

	// As we have modify the normal we need to recalculate the dot product nl. 
	// Note that  light.ndotl is a clamped cosine and only the ForwardSimple mode use a specific ndotL with BRDF3
	half nl = DotClamped(normal, lightdir);
//#else
//	half nl = light.ndotl;
//#endif
	half nh = DotClamped(normal, halfDir);
	half nv = DotClamped(normal, viewDir);

	half lv = DotClamped(lightdir, viewDir);
	half lh = DotClamped(lightdir, halfDir);

//#if UNITY_BRDF_GGX
	half V = SmithJointGGXVisibilityTerm (nl, nv, roughness);
	half D = GGXTerm (nh, roughness);
//#else
//	half V = SmithBeckmannVisibilityTerm (nl, nv, roughness);
//	half D = NDFBlinnPhongNormalizedTerm (nh, RoughnessToSpecPower (roughness));
//#endif

	half nlPow5 = Pow5 (1-nl);
	half nvPow5 = Pow5 (1-nv);
	half Fd90 = 0.5 + 2 * lh * lh * roughness;
	half disneyDiffuse = (1 + (Fd90-1) * nlPow5) * (1 + (Fd90-1) * nvPow5);
	
	// HACK: theoretically we should divide by Pi diffuseTerm and not multiply specularTerm!
	// BUT 1) that will make shader look significantly darker than Legacy ones
	// and 2) on engine side "Non-important" lights have to be divided by Pi to in cases when they are injected into ambient SH
	// NOTE: multiplication by Pi is part of single constant together with 1/4 now
	half specularTerm = (V * D) * (UNITY_PI/4); // Torrance-Sparrow model, Fresnel is applied later (for optimization reasons)
	//if (IsGammaSpace())
	//	specularTerm = sqrt(max(1e-4h, specularTerm));
	specularTerm = max(0, specularTerm * nl);

	half diffuseTerm = oneMinusReflectivity * disneyDiffuse * nl;

	// surfaceReduction = Int D(NdotH) * NdotH * Id(NdotL>0) dH = 1/(realRoughness^2+1)
	half realRoughness = roughness*roughness;		// need to square perceptual roughness
	half surfaceReduction;
	//if (IsGammaSpace()) 
	//	surfaceReduction = 1.0 - 0.28*realRoughness*roughness;		// 1-0.28*x^3 as approximation for (1/(x^4+1))^(1/2.2) on the domain [0;1]
	//else 
		surfaceReduction = 1.0 / (realRoughness*realRoughness + 1.0);			// fade \in [0.5;1]

	//
	half grazingTerm = saturate(oneMinusRoughness + (1-oneMinusReflectivity));

	float3 R = reflect(nv,normal);
	R.z = -R.z;
        float fLod = oneMinusReflectivity * 5;
	float3 reflColor = g_ReflectionTexture.SampleLevel(g_ReflectionSampler,R.xzy,fLod).rgb;

    	half3 color =	(diffColor * (lightcolor * diffuseTerm)
                    + specularTerm * lightcolor * FresnelTerm (specColor, lh));//
					//+ u * reflColor;// * FresnelLerp (specColor, grazingTerm, nv) * surfaceReduction;
 
	//color = 10.0f * reflColor * specularTerm * FresnelTerm (specColor, lh);
        //color += 1.0f * pow(reflColor,2.2f);
        
	//return half4(u,u,u,1.0f);
        //return half4(surfaceReduction,surfaceReduction,surfaceReduction,1.0f);
	//return half4(color, 1);
        return half4(pow(color,GAMMA_OUT),1.0f);
}

//////////////////////////////////////////////////////////////////////////////////////
half3 EnvBRDF( half3 SpecularColor, half Roughness, half NoV )
{
    // Importance sampled preintegrated G * F
    float2 AB = g_PreIntegratedGFTexture.SampleLevel( g_PreIntegratedGFSampler, float2( NoV, Roughness ), 0 ).rg;
    // Anything less than 2% is physically impossible and is instead considered to be shadowing 
    float3 GF = SpecularColor * AB.x + AB.y;//saturate( 50.0 * SpecularColor.g ) * 
    return GF;
}


//////////////////////////////////////////////////////////////////////////////////////
float3 SampleNormalMap(float2 uv)
{
	float3 bumpNormal = 2.0f * (g_NormalTexture.Sample(g_NormalSampler, uv).rgb - 0.5f);

	// Reorder
	//if (flipRed) bumpNormal.r = -bumpNormal.r;
	//if (flipGreen) bumpNormal.g = -bumpNormal.g;
	//if (swapRG) bumpNormal.rg = bumpNormal.gr;

#ifdef MAX_NITROUS 
	// Flip green value because by default green means -y in the normal map generated by 3ds Max.
	bumpNormal.g = -bumpNormal.g;
#endif
	return bumpNormal;
}

half4 PBR_Unity_IBL(half3 specColor, half oneMinusRoughness,
	half3 normal, half3 viewDir)
{
	half nv = DotClamped(normal, viewDir);
	float3 R = reflect(viewDir,normal);
	R.z = -R.z;

	float fLod = (1-oneMinusRoughness) * 5;
	float3 reflColor = g_ReflectionTexture.SampleLevel(g_ReflectionSampler,R.xzy,fLod).rgb;

     	half3 result = pow(reflColor,GAMMA_IN)*EnvBRDF(specColor,1-oneMinusRoughness,nv);
	return half4(pow(result,GAMMA_OUT),1.0f);
}

//////////////////////////////////////////////////////////////////////////////////////
float4 std_PSU(vertexOutput IN) : SV_Target {

    float roughness = g_roughness;
    roughness = max(0.01f,min(0.99f,roughness));
    float metallic = g_metallic;

    float3 LightColor = Lamp0Color;
    float3 SpecularColor = k_s.xyz;

	//
	float3 bumpNormal = SampleNormalMap(IN.UV0);

	// Convert bump normal from tangent space to world space
	float3 Nn = normalize(IN.WorldNormal);
	float3 Tn = normalize(IN.WorldTangent);
	float3 Bn = normalize(IN.WorldBinormal);

	//if (orthogonalizeTangentBitangentPerPixel)
	//{
		float3 bitangent = normalize(cross(Nn, Tn));
		Tn = normalize(cross(bitangent, Nn));
		// Bitangent need to be flipped if the map face is flipped. We don't have map face handedness in shader so make
		// the calculated bitangent point in the same direction as the interpolated bitangent which has considered the flip.
		Bn = sign(dot(bitangent, Bn)) * bitangent;
	//}

	bumpNormal = normalize(bumpNormal.x * Tn + bumpNormal.y * Bn + bumpNormal.z * Nn);

    float3 L = normalize(IN.LightVec);
    float3 V = normalize(IN.WorldView);
    float3 N = bumpNormal;//normalize(IN.WorldNormal);//
    float3 H = normalize(L + V);

    float3 BaseColor = g_BaseTexture.Sample(g_BaseSampler,IN.UV0.xy).rgb;
    

    SpecularColor = lerp(half3(0.03f,0.03f,0.03f),SpecularColor,metallic);
    half4 result = PBR_Unity_PL(pow(BaseColor,GAMMA_IN),SpecularColor,1-metallic,1-roughness,N,V,L,LightColor);
    //half4 result = PBR_Unity_IBL(SpecularColor,1-roughness,N,V);
    result += PBR_Unity_IBL(SpecularColor,1-roughness,N,V);
    return result;
}


///// TECHNIQUES /////////////////////////////
fxgroup dx11
{
technique11 PBR <
	string Script = "Pass=p0;";
> {
	pass p0 <
	string Script = "Draw=geometry;";
    > 
    {
        SetVertexShader(CompileShader(vs_5_0,std_VS()));
        SetGeometryShader( NULL );
		SetPixelShader(CompileShader(ps_5_0,std_PSU()));
    }
}
}
fxgroup dx10
{
technique10 PBR <
	string Script = "Pass=p0;";
> {
	pass p0 <
	string Script = "Draw=geometry;";
    > 
    {
        SetVertexShader(CompileShader(vs_4_0,std_VS()));
        SetGeometryShader( NULL );
		SetPixelShader(CompileShader(ps_4_0,std_PSU()));
    }
}
}
/////////////////////////////////////// eof //
