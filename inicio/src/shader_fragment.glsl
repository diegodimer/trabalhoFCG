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
uniform float timeCounter;

#define ZOMBIE 0
#define BUNNY  1
#define PLANE  2
#define CHAO   3
#define TETO   4
#define SOFA   5
#define SWITCH 6
#define DOOR   7
#define MESA   8
#define BOLA   9

uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;


// imagens de textura
uniform sampler2D BloodyTex;
uniform sampler2D BrickTex;
uniform sampler2D WoodTex;
uniform sampler2D ZombieTex;
uniform sampler2D FabricTex;
uniform sampler2D MesaTex;
uniform sampler2D BolaTex;

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

    vec4 spotlightPosition = camera_position; // onde a lanterna está
    vec4 spotlightDirection = normalize(view_global); //Direção da lanterna
    float spotlightOpening = radians(30.0f); //Ângulo de abertura da  lanterna

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 spotlightl = normalize(spotlightPosition - p);

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 spotlightv = normalize(camera_position - p);

    vec4 l = normalize(vec4(0.3f,1.0f,0.5f,0.0f));
    vec4 v = normalize(camera_position - p);

    // spotlight
    float alpha = radians(30.0f); //0.523599;
    // Vetor que define o sentido da reflexão especular ideal.

    vec4 r = -l + 2*n*(dot(n,l)); //  vetor de reflexão especular ideal

    // Parâmetros que definem as propriedades espectrais da superfície
    vec3 Kd; // Refletância difusa
    vec3 Ks; // Refletância especular
    vec3 Ka; // Refletância ambiente
    float q; // Expoente especular para o modelo de iluminação de Phong

    // cordenadas de textura
    float U=0,V=0;

    if ( object_id == ZOMBIE )
    {

        U = texcoords.x;
        V = texcoords.y;
        // usa a ZombieTex carregada na main passada pelo loadshader
        Kd = (texture(ZombieTex, vec2(U,V)).rgb);
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd/2;
        q = 1.0;

    }
       else if ( object_id == DOOR )
    {
        Kd = vec3(0.14f,0.08f,0.01f);
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd;
        q = 1.0f;
    }
    else if ( object_id == SOFA )
    {
        // PREENCHA AQUI
        // Propriedades espectrais do coelho
        U = texcoords.x;
        V = texcoords.y;
        // a refletancia difusa é da imagem agora, com as coordenadas de textura
        Kd = (texture(FabricTex, vec2(U,V)).rgb);
        Ks = vec3 (0, 0, 0);
        Ka = Kd/2;
        q = 1.0f;


    }

    else if ( object_id == PLANE )
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x*2;
        V = texcoords.y*2;
        // a refletancia difusa é da imagem agora, com as coordenadas de textura
        Kd = (texture(WoodTex, vec2(U,V)).rgb);
        // Equação de Iluminação
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd;
        q = 1.0f;
    }
    else if ( object_id == CHAO || object_id == TETO )
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x*3;
        V = texcoords.y*3;
        // a refletancia difusa é da imagem agora, com as coordenadas de textura
        Kd = (texture(BloodyTex, vec2(U,V)).rgb);
        Kd *=0.2f;
        // Equação de Iluminação
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd;
        q = 1.0f;
    }
    else if(object_id == SWITCH){
        Kd = vec3(0.95f,0.89f,0.4f);
        Ks = vec3(0.05,0.01,0.005);
        Ka = Kd/2;
        q = 0;
    }
    else if(object_id == MESA){
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;
        // a refletancia difusa é da imagem agora, com as coordenadas de textura
        Kd = (texture(MesaTex, vec2(U,V)).rgb);
        // Equação de Iluminação
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd;
        q = 1.0f;

    }
    else if(object_id == BOLA){
                // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;
        // a refletancia difusa é da imagem agora, com as coordenadas de textura
        Kd = (texture(BolaTex, vec2(U,V)).rgb);
        // Equação de Iluminação
        Ks = vec3(0.0,0.0,0.0);
        Ka = Kd;
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
    vec3 I = vec3(1.0f,1.0f,1.0f ); //  espectro da fonte de luz
    vec3 Ifundo = vec3(0.03f, 0.03f, 0.03f);
    // Espectro da luz ambiente
    vec3 Ia = vec3(0.2f,0.2f,0.2f); //  espectro da luz ambiente
    vec3 Iafundo = vec3(0.2f,0.2f,0.2f);

    vec3 lambert_diffuse_term;
    vec3 ambient_term;
    vec3 phong_specular_term;


    if(lightsOn == 0) // lanterna ligada (luzes desligadas)
    {
        float fatt = pow(dot(normalize(p - spotlightPosition), spotlightDirection), 35.12); // função de atenuação
        if( dot(normalize(p - spotlightPosition), spotlightDirection) > cos(spotlightOpening)) // se tá sendo iluminado pela spotlight
        {

            // Termo difuso utilizando a lei dos cossenos de Lambert
            lambert_diffuse_term = Kd * I * max(0, dot(n,spotlightl)); // PREENCHA AQUI o termo difuso de Lambert
            // Termo ambiente
            ambient_term = Ka*Ia; // PREENCHA AQUI o termo ambiente
            // Termo especular utilizando o modelo de iluminação de Phong
            phong_specular_term  = Ks*I*max(0,pow(dot(r,spotlightv), q)); // PREENCH AQUI o termo especular de Phong
            color = (lambert_diffuse_term + ambient_term + phong_specular_term)*fatt;
        }
        else
        {
            lambert_diffuse_term = Kd * Ifundo * max(0, dot(n,l));
            ambient_term = Ka*Iafundo;
            phong_specular_term  = Ks*Ifundo*max(0,pow(dot(r,v), q));
            color = (lambert_diffuse_term + ambient_term + phong_specular_term)*fatt;
        }
    }

    else // se a lanterna não tá ligada (tudo claro)
    {
        // Termo difuso utilizando a lei dos cossenos de Lambert
        lambert_diffuse_term = Kd * I * max(0, dot(n,l));
        // Termo ambiente
        ambient_term = Ka*Ia; // termo ambiente
        // Termo especular utilizando o modelo de iluminação de Phong
        phong_specular_term  = Ks*I*max(0,pow(dot(r,v), q)); // termo especular de Phong
        color = lambert_diffuse_term + ambient_term + phong_specular_term;

    }


    if(interruptor==0&&lightsOn == 1) //luzes desligadas e lanterna desligada!!!!!!!!!!!!!!!!!!!!!!!!!!teste do interruptor!!!!!!!!!!!!
    {
        lambert_diffuse_term = Kd * Ifundo * max(0, dot(n,l));
        ambient_term = Ka*Iafundo;
        phong_specular_term  = Ks*Ifundo*max(0,pow(dot(r,v), q));
        color = (lambert_diffuse_term + ambient_term + phong_specular_term)*0.002;
    }


    // Cor final com correção gamma, considerando monitor sRGB.
    // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
    color = pow(color, vec3(1.0,1.0,1.0)/2.2);
}

