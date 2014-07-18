Shader "Little Polygon/Rim" {
	Properties {
		_Color ("Main Color", Color) = (1.0, 1.0, 1.0, 1.0)
		_RimColor ("Rim Color", Color) = (1.0, 1.0, 1.0, 1.0)
		_MainTex ("Main Texture", 2D) = "white" {}
	}
	SubShader {
		Tags { "Queue" = "Geometry" }
		Lighting Off Fog { Mode Off }
		Pass {
		
CGPROGRAM
#pragma exclude_renderers ps3 xbox360 flash
#pragma fragmentoption ARB_precision_hint_fastest
#pragma vertex vert
#pragma fragment frag

#include "UnityCG.cginc"

uniform fixed4 _Color;
uniform fixed4 _RimColor;
uniform sampler2D _MainTex;
uniform float4 _MainTex_ST;

struct FragmentInput {
	float4 pos : SV_POSITION;
	half2 uv : TEXCOORD0;
	float rim : TEXCOORD1;
};

FragmentInput vert(
	float4 vertex : POSITION,
	float3 normal : NORMAL,
	float4 texcoord : TEXCOORD0
) {						
	FragmentInput o;
	o.pos = mul(UNITY_MATRIX_MVP, vertex);
	o.uv = TRANSFORM_TEX( texcoord, _MainTex );
	normal = mul( (float3x3)UNITY_MATRIX_IT_MV, normal);  
	o.rim = 1.0 - normal.z;
	//o.rim = 1.0 - o.rim * o.rim; // bright
	//o.rim = (1.0 - o.rim) * (1.0 - o.rim); // eased
	return o;
}

half4 frag(FragmentInput i) : COLOR {
	return _Color * tex2D( _MainTex, i.uv ) + (i.rim * _RimColor.a) * _RimColor;
}

ENDCG

		} 	
	}
	FallBack "Diffuse"
}
