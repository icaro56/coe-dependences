#include "TreeLoader2D.h"
#include "PGPrerequisites.h"
#include <Ogre.h>

class PagedGeometry;
class TreeLoader2D;
class Terrain;

class PAGEDGEOMETRY_EXPORT TreeVR
{
   public:
   /**
   *@brief Construtor da Classe TreeVR para criar Arvores e Plantas no mundo 3D.
   *@param camName - nome da camera da aplicação
   *@param t - terreno onde as arvores serao criadas
   *@param PageSize - tamanho da paginacao das plantas
   *@param lt,tp,rt,bt - sao os maximos e minimos de onde pode-se criar grama
   *(coloque os pontos maximos e minimos da aplicacao para gerar grama nele todo)
   *@param maxRange - e o maximo de distancia que a camera vizualizara a arvore
   *@param distance - distancia em media das arvores
   *@param ImaxRange - e o maximo de distancia que a camera vizualizara a arvore impostora
   *@param Idistance - distancia em média das arvores impostora
   *@param isInfiity - se a criacao de arvore e infinita(assim o lt, tp, rt, bt nao serao usados)
   *@param wind - se as arvores irao balancar(true a aplicacao ainda possui erro)
   */
   TreeVR(Ogre::SceneManager* scene, std::string& camName, Terrain*t, double PageSize, double lt, double tp, double rt, double bt, double maxRange = 150, double distance = 30, double ImaxRange = 500, double Idistance = 50, bool isInfinity = true, bool wind = false);


    /**
   *@brief Destrutor da Classe TreeVR.
   */
   ~TreeVR();

   /**
   *@brief cria uma arvore.
   *@param name - nome da arvore
   *@param mesh - o nome do mesh da arvore
   */
   void createTree(std::string& name, std::string& mesh);

   /**
   *@brief adiciona uma arvore no mundo 3d.
   *@param pos - vetor com a posição x,y,z
   *@param yaw - rotacao em Y
   *@param scale - tamanho da arvore
   */
   void addTree(Ogre::Vector3 &pos, double yaw, double scale);

   /**
   *@brief atribui uma material para a arvore.
   *@param name - nome do material
   */
   void setMaterialName(std::string& name);

   /**
   *@brief atribui a direcao do vento.
   *@param FactorX - direcao em X
   *@param FactorY - direcao em Y
   */
   void setWind(double FactorX, double FactorY);

   /**
   *@brief atribui um mapa de coloracao para as arvores(tipo um shadow map).
   *@param map - nome do imagem
   */
   void setColorMap(std::string& map);

   /**
   *@brief metodo necessario para atualizar as arvores(chame ele no loop principal)
   */
   void update();

   private:
     Forests::PagedGeometry* mTrees;
     Forests::TreeLoader2D* mTreeLoader;
     Terrain* mTerrain;
     Ogre::Entity* TreeEntity;
     Ogre::SceneManager* mSceneMgr;
};
