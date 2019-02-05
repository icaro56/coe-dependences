#include "GrassLoader.h"
#include "PGPrerequisites.h"
#include <Ogre.h>

class PagedGeometry;
class GrassLoader;
class Terrain;

class PAGEDGEOMETRY_EXPORT GrassVR
{
    public:

    enum GrassTechnique
    {   GRASSTECH_QUAD,	GRASSTECH_CROSSQUADS,GRASSTECH_SPRITE };


    enum FadeTechnique
    {	FADETECH_ALPHA,	FADETECH_GROW,	FADETECH_ALPHAGROW  };

    /**
    *@brief Construtor da Class GrassVR, utilizada para criar vasta grama no mundo 3D.
    *@param camName - nome da camera da aplicacao
    *@param PageSize - tamanho da paginacao das plantas
    *@param detailSize - tamanho do detalhe
    *@param material - nome do material da grama
    *@param t - terreno onde sera criada a grama em cima.
    */
    GrassVR(Ogre::SceneManager* scene, std::string& camName, double pageSize, double detailSize, std::string& material, Terrain* t);

    ~GrassVR();
    /**
    *@brief Atribui o menor tamanho que a grama pode ter.
    *@param width - largura da grama
    *@param height - altura da grama
    */
    void setMinimumSize(double width, double height);

	/**
    *@brief Atribui o maior tamanho que a grama pode ter.
    *@param width - largura da grama
    *@param height - altura da grama
    */
	void setMaximumSize(double width, double height);

	/**
    *@brief Atribui se a animação esta ativa.
    *@param animated - animado true ou false
    *(por padrao ela inicia como verdadeira)
	*/
	void setAnimationEnabled(bool animated = true);		//Enable animations

	/**
    *@brief Atribui a distribuicao do balanco.
    *@param freq - numero da frequencia do balanco
	*/
	void setSwayDistribution(double freq);		//Sway fairly unsynchronized

	/**
    *@brief Atribui a distancia maxima de balanco.
    *@param leng - distancia
	*/
	void setSwayLength(double leng);				//Sway back and forth 0.5 units in length

	/**
    *@brief Atribui a velocidade de balanco.
    *@param sec - a velocidade em segundos de animacao de balanco
	*/
	void setSwaySpeed(double sec);				//Sway 1/2 a cycle every second

	/**
    *@brief Atribui a densidade de grama.
    *@param den - quantidade da densidade
	*/
	void setDensity(double den);				//Relatively dense grass

	/**
    *@brief Atribui a tecnica de suavizacao.
    *@param ft - tipo da tenica (FADETECH_ALPHA,	FADETECH_GROW,	FADETECH_ALPHAGROW)
	*/
	void setFadeTechnique(FadeTechnique ft);	//Distant grass should slowly raise out of the ground when coming in range

	/**
    *@brief Atribui a tecnica de renderizacao.
    *@param qt - tipo da tenica (GRASSTECH_QUAD,	GRASSTECH_CROSSQUADS,  GRASSTECH_SPRITE)
    *@param blendBase - se mistura as bases
	*/
	void setRenderTechnique(GrassTechnique qt, bool blendBase = false);	//Draw grass as scattered quads

	/**
    *@brief atribui um mapa de coloracao para a grama.
    *@param texture - nome do imagem
    */
	void setColorMap(std::string& texture);

	/**
    *@brief Atribui minimo e maximo altura que pode criar grama.
    *@param minimum - o menor tamanho possivel
    *@param maximum - o maior tamanho possivel
    */
	void setHeightRange(double minimun, double maximum); //seta minimo e maximo altura que pode criar grama.

	/**
    *@brief Atribuio mapa  de densidade, onde tera ou nao grama no mundo 3D.
    *@param densityMap - imagem de densidade
    */
	void setDensityMap(std::string& densityMap);

	/**
    *@brief Atribuio os limites no mundo de onde se pode gerar grama.
    *@param left - ponto maximo esquerdo
    *@param top - ponto maximo cima
    *@param right - ponto maximo direito
    *@param bottom - ponto maximo baixo
    */
	void setMapBounds(double left, double top, double right, double bottom);

    /**
    *@brief atribui um novo material para a grama.
    *@param name - nome do material
    */
    void setMaterialName(std::string& material);

    /**
    *@brief atribui o maximo de inclinacao que a grama pode ter.
    *@param name - nome do material
    */
    void setMaxSlope(double inc);

    /**
    *@brief metodo necessario para atualizar a grama(chame ele no loop principal)
    */
    void update();

    private:
    //Pointers to PagedGeometry class instances:
    Terrain* mTerrain;
    Ogre::SceneManager* mSceneMgr;
	Forests::PagedGeometry *mGrass;
	Forests::GrassLoader *mGrassLoader;
    Forests::GrassLayer *mLayer;
};
