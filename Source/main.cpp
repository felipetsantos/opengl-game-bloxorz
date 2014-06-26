
/****************************************
 *  I N C L U D E S
 ****************************************/
#include "bibutil.h"
#include <list>
#include <vector>
#include <fstream>
#include <sstream>
#include <string.h>
#include <time.h>
#include "FrameTimer.h"

/****************************************
 *  D E F I N E S
 ****************************************/
#define SENS_ROT	5.0
#define SENS_OBS	15.0
#define SENS_TRANSL	20.0

#define MAX_TILE_NAME_SIZE  64U

#define TILE_MATRIX_SIZE_X  20U
#define TILE_MATRIX_SIZE_Y  20U

#define NORMAL_TILE_OBJ_FILE    "Objects/chao_normal.obj"

#define LEVEL_1_MAP "Maps/level1.txt"
#define LEVEL_2_MAP "Maps/level2.txt"
#define LEVEL_3_MAP "Maps/level3.txt"
#define LEVEL_4_MAP "Maps/level4.txt"

/****************************************
 *  N A M E S P A C E S
 ****************************************/
using namespace std;

/****************************************
 *  T Y P E D E F S
 ****************************************/
typedef enum
{
    TILE_TYPE_NONE,
    TILE_TYPE_NORMAL,
    TILE_TYPE_INIT,
    TILE_TYPE_END,
    NUM_OF_TILE_TYPES
} TileTypeEnum;

typedef enum
{
    BLOXOR_POS_EM_PE,
    BLOXOR_POS_DEITADO,
    NUM_OF_BLOXOR_POS
} BloxorPosEnum;

typedef enum
{
    BLOXOR_ORIENTATION_NONE,
    BLOXOR_ORIENTATION_LEFT_RIGHT,
    BLOXOR_ORIENTATION_UP_DOWN,
    NUM_OF_BLOXOR_ORIENTATION
} BloxorOrientationEnum;

typedef struct
{
    TileTypeEnum eTileType;
    GLfloat fPositionX;
    GLfloat fPositionY;
    GLfloat fPositionZ;
} TileStruct;

typedef enum
{
    GAME_STATUS_NORMAL,
    GAME_STATUS_DEATH,
    GAME_STATUS_VICTORY,
    NUM_OF_GAME_STATUS
} GameStatusEnum;

typedef enum
{
    LEVEL_1,
    LEVEL_2,
    LEVEL_3,
    LEVEL_4,
    NUM_OF_LEVELS
} LevelEnum;

/****************************************
 *  G L O B A L S
 ****************************************/
// Variáveis para controles de navegação
GLfloat fAngle, fAspect;
GLfloat fRotacaoX, fRotacaoY, fRotacaoX_Ini, fRotacaoY_Ini;
GLfloat fObservadorX, fObservadorY, fObservadorZ, fObservadorX_Ini, fObservadorY_Ini, fObservadorZ_Ini;

/* Variaveis controle do bloxor*/
GLfloat fBloxorX, fBloxorY, fBloxorZ;
BloxorPosEnum eBloxorPosition = BLOXOR_POS_EM_PE;
BloxorOrientationEnum eBloxorOrientation = BLOXOR_ORIENTATION_NONE;

LevelEnum eCurrentLevel = LEVEL_1;

bool bMorte = false;

int iMouseX_Ini, iMouseY_Ini, iMouseButton;

// Apontador para o objeto
OBJ *tileNormalObj;
OBJ *bloxorObj;

vector<TileStruct> listaTiles;

struct Particula {

	/// Posição corrente da partícula
	float posicao[3];

	/// Direção da partícula
	float direcao[3];

	/// Tempo de vida
	float vida;

	/// constructor
	Particula() {

		// set posição
		posicao[0] = fBloxorX;
		posicao[1] = fBloxorY-0.50;
		posicao[2] = fBloxorZ;

		// cria uma direção randômica
		direcao[0] = (10000 - rand()%20000)/10000.0f;
		direcao[1] = (10000 - rand()%20000)/10000.0f;
		direcao[2] = (10000 - rand()%20000)/10000.0f;

		// set um tempo de vida ranômico
		vida         = rand()%10000/9500.0f;
	}

	/// ponteiro para próxima particula
	Particula* proxima;
};

Particula* pLista;
/****************************************
 * F U N C T I O N S
 ****************************************/

/****************************************
 * Função: T2_AtualizaParticula
 ****************************************/
void T2_AtualizaParticula(float dt) {

	// traverse all particles and update
	Particula* p = pLista;
	while(p) {
		// diminui o tempo de vida da particula
		p->vida -= dt/1.2;

		// aplica a gravidade
		if(rand()%2 == 0){
            p->direcao[1] += 9.81f*dt;
		}else{
		    p->direcao[1] -= 9.81f*dt;
		}

		// modifica a posicão
		p->posicao[0] += dt * p->direcao[0];
		p->posicao[1] += dt * p->direcao[1];
		p->posicao[2] += dt * p->direcao[2];

		// vai par a próxima partícula
		p=p->proxima;
	}
}

/****************************************
 * Função: T2_NovaParticula
 ****************************************/
void T2_NovaParticula() {

	// cria uma nova particula e coloca como primeira da lista
	Particula* p = new Particula;
	p->proxima = pLista;
	pLista = p;
}

/****************************************
 * Função: T2_RemoveParticulasMortas
 ****************************************/
void T2_RemoveParticulasMortas() {

	// ponteiros para percorrer as particulas
	Particula* curr = pLista;
	Particula* prev = 0;

	while (curr) {

		// particula morta
		if (curr->vida<0) {

			// remove a partícula, da lista.
			if (prev) {
				prev->proxima = curr->proxima;
			}
			else {
				pLista = curr->proxima;
			}

			// refenrecia temporária
			Particula* temp = curr;

			// transforma a particula corrente na próxima
			curr = curr->proxima;

			// deleta a antiga particula corrente
			delete temp;
		}
		else {
			// vai para a próxima particula
			prev = curr;
			curr = curr->proxima;
		}
	}
}


/****************************************
 * Função: T2_RemoveParticulas
 ****************************************/
void T2_RemoveParticulas() {

	// ponteiros para percorrer as particulas
	Particula* curr = pLista;
	Particula* prev = 0;

	while (curr) {
			// refenrecia temporária
			Particula* temp = curr;
			// transforma a particula corrente na próxima
			curr = curr->proxima;
			// deleta a antiga particula corrente
			delete temp;
	}
}



/****************************************
 * Função: T2_DesenhaParticulas
 ****************************************/
void DesenhaParticulas() {

	// iterate over all particles and draw a point
	Particula* curr = pLista;

	glPointSize(2);
	glBegin(GL_POINTS);
	while (curr) {
		glVertex3fv(curr->posicao);
		curr = curr->proxima;
	}
	glEnd();
}

/****************************************
 * Função: T2_LeArquivoLevel
 ****************************************/
void T2_LeArquivoLevel(char* pArquivo)
{
    ifstream stArquivoLevel;

    /* Limpa lista */
    listaTiles.clear();

    /* Abre arquivo de level */
    stArquivoLevel.open(pArquivo, std::ifstream::in);

    /* Verifica se conseguiu abrir */
    if(stArquivoLevel.is_open())
    {
        string sLine;
        int iLineCount = 0;

        /* Itera sobre linhas do arquivo */
        while(getline(stArquivoLevel, sLine))
        {
            /* Variaveis para parsing de parametros */
            string sValue;
            istringstream iss(sLine);
            int iParamCount = 0;

            /* Itera sobre parametros da linha */
            while (iss >> sValue)
            {
                TileStruct stTileToAdd;

                memset(&stTileToAdd, 0, sizeof(TileStruct));

                /* NONE */
                if(sValue == "NONE")
                {
                    stTileToAdd.eTileType = TILE_TYPE_NONE;
                }
                /* NORMAL */
                else if(sValue == "NORMAL")
                {
                    stTileToAdd.eTileType = TILE_TYPE_NORMAL;
                }
                else if(sValue == "INIT")
                {
                    stTileToAdd.eTileType = TILE_TYPE_INIT;
                    fBloxorX = (GLfloat)iParamCount;
                    fBloxorY = 1.125f;
                    fBloxorZ = (GLfloat)iLineCount;
                }
                else if(sValue == "END")
                {
                    stTileToAdd.eTileType = TILE_TYPE_END;
                }
                else
                {
                    /* Nao deve entrar nunca aqui!!! Caso entrar, ha erros no arquivo lido */
                    cout << "Valor nao esperado no arquivo de level : " << sValue << endl;
                }

                /* Seta posicao */
                stTileToAdd.fPositionX = (GLfloat)iParamCount;
                stTileToAdd.fPositionY = 0.0f;
                stTileToAdd.fPositionZ = (GLfloat)iLineCount;

                /* Adiciona na lista */
                listaTiles.push_back(stTileToAdd);

                /* Conta parametro */
                iParamCount++;
            }

            /* Conta nova linha */
            iLineCount++;
        }
    }

    stArquivoLevel.close();

    if(eBloxorPosition == BLOXOR_POS_DEITADO)
    {
        eBloxorPosition = BLOXOR_POS_EM_PE;

        if(eBloxorOrientation == BLOXOR_ORIENTATION_LEFT_RIGHT)
        {
            /* Rotaciona objeto */
            MESH* pMeshAux = bloxorObj->meshes.at(0);
            vector<VERT>::iterator it;

            for(it = pMeshAux->vertices.begin(); it != pMeshAux->vertices.end(); it++)
            {
                VERT stCurrentVertex = (VERT)*it;
                VERT stOutputVertex = VERT(0.0f, 0.0f, 0.0f);

                RotaZ(stCurrentVertex, stOutputVertex, 90.0f);
                *it = stOutputVertex;
            }
        }
        else if(eBloxorOrientation == BLOXOR_ORIENTATION_UP_DOWN)
        {
            /* Rotaciona objeto */
            MESH* pMeshAux = bloxorObj->meshes.at(0);
            vector<VERT>::iterator it;

            for(it = pMeshAux->vertices.begin(); it != pMeshAux->vertices.end(); it++)
            {
                VERT stCurrentVertex = (VERT)*it;
                VERT stOutputVertex = VERT(0.0f, 0.0f, 0.0f);

                RotaX(stCurrentVertex, stOutputVertex, 90.0f);
                *it = stOutputVertex;
            }
        }
    }
}

/****************************************
 * Função: ConvertSingleTileTypeToGameStatus
 ****************************************/
void T2_CarregaLevel(LevelEnum eLevel)
{
    switch(eLevel)
    {
        case LEVEL_1:
            T2_LeArquivoLevel(LEVEL_1_MAP);
            break;
        case LEVEL_2:
            T2_LeArquivoLevel(LEVEL_2_MAP);
            break;
        case LEVEL_3:
            T2_LeArquivoLevel(LEVEL_3_MAP);
            break;
        case LEVEL_4:
            T2_LeArquivoLevel(LEVEL_4_MAP);
            break;
        default:
            T2_LeArquivoLevel(LEVEL_1_MAP);
            break;
    }
}

/****************************************
 * Função: ConvertSingleTileTypeToGameStatus
 ****************************************/
GameStatusEnum ConvertSingleTileTypeToGameStatus(TileTypeEnum eTileType)
{
    GameStatusEnum eStatusOut = GAME_STATUS_DEATH;

    switch(eTileType)
    {
        case TILE_TYPE_NONE:
            eStatusOut = GAME_STATUS_DEATH;
            break;
        case TILE_TYPE_NORMAL:
        case TILE_TYPE_INIT:
            eStatusOut = GAME_STATUS_NORMAL;
            break;
        case TILE_TYPE_END:
                eStatusOut = GAME_STATUS_VICTORY;
            break;
        default:
            break;
    }

    return eStatusOut;
}

/****************************************
 * Função: ConvertSingleTileTypeToGameStatus
 ****************************************/
GameStatusEnum ConvertTwoTileTypeToGameStatus(TileTypeEnum eTileType1, TileTypeEnum eTileType2)
{
    GameStatusEnum eStatusOut = GAME_STATUS_DEATH;

    if((eTileType1 == TILE_TYPE_NONE) || (eTileType2 == TILE_TYPE_NONE))
    {
        eStatusOut = GAME_STATUS_DEATH;
    }
    else
    {
        eStatusOut = GAME_STATUS_NORMAL;
    }

    return eStatusOut;
}

/****************************************
 * Função: T2_VerificaStatus
 ****************************************/
GameStatusEnum T2_VerificaStatus()
{
    GameStatusEnum eGameStatus = GAME_STATUS_DEATH;

    bool bFoundPosTile = false;
    bool bFoundNegTile = false;

    TileTypeEnum ePosTileType = TILE_TYPE_NONE;
    TileTypeEnum eNegTileType = TILE_TYPE_NONE;

    vector<TileStruct>::iterator it;

    for(it = listaTiles.begin(); it != listaTiles.end(); it++)
    {
        TileStruct stTile = (TileStruct)*it;

        if(eBloxorPosition == BLOXOR_POS_EM_PE)
        {
            if(stTile.fPositionX == fBloxorX && stTile.fPositionZ == fBloxorZ)
            {
                eGameStatus = ConvertSingleTileTypeToGameStatus(stTile.eTileType);
                break;
            }
        }
        else
        {
            /* Verifica orientacao para determinar X e Z como variavel ou fixo */
            if(eBloxorOrientation == BLOXOR_ORIENTATION_LEFT_RIGHT)
            {
                /* Z FIXO, X VARIAVEL */
                if(stTile.fPositionZ == fBloxorZ)
                {
                    if(stTile.fPositionX == fBloxorX + 0.5f)
                    {
                        bFoundPosTile = true;
                        ePosTileType = stTile.eTileType;
                    }
                    else if(stTile.fPositionX == fBloxorX - 0.5f)
                    {
                        bFoundNegTile = true;
                        eNegTileType = stTile.eTileType;
                    }
                }
            }
            else
            {
                /* X FIXO, Z VARIAVEL */
                if(stTile.fPositionX == fBloxorX)
                {
                    if(stTile.fPositionZ == fBloxorZ + 0.5f)
                    {
                        bFoundPosTile = true;
                        ePosTileType = stTile.eTileType;
                    }
                    else if(stTile.fPositionZ == fBloxorZ - 0.5f)
                    {
                        bFoundNegTile = true;
                        eNegTileType = stTile.eTileType;
                    }
                }
            }

            /* Found both tiles */
            if(bFoundNegTile && bFoundPosTile)
            {
                eGameStatus = ConvertTwoTileTypeToGameStatus(ePosTileType, eNegTileType);
                break;
            }
        }
    }

    return eGameStatus;
}

/****************************************
 * Função: T2_DesenhaTile
 ****************************************/
void T2_DesenhaTile(OBJ* pObj, TileStruct stTile)
{
    /* Move até posicao onde deve ser desenhada (considerando posicao inicial) */
    glTranslatef(stTile.fPositionX, stTile.fPositionY, stTile.fPositionZ);

    /* Desenha tile */
    DesenhaObjeto(pObj);

    /* Volta para posicao inicial para mover para proximo tile */
    glTranslatef(-stTile.fPositionX, -stTile.fPositionY, -stTile.fPositionZ);

}

/****************************************
 * Função: T2_DesenhaChao
 ****************************************/
void T2_DesenhaChao()
{
    vector<TileStruct>::iterator it;

    for(it = listaTiles.begin(); it != listaTiles.end(); it++)
    {
        TileStruct stTile = (TileStruct)*it;

        if(stTile.eTileType == TILE_TYPE_NORMAL)
        {
            /* Coloca cor como branca */
            glColor3f(1.0f, 1.0f, 1.0f);

            /* Desenha tile */
            T2_DesenhaTile(tileNormalObj, stTile);

        }
        else if(stTile.eTileType == TILE_TYPE_INIT)
        {
            /* Coloca cor como amarela */
            glColor3f(1.0f, 1.0f, 0.0f);

            /* Desenha tile */
            T2_DesenhaTile(tileNormalObj, stTile);

        }
        else if(stTile.eTileType == TILE_TYPE_END)
        {
            /* Coloca cor como azul */
            glColor3f(0.0f, 0.0f, 1.0f);

            /* Desenha tile */
            T2_DesenhaTile(tileNormalObj, stTile);
        }

    }
}

/****************************************
 * Função: T2_Desenha
 ****************************************/
void T2_Desenha(void)
{
    GameStatusEnum eGameStatus;
    static clock_t inicio = 0;
    static clock_t fim = 0;

	// Limpa a janela de visualização com a cor
	// de fundo definida previamente
	glClear(GL_COLOR_BUFFER_BIT);

	glLineWidth(3);

	// Desenha o wireframe do objeto 3D lido do arquivo com a cor corrente
	T2_DesenhaChao();

    if(!bMorte)
    {
        /* Seta cor de desenho do bloxor como verde */
        glColor3f(0.0f, 1.0f, 0.0f);
        glTranslatef(fBloxorX, fBloxorY, fBloxorZ);

        DesenhaObjeto(bloxorObj);

        eGameStatus = T2_VerificaStatus();

        switch(eGameStatus)
        {
            case GAME_STATUS_DEATH:
                bMorte = true;
                inicio = clock();
                cout << "DEATH!!!" << endl;
                break;
            case GAME_STATUS_VICTORY:
                eCurrentLevel = (LevelEnum)((int)eCurrentLevel + 1);
                T2_CarregaLevel(eCurrentLevel);
                cout << "VICTORY!!!" << endl;
                break;
            default:
                break;
        }
    }
    else
    {
        glColor3f(1.0f, 0.0f, 0.0f);
        DesenhaParticulas();
        fim = clock();
        if((fim / CLOCKS_PER_SEC) - (inicio / CLOCKS_PER_SEC) > 2)
        {
            bMorte = false;
            T2_CarregaLevel(eCurrentLevel);
        }
    }

	// Executa os comandos OpenGL
	glutSwapBuffers();
}

/****************************************
 * Função: T2_PosicionaObservador
 ****************************************/
void T2_PosicionaObservador(void)
{
	// Especifica sistema de coordenadas do modelo
	glMatrixMode(GL_MODELVIEW);
	// Inicializa sistema de coordenadas do modelo
	glLoadIdentity();
	// Posiciona e orienta o observador
	glTranslatef(-fObservadorX,-fObservadorY,-fObservadorZ);
	glRotatef(fRotacaoX,1,0,0);
	glRotatef(fRotacaoY,0,1,0);
}


/****************************************
 * Função: T2_EspecificaParametrosVisualizacao
 ****************************************/
void T2_EspecificaParametrosVisualizacao(void)
{
	// Especifica sistema de coordenadas de projeção
	glMatrixMode(GL_PROJECTION);
	// Inicializa sistema de coordenadas de projeção
	glLoadIdentity();

	// Especifica a projeção perspectiva(angulo,aspecto,zMin,zMax)
	gluPerspective(fAngle,fAspect,0.1,1200);

	T2_PosicionaObservador();
}

/****************************************
 * Função: T2_AlteraTamanhoJanela
 ****************************************/
void T2_AlteraTamanhoJanela(GLsizei iWidth, GLsizei iHeight)
{
	// Para previnir uma divisão por zero
	if(iHeight == 0)
    {
        iHeight = 1;
    }

	// Especifica as dimensões da viewport
	glViewport(0, 0, iWidth, iHeight);

	// Calcula a correção de aspecto
	fAspect = (GLfloat)iWidth/(GLfloat)iHeight;

	T2_EspecificaParametrosVisualizacao();
}

/****************************************
 * Função: T2_Teclado
 ****************************************/
void T2_Teclado(unsigned char tecla, int x, int y)
{
    switch(tecla)
    {
        /* ESC */
        case 27:
        {
            // Libera memória e finaliza programa
            LiberaObjeto(tileNormalObj);
            exit(0);
            break;
        }
        default:
        {
            break;
        }
    }
}

/****************************************
 * Função: T2_TeclasEspeciais
 ****************************************/
void T2_TeclasEspeciais (int tecla, int x, int y)
{
    bool bRotaciona = false;
    if(bMorte == false){
        switch (tecla)
        {
            case GLUT_KEY_HOME:
            {
                if(fAngle>=10)
                {
                    fAngle -=5;
                }
                break;
            }
            case GLUT_KEY_END:
            {
                if(fAngle<=150)
                {
                    fAngle +=5;
                }
                break;
            }
            case GLUT_KEY_LEFT:
            {
                if(eBloxorPosition == BLOXOR_POS_EM_PE)
                {
                    /* MOVE, SETA DEITADO, AJUSTA ORIENTACAO E ROTACIONA */
                    eBloxorPosition = BLOXOR_POS_DEITADO;
                    fBloxorY -= 0.5f;
                    fBloxorX -= 1.5f;
                    eBloxorOrientation = BLOXOR_ORIENTATION_LEFT_RIGHT;
                    bRotaciona = true;
                }
                else
                {
                    if(eBloxorOrientation == BLOXOR_ORIENTATION_LEFT_RIGHT)
                    {
                        /* ROTACIONA, MOVE E SETA EM PE*/
                        eBloxorPosition = BLOXOR_POS_EM_PE;
                        fBloxorX -= 1.5f;
                        fBloxorY += 0.5f;
                        bRotaciona = true;
                    }
                    else
                    {
                        /* MOVE E MANTEM DEITADO*/
                        fBloxorX -= 1.0f;
                    }
                }

                if(bRotaciona)
                {
                    /* Rotaciona objeto */
                    MESH* pMeshAux = bloxorObj->meshes.at(0);
                    vector<VERT>::iterator it;

                    for(it = pMeshAux->vertices.begin(); it != pMeshAux->vertices.end(); it++)
                    {
                        VERT stCurrentVertex = (VERT)*it;
                        VERT stOutputVertex = VERT(0.0f, 0.0f, 0.0f);

                        RotaZ(stCurrentVertex, stOutputVertex, 90.0f);
                        *it = stOutputVertex;
                    }
                }
                break;
            }
            case GLUT_KEY_RIGHT:
            {
                if(eBloxorPosition == BLOXOR_POS_EM_PE)
                {
                    /* MOVE, SETA DEITADO, AJUSTA ORIENTACAO E ROTACIONA */
                    eBloxorPosition = BLOXOR_POS_DEITADO;
                    fBloxorY -= 0.5f;
                    fBloxorX += 1.5f;
                    eBloxorOrientation = BLOXOR_ORIENTATION_LEFT_RIGHT;
                    bRotaciona = true;
                }
                else
                {
                    if(eBloxorOrientation == BLOXOR_ORIENTATION_LEFT_RIGHT)
                    {
                        /* ROTACIONA, MOVE E SETA EM PE*/
                        eBloxorPosition = BLOXOR_POS_EM_PE;
                        fBloxorX += 1.5f;
                        fBloxorY += 0.5f;
                        bRotaciona = true;
                    }
                    else
                    {
                        /* MOVE E MANTEM DEITADO */
                        fBloxorX += 1.0f;
                    }
                }

                if(bRotaciona)
                {
                    /* Rotaciona objeto */
                    MESH* pMeshAux = bloxorObj->meshes.at(0);
                    vector<VERT>::iterator it;

                    for(it = pMeshAux->vertices.begin(); it != pMeshAux->vertices.end(); it++)
                    {
                        VERT stCurrentVertex = (VERT)*it;
                        VERT stOutputVertex = VERT(0.0f, 0.0f, 0.0f);

                        RotaZ(stCurrentVertex, stOutputVertex, 90.0f);
                        *it = stOutputVertex;
                    }
                }

                break;
            }
            case GLUT_KEY_UP:
            {
                if(eBloxorPosition == BLOXOR_POS_EM_PE)
                {
                    /* MOVE, SETA DEITADO, AJUSTA ORIENTACAO E ROTACIONA */
                    eBloxorPosition = BLOXOR_POS_DEITADO;
                    fBloxorZ -= 1.5f;
                    fBloxorY -= 0.5f;
                    eBloxorOrientation = BLOXOR_ORIENTATION_UP_DOWN;
                    bRotaciona = true;
                }
                else
                {
                    if(eBloxorOrientation == BLOXOR_ORIENTATION_UP_DOWN)
                    {
                        /* ROTACIONA, MOVE E SETA EM PE*/
                        eBloxorPosition = BLOXOR_POS_EM_PE;
                        fBloxorZ -= 1.5f;
                        fBloxorY += 0.5f;
                        bRotaciona = true;
                    }
                    else
                    {
                        /* MOVE E MANTEM DEITADO*/
                        fBloxorZ -= 1.0f;
                    }
                }

                if(bRotaciona)
                {
                    /* Rotaciona objeto */
                    MESH* pMeshAux = bloxorObj->meshes.at(0);
                    vector<VERT>::iterator it;

                    for(it = pMeshAux->vertices.begin(); it != pMeshAux->vertices.end(); it++)
                    {
                        VERT stCurrentVertex = (VERT)*it;
                        VERT stOutputVertex = VERT(0.0f, 0.0f, 0.0f);

                        RotaX(stCurrentVertex, stOutputVertex, 90.0f);
                        *it = stOutputVertex;
                    }
                }
                break;
            }
            case GLUT_KEY_DOWN:
            {
                if(eBloxorPosition == BLOXOR_POS_EM_PE)
                {
                    /* MOVE, SETA DEITADO, AJUSTA ORIENTACAO E ROTACIONA */
                    eBloxorPosition = BLOXOR_POS_DEITADO;
                    fBloxorZ += 1.5f;
                    fBloxorY -= 0.5f;
                    eBloxorOrientation = BLOXOR_ORIENTATION_UP_DOWN;
                    bRotaciona = true;
                }
                else
                {
                    if(eBloxorOrientation == BLOXOR_ORIENTATION_UP_DOWN)
                    {
                        /* ROTACIONA, MOVE E SETA EM PE*/
                        eBloxorPosition = BLOXOR_POS_EM_PE;
                        fBloxorZ += 1.5f;
                        fBloxorY += 0.5f;
                        bRotaciona = true;
                    }
                    else
                    {
                        /* MOVE E MANTEM DEITADO*/
                        fBloxorZ += 1.0f;
                    }
                }

                if(bRotaciona)
                {
                    /* Rotaciona objeto */
                    MESH* pMeshAux = bloxorObj->meshes.at(0);
                    vector<VERT>::iterator it;

                    for(it = pMeshAux->vertices.begin(); it != pMeshAux->vertices.end(); it++)
                    {
                        VERT stCurrentVertex = (VERT)*it;
                        VERT stOutputVertex = VERT(0.0f, 0.0f, 0.0f);

                        RotaX(stCurrentVertex, stOutputVertex, 90.0f);
                        *it = stOutputVertex;
                    }
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
	T2_PosicionaObservador();
	glutPostRedisplay();
}

/****************************************
 * Função: T2_GerenciaMouse
 ****************************************/
void T2_GerenciaMouse(int button, int state, int x, int y)
{
	if(state==GLUT_DOWN)
	{
		// Salva os parâmetros atuais
		iMouseX_Ini = x;
		iMouseY_Ini = y;
		fObservadorX_Ini = fObservadorX;
		fObservadorY_Ini = fObservadorY;
		fObservadorZ_Ini = fObservadorZ;
		fRotacaoX_Ini = fRotacaoX;
		fRotacaoY_Ini = fRotacaoY;
		iMouseButton = button;
	}
	else
	{
        iMouseButton = -1;
    }
}

/****************************************
 * Função: T2_GerenciaMovim
 ****************************************/
void T2_GerenciaMovim(int x, int y)
{
	// Botão esquerdo ?
	if(iMouseButton==GLUT_LEFT_BUTTON)
	{
		// Calcula diferenças
		int deltax = iMouseX_Ini - x;
		int deltay = iMouseY_Ini - y;
		// E modifica ângulos
		fRotacaoY = fRotacaoY_Ini - deltax/SENS_ROT;
		fRotacaoX = fRotacaoX_Ini - deltay/SENS_ROT;
	}
	// Botão direito ?
	else if(iMouseButton==GLUT_RIGHT_BUTTON)
	{
		// Calcula diferença
		int deltaz = iMouseY_Ini - y;
		// E modifica distância do observador
		fObservadorZ = fObservadorZ_Ini + deltaz/SENS_OBS;
	}
	else if(iMouseButton==GLUT_MIDDLE_BUTTON)
	{
		// Calcula diferenças
		int deltax = iMouseX_Ini - x;
		int deltay = iMouseY_Ini - y;
		// E modifica posições
		fObservadorX = fObservadorX_Ini + deltax/SENS_TRANSL;
		fObservadorY = fObservadorY_Ini - deltay/SENS_TRANSL;
	}
	T2_PosicionaObservador();
	glutPostRedisplay();
}


/****************************************
 * Função: T2_Inicializa
 ****************************************/
void T2_Inicializa (void)
{

    T2_LeArquivoLevel(LEVEL_1_MAP);

    /* Inicializa o level */
    eCurrentLevel = LEVEL_1;

	// Define a cor de fundo da janela de visualização como branca
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Inicializa a variável que especifica o ângulo da projeção
	// perspectiva
	fAngle=54;

	// Inicializa as variáveis usadas para alterar a posição do
	// observador virtual
	fRotacaoX = 14.4;
	fRotacaoY = 28.6;
	fObservadorX = 5;
	fObservadorY = 5;
	fObservadorZ = 22;

	// Carrega o objeto 3D
	tileNormalObj = CarregaObjeto("Objects/chao_normal.obj",false);
	bloxorObj = CarregaObjeto("Objects/bloxor.obj",false);

	// Seta o modo de desenho como wireframe
	SetaModoDesenho('w');	// 's' para sólido


}


void OnIdle() {
    GameStatusEnum eGameStatus;
	// update the frame time
	SortFrameTimer();

	eGameStatus = T2_VerificaStatus();

	// create a new particle every frame
	//if( eGameStatus == GAME_STATUS_DEATH){
	for(int i=0;i<10;i++){
        T2_NovaParticula();
	}
	//}

	// update the particle simulation
	T2_AtualizaParticula(FrameTime());

	// remove any dead particles
	T2_RemoveParticulasMortas();

    T2_PosicionaObservador();
    glutPostRedisplay();

}

/****************************************
 * Função: main
 ****************************************/
int main(int argc, char** argv)
{
    glutInit(&argc, argv);

	// Define do modo de operação da GLUT
	glutInitDisplayMode(GLUT_DOUBLE| GLUT_RGB);

	// Especifica a posição inicial da janela GLUT
	glutInitWindowPosition(5,5);

	// Especifica o tamanho inicial em pixels da janela GLUT
	glutInitWindowSize(800,600);

	// Cria a janela passando como argumento o título da mesma
	glutCreateWindow("T2");

	// Registra a função callback de redesenho da janela de visualização
	glutDisplayFunc(T2_Desenha);

	// Registra a função callback de redimensionamento da janela de visualização
	glutReshapeFunc(T2_AlteraTamanhoJanela);

	// Registra a função callback para tratamento das teclas normais
	glutKeyboardFunc (T2_Teclado);

	// Registra a função callback para tratamento das teclas especiais
	glutSpecialFunc (T2_TeclasEspeciais);

	// Registra a função callback para eventos de botões do mouse
	glutMouseFunc(T2_GerenciaMouse);

	// Registra a função callback para eventos de movimento do mouse
	glutMotionFunc(T2_GerenciaMovim);

	glutIdleFunc(OnIdle);

	// initialise the frame timer
	InitFrameTimer();
	// Chama a função responsável por fazer as inicializações
	T2_Inicializa();

	// Inicia o processamento e aguarda interações do usuário
	glutMainLoop();

	return 0;
}
