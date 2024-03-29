//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//                   Trabalho Final
// ANDREI CORDOVA DE AZEVEDO
// DIEGO DIMER RODRIGUES
// EDUARDO PAIM
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <windows.h>
#include <mmsystem.h>
#include <ctime>

// Headers abaixo são específicos de C++
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <ctime>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Headers da biblioteca para carregar modelos obj
#include <tiny_obj_loader.h>

#include <stb_image.h>

// Headers locais, definidos na pasta "include/"
#include "utils.h"
#include "matrices.h"

// Estrutura que representa um modelo geométrico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .
struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
    {
        printf("Carregando modelo \"%s\"... ", filename);

        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        printf("OK.\n");
    }
};


struct sceneHelper
{
    glm::mat4 model;
    char name[100];
    int nameId;
};
// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void BuildTrianglesAndAddToVirtualScene(ObjModel*); // Constrói representação de um ObjModel como malha de triângulos para renderização
void ComputeNormals(ObjModel* model); // Computa normais de um ObjModel, caso não existam.
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
void LoadTextureImage(const char* filename); // Função que carrega imagens de textura
void DrawVirtualObject(const char* object_name); // Desenha um objeto armazenado em g_VirtualScene
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU
void PrintObjModelInfo(ObjModel*); // Função para debugging

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!DECLARAÇÃO DA INTERSEÇÃO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
bool TestRayOBBIntersection(
    glm::vec3 ray_origin,        // Ray origin, in world space
    glm::vec3 ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
    glm::vec3 aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
    glm::vec3 aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
    glm::mat4 ModelMatrix,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
    float& intersection_distance // Output : distance between ray_origin and the intersection with the OBB
);

// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void movimentacaoCamera(glm::vec4*);
/* construtor de cena */
void buildRoom(float, float);
void buildFirstScene();
void lightSwitch(float, float, float);
void bolaPapel(float, float, float);
void abrePorta (float, float, float);
void testaCubo(float, float, float);
void testaCaixas(float, float, float);
void testa_ordem();
bool collisionCheckPointBox(glm::vec4, glm::vec4, glm::vec4);
bool collisionCheckBoxBox(glm::vec4, glm::vec4, glm::vec4, glm::vec4);


// Número de texturas carregadas pela função LoadTextureImage()
GLuint numOfLoadedTextures{0};

glm::vec3 p_inicial_caixa1;
glm::vec3 p_inicial_caixa2;
glm::vec3 p_inicial_caixa3;
glm::vec3 p_inicial_caixa4;
glm::vec3 p_inicial_caixa5;
glm::vec3 p_inicial_zumbi;

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    std::string  name;        // Nome do objeto
    void*        first_index; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    int          num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint       vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
    glm::vec3    bbox_min; // Axis-Aligned Bounding Box do objeto
    glm::vec3    bbox_max;
};


// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTrianglesAndAddToVirtualScene() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<std::string, SceneObject> g_VirtualScene;

// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// Ângulos de Euler que controlam a rotação de um dos cubos da cena virtual
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false; // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraTheta = 0.0f; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.0f;   // Ângulo em relação ao eixo Y
float g_CameraDistance = 3.5f; // Distância da câmera para a origem


// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool g_UsePerspectiveProjection = true;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint vertex_shader_id;
GLuint fragment_shader_id;
GLuint program_id = 0;
GLint model_uniform;
GLint view_uniform;
GLint projection_uniform;
GLint object_id_uniform;
GLint lightsOn_uniform;
GLint bbox_min_uniform;
GLint bbox_max_uniform;
// vetor da cena (com todos os elementos)
std::vector<struct sceneHelper>  sceneVector;

//!!!!!!!!!!!!!!!!!!!!! VARIAVEIS DO BOTAO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
GLint interruptor_uniform;
int interruptor=1;
bool teste_interruptor=false;

//!!!!!!!!!!!!!!!!!!!!! VARIAVEIS DO PAPEL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
GLint paper_uniform;
int paper;
bool teste_paper=false;
//!!!!!!!!!!!!!!!!!!!!! VARIAVEIS DA PORTA!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
GLint door_uniform;
int door_locked=1;
bool teste_door=false;

//!!!!!!!!!!!!!!!!!!!!! VARIAVEL P TESTE DAS CAIXAS!!!!!!!!!!!!!!!!!!!!!!!!!!!
bool teste_caixas = false;

//!!!!!!!!!!!!!!!!!!!!! VARIAVEIS P CURVA BEZIER!!!!!!!!!!!!!!!!!!!!!!!!!!!
int moveCaixa1 =0;
int moveCaixa2 =0;
int moveCaixa3 =0;
int moveCaixa4 =0;
int moveZumbi=0;
int colisaoCaixa=0;
bool isMoving=false;

float rotacaoCaixa2=0;
float rotacaoCaixa3=0;
float rotacaoCaixa4=0;

int flagBezier=0;
float auxBezier;
double t=0;
//!!!!!!!!!!!!!!!!!!!!! VARIAVEIS USADAS P TEMPO !!!!!!!!!!!!!!!!!!!!!!!!!!!
double tAnterior;
double tAgora;
float deltaT;
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!VARIAVEIS DO JOGO !!!!!!!!!!!!!!!!!!!!!!!!!!!
std::vector<int> ordem (4,0);
int posVetor=0;

int lightsOn;

bool wPressed = false;
bool aPressed = false;
bool sPressed = false;
bool dPressed = false;
bool altPress = false;
int sceneNumber;
glm::vec4 camera_position_c;


int main(int argc, char* argv[])
{
    // seed do random
    srand(time(NULL));
    interruptor=0;
    paper=0;
    lightsOn=0;
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 800 colunas e 600 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(800, 600, "SCAPELAND", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);
    // ... ou clicar os botões do mouse ...
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // ... ou movimentar o cursor do mouse em cima da janela ...
    glfwSetCursorPosCallback(window, CursorPosCallback);
    // ... ou rolar a "rodinha" do mouse.
    glfwSetScrollCallback(window, ScrollCallback);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 800, 600); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);



    // Carregamos duas imagens para serem utilizadas como textura

    LoadTextureImage("../../data/textures/bloody.png");      // TextureImage0
    LoadTextureImage("../../data/textures/brick_wall.jpg");      // TextureImage1
    LoadTextureImage("../../data/textures/wooden_floor.jpg");      // TextureImage2
    // zombie texture
    LoadTextureImage("../../data/textures/zombie.jpg");      // textura do zombie
    LoadTextureImage("../../data/textures/fabric.jpg");      // textura do sofá
    LoadTextureImage("../../data/textures/mesa.jpg");      // textura da mesa
    LoadTextureImage("../../data/textures/bola.jpg");      // textura da bola de papel
    LoadTextureImage("../../data/textures/cube.jpg");      // textura do cubo
    LoadTextureImage("../../data/textures/caixa1.jpeg");      // textura da caixa
    LoadTextureImage("../../data/textures/caixa2.jpeg");      // textura da caixa
    LoadTextureImage("../../data/textures/caixa3.jpeg");      // textura da caixa
    LoadTextureImage("../../data/textures/caixa4.jpeg");      // textura da caixa
    LoadTextureImage("../../data/textures/caixa5.jpeg");      // textura da caixa
    LoadTextureImage("../../data/textures/papel2.png");      // textura da folha

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 217-219 do documento "Aula_03_Rendering_Pipeline_Grafico.pdf".
    //
    LoadShadersFromFiles();

    // Construímos a representação de objetos geométricos através de malhas de triângulos
    ObjModel zombiemodel("../../data/obj/zombie.obj");
    ComputeNormals(&zombiemodel);
    BuildTrianglesAndAddToVirtualScene(&zombiemodel);

    ObjModel planemodel("../../data/obj/plane.obj");
    ComputeNormals(&planemodel);
    BuildTrianglesAndAddToVirtualScene(&planemodel);

    ObjModel sofamodel("../../data/obj/sofa.obj");
    ComputeNormals(&sofamodel);
    BuildTrianglesAndAddToVirtualScene(&sofamodel);

    ObjModel switch_onmodel("../../data/obj/switch_on.obj");
    ComputeNormals(&switch_onmodel);
    BuildTrianglesAndAddToVirtualScene(&switch_onmodel);

    ObjModel switch_offmodel("../../data/obj/switch_off.obj");
    ComputeNormals(&switch_offmodel);
    BuildTrianglesAndAddToVirtualScene(&switch_offmodel);

    ObjModel doormodel("../../data/obj/door.obj");
    ComputeNormals(&doormodel);
    BuildTrianglesAndAddToVirtualScene(&doormodel);

    ObjModel mesamodel("../../data/obj/mesa.obj");
    ComputeNormals(&mesamodel);
    BuildTrianglesAndAddToVirtualScene(&mesamodel);

    ObjModel bolamodel("../../data/obj/bola.obj");
    ComputeNormals(&bolamodel);
    BuildTrianglesAndAddToVirtualScene(&bolamodel);

    ObjModel cubemodel("../../data/obj/cube.obj");
    ComputeNormals(&cubemodel);
    BuildTrianglesAndAddToVirtualScene(&cubemodel);

    ObjModel caixamodel("../../data/obj/caixa.obj");
    ComputeNormals(&caixamodel);
    BuildTrianglesAndAddToVirtualScene(&caixamodel);


    if ( argc > 1 )
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Habilitamos o Z-buffer. Veja slide 108 do documento "Aula_09_Projecoes.pdf".
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 22-34 do documento "Aula_13_Clipping_and_Culling.pdf".
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Variáveis auxiliares utilizadas para chamada à função
    // TextRendering_ShowModelViewProjection(), armazenando matrizes 4x4.
    glm::mat4 the_projection;
    glm::mat4 the_model;
    glm::mat4 the_view;


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
#define CUBE   10
#define PAPEL  11
#define CAIXA1 12
#define CAIXA2 13
#define CAIXA3 14
#define CAIXA4 15
#define CAIXA5 16
#define FOLHA  17


    sceneNumber = 1;
    tAnterior = glfwGetTime();
    glm::vec4 cameraMov;
    camera_position_c = glm::vec4(3.0f, 2.0f, 5.0f, 1.0f); // seto a posição inicial
    // seto o deslocamento inicial das caixas em 0
    p_inicial_caixa1 = glm::vec3(6.0f,-1.0f,19.5f);
    p_inicial_caixa2 = glm::vec3(-9.0f,-1.0f,0.0f);
    p_inicial_caixa3 = glm::vec3(9.0f,-1.0f,-3.5f);
    p_inicial_caixa4 = glm::vec3(-7.5f,-1.0f,-19.5f);
    p_inicial_caixa5 = glm::vec3(9.0f,0.5f,2.0f);
    p_inicial_zumbi  = glm::vec3(-6.0f,1.5f,-15.0f);


    // Ficamos em loop, renderizando, até que o usuário feche a janela
    bool canMove ;
    while (!glfwWindowShouldClose(window))
    {

        // limpa o vetor
        sceneVector.clear();
        // constrói primeira cena (aqui se vir mais cenas fazemos aqui mesmo)
        // precisa que esse vetor seja reconstruído varias vezes por causa da interação, tipo o movimento do coelho
        if (sceneNumber == 1  )
        {
            // constrói o vetor de novo
            buildFirstScene();
        }
        else
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program_id);

        tAgora = glfwGetTime();
        deltaT = tAgora - tAnterior;
        deltaT*=180.0f;


        movimentacaoCamera(&cameraMov);


        // Computamos a posição da câmera utilizando coordenadas esféricas.  As
        float r = g_CameraDistance;
        float y = r*sin(g_CameraPhi);
        float z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
        float x = r*cos(g_CameraPhi)*sin(g_CameraTheta);


        canMove = true;

        /// TESTE DE COLISAO
        //Calculamos se a camera pode, de fato, se mover e não vai colidir com nada
        for(int i=0; (unsigned int)i<sceneVector.size(); i++)
        {
            glm::vec4 bbox_max =  sceneVector[i].model * glm::vec4(g_VirtualScene[sceneVector[i].name].bbox_max.x, g_VirtualScene[sceneVector[i].name].bbox_max.y,
                                  g_VirtualScene[sceneVector[i].name].bbox_max.z, 1.0f);
            glm::vec4 bbox_min =   sceneVector[i].model * glm::vec4(g_VirtualScene[sceneVector[i].name].bbox_min.x, g_VirtualScene[sceneVector[i].name].bbox_min.y,
                                   g_VirtualScene[sceneVector[i].name].bbox_min.z, 1.0f) ;
            glm::vec4 fixedBBoxMax = bbox_max;
            glm::vec4 fixedBBoxMin = bbox_min;

            // com as transoformações as bounding boxes podem ter sofrido alteração (rotações por ex), então volta a ordenar elas
            bbox_min.x = std::min(fixedBBoxMin.x, fixedBBoxMax.x);
            bbox_min.y = std::min(fixedBBoxMin.y, fixedBBoxMax.y);
            bbox_min.z = std::min(fixedBBoxMin.z, fixedBBoxMax.z);
            bbox_max.x = std::max(fixedBBoxMin.x, fixedBBoxMax.x);
            bbox_max.y = std::max(fixedBBoxMin.y, fixedBBoxMax.y);
            bbox_max.z = std::max(fixedBBoxMin.z, fixedBBoxMax.z);

            // planos nao tem volume então é um pouco diferente o role
            if(sceneVector[i].nameId != CHAO && sceneVector[i].nameId != TETO && sceneVector[i].nameId != PLANE)
            {
                if(collisionCheckPointBox(camera_position_c + (cameraMov * deltaT), bbox_min, bbox_max))
                    canMove = false;
            }
            else if (sceneVector[i].nameId == PLANE)
            {
                // pegamos as normais previamente computadas dos planos (pela função computenormals)
                glm::vec4 normal = glm::vec4(planemodel.attrib.normals[0], planemodel.attrib.normals[1], planemodel.attrib.normals[2], 0.0f);
                normal = sceneVector[i].model * normal; // aplicamos a matriz de transformação de cada plano nelas
                glm::vec4 position_vector = camera_position_c + (cameraMov * deltaT);
                // position vector é feito a partir da posição da camera após o movimento e a normal.
                // sei que não é geometricamente correto tratar normal como um ponto como eu faço aqui
                // PORÉM, eu sei que normal é um vetor que aponta para o plano e eu preciso de um ponto no plano, a normal serve perfeitamente
                position_vector = glm::vec4(normal.x - position_vector.x, normal.y - position_vector.y, normal.z - position_vector.z, 0.0f);
                if(dotproduct(position_vector, normal)<=0)  // se o sinal do produto vetorial entre a posição e a normal é <0 significa que ele está a mais de 90º entao nao pode ir
                {
                    canMove=false;
                }
            }
            if (sceneVector[i].nameId==ZOMBIE)
            {
                glm::vec4 bbox_max_aux = sceneVector[11].model * glm::vec4(g_VirtualScene[sceneVector[11].name].bbox_max.x, g_VirtualScene[sceneVector[11].name].bbox_max.y,
                                         g_VirtualScene[sceneVector[11].name].bbox_max.z, 1.0f);
                glm::vec4 bbox_min_aux = sceneVector[11].model * glm::vec4(g_VirtualScene[sceneVector[11].name].bbox_min.x, g_VirtualScene[sceneVector[11].name].bbox_min.y,
                                         g_VirtualScene[sceneVector[11].name].bbox_min.z, 1.0f) ;
                glm::vec4 fixedBBoxMaxAux = bbox_max_aux;
                glm::vec4 fixedBBoxMinAux = bbox_min_aux;

                bbox_min_aux.x = std::min(fixedBBoxMinAux.x, fixedBBoxMaxAux.x);
                bbox_min_aux.y = std::min(fixedBBoxMinAux.y, fixedBBoxMaxAux.y);
                bbox_min_aux.z = std::min(fixedBBoxMinAux.z, fixedBBoxMaxAux.z);
                bbox_max_aux.x = std::max(fixedBBoxMinAux.x, fixedBBoxMaxAux.x);
                bbox_max_aux.y = std::max(fixedBBoxMinAux.y, fixedBBoxMaxAux.y);
                bbox_max_aux.z = std::max(fixedBBoxMinAux.z, fixedBBoxMaxAux.z);

                if (collisionCheckBoxBox(bbox_min_aux, bbox_max_aux, bbox_min, bbox_max))
                {
                    colisaoCaixa=1;
                    if(flagBezier!=2)
                        flagBezier=0;
                }
            }

        }
        /// FIM TESTE DE COLISAO

        tAnterior = tAgora;


        // Agora computamos a matriz de Projeção.
        glm::mat4 projection;

        float nearplane = -0.1f;  // Posição do "near plane"
        float farplane  = -50.0f; // Posição do "far plane"

        // Projeção Perspectiva.
        float field_of_view = 3.141592 / 3.0f;
        projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);


        glm::vec4 camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f); // Vetor "up" fixado para apontar para o "céu" (eito Y global)
        glm::vec4 camera_view_vector;
        glm::vec4 camera_lookat_l;

        if (g_UsePerspectiveProjection)
        {
            // Abaixo definimos as varáveis que efetivamente definem a câmera virtual.
            if(canMove)
                camera_position_c  = camera_position_c + (cameraMov * deltaT); // Ponto "l", para onde a câmera (look-at) estará sempre olhando
            else
                camera_position_c = camera_position_c;

            camera_view_vector = glm::vec4(-x, -y, -z, 0.0f); // Vetor "view", sentido para onde a câmera está virada


        }
        else
        {
            // camera lookat
            camera_position_c  = glm::vec4(x+7.0f,2.0f,z+1.50f, 1.0f); // Ponto "l", para onde a câmera (look-at) estará sempre olhando
            glm::vec4 camera_lookat_l  = glm::vec4(7.0f,0.7f,1.50f,1.0f); // Ponto "l", para onde a câmera (look-at) estará sempre olhando
            camera_view_vector = camera_lookat_l - camera_position_c;// Vetor "view", sentido para onde a câmera está virad
            // papel
            //vetor ray_direction (sentido da camera)
            glm::vec3 ray_direction = camera_position_c + camera_view_vector;
            sceneHelper Obj;
            Obj.model = Matrix_Translate(ray_direction.x-1.0f, ray_direction.y, ray_direction.z)
                      * Matrix_Rotate_X(1.57f) *Matrix_Rotate_Z(1.57f);
            strcpy(Obj.name,"plane");
            Obj.nameId = FOLHA;
            sceneVector.push_back(Obj);
        }




        glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);


        lightSwitch(x,y,z);
        bolaPapel(x,y,z);
        abrePorta(x,y,z);
        testaCaixas(x,y,z);
        if (!isMoving && posVetor==4)
        {
            posVetor=0;
            testa_ordem();
        }
        if(moveCaixa1==-1 && moveCaixa2==-1 && moveCaixa3==-1 && moveCaixa4==-1)
        {
            moveCaixa1=0;
            moveCaixa2=0;
            moveCaixa3=0;
            moveCaixa4=0;
            isMoving=false;
            auxBezier=0;
            flagBezier=0;
        }

        glUniform1i(door_uniform,door_locked);
        glUniform1i(interruptor_uniform,interruptor);
        glUniform1i(paper_uniform,paper);
        glUniform1i(lightsOn_uniform,lightsOn);
        glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, glm::value_ptr(projection));




        /// desenho da cena

        for(std::vector<int>::size_type i = 0; i != sceneVector.size(); i++)
        {
            glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem
            model = sceneVector[i].model;
            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(object_id_uniform, sceneVector[i].nameId);
            DrawVirtualObject(sceneVector[i].name);
        }




        // Imprimimos na tela informação sobre o número de quadros renderizados
        TextRendering_ShowFramesPerSecond(window);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}

// Função que desenha um objeto armazenado em g_VirtualScene. Veja definição
// dos objetos na função BuildTrianglesAndAddToVirtualScene().
void DrawVirtualObject(const char* object_name)
{
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    // Setamos as variáveis "bbox_min" e "bbox_max" do fragment shader
    // com os parâmetros da axis-aligned bounding box (AABB) do modelo.
    glm::vec3 bbox_min = g_VirtualScene[object_name].bbox_min;
    glm::vec3 bbox_max = g_VirtualScene[object_name].bbox_max;
    glUniform4f(bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
    glUniform4f(bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);

    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a definição de
    // g_VirtualScene[""] dentro da função BuildTrianglesAndAddToVirtualScene(), e veja
    // a documentação da função glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void*)g_VirtualScene[object_name].first_index
    );

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Função que carrega os shaders de vértices e de fragmentos que serão
// utilizados para renderização. Veja slides 217-219 do documento "Aula_03_Rendering_Pipeline_Grafico.pdf".
//
void LoadShadersFromFiles()
{

    vertex_shader_id = LoadShader_Vertex("../../src/shader_vertex.glsl");
    fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if ( program_id != 0 )
        glDeleteProgram(program_id);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    program_id = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    // Buscamos o endereço das variáveis definidas dentro do Vertex Shader.
    // Utilizaremos estas variáveis para enviar dados para a placa de vídeo
    // (GPU)! Veja arquivo "shader_vertex.glsl" e "shader_fragment.glsl".
    model_uniform           = glGetUniformLocation(program_id, "model"); // Variável da matriz "model"
    view_uniform            = glGetUniformLocation(program_id, "view"); // Variável da matriz "view" em shader_vertex.glsl
    projection_uniform      = glGetUniformLocation(program_id, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    object_id_uniform       = glGetUniformLocation(program_id, "object_id"); // Variável "object_id" em shader_fragment.glsl
    bbox_min_uniform        = glGetUniformLocation(program_id, "bbox_min");
    bbox_max_uniform        = glGetUniformLocation(program_id, "bbox_max");

    lightsOn_uniform = glGetUniformLocation(program_id, "lightsOn");
    interruptor_uniform = glGetUniformLocation(program_id, "interruptor");
    paper_uniform = glGetUniformLocation(program_id, "paper");
    door_uniform = glGetUniformLocation(program_id, "door_locked");
    // Variáveis em "shader_fragment.glsl" para acesso das imagens de textura
    glUseProgram(program_id);
    glUniform1i(glGetUniformLocation(program_id, "BloodyTex"), 0);
    glUniform1i(glGetUniformLocation(program_id, "BrickTex"), 1);
    glUniform1i(glGetUniformLocation(program_id, "WoodTex"), 2);
    glUniform1i(glGetUniformLocation(program_id, "ZombieTex"), 3);
    glUniform1i(glGetUniformLocation(program_id, "FabricTex"), 4);
    glUniform1i(glGetUniformLocation(program_id, "MesaTex"), 5);
    glUniform1i(glGetUniformLocation(program_id, "BolaTex"), 6);
    glUniform1i(glGetUniformLocation(program_id, "CubeTex"), 7);
    glUniform1i(glGetUniformLocation(program_id, "Caixa1Tex"), 8);
    glUniform1i(glGetUniformLocation(program_id, "Caixa2Tex"), 9);
    glUniform1i(glGetUniformLocation(program_id, "Caixa3Tex"), 10);
    glUniform1i(glGetUniformLocation(program_id, "Caixa4Tex"), 11);
    glUniform1i(glGetUniformLocation(program_id, "Caixa5Tex"), 12);
    glUniform1i(glGetUniformLocation(program_id, "FolhaTex"), 13);

    glUseProgram(0);
}


// Função que pega a matriz M e guarda a mesma no topo da pilha
void PushMatrix(glm::mat4 M)
{
    g_MatrixStack.push(M);
}

// Função que remove a matriz atualmente no topo da pilha e armazena a mesma na variável M
void PopMatrix(glm::mat4& M)
{
    if ( g_MatrixStack.empty() )
    {
        M = Matrix_Identity();
    }
    else
    {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

// Função que computa as normais de um ObjModel, caso elas não tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(ObjModel* model)
{
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRIÂNGULOS.
    // Segundo, computamos as normais dos VÉRTICES através do método proposto
    // por Gouraud, onde a normal de cada vértice vai ser a média das normais de
    // todas as faces que compartilham este vértice.

    size_t num_vertices = model->attrib.vertices.size() / 3;

    std::vector<int> num_triangles_per_vertex(num_vertices, 0);
    std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            glm::vec4  vertices[3];
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
            }

            const glm::vec4  a = vertices[0];
            const glm::vec4  b = vertices[1];
            const glm::vec4  c = vertices[2];

            // PREENCHA AQUI o cálculo da normal de um triângulo cujos vértices
            // estão nos pontos "a", "b", e "c", definidos no sentido anti-horário.
            glm::vec4 u = b-a;
            glm::vec4 v = c-a;
            const glm::vec4  n = crossproduct(u,v);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                num_triangles_per_vertex[idx.vertex_index] += 1;
                vertex_normals[idx.vertex_index] += n;
                model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index = idx.vertex_index;
            }
        }
    }

    model->attrib.normals.resize( 3*num_vertices );

    for (size_t i = 0; i < vertex_normals.size(); ++i)
    {
        glm::vec4 n = vertex_normals[i] / (float)num_triangles_per_vertex[i];
        n /= norm(n);
        model->attrib.normals[3*i + 0] = n.x;
        model->attrib.normals[3*i + 1] = n.y;
        model->attrib.normals[3*i + 2] = n.z;
    }
}

// Constrói triângulos para futura renderização a partir de um ObjModel.
void BuildTrianglesAndAddToVirtualScene(ObjModel* model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        const float minval = std::numeric_limits<float>::min();
        const float maxval = std::numeric_limits<float>::max();

        glm::vec3 bbox_min = glm::vec3(maxval,maxval,maxval);
        glm::vec3 bbox_max = glm::vec3(minval,minval,minval);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                // printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);


                bbox_min.x = std::min(bbox_min.x, vx);
                bbox_min.y = std::min(bbox_min.y, vy);
                bbox_min.z = std::min(bbox_min.z, vz);
                bbox_max.x = std::max(bbox_max.x, vx);
                bbox_max.y = std::max(bbox_max.y, vy);
                bbox_max.z = std::max(bbox_max.z, vz);

                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W

                // Inspecionando o código da tinyobjloader, o aluno Bernardo
                // Sulzbach (2017/1) apontou que a maneira correta de testar se
                // existem normais e coordenadas de textura no ObjModel é
                // comparando se o índice retornado é -1. Fazemos isso abaixo.

                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }

                if ( idx.texcoord_index != -1 )
                {
                    const float u = model->attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }
            }
        }

        size_t last_index = indices.size() - 1;

        SceneObject theobject;
        theobject.name           = model->shapes[shape].name;
        theobject.first_index    = (void*)first_index; // Primeiro índice
        theobject.num_indices    = last_index - first_index + 1; // Número de indices
        theobject.rendering_mode = GL_TRIANGLES;       // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        theobject.vertex_array_object_id = vertex_array_object_id;

        theobject.bbox_min = bbox_min;
        theobject.bbox_max = bbox_max;

        g_VirtualScene[theobject.name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}


// Carrega um Vertex Shader de um arquivo GLSL. Veja definição de LoadShader() abaixo.
GLuint LoadShader_Vertex(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos vértices.
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, vertex_shader_id);

    // Retorna o ID gerado acima
    return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja definição de LoadShader() abaixo.
GLuint LoadShader_Fragment(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos fragmentos.
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, fragment_shader_id);

    // Retorna o ID gerado acima
    return fragment_shader_id;
}

// Função auxilar, utilizada pelas duas funções acima. Carrega código de GPU de
// um arquivo GLSL e faz sua compilação.
void LoadShader(const char* filename, GLuint shader_id)
{
    // Lemos o arquivo de texto indicado pela variável "filename"
    // e colocamos seu conteúdo em memória, apontado pela variável
    // "shader_string".
    std::ifstream file;
    try
    {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    }
    catch ( std::exception& e )
    {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );

    // Define o código do shader GLSL, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    // Compila o código do shader GLSL (em tempo de execução)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compilação
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos memória para guardar o log de compilação.
    // A chamada "new" em C++ é equivalente ao "malloc()" do C.
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    // Imprime no terminal qualquer erro ou "warning" de compilação
    if ( log_length != 0 )
    {
        std::string  output;

        if ( !compiled_ok )
        {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    // A chamada "delete" em C++ é equivalente ao "free()" do C
    delete [] log;
}

// Esta função cria um programa de GPU, o qual contém obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    // Criamos um identificador (ID) para este programa de GPU
    GLuint program_id = glCreateProgram();

    // Definição dos dois shaders GLSL que devem ser executados pelo programa
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Linkagem dos shaders acima ao programa
    glLinkProgram(program_id);

    // Verificamos se ocorreu algum erro durante a linkagem
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    // Imprime no terminal qualquer erro de linkagem
    if ( linked_ok == GL_FALSE )
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        // Alocamos memória para guardar o log de compilação.
        // A chamada "new" em C++ é equivalente ao "malloc()" do C.
        GLchar* log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";

        // A chamada "delete" em C++ é equivalente ao "free()" do C
        delete [] log;

        fprintf(stderr, "%s", output.c_str());
    }

    // Os "Shader Objects" podem ser marcados para deleção após serem linkados
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Retornamos o ID gerado acima
    return program_id;
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula (slides 33-44 do documento "Aula_07_Transformacoes_Geometricas_3D.pdf").
    glViewport(0, 0, width, height);

    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slide 227 do documento "Aula_09_Projecoes.pdf".
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    g_ScreenRatio = (float)width / height;
}

// Variáveis globais que armazenam a última posição do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.
double g_LastCursorPosX, g_LastCursorPosY;

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_LeftMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_LeftMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_RightMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_RightMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_RightMouseButtonPressed = false;

        //!!!!!!!!!!!!!!!!!!!!!!!!!TESTE VARIAVEIS AUXILIARES!!!!!!!!!!!!!!!!!!!!!
        teste_interruptor = true;
        teste_paper = true;
        teste_door = true;
        teste_caixas = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_MiddleMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_MiddleMouseButtonPressed = false;
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    // Abaixo executamos o seguinte: caso o botão esquerdo do mouse esteja
    // pressionado, computamos quanto que o mouse se movimento desde o último
    // instante de tempo, e usamos esta movimentação para atualizar os
    // parâmetros que definem a posição da câmera dentro da cena virtual.
    // Assim, temos que o usuário consegue controlar a câmera.

    if (g_LeftMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;

        // Atualizamos parâmetros da câmera com os deslocamentos
        g_CameraTheta -= 0.01f*dx;
        g_CameraPhi   += 0.01f*dy;

        // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
        float phimax = 3.141592f/2;
        float phimin = -phimax;

        if (g_CameraPhi > phimax)
            g_CameraPhi = phimax;

        if (g_CameraPhi < phimin)
            g_CameraPhi = phimin;

        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }


}

// Função callback chamada sempre que o usuário movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Atualizamos a distância da câmera para a origem utilizando a
    // movimentação da "rodinha", simulando um ZOOM.
    g_CameraDistance -= 0.1f*yoffset;

    // Uma câmera look-at nunca pode estar exatamente "em cima" do ponto para
    // onde ela está olhando, pois isto gera problemas de divisão por zero na
    // definição do sistema de coordenadas da câmera. Isto é, a variável abaixo
    // nunca pode ser zero. Versões anteriores deste código possuíam este bug,
    // o qual foi detectado pelo aluno Vinicius Fraga (2017/2).
    const float verysmallnumber = std::numeric_limits<float>::epsilon();
    if (g_CameraDistance < verysmallnumber)
        g_CameraDistance = verysmallnumber;
}

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{


    if (key == GLFW_KEY_W)
    {
        if(!(action==GLFW_RELEASE))
        {
            wPressed=true;
        }
        else
            wPressed = false;
    }

    if (key == GLFW_KEY_S)
    {
        if(!(action==GLFW_RELEASE))
        {
            sPressed = true;
        }
        else
            sPressed = false;
    }

    if (key == GLFW_KEY_A)
    {
        if(!(action==GLFW_RELEASE))
        {
            aPressed = true;
        }
        else
            aPressed = false;
    }
    if (key == GLFW_KEY_D)
    {
        if(!(action==GLFW_RELEASE))
        {
            dPressed = true;
        }
        else
            dPressed = false;
    }

    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // O código abaixo implementa a seguinte lógica:
    //   Se apertar tecla X       então g_AngleX += delta;
    //   Se apertar tecla shift+X então g_AngleX -= delta;
    //   Se apertar tecla Y       então g_AngleY += delta;
    //   Se apertar tecla shift+Y então g_AngleY -= delta;
    //   Se apertar tecla Z       então g_AngleZ += delta;
    //   Se apertar tecla shift+Z então g_AngleZ -= delta;

    float delta = 0.5f; // 22.5 graus, em radianos.

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        g_AngleX += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        g_AngleY += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        g_AngleZ += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    // Se o usuário apertar a tecla espaço, resetamos os ângulos de Euler para zero.
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        g_AngleX = 0.0f;
        g_AngleY = 0.0f;
        g_AngleZ = 0.0f;

    }

    // Se o usuário apertar a tecla P, utilizamos camera livre.
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        g_UsePerspectiveProjection = true;
        camera_position_c.y = 2.0f;
        camera_position_c.x = 6.0f;
        camera_position_c.z = 1.50f;
    }

    if (key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        lightsOn = lightsOn == 0? 1: 0;
    }

    // Se o usuário apertar a tecla O, utilizamos camera lookat.
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        if(paper)
            g_UsePerspectiveProjection = false;

    }

    // Se o usuário apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        g_ShowInfoText = !g_ShowInfoText;
    }

    // Se o usuário apertar a tecla R, recarregamos os shaders dos arquivos "shader_fragment.glsl" e "shader_vertex.glsl".
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        fprintf(stdout,"Shaders recarregados!\n");
        fflush(stdout);
    }
}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}


// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);

        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}



void buildFirstScene()
{

    struct sceneHelper Objeto;

    // aqui é o desenho do zombie
    if (moveZumbi == 1)
    {
        if (flagBezier==0)
        {
            auxBezier=glfwGetTime();
            flagBezier=1;
        }
        t = (glfwGetTime() - auxBezier)/2;
        t = sin(t);


        //pontos de controle
        glm::vec4 p1  = glm::vec4(p_inicial_zumbi.x,       p_inicial_zumbi.y,    p_inicial_zumbi.z, 1.0f);
        glm::vec4 p2  = glm::vec4(p_inicial_zumbi.x +2.0f, p_inicial_zumbi.y,    p_inicial_zumbi.z, 1.0f);
        glm::vec4 p3  = glm::vec4(p_inicial_zumbi.x +6.0f, p_inicial_zumbi.y,    p_inicial_zumbi.z, 1.0f);
        glm::vec4 p4  = glm::vec4(p_inicial_zumbi.x +10.0f,p_inicial_zumbi.y,    p_inicial_zumbi.z, 1.0f);

        // polinomio de bernstein (achei mais facil escrever por extenso que usar a série)
        float b03 = pow( (1-t), 3);
        float b13 = 3*t * pow( (1-t),2);
        float b23 = 3*t*t * (1-t);
        float b33 = pow(t,3);
        glm::vec4 centro_zumbi  = b03*p1 + b13*p2 + b23*p3 + b33*p4;

        Objeto.model = Matrix_Translate(centro_zumbi.x, centro_zumbi.y, centro_zumbi.z)
                       *Matrix_Scale(3.0f,3.0f,3.0f);
        strcpy(Objeto.name,"zombie");
        Objeto.nameId = ZOMBIE;
        sceneVector.push_back(Objeto);

        if (centro_zumbi.x >=3.9999f)
        {
            moveZumbi=-1;
            p_inicial_zumbi.x = 4.0f;
            p_inicial_zumbi.y = 1.5f;
            p_inicial_zumbi.z = -15.0f;
            auxBezier=0;
            flagBezier=0;
            isMoving=false;
        }
    }
    else if (moveZumbi == 2)
    {
        if (flagBezier==0)
        {
            auxBezier=glfwGetTime();
            flagBezier=1;
        }
        t = (glfwGetTime() - auxBezier)/2;
        t = sin(t);


        //pontos de controle
        glm::vec4 p1  = glm::vec4(p_inicial_zumbi.x,       p_inicial_zumbi.y,    p_inicial_zumbi.z, 1.0f);
        glm::vec4 p2  = glm::vec4(p_inicial_zumbi.x -2.0f, p_inicial_zumbi.y,    p_inicial_zumbi.z, 1.0f);
        glm::vec4 p3  = glm::vec4(p_inicial_zumbi.x -6.0f, p_inicial_zumbi.y,    p_inicial_zumbi.z, 1.0f);
        glm::vec4 p4  = glm::vec4(p_inicial_zumbi.x -10.0f,p_inicial_zumbi.y,    p_inicial_zumbi.z, 1.0f);

        // polinomio de bernstein (achei mais facil escrever por extenso que usar a série)
        float b03 = pow( (1-t), 3);
        float b13 = 3*t * pow( (1-t),2);
        float b23 = 3*t*t * (1-t);
        float b33 = pow(t,3);
        glm::vec4 centro_zumbi  = b03*p1 + b13*p2 + b23*p3 + b33*p4;

        Objeto.model = Matrix_Translate(centro_zumbi.x, centro_zumbi.y, centro_zumbi.z)
                       *Matrix_Scale(3.0f,3.0f,3.0f);
        strcpy(Objeto.name,"zombie");
        Objeto.nameId = ZOMBIE;
        sceneVector.push_back(Objeto);

        if (centro_zumbi.x <=-5.9999f)
        {
            moveZumbi=0;
            p_inicial_zumbi.x = -6.0f;
            p_inicial_zumbi.y = 1.5f;
            p_inicial_zumbi.z = -15.0f;
            auxBezier=0;
            flagBezier=0;
            isMoving=false;
        }
    }
    else
    {
        Objeto.model = Matrix_Translate(p_inicial_zumbi.x, p_inicial_zumbi.y, p_inicial_zumbi.z)
                       * Matrix_Scale(3.0f,3.0f,3.0f);
        strcpy(Objeto.name,"zombie");
        Objeto.nameId = ZOMBIE;
        sceneVector.push_back(Objeto);
    }

    //desenho da porta
    Objeto.model = Matrix_Translate(-10.0f,2.50f,12.50f)
                   * Matrix_Scale(0.02f,0.035f,0.05f)
                   * Matrix_Rotate_Y(1.57)
                   * Matrix_Rotate_X(-1.57);
    strcpy(Objeto.name,"door");
    Objeto.nameId = DOOR;
    sceneVector.push_back(Objeto);

// desenho do sofa
    Objeto.model =   Matrix_Translate(8.50f, -1.0f, 10.0f)
                     * Matrix_Scale(0.025f, 0.025f, 0.025f)
                     * Matrix_Rotate_Y(-1.57)
                     * Matrix_Rotate_X(-1.57);
    strcpy(Objeto.name,"sofa");
    Objeto.nameId = SOFA;
    sceneVector.push_back(Objeto);


// desenho do sofa
    Objeto.model =   Matrix_Translate(2.0f, -1.0f, 18.50f)
                     * Matrix_Scale(0.025f, 0.025f, 0.025f)
                     * Matrix_Rotate_Y(3.14159f)
                     * Matrix_Rotate_X(-1.57);
    strcpy(Objeto.name,"sofa");
    Objeto.nameId = SOFA;
    sceneVector.push_back(Objeto);

// desenho do interruptor de luz. Se ligado é o switch_on, senão o off.
    if(interruptor)
    {

        Objeto.model = Matrix_Translate(-10.0f,2.5f,16.50f)
                       * Matrix_Rotate_Y(1.57);
        strcpy(Objeto.name,"switch_on");
        Objeto.nameId = SWITCH;
        sceneVector.push_back(Objeto);
    }
    else
    {
        Objeto.model =Matrix_Translate(-10.0f,2.5f,16.50f)
                      * Matrix_Rotate_Y(1.57);
        strcpy(Objeto.name,"switch_off");
        Objeto.nameId = SWITCH;
        sceneVector.push_back(Objeto);
    }



    // mesinha

    Objeto.model = Matrix_Translate(8.50f,-0.7f,1.50f)
                   * Matrix_Scale(2.5f, 2.5f,2.5f)
                   * Matrix_Rotate_Y(1.50f);
    strcpy(Objeto.name,"mesa");
    Objeto.nameId = MESA;
    sceneVector.push_back(Objeto);

    //bola

    Objeto.model = Matrix_Translate(8.0f,0.7f,1.50f)
                   * Matrix_Scale(0.2f, 0.2f,0.2f);
    strcpy(Objeto.name,"bola");
    Objeto.nameId = BOLA;
    sceneVector.push_back(Objeto);

    //cubo


    Objeto.model = Matrix_Translate(-7.50f,-0.3f,-5.50f)
                   * Matrix_Scale(0.7f, 0.7f,0.7f)
                   * Matrix_Rotate_Y(1.57f)
                   * Matrix_Rotate_X(g_AngleX)
                   * Matrix_Rotate_Y(g_AngleY)
                   * Matrix_Rotate_Z(g_AngleZ);
    strcpy(Objeto.name,"cube");
    Objeto.nameId = CUBE;
    sceneVector.push_back(Objeto);

    //caixa 1
    if (moveCaixa1 == 1)
    {
        if (flagBezier==0)
        {
            auxBezier=glfwGetTime();
            flagBezier=1;
        }
        t = (glfwGetTime() - auxBezier)/2;
        t = sin(t);


        //pontos de controle
        glm::vec4 p1  = glm::vec4(p_inicial_caixa1.x,       p_inicial_caixa1.y,         p_inicial_caixa1.z, 1.0f);
        glm::vec4 p2  = glm::vec4(p_inicial_caixa1.x,       p_inicial_caixa1.y+2.0,     p_inicial_caixa1.z-7.0f, 1.0f);
        glm::vec4 p3  = glm::vec4(p_inicial_caixa1.x -2.0f, p_inicial_caixa1.y + 3.0f,  p_inicial_caixa1.z-12.0f, 1.0f);
        glm::vec4 p4  = glm::vec4(p_inicial_caixa1.x -7.0f, p_inicial_caixa1.y + 5.7f,  p_inicial_caixa1.z-18.0f, 1.0f);

        // polinomio de bernstein (achei mais facil escrever por extenso que usar a série)
        float b03 = pow( (1-t), 3);
        float b13 = 3*t * pow( (1-t),2);
        float b23 = 3*t*t * (1-t);
        float b33 = pow(t,3);
        glm::vec4 centro_caixa1  = b03*p1 + b13*p2 + b23*p3 + b33*p4;

        Objeto.model = Matrix_Translate(centro_caixa1.x, centro_caixa1.y, centro_caixa1.z);
        strcpy(Objeto.name,"caixa");
        Objeto.nameId = CAIXA1;
        sceneVector.push_back(Objeto);

        if (centro_caixa1.x <=-0.9999f)
        {
            moveCaixa1=0;
            p_inicial_caixa1.x = -1.0f;
            p_inicial_caixa1.y = 4.7f;
            p_inicial_caixa1.z = 1.5f;
            auxBezier=0;
            flagBezier=0;
            isMoving=false;
        }
    }

    else if (moveCaixa1 == 2)
    {
        if (flagBezier==0)
        {
            auxBezier=glfwGetTime();
            flagBezier=1;
        }
        t = (glfwGetTime() - auxBezier)/2;
        t = sin(t);


        //pontos de controle
        glm::vec4 p1  = glm::vec4(p_inicial_caixa1.x,       p_inicial_caixa1.y,         p_inicial_caixa1.z, 1.0f);
        glm::vec4 p2  = glm::vec4(p_inicial_caixa1.x,       p_inicial_caixa1.y - 2.0f,   p_inicial_caixa1.z+7.0f, 1.0f);
        glm::vec4 p3  = glm::vec4(p_inicial_caixa1.x +5.0f, p_inicial_caixa1.y - 3.0f,  p_inicial_caixa1.z+12.0f, 1.0f);
        glm::vec4 p4  = glm::vec4(p_inicial_caixa1.x +7.0f, p_inicial_caixa1.y - 5.7f,  p_inicial_caixa1.z+18.0f, 1.0f);

        // polinomio de bernstein (achei mais facil escrever por extenso que usar a série)
        float b03 = pow( (1-t), 3);
        float b13 = 3*t * pow( (1-t),2);
        float b23 = 3*t*t * (1-t);
        float b33 = pow(t,3);
        glm::vec4 centro_caixa1  = b03*p1 + b13*p2 + b23*p3 + b33*p4;

        Objeto.model = Matrix_Translate(centro_caixa1.x, centro_caixa1.y, centro_caixa1.z);
        strcpy(Objeto.name,"caixa");
        Objeto.nameId = CAIXA1;
        sceneVector.push_back(Objeto);

        if (centro_caixa1.x >=5.9999f)
        {
            moveCaixa1=-1;
            p_inicial_caixa1.x = 6.0f;
            p_inicial_caixa1.y = -1.0f;
            p_inicial_caixa1.z = 19.5f;
        }
    }
    else
    {
        Objeto.model = Matrix_Translate(p_inicial_caixa1.x, p_inicial_caixa1.y, p_inicial_caixa1.z);
        strcpy(Objeto.name,"caixa");
        Objeto.nameId = CAIXA1;
        sceneVector.push_back(Objeto);
    }

    //caixa 2
    if(moveCaixa2==1)
    {
        if (flagBezier==0)
        {
            auxBezier=glfwGetTime();
            flagBezier=1;
        }
        t = (glfwGetTime() - auxBezier)/2;
        t = sin(t);

        glm::vec4 p1  = glm::vec4(p_inicial_caixa2.x,       p_inicial_caixa2.y,         p_inicial_caixa2.z, 1.0f);
        glm::vec4 p2  = glm::vec4(p_inicial_caixa2.x,       p_inicial_caixa2.y +1.0,    p_inicial_caixa2.z+1.0f, 1.0f);
        glm::vec4 p3  = glm::vec4(p_inicial_caixa2.x +3.0f, p_inicial_caixa2.y +2.0f,   p_inicial_caixa2.z+1.0f, 1.0f);
        glm::vec4 p4  = glm::vec4(p_inicial_caixa2.x +8.0f, p_inicial_caixa2.y +3.95f,   p_inicial_caixa2.z+1.5f, 1.0f);

        // polinomio de bernstein (achei mais facil escrever por extenso que usar a série)
        float b03 = pow( (1-t), 3);
        float b13 = 3*t * pow( (1-t),2);
        float b23 = 3*t*t * (1-t);
        float b33 = pow(t,3);
        glm::vec4 centro_caixa2  = b03*p1 + b13*p2 + b23*p3 + b33*p4;

        if (rotacaoCaixa2<1.57f)
            rotacaoCaixa2+=0.0015f;

        Objeto.model = Matrix_Translate(centro_caixa2.x, centro_caixa2.y, centro_caixa2.z) * Matrix_Rotate_Y(-1.57f + rotacaoCaixa2);
        strcpy(Objeto.name,"caixa");
        Objeto.nameId = CAIXA2;
        sceneVector.push_back(Objeto);

        if (centro_caixa2.x >=-1.0001f)
        {
            moveCaixa2=0;
            p_inicial_caixa2.x = -1.0f;
            p_inicial_caixa2.y = 2.95f;
            p_inicial_caixa2.z = 1.5f;
            rotacaoCaixa2=1.57f;
            auxBezier=0;
            flagBezier=0;
            isMoving=false;
        }

    }

    else if(moveCaixa2==2)
    {
        if (flagBezier==0)
        {
            auxBezier=glfwGetTime();
            flagBezier=1;
        }
        t = (glfwGetTime() - auxBezier)/2;
        t = sin(t);

        glm::vec4 p1  = glm::vec4(p_inicial_caixa2.x,       p_inicial_caixa2.y,         p_inicial_caixa2.z, 1.0f);
        glm::vec4 p2  = glm::vec4(p_inicial_caixa2.x,       p_inicial_caixa2.y -1.0,    p_inicial_caixa2.z-1.0f, 1.0f);
        glm::vec4 p3  = glm::vec4(p_inicial_caixa2.x -3.0f, p_inicial_caixa2.y -2.0f,   p_inicial_caixa2.z-1.0f, 1.0f);
        glm::vec4 p4  = glm::vec4(p_inicial_caixa2.x -8.0f, p_inicial_caixa2.y -3.95f,   p_inicial_caixa2.z-1.5f, 1.0f);

        // polinomio de bernstein (achei mais facil escrever por extenso que usar a série)
        float b03 = pow( (1-t), 3);
        float b13 = 3*t * pow( (1-t),2);
        float b23 = 3*t*t * (1-t);
        float b33 = pow(t,3);
        glm::vec4 centro_caixa2  = b03*p1 + b13*p2 + b23*p3 + b33*p4;

        if (rotacaoCaixa2>-1.57f)
            rotacaoCaixa2-=0.0012f;

        Objeto.model = Matrix_Translate(centro_caixa2.x, centro_caixa2.y, centro_caixa2.z) * Matrix_Rotate_Y(-1.57f + rotacaoCaixa2);
        strcpy(Objeto.name,"caixa");
        Objeto.nameId = CAIXA2;
        sceneVector.push_back(Objeto);

        if (centro_caixa2.x <=-8.999f)
        {
            moveCaixa2=-1;
            p_inicial_caixa2.x = -9.0f;
            p_inicial_caixa2.y = -1.0f;
            p_inicial_caixa2.z = 0.0f;
            rotacaoCaixa2=0.0f;
        }
    }
    else
    {
        Objeto.model = Matrix_Translate(p_inicial_caixa2.x,p_inicial_caixa2.y,p_inicial_caixa2.z) * Matrix_Rotate_Y(-1.57f + rotacaoCaixa2);
        strcpy(Objeto.name,"caixa");
        Objeto.nameId = CAIXA2;
        sceneVector.push_back(Objeto);
    }

    //caixa 3
    if(moveCaixa3==1)
    {
        if (flagBezier==0)
        {
            auxBezier=glfwGetTime();
            flagBezier=1;
        }
        t = (glfwGetTime() - auxBezier)/2;
        t = sin(t);

        glm::vec4 p1  = glm::vec4(p_inicial_caixa3.x,       p_inicial_caixa3.y,         p_inicial_caixa3.z, 1.0f);
        glm::vec4 p2  = glm::vec4(p_inicial_caixa3.x-2.0f,  p_inicial_caixa3.y +1.0f,   p_inicial_caixa3.z+1.0f, 1.0f);
        glm::vec4 p3  = glm::vec4(p_inicial_caixa3.x -5.0f, p_inicial_caixa3.y +2.3f,   p_inicial_caixa3.z+3.0f, 1.0f);
        glm::vec4 p4  = glm::vec4(p_inicial_caixa3.x -10.0f,p_inicial_caixa3.y +2.3f,   p_inicial_caixa3.z+5.0f, 1.0f);

        // polinomio de bernstein (achei mais facil escrever por extenso que usar a série)
        float b03 = pow( (1-t), 3);
        float b13 = 3*t * pow( (1-t),2);
        float b23 = 3*t*t * (1-t);
        float b33 = pow(t,3);
        glm::vec4 centro_caixa3  = b03*p1 + b13*p2 + b23*p3 + b33*p4;

        if (rotacaoCaixa3>-1.57f)
            rotacaoCaixa3-=0.002f;

        Objeto.model = Matrix_Translate(centro_caixa3.x, centro_caixa3.y, centro_caixa3.z) * Matrix_Rotate_Y(1.57f + rotacaoCaixa3);
        strcpy(Objeto.name,"caixa");
        Objeto.nameId = CAIXA3;
        sceneVector.push_back(Objeto);
        if (centro_caixa3.x <=-0.9999f)
        {
            moveCaixa3=0;
            p_inicial_caixa3.x = -1.0f;
            p_inicial_caixa3.y = 1.3f;
            p_inicial_caixa3.z = 1.5f;
            rotacaoCaixa3=-1.57f;
            auxBezier=0;
            flagBezier=0;
            isMoving=false;
        }
    }
    else if(moveCaixa3==2)
    {
        if (flagBezier==0)
        {
            auxBezier=glfwGetTime();
            flagBezier=1;
        }
        t = (glfwGetTime() - auxBezier)/2;
        t = sin(t);

        glm::vec4 p1  = glm::vec4(p_inicial_caixa3.x,       p_inicial_caixa3.y,         p_inicial_caixa3.z, 1.0f);
        glm::vec4 p2  = glm::vec4(p_inicial_caixa3.x +2.0f, p_inicial_caixa3.y -1.0f,   p_inicial_caixa3.z-1.0f, 1.0f);
        glm::vec4 p3  = glm::vec4(p_inicial_caixa3.x +5.0f, p_inicial_caixa3.y -2.3f,   p_inicial_caixa3.z-3.0f, 1.0f);
        glm::vec4 p4  = glm::vec4(p_inicial_caixa3.x +10.0f,p_inicial_caixa3.y -2.3f,   p_inicial_caixa3.z-5.0f, 1.0f);

        // polinomio de bernstein (achei mais facil escrever por extenso que usar a série)
        float b03 = pow( (1-t), 3);
        float b13 = 3*t * pow( (1-t),2);
        float b23 = 3*t*t * (1-t);
        float b33 = pow(t,3);
        glm::vec4 centro_caixa3  = b03*p1 + b13*p2 + b23*p3 + b33*p4;

        if (rotacaoCaixa3<1.57f)
            rotacaoCaixa3+=0.0012f;

        Objeto.model = Matrix_Translate(centro_caixa3.x, centro_caixa3.y, centro_caixa3.z) * Matrix_Rotate_Y(1.57f + rotacaoCaixa3);
        strcpy(Objeto.name,"caixa");
        Objeto.nameId = CAIXA3;
        sceneVector.push_back(Objeto);
        if (centro_caixa3.x >=8.9999f)
        {
            moveCaixa3=-1;
            p_inicial_caixa3.x = 9.0f;
            p_inicial_caixa3.y = -1.0f;
            p_inicial_caixa3.z = -3.5f;
            rotacaoCaixa3=0.0f;
        }
    }
    else
    {
        Objeto.model = Matrix_Translate(p_inicial_caixa3.x,p_inicial_caixa3.y, p_inicial_caixa3.z) * Matrix_Rotate_Y(1.57f + rotacaoCaixa3);
        strcpy(Objeto.name,"caixa");
        Objeto.nameId = CAIXA3;
        sceneVector.push_back(Objeto);
    }

    //caixa 4
    if(moveCaixa4==1 && colisaoCaixa==0)
    {
        if (flagBezier==0)
        {
            auxBezier=glfwGetTime();
            flagBezier=1;
        }
        t = (glfwGetTime() - auxBezier)/2;
        t = sin(t);

        glm::vec4 p1  = glm::vec4(p_inicial_caixa4.x,       p_inicial_caixa4.y,         p_inicial_caixa4.z, 1.0f);
        glm::vec4 p2  = glm::vec4(p_inicial_caixa4.x+2.0f,  p_inicial_caixa4.y,         p_inicial_caixa4.z+6.0f, 1.0f);
        glm::vec4 p3  = glm::vec4(p_inicial_caixa4.x+5.0f,  p_inicial_caixa4.y+0.6f,    p_inicial_caixa4.z+15.0f, 1.0f);
        glm::vec4 p4  = glm::vec4(p_inicial_caixa4.x+6.5f,  p_inicial_caixa4.y+0.6f,    p_inicial_caixa4.z+21.0f, 1.0f);

        // polinomio de bernstein (achei mais facil escrever por extenso que usar a série)
        float b03 = pow( (1-t), 3);
        float b13 = 3*t * pow( (1-t),2);
        float b23 = 3*t*t * (1-t);
        float b33 = pow(t,3);
        glm::vec4 centro_caixa4  = b03*p1 + b13*p2 + b23*p3 + b33*p4;

        if (rotacaoCaixa4>-3.1415f)
            rotacaoCaixa4-=0.004f;

        Objeto.model = Matrix_Translate(centro_caixa4.x, centro_caixa4.y, centro_caixa4.z) * Matrix_Rotate_Y(3.1415f + rotacaoCaixa4);
        strcpy(Objeto.name,"caixa");
        Objeto.nameId = CAIXA4;
        sceneVector.push_back(Objeto);
        if (centro_caixa4.z >=1.4999f)
        {
            moveCaixa4=0;
            p_inicial_caixa4.x = -1.0f;
            p_inicial_caixa4.y = -0.4f;
            p_inicial_caixa4.z = 1.5f;
            rotacaoCaixa4=-3.1415f;
            auxBezier=0;
            flagBezier=0;
            isMoving=false;
        }
    }
    else if (moveCaixa4==1 && colisaoCaixa==1)
    {
        if (flagBezier==0)
        {
            posVetor--;
            PlaySoundA((LPCSTR) "..\\..\\data\\sounds\\collision.wav", NULL, SND_FILENAME | SND_ASYNC);
            auxBezier=glfwGetTime();
            flagBezier=2;
        }
        t = (glfwGetTime() - auxBezier)/2;
        t = sin(t);

        glm::vec4 p1  = glm::vec4(-6.478505f,       -0.959197f,         -16.429457f,    1.0f);
        glm::vec4 p2  = glm::vec4(-7.0f,            -0.98f,             -17.0f,         1.0f);
        glm::vec4 p3  = glm::vec4(-7.0f,            -1.0f,              -19.0f,         1.0f);
        glm::vec4 p4  = glm::vec4(p_inicial_caixa4.x,  p_inicial_caixa4.y,    p_inicial_caixa4.z, 1.0f);

        // polinomio de bernstein (achei mais facil escrever por extenso que usar a série)
        float b03 = pow( (1-t), 3);
        float b13 = 3*t * pow( (1-t),2);
        float b23 = 3*t*t * (1-t);
        float b33 = pow(t,3);
        glm::vec4 centro_caixa4  = b03*p1 + b13*p2 + b23*p3 + b33*p4;

        if (rotacaoCaixa4<0.272f)
            rotacaoCaixa4+=0.020f;

        Objeto.model = Matrix_Translate(centro_caixa4.x, centro_caixa4.y, centro_caixa4.z) * Matrix_Rotate_Y(3.1415f -0.272f + rotacaoCaixa4);
        strcpy(Objeto.name,"caixa");
        Objeto.nameId = CAIXA4;
        sceneVector.push_back(Objeto);
        if (centro_caixa4.z <=-19.4999f)
        {
            moveCaixa4=0;
            p_inicial_caixa4.x = -7.5f;
            p_inicial_caixa4.y = -1.0f;
            p_inicial_caixa4.z = -19.5f;
            rotacaoCaixa4=0;
            auxBezier=0;
            flagBezier=0;
            colisaoCaixa=0;
            isMoving=false;
        }

    }
    else if(moveCaixa4==2)
    {
        if (flagBezier==0)
        {
            auxBezier=glfwGetTime();
            flagBezier=1;
        }
        t = (glfwGetTime() - auxBezier)/2;
        t = sin(t);

        glm::vec4 p1  = glm::vec4(p_inicial_caixa4.x,       p_inicial_caixa4.y,         p_inicial_caixa4.z, 1.0f);
        glm::vec4 p2  = glm::vec4(p_inicial_caixa4.x-2.0f,  p_inicial_caixa4.y,         p_inicial_caixa4.z-6.0f, 1.0f);
        glm::vec4 p3  = glm::vec4(p_inicial_caixa4.x-5.0f,  p_inicial_caixa4.y-0.6f,    p_inicial_caixa4.z-15.0f, 1.0f);
        glm::vec4 p4  = glm::vec4(p_inicial_caixa4.x-6.5f,  p_inicial_caixa4.y-0.6f,    p_inicial_caixa4.z-21.0f, 1.0f);

        // polinomio de bernstein (achei mais facil escrever por extenso que usar a série)
        float b03 = pow( (1-t), 3);
        float b13 = 3*t * pow( (1-t),2);
        float b23 = 3*t*t * (1-t);
        float b33 = pow(t,3);
        glm::vec4 centro_caixa4  = b03*p1 + b13*p2 + b23*p3 + b33*p4;

        if (rotacaoCaixa4<3.1415f)
            rotacaoCaixa4+=0.0025f;

        Objeto.model = Matrix_Translate(centro_caixa4.x, centro_caixa4.y, centro_caixa4.z) * Matrix_Rotate_Y(3.1415f + rotacaoCaixa4);
        strcpy(Objeto.name,"caixa");
        Objeto.nameId = CAIXA4;
        sceneVector.push_back(Objeto);
        if (centro_caixa4.z <=-19.4999f)
        {
            moveCaixa4=-1;
            p_inicial_caixa4.x = -7.5f;
            p_inicial_caixa4.y = -1.0f;
            p_inicial_caixa4.z = -19.5f;
            rotacaoCaixa4=0;
        }
    }
    else
    {
        Objeto.model = Matrix_Translate(p_inicial_caixa4.x,p_inicial_caixa4.y,p_inicial_caixa4.z) * Matrix_Rotate_Y(3.1415f + rotacaoCaixa4);
        strcpy(Objeto.name,"caixa");
        Objeto.nameId = CAIXA4;
        sceneVector.push_back(Objeto);
    }

    Objeto.model = Matrix_Translate(p_inicial_caixa5.x, p_inicial_caixa5.y,p_inicial_caixa5.z) * Matrix_Rotate_Y(-1.57);
    strcpy(Objeto.name,"caixa");
    Objeto.nameId = CAIXA5;
    sceneVector.push_back(Objeto);



    // constrói o quarto 20x10 (todos quartos tem 4 de altura)
    buildRoom(10,20);

}


// testa a intersecção entre um raio e algum objeto
bool TestRayOBBIntersection(
    glm::vec3 ray_origin,        // Ray origin, in world space
    glm::vec3 ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
    glm::vec3 aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
    glm::vec3 aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
    glm::mat4 ModelMatrix,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
    float& intersection_distance // Output : distance between ray_origin and the intersection with the OBB
)
{
    // tMin is the largest “near” intersection currently found;
    // tMax is the smallest “far” intersection currently found.
    // Delta is used to compute the intersections with the planes.
    float tMin = std::numeric_limits<float>::min();
    float tMax = std::numeric_limits<float>::max();

    //aparentemente o indice do vetor é algo como 0 = x, 1 = y, 2 = z, 3 = x,y e z
    glm::vec3 OBBposition_worldspace(ModelMatrix[3].x, ModelMatrix[3].y, ModelMatrix[3].z);

    glm::vec3 delta = OBBposition_worldspace - ray_origin;



    {
        //teste para o x
        glm::vec3 xaxis(ModelMatrix[0].x, ModelMatrix[0].y, ModelMatrix[0].z);
        float e = glm::dot(xaxis, delta);
        float f = glm::dot(ray_direction, xaxis);
        if ( fabs(f) > 0.001f )
        {
            float t1 = (e+aabb_min.x)/f; // Intersection with the "left" plane
            float t2 = (e+aabb_max.x)/f; // Intersection with the "right" plane

            if (t1>t2)
            {
                float w=t1;
                t1=t2;
                t2=w; // swap t1 and t2
            }

            if ( t2 < tMax )
                tMax = t2; // tMax is the nearest "far" intersection amongst x planes

            if ( t1 > tMin )
                tMin = t1; // tMin is the farthest "near" intersection amongst x planes

            if (tMax < tMin )
                return false; // If "far" is closer than "near", then there is NO intersection.
        }
        else if(-e+aabb_min.x > 0.0f || -e+aabb_max.x < 0.0f)
            return false;

    }
    {
        //teste para o y
        glm::vec3 yaxis(ModelMatrix[1].x, ModelMatrix[1].y, ModelMatrix[1].z);
        float e = glm::dot(yaxis, delta);
        float f = glm::dot(ray_direction, yaxis);

        if ( fabs(f) > 0.001f )
        {
            float t1 = (e+aabb_min.y)/f; // Intersection with the "left" plane
            float t2 = (e+aabb_max.y)/f; // Intersection with the "right" plane

            if (t1>t2)
            {
                float w=t1;
                t1=t2;
                t2=w; // swap t1 and t2
            }

            if ( t2 < tMax )
                tMax = t2; // tMax is the nearest "far" intersection amongst y planes

            if ( t1 > tMin )
                tMin = t1; // tMin is the farthest "near" intersection amongst y planes

            if (tMax < tMin )
                return false; // If "far" is closer than "near", then there is NO intersection.
        }
        else if(-e+aabb_min.y > 0.0f || -e+aabb_max.y < 0.0f)
            return false;
    }

    {
        //teste para o z
        glm::vec3 zaxis(ModelMatrix[2].x, ModelMatrix[2].y, ModelMatrix[2].z);
        float e = glm::dot(zaxis, delta);
        float f = glm::dot(ray_direction, zaxis);

        if ( fabs(f) > 0.001f )
        {
            float t1 = (e+aabb_min.z)/f; // Intersection with the "left" plane
            float t2 = (e+aabb_max.z)/f; // Intersection with the "right" plane
            float w;
            if (t1>t2)
                w=t1;

            if (t1>t2)
            {
                w=t1;
                t1=t2;
                t2=w; // swap t1 and t2
            }

            if ( t2 < tMax )
                tMax = t2; // tMax is the nearest "far" intersection amongst z planes

            if ( t1 > tMin )
                tMin = t1; // tMin is the farthest "near" intersection amongst z planes

            if (tMax < tMin )
                return false; // If "far" is closer than "near", then there is NO intersection.
        }
        else if(-e+aabb_min.z > 0.0f || -e+aabb_max.z < 0.0f)
            return false;
    }
    intersection_distance = tMin;
    return true;


}

// Função que carrega uma imagem para ser utilizada como textura
void LoadTextureImage(const char* filename)
{
    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);

    if ( data == NULL )
    {
        fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
        exit(EXIT_FAILURE);
    }

    GLuint texture_id;
    GLuint sampler_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);

    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    GLuint textureunit = numOfLoadedTextures;
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindSampler(textureunit, sampler_id);

    stbi_image_free(data);

    numOfLoadedTextures += 1;
}

void movimentacaoCamera(glm::vec4 *vec)
{
    float g_FreeCamX=0;
    float g_FreeCamz=0;
    if(sPressed)
    {
        g_FreeCamX += 0.05f*cos(g_CameraPhi)*sin(g_CameraTheta);
        g_FreeCamz += 0.05f*cos(g_CameraPhi)*cos(g_CameraTheta);
    }
    if(wPressed)
    {
        g_FreeCamX += -0.05f*cos(g_CameraPhi)*sin(g_CameraTheta);
        g_FreeCamz += -0.05f*cos(g_CameraPhi)*cos(g_CameraTheta);
    }
    if(dPressed)
    {
        g_FreeCamz += -0.05f*cos(g_CameraPhi)*sin(g_CameraTheta);
        g_FreeCamX += 0.05f*cos(g_CameraPhi)*cos(g_CameraTheta);
    }
    if (aPressed)
    {
        g_FreeCamX += -0.05*cos(g_CameraPhi) * cos(g_CameraTheta);
        g_FreeCamz += 0.05*cos(g_CameraPhi) * sin(g_CameraTheta);
    }



    vec->x = g_FreeCamX;
    vec->y = 0;
    vec->z = g_FreeCamz;
    vec->w = 0.0f;
}

void lightSwitch(float x, float y, float z)
{
    if(teste_interruptor)
    {
        //variavel é true quando usuario solta botao direito
        teste_interruptor = false;
        //vetor ray_direction (sentido da camera)
        float norma_camera = sqrt( x*x + y*y + z*z );
        glm::vec3 ray_direction = glm::vec3(-x/norma_camera,-y/norma_camera,-z/norma_camera);

        // coordenadas minimas e máximas da esfera
        glm::vec4 aabb_min = glm::vec4(-1.0f,-1.0f,-1.0f,1.0f);
        glm::vec4 aabb_max = glm::vec4(1.0f,1.0f,1.0f, 1.0f);

        // transformações da esfera
        glm::mat4 target_model = sceneVector[4].model; // target_model é o objeto que vai ligar/desligar a luz aqui scenevetor[1] é o coelho

//        aabb_min = aabb_min * target_model;
//        aabb_max = aabb_min * target_model;

        float intersection_distance;
        //testa se tocou o botão
        if(TestRayOBBIntersection(
                    glm::vec3(camera_position_c.x,camera_position_c.y,camera_position_c.z),  // Ray origin = posição da câmera
                    ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
                    aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
                    aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
                    target_model,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
                    intersection_distance // Output : distance between ray_origin and the intersection with the OBB
                ))
        {
            printf("\nlol");

            if(interruptor==0&&intersection_distance<=3.5f)
            {
                interruptor=1;
                PlaySoundA((LPCSTR) "..\\..\\data\\sounds\\switch-on.wav", NULL, SND_FILENAME | SND_ASYNC);
                if(rand()%20 == 0)
                    PlaySoundA((LPCSTR) "..\\..\\data\\sounds\\ghast2.wav", NULL, SND_FILENAME | SND_ASYNC);

            }
            else if (intersection_distance<=3.5f)
            {
                interruptor=0;
                PlaySoundA((LPCSTR) "..\\..\\data\\sounds\\switch-off.wav", NULL, SND_FILENAME | SND_ASYNC);
            }

            //glUniform1i(interruptor_uniform,interruptor);

        }
    }
};

void bolaPapel(float x, float y, float z)
{
    if(teste_paper)
    {
        //variavel é true quando usuario solta botao direito
        teste_paper = false;
        //vetor ray_direction (sentido da camera)
        float norma_camera = sqrt( x*x + y*y + z*z );
        glm::vec3 ray_direction = glm::vec3(-x/norma_camera,-y/norma_camera,-z/norma_camera);

        // coordenadas minimas e máximas da esfera
        glm::vec4 aabb_min = glm::vec4(-1.0f,-1.0f,-1.0f,1.0f);
        glm::vec4 aabb_max = glm::vec4(1.0f,1.0f,1.0f,1.0f);

        // transformações da esfera
        glm::mat4 target_model = sceneVector[6].model; // target_model é o objeto que vai ligar/desligar a luz aqui scenevetor[1] é o coelho

        aabb_min = aabb_min * target_model;
        aabb_max = aabb_max * target_model;

        float intersection_distance;
        //testa se tocou o botão
        if(TestRayOBBIntersection(
                    glm::vec3(camera_position_c.x,camera_position_c.y,camera_position_c.z),  // Ray origin = posição da câmera
                    ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
                    aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
                    aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
                    target_model,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
                    intersection_distance // Output : distance between ray_origin and the intersection with the OBB
                ))
        {
            printf("\nA bola foi clicada \n");

            if(paper==0&&intersection_distance<=5.0f)
            {
                paper=1;
                PlaySoundA((LPCSTR) "..\\..\\data\\sounds\\paper_open.wav", NULL, SND_FILENAME | SND_ASYNC);
                if(rand()%20 == 0)
                    PlaySoundA((LPCSTR) "..\\..\\data\\sounds\\ghast2.wav", NULL, SND_FILENAME | SND_ASYNC);

            }
            else if (intersection_distance<=5.0f)
            {
                paper=0;
                //PlaySoundA((LPCSTR) "..\\..\\data\\sounds\\switch-off.wav", NULL, SND_FILENAME | SND_ASYNC);
            }

            //glUniform1i(interruptor_uniform,interruptor);

        }
    }

};

void abrePorta (float x, float y, float z)
{
    if(teste_door)
    {
        //variavel é true quando usuario solta botao direito
        teste_door = false;
        //vetor ray_direction (sentido da camera)
        float norma_camera = sqrt( x*x + y*y + z*z );
        glm::vec3 ray_direction = glm::vec3(-x/norma_camera,-y/norma_camera,-z/norma_camera);

        // coordenadas minimas e máximas da esfera
        glm::vec4 aabb_min = glm::vec4(-1.0f,-1.0f,-1.0f,1.0f);
        glm::vec4 aabb_max = glm::vec4(1.0f,1.0f,1.0f,1.0f);

        // transformações da esfera
        glm::mat4 target_model = sceneVector[1].model; // target_model é o objeto que vai ligar/desligar a luz aqui scenevetor[1] é o coelho

        aabb_min = aabb_min * target_model;
        aabb_max = aabb_max * target_model;

        float intersection_distance;
        //testa se tocou o botão
        if(TestRayOBBIntersection(
                    glm::vec3(camera_position_c.x,camera_position_c.y,camera_position_c.z),  // Ray origin = posição da câmera
                    ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
                    aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
                    aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
                    target_model,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
                    intersection_distance // Output : distance between ray_origin and the intersection with the OBB
                ))
        {
            printf("\nA porta foi clicada");
            if(door_locked==1&&intersection_distance<=5.0f)
            {
                PlaySoundA((LPCSTR) "..\\..\\data\\sounds\\locked-door.wav", NULL, SND_FILENAME | SND_ASYNC);
            }
            else if(intersection_distance<=5.0f) //!!!!!!!!!!!!!!!!PORTA DESTRANCADA, ACABOU
            {
                PlaySoundA((LPCSTR) "..\\..\\data\\sounds\\door-open.wav", NULL, SND_FILENAME | SND_ASYNC);
                sceneNumber=2;
            }
        }
    }
};

void testaCaixas(float x, float y, float z)
{
    if(teste_caixas)
    {
        float norma_camera = sqrt( x*x + y*y + z*z );
        glm::vec3 ray_direction = glm::vec3(-x/norma_camera,-y/norma_camera,-z/norma_camera);

        // coordenadas minimas e máximas da esfera
        glm::vec4 aabb_min = glm::vec4(-1.0f,-1.0f,-1.0f,1.0f);
        glm::vec4 aabb_max = glm::vec4(1.0f,1.0f,1.0f,1.0f);

        float intersection_distance;
        // transformações do obj

        glm::mat4 target_model = sceneVector[0].model; //testa zumbi
        aabb_min = aabb_min * target_model;
        aabb_max = aabb_max * target_model;

        if(TestRayOBBIntersection(
                    glm::vec3(camera_position_c.x,camera_position_c.y,camera_position_c.z),  // Ray origin = posição da câmera
                    ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
                    aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
                    aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
                    target_model,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
                    intersection_distance // Output : distance between ray_origin and the intersection with the OBB
                ))
        {
            if (moveZumbi ==0 && intersection_distance<=5.0f && isMoving==false)
            {
                moveZumbi=1;
                isMoving=true;
                PlaySoundA((LPCSTR) "..\\..\\data\\sounds\\zombie.wav", NULL, SND_FILENAME | SND_ASYNC);
            }
            else if (moveZumbi ==-1 && intersection_distance<=5.0f && isMoving==false)
            {
                moveZumbi=2;
                isMoving=true;
                PlaySoundA((LPCSTR) "..\\..\\data\\sounds\\zombie.wav", NULL, SND_FILENAME | SND_ASYNC);
            }
        }

        target_model = sceneVector[8].model; //testa primeira caixa
        aabb_min = aabb_min * target_model;
        aabb_max = aabb_max * target_model;

        if(TestRayOBBIntersection(
                    glm::vec3(camera_position_c.x,camera_position_c.y,camera_position_c.z),  // Ray origin = posição da câmera
                    ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
                    aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
                    aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
                    target_model,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
                    intersection_distance // Output : distance between ray_origin and the intersection with the OBB
                ))
        {
            if (moveCaixa1 ==0 && intersection_distance<=5.0f && isMoving==false)
            {
                moveCaixa1=1;
                isMoving=true;
                ordem[posVetor]=1;
                posVetor++;
            }
        }

        target_model = sceneVector[9].model; //testa segunda caixa
        aabb_min = aabb_min * target_model;
        aabb_max = aabb_max * target_model;

        if(TestRayOBBIntersection(
                    glm::vec3(camera_position_c.x,camera_position_c.y,camera_position_c.z),  // Ray origin = posição da câmera
                    ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
                    aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
                    aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
                    target_model,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
                    intersection_distance // Output : distance between ray_origin and the intersection with the OBB
                ))
        {
            if (moveCaixa2 ==0 && intersection_distance<=5.0f && isMoving==false)
            {
                moveCaixa2=1;
                isMoving=true;
                ordem[posVetor]=2;
                posVetor++;
            }
        }

        target_model = sceneVector[10].model; //testa terceira caixa
        aabb_min = aabb_min * target_model;
        aabb_max = aabb_max * target_model;

        if(TestRayOBBIntersection(
                    glm::vec3(camera_position_c.x,camera_position_c.y,camera_position_c.z),  // Ray origin = posição da câmera
                    ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
                    aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
                    aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
                    target_model,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
                    intersection_distance // Output : distance between ray_origin and the intersection with the OBB
                ))
        {
            if (moveCaixa3 ==0 && intersection_distance<=5.0f && isMoving==false)
            {
                moveCaixa3=1;
                isMoving=true;
                ordem[posVetor]=3;
                posVetor++;
            }
        }

        target_model = sceneVector[11].model; //testa quarta caixa
        aabb_min = aabb_min * target_model;
        aabb_max = aabb_max * target_model;

        if(TestRayOBBIntersection(
                    glm::vec3(camera_position_c.x,camera_position_c.y,camera_position_c.z),  // Ray origin = posição da câmera
                    ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
                    aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
                    aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
                    target_model,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
                    intersection_distance // Output : distance between ray_origin and the intersection with the OBB
                ))
        {
            if (moveCaixa4 ==0 && intersection_distance<=5.0f && isMoving==false)
            {
                moveCaixa4=1;
                isMoving=true;
                ordem[posVetor]=4;
                posVetor++;
            }
        }
        teste_caixas=false;
    }
};

void buildRoom(float largura, float comprimento)
{

    struct sceneHelper Objeto;


    //formato longo do corredor
    Objeto.model = Matrix_Scale(largura,1.0f,comprimento);
    strcpy(Objeto.name, "plane");
    Objeto.nameId = PLANE;
    PushMatrix(Objeto.model);

    // CHÃO
    Objeto.model = Objeto.model * Matrix_Translate(0.0f,-1.0f,0);
    Objeto.nameId = CHAO;
    sceneVector.push_back(Objeto);


    PopMatrix(Objeto.model);
    PushMatrix(Objeto.model);

    // TETO
    Objeto.model = Objeto.model * Matrix_Translate(0,7.0f,0) * Matrix_Rotate_X(3.14159f);
    Objeto.nameId = TETO;
    sceneVector.push_back(Objeto);


    PopMatrix(Objeto.model);
    PushMatrix(Objeto.model);

    // parede da frente do zumbi
    Objeto.model = Objeto.model * Matrix_Rotate_X(-1.5708f) * Matrix_Translate(0,-1.0f,3.0f) * Matrix_Scale(1.0f,1.0f,4.0f);
    Objeto.nameId = PLANE;
    sceneVector.push_back(Objeto);

    PopMatrix(Objeto.model);
    PushMatrix(Objeto.model);

    // parede atrás do zumbi
    Objeto.model = Objeto.model * Matrix_Rotate_X(1.5708f) * Matrix_Rotate_Y(3.14159f) * Matrix_Translate(0,-1.0f,3.0f) * Matrix_Scale(1.0f,1.0f,4.0f);
    Objeto.nameId = PLANE;
    sceneVector.push_back(Objeto);



    PopMatrix(Objeto.model);
    PushMatrix(Objeto.model);

    Objeto.model = Objeto.model * Matrix_Rotate_Z(1.5708f) * Matrix_Rotate_Y(1.5708f) * Matrix_Translate(0,-1.0f,3.0f) * Matrix_Scale(1.0f,1.0f,4.0f);
    Objeto.nameId = PLANE;
    sceneVector.push_back(Objeto);


    PopMatrix(Objeto.model);
    PushMatrix(Objeto.model);

    Objeto.model = Objeto.model * Matrix_Rotate_Z(-1.5708f) *Matrix_Rotate_Y(-1.5708f)* Matrix_Translate(0,-1.0f,3.0f) * Matrix_Scale(1.0f,1.0f,4.0f);
    Objeto.nameId = PLANE;
    sceneVector.push_back(Objeto);

}

void testa_ordem()
{
    if (ordem[0] ==1 && ordem[1]==2 &&ordem[2]==3 && ordem[3]==4)
    {
        PlaySoundA((LPCSTR) "..\\..\\data\\sounds\\unlocked-door.wav", NULL, SND_FILENAME | SND_ASYNC);
        door_locked=0;
    }
    else
    {
        PlaySoundA((LPCSTR) "..\\..\\data\\sounds\\wrong.wav", NULL, SND_FILENAME | SND_ASYNC);
        moveCaixa1=2;
        moveCaixa2=2;
        moveCaixa3=2;
        moveCaixa4=2;
    }
}


bool collisionCheckPointBox(glm::vec4 point, glm::vec4 bboxMin, glm::vec4 bboxMax)
{
    if (point.x >= bboxMin.x && point.x <= bboxMax.x)
        if(point.z >= bboxMin.z && point.z <= bboxMax.z)
            return true;
    return false;
}

bool collisionCheckBoxBox(glm::vec4 bbox1_Min, glm::vec4 bbox1_Max, glm::vec4 bbox2_Min, glm::vec4 bbox2_Max)
{
    if (bbox1_Min.x <= bbox2_Max.x && bbox1_Max.x >= bbox2_Min.x)
        if (bbox1_Min.z <= bbox2_Max.z && bbox1_Max.z >= bbox2_Min.z)
            return true;
    return false;
}

