/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
Copyright (c) 2009-2010 Ricardo Quesada
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2012		UA Software Inc. 

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#include "CCTMXTiledMap.h"
#include "CCTMXXMLParser.h"
#include "CCTMXLayer.h"
#include "CCSprite.h"
#include "CCPointExtension.h"
#include "CCTextureCache.h"
#include "CCFileUtils.h"
#include "cocoa/CCNS.h"
#include "CCDirector.h"

#include <sstream>
#include <limits>

namespace cocos2d{

	// implementation CCTMXTiledMap
	CCTMXTiledMap * CCTMXTiledMap::tiledMapWithTMXFile(const char *tmxFile)
	{
		CCTMXTiledMap *pRet = new CCTMXTiledMap();
		if (pRet->initWithTMXFile(tmxFile))
		{
			pRet->autorelease();
			return pRet;
		}
        CC_SAFE_DELETE(pRet);
		return NULL;
	}
	
	bool CCTMXTiledMap::initWithTMXFile(const char *tmxFile)
	{
		CCAssert(tmxFile != NULL && strlen(tmxFile)>0, "TMXTiledMap: tmx file should not bi nil");
		
		setContentSize(CCSizeZero);
		
		CCTMXMapInfo *mapInfo = CCTMXMapInfo::formatWithTMXFile(tmxFile);
		
        if (! mapInfo)
        {
            return false;
        }
		
		return initWithTMXMapInfo(mapInfo);
	}
	
	bool CCTMXTiledMap::initWithTMXMapInfo(CCTMXMapInfo *mapInfo)
	{
		CCAssert( mapInfo->getTilesets()->count() != 0, "TMXTiledMap: Map not found. Please check the filename.");

		m_tMapSize = mapInfo->getMapSize();
		m_tTileSize = mapInfo->getTileSize();
		m_nMapOrientation = mapInfo->getOrientation();
		setObjectGroups(mapInfo->getObjectGroups());
		setProperties(mapInfo->getProperties());
		CC_SAFE_RELEASE(m_pTileProperties);
		m_pTileProperties = mapInfo->getTileProperties();
		CC_SAFE_RETAIN(m_pTileProperties);

		int idx = 0;

		CCMutableArray<CCTMXLayerInfo*>* layers = mapInfo->getLayers();
		if (layers && layers->count()>0)
		{
            if (NULL == m_pTMXLayers)
            {
                m_pTMXLayers = new CCDictionary<std::string, CCTMXLayer*>();
                CCAssert(m_pTMXLayers, "Allocate memory failed!");
            }

			CCTMXLayerInfo *layerInfo = NULL;
			CCMutableArray<CCTMXLayerInfo*>::CCMutableArrayIterator it;
			for (it = layers->begin(); it != layers->end(); ++it)
			{
				layerInfo = *it;
				if (layerInfo && layerInfo->m_bVisible)
				{
#if MULTI_TILESETS_PER_LAYER == 0
					CCTMXLayer *child = parseLayer(layerInfo, mapInfo);
					addChild((CCNode*)child, layerInfo->m_zOrder, idx);

                    // record the CCTMXLayer object by it's name
                    std::string layerName = child->getLayerName();
                    m_pTMXLayers->setObject(child, layerName);

					// update content size with the max size
					const CCSize& childSize = child->getContentSize();
					CCSize currentSize = this->getContentSize();
					currentSize.width = MAX( currentSize.width, childSize.width );
					currentSize.height = MAX( currentSize.height, childSize.height );
					this->setContentSize(currentSize);

					idx++;
#else
					CCArray *childs = parseLayers(layerInfo, mapInfo);
					for (unsigned int i = 0; i < childs->count(); i++)
					{
						CCTMXLayer *child = (CCTMXLayer*)childs->objectAtIndex(i);

						addChild((CCNode*)child, layerInfo->m_zOrder, idx);

						std::string layerName = child->getLayerName();
						if (childs->count() > 1)
						{
							std::ostringstream strName;
							strName << layerName << "_duplicate" << i;
							layerName = strName.str();
						}

						m_pTMXLayers->setObject(child, layerName);

						// update content size with the max size
						const CCSize& childSize = child->getContentSize();
						CCSize currentSize = this->getContentSize();
						currentSize.width = MAX( currentSize.width, childSize.width );
						currentSize.height = MAX( currentSize.height, childSize.height );
						this->setContentSize(currentSize);

						idx++;
					}
#endif
				}
			}

				idx++;
		}
		return true;
	}
	CCTMXTiledMap::CCTMXTiledMap()
        :m_tMapSize(CCSizeZero)
		,m_tTileSize(CCSizeZero)		
		,m_pObjectGroups(NULL)
		,m_pProperties(NULL)
		,m_pTileProperties(NULL)
        ,m_pTMXLayers(NULL)
	{
	}
	CCTMXTiledMap::~CCTMXTiledMap()
	{
		CC_SAFE_RELEASE(m_pProperties);
		CC_SAFE_RELEASE(m_pObjectGroups);
		CC_SAFE_RELEASE(m_pTileProperties);
        CC_SAFE_RELEASE(m_pTMXLayers);
	}
	CCMutableArray<CCTMXObjectGroup*> * CCTMXTiledMap::getObjectGroups()
	{
		return m_pObjectGroups;
	}
	void CCTMXTiledMap::setObjectGroups(CCMutableArray<CCTMXObjectGroup*>* var)
	{
		CC_SAFE_RETAIN(var);
		CC_SAFE_RELEASE(m_pObjectGroups);
		m_pObjectGroups = var;
	}
	CCStringToStringDictionary * CCTMXTiledMap::getProperties()
	{
		return m_pProperties;
	}
	void CCTMXTiledMap::setProperties(CCStringToStringDictionary* var)
	{
		CC_SAFE_RETAIN(var);
		CC_SAFE_RELEASE(m_pProperties);
		m_pProperties = var;
	}
	// private
	CCTMXLayer * CCTMXTiledMap::parseLayer(CCTMXLayerInfo *layerInfo, CCTMXMapInfo *mapInfo)
	{
		CCTMXLayer *layer = NULL;

		// tell the layerinfo to release the ownership of the tiles map.
		layerInfo->m_bOwnTiles = false;

		CCTMXTilesetInfo *tileset = tilesetForLayer(layerInfo, mapInfo);
		if (tileset && tileset->m_sAtlasSourceImage.empty())
		{
			layer = CCTMXLayer::layerWithTilesetInfo(tileset, layerInfo, mapInfo);
			layer->setupTiles();
		}
		else
		{
			layer = CCTMXLayer::layerWithAtlasInfo(tileset, layerInfo, mapInfo);
			layer->setupTilesWithAtlas();
		}

		return layer;
	}
	CCArray* CCTMXTiledMap::parseLayers(CCTMXLayerInfo *layerInfo, CCTMXMapInfo *mapInfo)
	{
		CCArray* layers = CCArray::array();
		layers->retain();

		// tell the layerinfo to release the ownership of the tiles map.
		layerInfo->m_bOwnTiles = false;

		CCArray *tilesets = tilesetsForLayer(layerInfo, mapInfo);
		tilesets->reverseObjects();
		for (unsigned int i = 0; i < tilesets->count(); i++)
		{
			CCTMXTilesetInfo* tileset = (CCTMXTilesetInfo*)tilesets->objectAtIndex(i);

			CCTMXLayer *layer = NULL;

			if (tileset && tileset->m_sAtlasSourceImage.empty())
			{
				layer = CCTMXLayer::layerWithTilesetInfo(tileset, layerInfo, mapInfo);
				layer->setupTiles();
			}
			else
			{
				layer = CCTMXLayer::layerWithAtlasInfo(tileset, layerInfo, mapInfo);
				layer->setupTilesWithAtlas();
			}
			layers->addObject(layer);
		}

		tilesets->release();

		return layers;
	}

	CCTMXTilesetInfo * CCTMXTiledMap::tilesetForLayer(CCTMXLayerInfo *layerInfo, CCTMXMapInfo *mapInfo)
	{
		CCSize size = layerInfo->m_tLayerSize;
		CCMutableArray<CCTMXTilesetInfo*>* tilesets = mapInfo->getTilesets();
		if (tilesets && tilesets->count()>0)
		{
			CCTMXTilesetInfo *tileset = NULL;
			CCMutableArray<CCTMXTilesetInfo*>::CCMutableArrayRevIterator rit;
			for (rit = tilesets->rbegin(); rit != tilesets->rend(); ++rit)
			{
				tileset = *rit;
				if (tileset)
				{
					for( unsigned int y=0; y < size.height; y++ )
					{
						for( unsigned int x=0; x < size.width; x++ ) 
						{
							unsigned int pos = (unsigned int)(x + size.width * y);
							unsigned int gid = layerInfo->m_pTiles[ pos ];

							// gid are stored in little endian.
							// if host is big endian, then swap
							//if( o == CFByteOrderBigEndian )
							//	gid = CFSwapInt32( gid );
							/* We support little endian.*/

							// XXX: gid == 0 --> empty tile
							if( gid != 0 ) 
							{
								// Optimization: quick return
								// if the layer is invalid (more than 1 tileset per layer) an CCAssert will be thrown later
								if( gid >= tileset->m_uFirstGid )
									return tileset;
							}
						}
					}		
				}
			}
		}

		// If all the tiles are 0, return empty tileset
		CCLOG("cocos2d: Warning: TMX Layer '%@' has no tiles", layerInfo->m_sName.c_str());
		return NULL;
	}
	CCArray* CCTMXTiledMap::tilesetsForLayer(CCTMXLayerInfo *layerInfo, CCTMXMapInfo *mapInfo)
	{
		CCArray* resTilesets = CCArray::array();
		resTilesets->retain();

		CCSize size = layerInfo->m_tLayerSize;
		CCMutableArray<CCTMXTilesetInfo*>* tilesets = mapInfo->getTilesets();
		if (tilesets && tilesets->count()>0)
		{
			unsigned int tsFirstGidPrev = 2147483647;

			CCTMXTilesetInfo *tileset = NULL;
			CCMutableArray<CCTMXTilesetInfo*>::CCMutableArrayRevIterator rit;
			for (rit = tilesets->rbegin(); rit != tilesets->rend(); ++rit)
			{
				tileset = *rit;
				if (tileset)
				{
					for( unsigned int y=0; y < size.height; y++ )
					{
						for( unsigned int x=0; x < size.width; x++ ) 
						{
							unsigned int pos = (unsigned int)(x + size.width * y);
							unsigned int gid = layerInfo->m_pTiles[ pos ];

							// XXX: gid == 0 --> empty tile
							if( gid != 0 ) 
							{
								if( gid >= tileset->m_uFirstGid && gid < tsFirstGidPrev )
								{
									if (!resTilesets->containsObject(tileset))
										resTilesets->addObject(tileset);
								}
							}
						}
					}		
					tsFirstGidPrev = tileset->m_uFirstGid;
				}
			}
		}
		return resTilesets;
	}

	// public
	CCTMXLayer * CCTMXTiledMap::layerNamed(const char *layerName)
	{
		std::string sLayerName = layerName;
        CCTMXLayer * pRet = m_pTMXLayers->objectForKey(sLayerName);
		return pRet;
	}
	CCTMXObjectGroup * CCTMXTiledMap::objectGroupNamed(const char *groupName)
	{
		std::string sGroupName = groupName;
		if (m_pObjectGroups && m_pObjectGroups->count()>0)
		{
			CCTMXObjectGroup *objectGroup;
			CCMutableArray<CCTMXObjectGroup*>::CCMutableArrayIterator it;
			for (it = m_pObjectGroups->begin(); it != m_pObjectGroups->end(); ++it)
			{
				objectGroup = (CCTMXObjectGroup*)(*it);
				if (objectGroup && objectGroup->getGroupName() == sGroupName)
				{
					return objectGroup;
				}
			}
		}

		// objectGroup not found
		return NULL;
	}

	CCString * CCTMXTiledMap::propertyNamed(const char *propertyName)
	{
		return m_pProperties->objectForKey(std::string(propertyName));
	}
	CCDictionary<std::string, CCString*> * CCTMXTiledMap::propertiesForGID(int GID)
	{
		return m_pTileProperties->objectForKey(GID);
	}

}// namespace cocos2d

