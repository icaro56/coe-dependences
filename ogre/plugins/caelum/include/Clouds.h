#include "Caelum.h"
#include "CaelumPrerequisites.h"

class CAELUM_EXPORT Clouds
{
protected:
    Caelum::CaelumSystem *mCaelumSystem;
    Ogre::SceneManager* mSceneMgr;

public:
     /**
     * @brief Contrutor de c�u, ja vem com par�metros padr�es
     */
    Clouds(Ogre::SceneManager* scene);
    /**
     * @brief M�todo necess�rio para atualiza��o, chama-lo no loop principal
     * @param dt - tempo desde o �ltimo frame
     */
    void update(double dt);
    /**
     * @brief Atribui velocidade do tempo
     * @param scale - tempo onde 1 significa passar em tempo real.
     */
    void setTimeScale(double scale);
    /**
     * @brief Esconde/Mostra as nuvens.
     * @param visible bool, true para aparecer e false para esconder.
     */
    void setCloudsVisible(bool visible);
     /**
     * @brief Atribui uma data para c�lculo da fase da lua
     * @param ano, mes, dia, hora, minutos e segundos
     */
    void setGregorianDateTime(int ano, int mes, int dia, int hora, int min, int sec);
    /**
     * @brief Atribui cobertura de nuvens, o qu�o est� nublado
     * @param coverage - 0 = c�u limpo, 1 = c�u totalmente coberto
     */
    void setCloudCover(double coverage);
    /**
     * @brief Atribui a cor da emiss�o do sol
     * @param r,g,b - cor varia de 0 a 255
     */
    void setDiffuseMultplier_Sun(double r, double g, double b);
    /**
     * @brief Intensidade de como a luz do sol implica no solo .
     * @param intensity - intensidade da luz
     */
    void setAutoDisableThreshold_Sun(double intensity);
    /**
     * @brief Turn on and off auto-disabling of the light
     * when too dim. This is off by default. If you set it
     * to true you probably also want to set the autoDisableThreshold.
     * The "intensity" of the light for the threshold is calculated as the plain sum of r, g and b.
     * @param disable - ativa e desativa
     */
    void setAutoDisable_Sun(bool disable);
    /**
     * @brief Atribui a cor da emiss�o da lua
     * @param r,g,b - cor varia de 0 a 255
     */
    void setDiffuseMultplier_Moon(double r, double g, double b);
    /**
     * @brief Intensidade de como a luz da lua implica no solo .
     * @param intensity - intensidade da luz
     */
    void setAutoDisableThreshold_Moon(double intensity);
    /**
     * @brief Turn on and off auto-disabling of the light
     * when too dim. This is off by default. If you set it
     * to true you probably also want to set the autoDisableThreshold.
     * The "intensity" of the light for the threshold is calculated as the plain sum of r, g and b.
     * @param disable - ativa e desativa
     */
    void setAutoDisable_Moon(bool disable);
    /**
     * @brief Atribui a altura do layer de nuvem.
     * @param height - altura,
     * @param index - n�mero do layer(por padr�o existe 2 layers de nuvens)
     */
    void setCloudHeight(double height, int index);
    /**
     * @brief Atribui a velocidade do layer de nuvem.
     * @param xFactor - velocidade no eixo X,
     * @param zFactor - velocidade no eixo Z,
     * @param index - n�mero do layer(por padr�o existe 2 layers de nuvens)
     */
    void setCloudSpeed(double xFactor, double zFactor, int index);
    /**
     * @brief Atribui a densidade de n�voa
     * @param density - valor da densidade
     */
    void setSceneFogDensityMultiplier(double density);
    /**
     * @brief Atribui o m�nimo de ilumina��o na cena
     * @param r,g,b - cor varia de 0 a 255
     */
    void setMinimumAmbientLight(double r, double g, double b);

    void notifyCameraChanged(Ogre::Camera* cam);

};


