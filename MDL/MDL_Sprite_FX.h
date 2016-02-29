#pragma once
namespace MDL
{

	char defaultSpriteFx[] =
		/*
		Beginning DirectX 11 Game Programming
		By Allen Sherrod and Wendy Jones

		Texture Mapping Shader for the Game Sprite Demo
		*/
		""
		"cbuffer cbChangesPerFrame : register(b0)"
		"{"
		"	matrix mvp_;"
		"};"
		"Texture2D colorMap_ : register(t0);"
		"SamplerState colorSampler_ : register(s0);"
		"struct VS_Input"
		"{"
		"	float4 pos : POSITION;"
		"	float2 tex0 : TEXCOORD0;"
		"};"
		"struct PS_Input"
		"{"
		"	float4 pos : SV_POSITION;"
		"	float2 tex0 : TEXCOORD0;"
		"};"
		"struct PS_Output"
		"{"
		"	float4 color : SV_TARGET;"
//		"   float  dep : SV_Depth;"
		"};"
		"PS_Input VS_Main(VS_Input vertex)"
		"{"
		"	PS_Input vsOut = (PS_Input)0;"
		"	vsOut.pos = mul(vertex.pos, mvp_);"
		"	vsOut.tex0 = vertex.tex0;"
		""
		"	return vsOut;"
		"}"
		"PS_Output PS_Main(PS_Input frag)"
		"{"
		"	float4 color = colorMap_.Sample(colorSampler_, frag.tex0);"
		"   PS_Output output;"
//		"	clip(color.a<0.5?-1:1);"
		"   output.color = color;"
		"   return output;"
		"}"
		""
		;
}