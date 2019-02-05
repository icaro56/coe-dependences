#include "SplatDisplacement.h"

using namespace Ogre;

SplatDisplacement::SplatDisplacement(Terrain* terrain, int textureWidth, const std::string passName)
:mImageWidth(textureWidth), undoIdx(0), currentIdx(0), initPoint(0,0), mTerrain(terrain), mPassName(passName),
visible(true)
{
    //CRIANDO IMAGEM PARA TEXTURA
    Ogre::uchar* pImgData = new Ogre::uchar[mImageWidth * mImageWidth * 4];
    Ogre::uchar* pUndoData[10];
    for(int i = 0; i < 10; i++)
    {
         pUndoData[i] = new Ogre::uchar[mImageWidth * mImageWidth * 4];
    }

	for(int i = 0; i < mImageWidth * 4; i += 4)
    {
        for(int j = 0; j < mImageWidth * 4; j += 4)
        {
            int iMaskPos = mImageWidth * i + j;
            pImgData[ iMaskPos ] = 0; //B
            pImgData[ iMaskPos + 1 ] = 0; //G
            pImgData[ iMaskPos + 2 ] = 0; //R
            pImgData[ iMaskPos + 3 ] = 0; //A

            for(int k = 0; k < 10; k++)
            {
                pUndoData[k][ iMaskPos ] = 0; //B
                pUndoData[k][ iMaskPos + 1] = 0; //B
                pUndoData[k][ iMaskPos + 2] = 0; //B
                pUndoData[k][ iMaskPos + 3] = 0; //B
            }
        }
    }

    imgCoverAlpha.loadDynamicImage(pImgData, mImageWidth, mImageWidth, 1, Ogre::PF_A8R8G8B8 );

    for(int i = 0; i < 10; i++)
    {
        mUndo[i].loadDynamicImage(pUndoData[i], mImageWidth, mImageWidth, 1, Ogre::PF_A8R8G8B8 );
    }

    //CARREGANDO IMAGEM PARA TEXTURA
    texCoverAlpha = TextureManager::getSingleton().loadImage("splat_img_" + mPassName, "General", imgCoverAlpha,Ogre::TEX_TYPE_2D);

    if(mTerrain->getMaterial()->getTechnique(0)->getPass("splat_pass_" + mPassName) == NULL)
    {
        Ogre::Pass* pass = mTerrain->getMaterial()->getTechnique(0)->createPass();
        pass->setName("splat_pass_" + mPassName);
        TextureUnitState * tus = pass->createTextureUnitState("splat_img_" + mPassName);
        pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
        tus->setTextureAnisotropy(16);
        tus->setTextureFiltering(Ogre::FT_MIP, Ogre::FO_ANISOTROPIC);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
SplatDisplacement::~SplatDisplacement()
{

}

//////////////////////////////////////////////////////////////////////////////////////////////
::Vector2 SplatDisplacement::getArrayPosition(::Vector2 origin, double fWidth, int imageTerrainWidth)
{
    origin.x += fWidth * 0.5;
    origin.y += fWidth * 0.5;
    origin.x *= (imageTerrainWidth )/fWidth;
    origin.y *= (imageTerrainWidth )/fWidth;
    origin.x = (int)std::min( std::max( (int)origin.x, 0), imageTerrainWidth-1);
    origin.y = (int)std::min( std::max( (int)origin.y, 0), imageTerrainWidth-1);

    return ::Vector2(origin.x, origin.y);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void SplatDisplacement::paintFree(const ::Vector3 color, const ::Vector2 origin, double iMapWidth, float radius, float opacity  )
{
     if( origin.x == 599994 && origin.y == 599994 || initPoint.x == 599994 && initPoint.y == 599994 )
        return;

    float distance = Ogre::Math::Sqrt(Ogre::Math::Pow(initPoint.x - origin.x ,2) + Ogre::Math::Pow(initPoint.y - origin.y ,2));
    float delta = distance/radius ;

    ::Vector2 direction = ::Vector2(origin.x - initPoint.x, origin.y - initPoint.y);
    direction.normalise();
    ::Vector2 current = ::Vector2(initPoint.x, initPoint.y);

    Ogre::uchar* pImgData = imgCoverAlpha.getData();
    for(int i = 0; i < delta; i ++)
    {
        ::Vector2 vMin( current.x - radius, current.y - radius);
        ::Vector2 vMax( current.x + radius, current.y + radius);

        vMin = getArrayPosition(vMin, 32000 , iMapWidth-1);
        vMax = getArrayPosition(vMax, 32000 , iMapWidth-1);

        for (int y = vMin.y ;y <= vMax.y ; y++)
        {
            for (int x = vMin.x ; x < vMax.x ; x++)
            {
                int _x = ((float)(x + 10 ) / iMapWidth) * mImageWidth;
                int _y = ((float)(y) / iMapWidth) * mImageWidth;
                int iMaskPos = mImageWidth * _y * 4 + (_x * 4 - 1);

                pImgData[ iMaskPos ] = 255 * opacity; //A
                pImgData[ iMaskPos - 1] = color.x; //R
                pImgData[ iMaskPos - 2] = color.y; //G
                pImgData[ iMaskPos - 3] = color.z; //B
            }
        }

        current.x += (direction.x * radius);
        current.y += (direction.y * radius);
    }
    if(visible)
    {
        texCoverAlpha->unload();
        texCoverAlpha->loadImage(imgCoverAlpha);
    }

    initPoint = origin;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void SplatDisplacement::paintLine(const ::Vector3 color, const ::Vector2 origin, double iMapWidth, float radius, float opacity  )
{
    if( origin.x == 599994 && origin.y == 599994 || initPoint.x == 599994 && initPoint.y == 599994 )
        return;

    freeze();

    float distance = Ogre::Math::Sqrt(Ogre::Math::Pow(initPoint.x - origin.x ,2) + Ogre::Math::Pow(initPoint.y - origin.y ,2));
    float delta = distance/radius ;

    ::Vector2 direction = ::Vector2(origin.x - initPoint.x, origin.y - initPoint.y);
    direction.normalise();
    ::Vector2 current = ::Vector2(initPoint.x, initPoint.y);

    Ogre::uchar* pImgData = imgCoverAlpha.getData();
    for(int i = 0; i < delta; i ++)
    {
        ::Vector2 vMin(current.x - radius, current.y - radius);
        ::Vector2 vMax(current.x + radius, current.y + radius);

        vMin = getArrayPosition(vMin, 32000 , iMapWidth-1);
        vMax = getArrayPosition(vMax, 32000 , iMapWidth-1);

        for (int y = vMin.y ;y <= vMax.y ; y++)
        {
            for (int x = vMin.x ; x < vMax.x ; x++)
            {
                int _x = ((float)(x + 10 ) / iMapWidth) * mImageWidth;
                int _y = ((float)y / iMapWidth) * mImageWidth;
                int iMaskPos = mImageWidth * _y * 4 + (_x * 4 - 1);

                pImgData[ iMaskPos ] = 255 * opacity; //A
                pImgData[ iMaskPos - 1] = color.x; //R
                pImgData[ iMaskPos - 2] = color.y; //G
                pImgData[ iMaskPos - 3] = color.z; //B
            }
        }

        current.x += (direction.x * radius);
        current.y += (direction.y * radius);
    }
    if(visible)
    {
        texCoverAlpha->unload();
        texCoverAlpha->loadImage(imgCoverAlpha);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void SplatDisplacement::paintRectangle(const ::Vector3 color, const ::Vector2 origin, double iMapWidth, float radius, float opacity  )
{
    if( origin.x == 599994 && origin.y == 599994 || initPoint.x == 599994 && initPoint.y == 599994 )
        return;

    freeze();

    Ogre::uchar* pImgData = imgCoverAlpha.getData();

    ::Vector2 vMin(std::min(origin.x, initPoint.x),std::min(origin.y, initPoint.y));
    ::Vector2 vMax(std::max(origin.x, initPoint.x),std::max(origin.y, initPoint.y));

    vMin = getArrayPosition(vMin, 32000 , iMapWidth-1);
    vMax = getArrayPosition(vMax, 32000 , iMapWidth-1);

    for (int y = vMin.y ;y <= vMax.y ; y++)
    {
        for (int x = vMin.x ; x < vMax.x ; x++)
        {
            if(x <= (vMin.x + radius) || x >= (vMax.x - radius) ||
                y <= (vMin.y + radius) || y >= (vMax.y - radius))
            {
                int _x = ((float)(x + 10 ) / iMapWidth) * mImageWidth;
                int _y = ((float)y / iMapWidth) * mImageWidth;
                int iMaskPos = mImageWidth * _y * 4 + (_x * 4 - 1);

                pImgData[ iMaskPos ] = 255 * opacity; //A
                pImgData[ iMaskPos - 1] = color.x; //R
                pImgData[ iMaskPos - 2] = color.y; //G
                pImgData[ iMaskPos - 3] = color.z; //B
            }
        }
    }

    if(visible)
    {
        texCoverAlpha->unload();
        texCoverAlpha->loadImage(imgCoverAlpha);
    }

}

//////////////////////////////////////////////////////////////////////////////////////////////
void SplatDisplacement::paintCircle(const ::Vector3 color, const ::Vector2 origin, double iMapWidth, float radius, float opacity  )
{
    if( origin.x == 599994 && origin.y == 599994 || initPoint.x == 599994 && initPoint.y == 599994 )
        return;

    freeze();

    Ogre::uchar* pImgData = imgCoverAlpha.getData();

    double raio = (((origin.x - initPoint.x)) - ((origin.y - initPoint.y)))/2;

    for(double i = 0; i < 360; i += Math::Abs(360/raio))
    {
        double X = raio * Ogre::Math::Cos(Ogre::Degree(i));
        double Y = raio * Ogre::Math::Sin(Ogre::Degree(i));

        ::Vector2 vMin( (initPoint.x - X) - radius, (initPoint.y - Y) - radius);
        ::Vector2 vMax( (initPoint.x - X) + radius, (initPoint.y - Y) + radius);

        vMin = getArrayPosition(vMin, 32000 , iMapWidth-1);
        vMax = getArrayPosition(vMax, 32000 , iMapWidth-1);

        for (int y = vMin.y ;y <= vMax.y ; y++)
        {
            for (int x = vMin.x ; x < vMax.x ; x++)
            {
                int _x = ((float)(x + 10 ) / iMapWidth) * mImageWidth;
                int _y = ((float)y / iMapWidth) * mImageWidth;
                int iMaskPos = mImageWidth * _y * 4 + (_x * 4 - 1);

                pImgData[ iMaskPos ] = 255 * opacity; //A
                pImgData[ iMaskPos - 1] = color.x; //R
                pImgData[ iMaskPos - 2] = color.y; //G
                pImgData[ iMaskPos - 3] = color.z; //B

            }
        }
    }
    if(visible)
    {
        texCoverAlpha->unload();
        texCoverAlpha->loadImage(imgCoverAlpha);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void SplatDisplacement::erase(const ::Vector2 origin, double iMapWidth, float radius, float opacity  )
{
    float distance = Ogre::Math::Sqrt(Ogre::Math::Pow(initPoint.x - origin.x ,2) + Ogre::Math::Pow(initPoint.y - origin.y ,2));
    float delta = distance/radius ;

    ::Vector2 direction = ::Vector2(origin.x - initPoint.x, origin.y - initPoint.y);
    direction.normalise();
    ::Vector2 current = ::Vector2(initPoint.x, initPoint.y);

    Ogre::uchar* pImgData = imgCoverAlpha.getData();
    for(int i = 0; i < delta; i ++)
    {
        ::Vector2 vMin( current.x - radius, current.y - radius);
        ::Vector2 vMax( current.x + radius, current.y + radius);

        vMin = getArrayPosition(vMin, 32000 , iMapWidth-1);
        vMax = getArrayPosition(vMax, 32000 , iMapWidth-1);

        for (int y = vMin.y ;y <= vMax.y ; y++)
        {
            for (int x = vMin.x ; x < vMax.x ; x++)
            {
                int _x = ((float)(x + 10 ) / iMapWidth) * mImageWidth;
                int _y = ((float)y / iMapWidth) * mImageWidth;
                int iMaskPos = mImageWidth * _y * 4 + (_x * 4 - 1);

                pImgData[ iMaskPos ] = 0; //A
                pImgData[ iMaskPos - 1] = 0; //R
                pImgData[ iMaskPos - 2] = 0; //G
                pImgData[ iMaskPos - 3] = 0; //B
            }
        }

        current.x += (direction.x * radius);
        current.y += (direction.y * radius);
    }
    if(visible)
    {
        texCoverAlpha->unload();
        texCoverAlpha->loadImage(imgCoverAlpha);
    }
    initPoint = origin;
}


//////////////////////////////////////////////////////////////////////////////////////////////
void SplatDisplacement::setVisible(bool show)
{
    visible = show;
    if(show)
    {
        texCoverAlpha->unload();
        texCoverAlpha->loadImage(imgCoverAlpha);
    }
    else
    {
        texCoverAlpha->unload();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void SplatDisplacement::save(const char* fileName)
{
    imgCoverAlpha.save(fileName);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void SplatDisplacement::undo()
{
    if(undoIdx < 1)
        return;

    setRedo();

	Ogre::uchar* pImgData = imgCoverAlpha.getData();
	Ogre::uchar* data = mUndo[undoIdx - 1].getData();

	int iMaskPos;
	for(int i = 0; i < mImageWidth * 4; i += 4)
    {
        for(int j = 0; j < mImageWidth * 4; j += 4)
        {
            iMaskPos = mImageWidth * i + j;
            pImgData[ iMaskPos ] = data[ iMaskPos ]; //A
            pImgData[ iMaskPos + 1] = data[ iMaskPos + 1 ]; //B
            pImgData[ iMaskPos + 2] = data[ iMaskPos + 2 ]; //G
            pImgData[ iMaskPos + 3] = data[ iMaskPos + 3 ]; //R
        }
    }
    texCoverAlpha->unload();
    texCoverAlpha->loadImage(imgCoverAlpha);

    if(undoIdx > 1)
        undoIdx--;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void SplatDisplacement::freeze()
{
    if(undoIdx < 1)
        return;

	Ogre::uchar* pImgData = imgCoverAlpha.getData();
	Ogre::uchar* data = mUndo[undoIdx - 1].getData();

	int iMaskPos;
	for(int i = 0; i < mImageWidth * 4; i += 4)
    {
        for(int j = 0; j < mImageWidth * 4; j += 4)
        {
            iMaskPos = mImageWidth * i + j;
            pImgData[ iMaskPos ] = data[ iMaskPos ]; //A
            pImgData[ iMaskPos + 1] = data[ iMaskPos + 1 ]; //B
            pImgData[ iMaskPos + 2] = data[ iMaskPos + 2 ]; //G
            pImgData[ iMaskPos + 3] = data[ iMaskPos + 3 ]; //R
        }
    }
    texCoverAlpha->unload();
    texCoverAlpha->loadImage(imgCoverAlpha);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void SplatDisplacement::redo()
{
    if(undoIdx >= currentIdx)
        return;

    undoIdx++;

	Ogre::uchar* pImgData = imgCoverAlpha.getData();
	Ogre::uchar* data = mUndo[undoIdx].getData();

	int iMaskPos;
	for(int i = 0; i < mImageWidth * 4; i += 4)
    {
        for(int j = 0; j < mImageWidth * 4; j += 4)
        {
            iMaskPos = mImageWidth * i + j;
            pImgData[ iMaskPos ] = data[ iMaskPos ]; //A
            pImgData[ iMaskPos + 1] = data[ iMaskPos + 1 ]; //B
            pImgData[ iMaskPos + 2] = data[ iMaskPos + 2 ]; //G
            pImgData[ iMaskPos + 3] = data[ iMaskPos + 3 ]; //R
        }
    }
    texCoverAlpha->unload();
    texCoverAlpha->loadImage(imgCoverAlpha);

}

//////////////////////////////////////////////////////////////////////////////////////////////
void SplatDisplacement::setUndo()
{
	if(undoIdx >= 9)
    {
        realocateUndo();
        undoIdx--;
    }
	Ogre::uchar* pImgData = mUndo[undoIdx].getData();
	Ogre::uchar* data = imgCoverAlpha.getData();

	int iMaskPos;
	for(int i = 0; i < mImageWidth * 4; i += 4)
    {
        for(int j = 0; j < mImageWidth * 4; j += 4)
        {
            iMaskPos = mImageWidth * i + j;
            pImgData[ iMaskPos ] = data[ iMaskPos ]; //B
            pImgData[ iMaskPos + 1] = data[ iMaskPos + 1 ]; //G
            pImgData[ iMaskPos + 2] = data[ iMaskPos + 2 ]; //R
            pImgData[ iMaskPos + 3] = data[ iMaskPos + 3 ]; //A
        }
    }
    undoIdx++;
    currentIdx = undoIdx;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void SplatDisplacement::setRedo()
{
    Ogre::uchar* pImgData = mUndo[undoIdx].getData();
	Ogre::uchar* data = imgCoverAlpha.getData();

	int iMaskPos;
	for(int i = 0; i < mImageWidth * 4; i += 4)
    {
        for(int j = 0; j < mImageWidth * 4; j += 4)
        {
            iMaskPos = mImageWidth * i + j;
            pImgData[ iMaskPos ] = data[ iMaskPos ]; //B
            pImgData[ iMaskPos + 1] = data[ iMaskPos + 1 ]; //G
            pImgData[ iMaskPos + 2] = data[ iMaskPos + 2 ]; //R
            pImgData[ iMaskPos + 3] = data[ iMaskPos + 3 ]; //A
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void SplatDisplacement::clear()
{
    setUndo();

	Ogre::uchar* pImgData = imgCoverAlpha.getData();

	int iMaskPos;
	for(int i = 0; i < mImageWidth * 4; i += 4)
    {
        for(int j = 0; j < mImageWidth * 4; j += 4)
        {
            iMaskPos = mImageWidth * i + j;
            pImgData[ iMaskPos ] = 0; //A
            pImgData[ iMaskPos + 1] = 0; //B
            pImgData[ iMaskPos + 2] = 0; //G
            pImgData[ iMaskPos + 3] = 0; //R
        }
    }
    texCoverAlpha->unload();
    texCoverAlpha->loadImage(imgCoverAlpha);
}
////////////////////////////////////////////////////////////////////////////////////////////////
void SplatDisplacement::setImage(const char* fileName, const char* dirName)
{
    Image img;

    if(! Ogre::ResourceGroupManager::getSingleton().resourceExists("General", fileName))
    {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(dirName,"FileSystem", "Custom");
        Ogre::ResourceGroupManager::getSingleton().loadResourceGroup("Custom");
        if(!Ogre::ResourceGroupManager::getSingleton().resourceExists("Custom", fileName))
            OGRE_EXCEPT( Exception::ERR_FILE_NOT_FOUND,  "Não foi possível carregar a Imagem.", "SplatDisplacement::setImage");
        img.load(fileName, "Custom");
    }
    else
        img.load(fileName, "General");

    setUndo();

    if(img.getWidth() != imgCoverAlpha.getWidth() )
        OGRE_EXCEPT( Exception::ERR_FILE_NOT_FOUND,  "Largura Inválida da Imagem", "SplatDisplacement::setImage");
    if(img.getHeight() != imgCoverAlpha.getHeight())
        OGRE_EXCEPT( Exception::ERR_FILE_NOT_FOUND,  "Altura Inválida da Imagem", "SplatDisplacement::setImage");
    if(img.getFormat() != 12)
        OGRE_EXCEPT( Exception::ERR_FILE_NOT_FOUND,  "Formato Inválido da Imagem(PNG sem transparência)", "SplatDisplacement::setImage");

    Ogre::uchar* data = img.getData();
    Ogre::uchar* pImgData = imgCoverAlpha.getData();

	int iMaskPos;
	for(int i = 0; i < mImageWidth * 4; i += 4)
    {
        for(int j = 0; j < mImageWidth * 4; j += 4)
        {
            iMaskPos = mImageWidth * i + j;
            pImgData[ iMaskPos ] = data[ iMaskPos ]; //A
            pImgData[ iMaskPos + 1] = data[ iMaskPos + 1]; //B
            pImgData[ iMaskPos + 2] = data[ iMaskPos + 2]; //G
            pImgData[ iMaskPos + 3] = data[ iMaskPos + 3]; //R
        }
    }
    texCoverAlpha->unload();
    texCoverAlpha->loadImage(imgCoverAlpha);

}


////////////////////////////////////////////////////////////////////////////////////////////////
void SplatDisplacement::realocateUndo()
{

    Ogre::uchar* undo[9] = { mUndo[0].getData(), mUndo[1].getData(), mUndo[2].getData(), mUndo[3].getData(), mUndo[4].getData(),
                              mUndo[5].getData(), mUndo[6].getData(), mUndo[7].getData(), mUndo[8].getData()};

    int iMaskPos;
	for(int i = 0; i < mImageWidth * 4; i += 4)
    {
        for(int j = 0; j < mImageWidth * 4; j += 4)
        {
            iMaskPos = mImageWidth * i + j;
            sendToLeft( undo, iMaskPos, 0);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void SplatDisplacement::sendToLeft(Ogre::uchar* undo[], int pos, int init)
{
    if(init >= 8)
        return;

    undo[init][pos] = undo[init + 1][pos];
    undo[init][pos + 1] = undo[init + 1][pos + 1];
    undo[init][pos + 2] = undo[init + 1][pos + 2];
    undo[init][pos + 3] = undo[init + 1][pos + 3];

    init++;
    sendToLeft( undo, pos, init);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void SplatDisplacement::setInitPoint(::Vector2 origin)
{
    initPoint = origin;
}
