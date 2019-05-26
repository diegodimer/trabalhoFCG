#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int lightsOn;
uniform vec4 view_vector;

// Identificador que define qual objeto está sendo desenhado no momento
#define SPHERE 0
#define BUNNY  1
#define PLANE  2
uniform int object_id;

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec3 color;

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    vec4 view_camera = vec4(0.0,0.0,-1.0,0.0);
    vec4 view_global = normalize(inverse(view) * view_camera);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    // esse é o da spotlight
    vec4 l = normalize(view_global);
    // esse é o normal (pras outras iluminações)
    vec4 l2 = normalize(camera_position-p);

    // Vetor que define o sentido da câmera em relação ao ponto atual.

    // v da spotlight
    vec4 v = normalize(view_global);

    // v das outras luzes
    vec4 v2 = normalize(camera_position - p);

    // spotlight
    float alpha = radians(30.0f); //0.523599;
    // Vetor que define o sentido da reflexão especular ideal.
    // -l + 2n(n*l)
    vec4 r = -l2 + 2*n*(dot(n,l2)); //  vetor de reflexão especular ideal

    // Parâmetros que definem as propriedades espectrais da superfície
    vec3 Kd; // Refletância difusa
    vec3 Ks; // Refletância especular
    vec3 Ka; // Refletância ambiente
    float q; // Expoente especular para o modelo de iluminação de Phong

    if ( object_id == SPHERE )
    {
        // PREENCHA AQUI
        // Propriedades espectrais da esfera
        Kd = vec3( 0.8f, 0.4f, 0.08f);
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd/2;
        q = 1.0;
    }
    else if ( object_id == BUNNY )
    {
        // PREENCHA AQUI
        // Propriedades espectrais do coelho
        Kd = vec3(0.08f, 0.4f, 0.8f);
        Ks = vec3 (0.8f, 0.8f, 0.8f);
        Ka = Kd/2;
        q = 32.0f;
    }
    else if ( object_id == PLANE )
    {
        // PREENCHA AQUI
        // Propriedades espectrais do plano
        Kd = vec3(0.2f,0.2f,0.2f);
        Ks = vec3(0.3f,0.3f,0.3f);
        Ka = vec3(0.0,0.0,0.0);
        q = 20.0f;
    }
    else // Objeto desconhecido = preto
    {
        Kd = vec3(0.0,0.0,0.0);
        Ks = vec3(0.0,0.0,0.0);
        Ka = vec3(0.0,0.0,0.0);
        q = 1.0;
    }

    // Espectro da fonte de iluminação
    vec3 I = vec3(1.0f,1.0f,1.0f); //  espectro da fonte de luz
    vec3 Ifundo = vec3(0.03f, 0.03f, 0.03f);
    // Espectro da luz ambiente
    vec3 Ia = vec3(0.2f,0.2f,0.2f); //  espectro da luz ambiente
    vec3 Iafundo = vec3(0.2f,0.2f,0.2f);

    vec3 lambert_diffuse_term;
    vec3 ambient_term;
    vec3 phong_specular_term;


    if(lightsOn == 0) // lanterna ligada (luzes desligadas)
    {
        if( dot( normalize(p-l), normalize(v)) > cos(alpha)) // se tá sendo iluminado pela spotlight
        {
            float fatt = pow(dot( normalize(p-l), normalize(v)), 10.12); // função de atenuação
            // Termo difuso utilizando a lei dos cossenos de Lambert
            lambert_diffuse_term = Kd * I * max(0, dot(n,l2)); // PREENCHA AQUI o termo difuso de Lambert
            // Termo ambiente
            ambient_term = Ka*Ia; // PREENCHA AQUI o termo ambiente
            // Termo especular utilizando o modelo de iluminação de Phong
            phong_specular_term  = Ks*I*max(0,pow(dot(r,v2), q)); // PREENCH AQUI o termo especular de Phong
            color = (lambert_diffuse_term + ambient_term + phong_specular_term)*fatt;
        }
        else
        {
            lambert_diffuse_term = Kd * Ifundo * max(0, dot(n,l2)); // PREENCHA AQUI o termo difuso de Lambert
            // Termo ambiente
            ambient_term = Ka*Iafundo; // PREENCHA AQUI o termo ambiente
            // Termo especular utilizando o modelo de iluminação de Phong
            phong_specular_term  = Ks*Ifundo*max(0,pow(dot(r,v2), q));
            color = (lambert_diffuse_term + ambient_term + phong_specular_term);
        }
        // Cor final do fragmento calculada com uma combinação dos termos difuso,
        // especular, e ambiente. Veja slide 133 do documento "Aula_17_e_18_Modelos_de_Iluminacao.pdf".
    }
    else // se a lanterna não tá ligada (tudo claro)
    {
        // Termo difuso utilizando a lei dos cossenos de Lambert
        lambert_diffuse_term = Kd * I * max(0, dot(n,l2)); // PREENCHA AQUI o termo difuso de Lambert
        // Termo ambiente
        ambient_term = Ka*Ia; // PREENCHA AQUI o termo ambiente
        // Termo especular utilizando o modelo de iluminação de Phong
        phong_specular_term  = Ks*I*max(0,pow(dot(r,v2), q)); // PREENCH AQUI o termo especular de Phong
        color = lambert_diffuse_term + ambient_term + phong_specular_term;
    }



    // Cor final com correção gamma, considerando monitor sRGB.
    // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
    color = pow(color, vec3(1.0,1.0,1.0)/2.2);
}

