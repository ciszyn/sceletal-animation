#version 330 core

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in ivec3 jointIndeces;
layout(location = 3) in vec3 jointWeights;
layout(location = 4) in vec2 UV;

out vec3 fragNormal;
out vec3 fragPos;
out vec2 fragUV;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 V;
uniform mat4 Transforms[30];

void main()
{
	vec4 totalPos = vec4(0.0);
	vec4 totalNormal = vec4(0.0);
	vec4 posePosition;
	vec4 worldNormal;
	mat4 jointTransform;

	for(int i = 0; i < 3; i++)
	{
		jointTransform = Transforms[jointIndeces[i]];
		posePosition = jointTransform * vec4(Pos, 1.0);
		totalPos = totalPos + jointWeights[i] * posePosition;

		worldNormal = jointTransform * vec4(Normal, 0.0);
		totalNormal = totalNormal + jointWeights[i] * worldNormal;
	}

	gl_Position =  MVP * totalPos;
	fragPos = vec3(M * totalPos);
	fragNormal = vec3(transpose(inverse(M)) * totalNormal);
	fragUV = UV;
}
