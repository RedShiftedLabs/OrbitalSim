#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;

// Lighting uniforms
uniform vec3 lightPos;      // Sun position
uniform vec3 viewPos;       // Camera position
uniform vec4 lightColor;    // Sun color
uniform vec4 objectColor;   // Object base color
uniform float ambientStrength;

void main() {
    // Ambient lighting
    vec3 ambient = ambientStrength * lightColor.rgb;

    // Diffuse lighting
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor.rgb;

    // Specular lighting (subtle)
    float specularStrength = 0.3;
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor.rgb;

    // Combine lighting
    vec3 result = (ambient + diffuse + specular) * objectColor.rgb;
    finalColor = vec4(result, objectColor.a);
}
