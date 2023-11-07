#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D outTexture;	//--- 텍스처 샘플러

void main()
{
	FragColor = texture(outTexture, TexCoord);
}
