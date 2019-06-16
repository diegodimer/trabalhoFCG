#version 330 core

// Atributos de vértice recebidos como entrada ("in") pelo Vertex Shader.
// Veja a função BuildTrianglesAndAddToVirtualScene() em "main.cpp".
layout (location = 0) in vec4 model_coefficients;
layout (location = 1) in vec4 normal_coefficients;
layout (location = 2) in vec2 texture_coefficients;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int object_id;
uniform vec4 light_position;

// Atributos de vértice que serão gerados como saída ("out") pelo Vertex Shader.
// ** Estes serão interpolados pelo rasterizador! ** gerando, assim, valores
// para cada fragmento, os quais serão recebidos como entrada pelo Fragment
// Shader. Veja o arquivo "shader_fragment.glsl".
out vec4 position_world;
out vec4 position_model;
out vec4 normal;
out vec2 texcoords;
out vec3 cor_v;
void main()
{

    gl_Position = projection * view * model * model_coefficients;


    position_world = model * model_coefficients;

    // Posição do vértice atual no sistema de coordenadas local do modelo.
    position_model = model_coefficients;

    // Normal do vértice atual no sistema de coordenadas global (World).
    normal = inverse(transpose(model)) * normal_coefficients;
    normal.w = 0.0;

    // Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
    texcoords = texture_coefficients;
    float U,V;
    if (object_id == 6)
    {

        vec3 lightfontColor = vec3(1.0f, 1.0f, 1.0f);
        vec3 ambientColor = vec3(1.0f, 1.0f, 1.0f);

        vec4 n = normalize(normal);
        vec4 lightDirection = normalize(light_position - position_world);

        // cordenadas de textura
        vec3 kd = vec3(0.4f, 0.2f, 0.01f);
        vec3 ka = vec3(0.02f, 0.01f, 0.01f);

        cor_v = kd * lightfontColor * max(0, dot(n, lightDirection)) + ka * ambientColor;
    }
}

