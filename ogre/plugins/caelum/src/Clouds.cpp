#include "Clouds.h"

Clouds::Clouds(Ogre::SceneManager* scene)
:mSceneMgr(scene)
{
    Caelum::CaelumSystem::CaelumComponent componentMask;
    componentMask = static_cast<Caelum::CaelumSystem::CaelumComponent> (Caelum::CaelumSystem::CAELUM_COMPONENT_CLOUDS | 0);

    mCaelumSystem = new Caelum::CaelumSystem (Ogre::Root::getSingletonPtr(), mSceneMgr, Caelum::CaelumSystem::CAELUM_COMPONENTS_NONE);
    mCaelumSystem->setCloudSystem (new Caelum::CloudSystem ( mSceneMgr, mCaelumSystem->getCaelumGroundNode ()));
    mCaelumSystem->setAutoNotifyCameraChanged (true);
    mCaelumSystem->setAutoMoveCameraNode (true);
    mCaelumSystem->setManageSceneFog (Ogre::FOG_NONE);
    mCaelumSystem->setManageAmbientLight (false);
    mCaelumSystem->setEnsureSingleShadowSource(false);

    if (mCaelumSystem->getCloudSystem ())
    {
        mCaelumSystem->getCloudSystem ()->createLayerAtHeight(8000);
        mCaelumSystem->getCloudSystem ()->createLayerAtHeight(9000);
        mCaelumSystem->getCloudSystem ()->createLayerAtHeight(14000);
        mCaelumSystem->getCloudSystem ()->getLayer(0)->setCloudSpeed(Ogre::Vector2(0.000005, -0.000009));
        mCaelumSystem->getCloudSystem ()->getLayer(0)->setCloudCover(0.2);
        mCaelumSystem->getCloudSystem ()->getLayer(0)->setQueryFlags(4);
        mCaelumSystem->getCloudSystem ()->getLayer(1)->setCloudSpeed(Ogre::Vector2(0.0000045, -0.0000085));
        mCaelumSystem->getCloudSystem ()->getLayer(1)->setCloudCover(0.1);
        mCaelumSystem->getCloudSystem ()->getLayer(1)->setQueryFlags(4);
        mCaelumSystem->getCloudSystem ()->getLayer(2)->setCloudSpeed(Ogre::Vector2(0.0000055, -0.0000095));
        mCaelumSystem->getCloudSystem ()->getLayer(2)->setQueryFlags(4);
    }
}

void Clouds::setCloudsVisible(bool visible)
{
    mCaelumSystem->getCloudSystem()->visibleLayers(visible);
}

void Clouds::update(double dt)
{
     mCaelumSystem->updateSubcomponents(dt);
     //FEITO APENAS PARA O MINEINSIDE, TIRAR DEPOIS
     mCaelumSystem->getUniversalClock ()->setGregorianDateTime (2009, 2, 5, 15, 25, 0);
}

void Clouds::setTimeScale(double scale)
{
    mCaelumSystem->getUniversalClock()->setTimeScale(scale);
}

void Clouds::setGregorianDateTime(int ano, int mes, int dia, int hora, int min, int sec)
{
    mCaelumSystem->getUniversalClock ()->setGregorianDateTime (ano, mes, dia, hora, min, sec);
}

void Clouds::setCloudCover(double coverage)
{
    mCaelumSystem->getCloudSystem ()->getLayer(0)->setCloudCover(coverage);
}

void Clouds::setDiffuseMultplier_Sun(double r, double g, double b)
{
    mCaelumSystem->getSun ()->~BaseSkyLight ();
    mCaelumSystem->setSun (new Caelum::SphereSun( mSceneMgr, mCaelumSystem->getCaelumCameraNode ()));
    mCaelumSystem->getSun ()->setDiffuseMultiplier (Ogre::ColourValue (r, g, b));
    mCaelumSystem->getSun ()->setAutoDisableThreshold (0.05);
    mCaelumSystem->getSun ()->setAutoDisable (false);
}

void Clouds::setAutoDisableThreshold_Sun(double intensity)
{
    mCaelumSystem->getSun ()->setAutoDisableThreshold (intensity);
}

void Clouds::setAutoDisable_Sun(bool disable)
{
     mCaelumSystem->getSun ()->setAutoDisable (disable);
}

void Clouds::setDiffuseMultplier_Moon(double r, double g, double b)
{
    mCaelumSystem->getMoon ()->setDiffuseMultiplier (Ogre::ColourValue (r, g, b));
}

void Clouds::setAutoDisableThreshold_Moon(double intensity)
{
   mCaelumSystem->getMoon ()->setAutoDisableThreshold (intensity);
}

void Clouds::setAutoDisable_Moon(bool disable)
{
    mCaelumSystem->getMoon()->setAutoDisable (disable);
}

void Clouds::setCloudHeight(double height, int index)
{
   mCaelumSystem->getCloudSystem()->getLayer(index)->setHeight(height);
}

void Clouds::setCloudSpeed(double xFactor, double zFactor, int index)
{
   mCaelumSystem->getCloudSystem()->getLayer(index)->setCloudSpeed(Ogre::Vector2(xFactor, zFactor));
}

void Clouds::setSceneFogDensityMultiplier(double density)
{
   mCaelumSystem->setSceneFogDensityMultiplier (density);
}

void Clouds::setMinimumAmbientLight(double r, double g, double b)
{
   mCaelumSystem->setMinimumAmbientLight (Ogre::ColourValue (r, g, b));
}

void Clouds::notifyCameraChanged(Ogre::Camera* cam)
{
    mCaelumSystem->notifyCameraChanged(cam);
}
