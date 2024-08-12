#version 450

const vec3 lightPosition = vec3(-0.25, 1.0, 0.0);

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec3 vertexPosition;
layout(location = 2) in vec3 interpolatedNormal;

layout(location = 0) out vec4 outColor;

void main() {
    // ambient term
    vec3 ambient = vec3(fragColor); 
    vec3 diffuseColor = vec3(0.5, 0.5, 0.5);
    vec3 specularColor = vec3(0.6, 0.0, 0.6);

    // diffuse term
    vec3 normal = normalize(interpolatedNormal);
    // points in the direction of the light source
    vec3 light = normalize(lightPosition - vertexPosition);
    // N*L factor
    float lambert = max(0.0, dot(normal, light));
    // assume ka=1.0
    vec3 diffuse = diffuseColor*lambert;

    // specular term
    // this is from where we look, so it's the opposite direction from the vertex vector
    vec3 eye = normalize(-vertexPosition);
    // R factor: lambert is basically dot(normal, light) = dot(light, normal) = N*L
    // the ...*normal is the second N in (L*N)*N...
    vec3 r = 2.0*(lambert)*normal - light;
    // final term, 10.0 is chosen arbitrarily
    vec3 specular = specularColor*pow(max(0.0, dot(eye, r)), 10.0);

    // combine to find lit color
    vec3 litColor = ambient + diffuse + specular; 
    outColor = vec4(litColor, fragColor.w);
}
