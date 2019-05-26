#version 330 core

in vec4 position_world;
in vec4 normal;

in vec4 position_model;


in vec2 texcoords;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int lightsOn;
uniform int interruptor;


#define SPHERE 0
#define BUNNY  1
#define PLANE  2
#define CHAO   3
#define TETO   4
uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;


// imagens de textura
uniform sampler2D TextureImage0;
uniform sampler2D TextureImage1;
uniform sampler2D TextureImage2;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923


// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec3 color;

void main()
{

    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

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

    vec4 r = -l2 + 2*n*(dot(n,l2)); //  vetor de reflexão especular ideal

    // Parâmetros que definem as propriedades espectrais da superfície
    vec3 Kd; // Refletância difusa
    vec3 Ks; // Refletância especular
    vec3 Ka; // Refletância ambiente
    float q; // Expoente especular para o modelo de iluminação de Phong

    // cordenadas de textura
    float U=0,V=0;

    if ( object_id == SPHERE )
    {

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
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;
        // a refletancia difusa é da imagem agora, com as coordenadas de textura
        Kd = (texture(TextureImage1, vec2(U,V)).rgb);
            // Equação de Iluminação
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd/2;
        q = 1.0f;
    }
        else if ( object_id == CHAO || object_id == TETO )
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;
        // a refletancia difusa é da imagem agora, com as coordenadas de textura
        Kd = (texture(TextureImage1, vec2(U,V)).rgb);
            // Equação de Iluminação
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd/2;
        q = 1.0f;
    }
    else // Objeto desconhecido = preto
    {
        Kd = vec3(0,0,0);
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd/2;
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

    float fatt = pow(dot( normalize(p-l), normalize(v)), 10.12); // função de atenuação
    if(lightsOn == 0) // lanterna ligada (luzes desligadas)
    {
        if( dot( normalize(p-l), normalize(v)) > cos(alpha)) // se tá sendo iluminado pela spotlight
        {

            // Termo difuso utilizando a lei dos cossenos de Lambert
            lambert_diffuse_term = Kd * I * max(0, dot(n,l2)); // PREENCHA AQUI o termo difuso de Lambert
            // Termo ambiente
            ambient_term = Ka*Ia; // PREENCHA AQUI o termo ambiente
            // Termo especular utilizando o modelo de iluminação de Phong
            phong_specular_term  = Ks*I*max(0,pow(dot(r,v2), q)); // PREENCH AQUI o termo especular de Phong
            color = (lambert_diffuse_term + ambient_term + phong_specular_term)*(fatt);
        }
        else
        {
                   lambert_diffuse_term = Kd * Ifundo * max(0, dot(n,l2));
        ambient_term = Ka*Iafundo;
        phong_specular_term  = Ks*Ifundo*max(0,pow(dot(r,v2), q));
        color = (lambert_diffuse_term + ambient_term + phong_specular_term)*(fatt);
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


    if(interruptor==0&&lightsOn == 1) //luzes desligadas e lanterna desligada!!!!!!!!!!!!!!!!!!!!!!!!!!teste do interruptor!!!!!!!!!!!!
    {
        lambert_diffuse_term = Kd * Ifundo * max(0, dot(n,l2));
        ambient_term = Ka*Iafundo;
        phong_specular_term  = Ks*Ifundo*max(0,pow(dot(r,v2), q));
        color = (lambert_diffuse_term + ambient_term + phong_specular_term)*(fatt*0.05);
    }

        // Obtemos a refletância difusa a partir da leitura da imagem TextureImage0

    //if(interruptor==0 && object_id == BUNNY){
    //   color = vec3(0.5f,0.5f,0.0f);
    //}

    // Cor final com correção gamma, considerando monitor sRGB.
    // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
    color = pow(color, vec3(1.0,1.0,1.0)/2.2);
}

