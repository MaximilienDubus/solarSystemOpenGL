#version 330 core	     // Minimal GL version support expected from the GPU

struct Material {
    vec3 color;
    float alpha;
    float ka;
    float kd;
    float ks;
	sampler2D albedoTex;
};

// Input

uniform vec3 camPos;

uniform Material material;
uniform bool useAlbedoTexture;

uniform vec3 diffuseLightColor;
uniform vec3 specularLightColor;
uniform vec3 ambientLightColor;

uniform vec3 lightPos;

in vec3 fNormal;
in vec3 fWorldPosition;
in vec2 fTexCoord;

// Output

out vec4 color;	  // Shader output: the color response attached to this fragment


void main() {

	vec3 baseColor = material.color;

	if (useAlbedoTexture) {
		baseColor = texture(material.albedoTex, fTexCoord).rgb;
	}

	vec3 toLight = lightPos - fWorldPosition;

	vec3 n = normalize(fNormal);  // because interpolation between vertex do not preserve length
	vec3 l = normalize(toLight);  // light direction vector 

	vec3 v = normalize(camPos - fWorldPosition);  // view vector : normalized vector from vertex position toward camera position
	vec3 r = 2 * dot(n, l) * n - l;  // reflection vector : 2(n.l)n − l  // reflect(-l, n)
	float dot_nl = dot(n, l);

	// ambient light
	vec3 ambient = ambientLightColor * baseColor;
	
	// diffuse lighting : grey
	float diffuseIntensity = max(dot_nl, 0.0);
	vec3 diffuse = diffuseIntensity * diffuseLightColor * baseColor;

	// specular lighting
	float specularIntensity = 0.0;
	if (dot_nl > 0.0) {  // if the face is oriented toward light
		specularIntensity = pow(max(dot(v, r), 0.0), material.alpha);
	}
	vec3 specular = specularIntensity * specularLightColor;

	color = vec4(material.ka*ambient + material.kd*diffuse + material.ks*specular, 1.0);
}
