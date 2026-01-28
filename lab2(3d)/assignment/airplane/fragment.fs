#version 330 core
out vec4 FragColor;
uniform vec3 ourColor;
uniform bool lightOn;

void main() {
    float ambient = lightOn ? 1.0 : 0.2;
    FragColor = vec4(ourColor * ambient, 1.0);
}