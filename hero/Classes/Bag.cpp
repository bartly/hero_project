#include "Bag.h"
#include "sqlite3.h"

using namespace cocos2d;
using namespace cocos2d::extension;

enum
{
	EQUIP_TYPE_HELMET = 1,
	EQUIP_TYPE_NECKLACE,
	EQUIP_TYPE_ARMOUR,
	
	EQUIP_TYPE_WEAPON,
	EQUIP_TYPE_SKILL,
	EQUIP_TYPE_SHIELD,
	
	EQUIP_TYPE_OTHER,
	EQUIP_TYPE_BELT,
	EQUIP_TYPE_GLOVE
};


Bag* Bag::create()
{
	Bag *pRet = new Bag();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    else
    {
        CC_SAFE_DELETE(pRet);
        return NULL;
    }
}

bool Bag::init()
{
	if(!CCLayer::init()) return false;
    
    initSqlite();

	initTestUILayer();
    
    return true;
    
	initArmature();
	initEquips();
	initPlayerEquipGrid();

	return true;
}

void Bag::initSqlite()
{
//    char* a[1] = {0};
//    sqlite_main(0, &a[0]);
    
    sqlite3 *pDB = NULL;//数据库指针
    char * errMsg = NULL;//错误信息
    std::string sqlstr;//SQL指令
    int result;//sqlite3_exec返回值
    
    CCString* fileName = CCString::createWithFormat("%s/%s", CCFileUtils::sharedFileUtils()->getWritablePath().c_str(), "save.db");
    
    //打开一个数据库，如果该数据库不存在，则创建一个数据库文件
    result = sqlite3_open(fileName->getCString(), &pDB);
    if( result != SQLITE_OK )
        CCLog( "打开数据库失败，错误码:%d ，错误原因:%s\n" , result, errMsg );
    
    //创建表，设置ID为主键，且自动增加
    result=sqlite3_exec( pDB, "create table MyTable_1( ID integer primary key autoincrement, name nvarchar(32) ) " , NULL, NULL, &errMsg );
    if( result != SQLITE_OK )
        CCLog( "创建表失败，错误码:%d ，错误原因:%s\n" , result, errMsg );
    
    //插入数据
    sqlstr=" insert into MyTable_1( name ) values ( '克塞' ) ";
    result = sqlite3_exec( pDB, sqlstr.c_str() , NULL, NULL, &errMsg );
    if(result != SQLITE_OK )
        CCLog( "插入记录失败，错误码:%d ，错误原因:%s\n" , result, errMsg );
    
    //插入数据
    sqlstr=" insert into MyTable_1( name ) values ( '葫芦娃' ) ";
    result = sqlite3_exec( pDB, sqlstr.c_str() , NULL, NULL, &errMsg );
    if(result != SQLITE_OK )
        CCLog( "插入记录失败，错误码:%d ，错误原因:%s\n" , result, errMsg );
    
    //插入数据
    sqlstr=" insert into MyTable_1( name ) values ( '擎天柱' ) ";
    result = sqlite3_exec( pDB, sqlstr.c_str() , NULL, NULL, &errMsg );
    if(result != SQLITE_OK )
        CCLog( "插入记录失败，错误码:%d ，错误原因:%s\n" , result, errMsg ); 
    
    //关闭数据库 
    sqlite3_close(pDB);
}

void Bag::initTestUILayer()
{
    UIWidget* widget = dynamic_cast<Layout*>(cocos2d::extension::GUIReader::shareReader()->widgetFromJsonFile("res/hero_1.ExportJson"));
	uiLayer = UILayer::create();
	uiLayer->addWidget(widget);
	this->addChild(uiLayer);
}

void Bag::initUILayer()
{
	UIWidget* widget = dynamic_cast<Layout*>(cocos2d::extension::GUIReader::shareReader()->widgetFromJsonFile("SampleChangeEquip_UI_1/SampleChangeEquip_UI_1.ExportJson"));
	uiLayer = UILayer::create();
	uiLayer->addWidget(widget);
	this->addChild(uiLayer);

	UIWidget* closeButton = dynamic_cast<UIWidget*>(uiLayer->getWidgetByName("closebutton"));
	closeButton->addPushDownEvent(this,coco_pushselector(Bag::closeCallback));
}
void Bag::initArmature()
{
	CCArmatureDataManager::sharedArmatureDataManager()->
		addArmatureFileInfo("ArmatureAndEquip/EquipArmature.ExportJson");
	armature = CCArmature::create("EquipArmature");
	armature->getAnimation()->playByIndex(0);
	armature->setScale(0.28);
	armature->setPosition(ccp(CCDirector::sharedDirector()->getVisibleSize().width * 0.28,
		CCDirector::sharedDirector()->getVisibleSize().height * 0.55));

	UIWidget* armatureWidget = UIWidget::create();
	uiLayer->addWidget(armatureWidget);
	armatureWidget->addCCNode(armature);

	initArmatureOriginEquips();
}
void Bag::initArmatureOriginEquips()
{
	armature->getBone("beltbone")->changeDisplayByIndex(-1,true);
	armature->getBone("necklacebone")->changeDisplayByIndex(-1,true);
	armature->getBone("weaponbone")->changeDisplayByIndex(-1,true);
	armature->getBone("helmetbone")->changeDisplayByIndex(-1,true);
}

void Bag::initEquips()
{
	UIPanel* equipPanel = dynamic_cast<UIPanel*>(uiLayer->getWidgetByName("equippanel"));
	CCArray* equips = equipPanel->getChildren();
	CCObject* object = NULL;

	int bagGridCount = 1;

	int equipType = EQUIP_TYPE_HELMET;		//the first type of equip

	CCARRAY_FOREACH(equips,object)
	{
		UIPanel* equipChildPanel = (UIPanel*)object;
		CCArray* equips = equipChildPanel->getChildren();

		int equipStartNum = 1;

		CCARRAY_FOREACH_REVERSE(equips, object)
		{
			UIWidget* equip = dynamic_cast<UIWidget*>(object);
			equip->addTouchEventListener(this,toucheventselector(Bag::touchEvent));
			initEquipID(equip,equipType,equipStartNum);
			
			UIWidget* bagGrid = getBagGrid(bagGridCount++);
			changeParent(bagGrid,equip);

			equipStartNum++;
		}
		equipType++;
	}
}

UIWidget* Bag::getBagGrid(int count)
{
	CCString* ccstring = CCString::createWithFormat("baggrid%d",count);
	UIWidget* bagGrid = dynamic_cast
				<UIWidget*>(uiLayer->getWidgetByName(ccstring->getCString()));
	return bagGrid;
}

void Bag::initEquipID(UIWidget* pEquip,int type,int num)
{
	pEquip->setTag(type*100 + num *10);
}

void Bag::initPlayerEquipGrid()
{
	UIPanel* playerPanel = dynamic_cast<UIPanel*>(uiLayer->getWidgetByName("playerpanel"));

	CCArray* equipGrids = playerPanel->getChildren();
	CCObject* object = NULL;

	int gridType = EQUIP_TYPE_HELMET;
	CCARRAY_FOREACH(equipGrids,object)
	{
		UIWidget* equipGrid = (UIWidget*)object;
		equipGrid->setTag(gridType * 100);
		gridType++;
	}
}

void Bag::touchEvent(CCObject* pSender,TouchEventType type)
{
	UIWidget* equip = (UIWidget*)pSender;
	if (type == TOUCH_EVENT_BEGAN)
	{
		touchBeganEvent(equip);
	}
	if (type == TOUCH_EVENT_MOVED)
	{
		touchMoveEvent(equip);
	}
	if (type == TOUCH_EVENT_ENDED)
	{
		touchEndedEvent(equip);
	}
}

void Bag::touchBeganEvent(UIWidget* pEquip)
{
	startGrid = (UIWidget*)pEquip->getParent();
	
	pEquip->retain();
	pEquip->removeFromParent();
	pEquip->setPosition(CCPointZero);
	pEquip->setPosition(pEquip->getTouchStartPos());
	uiLayer->addWidget(pEquip);
	pEquip->release();

	pEquip->setZOrder(2);
}
void Bag::touchMoveEvent(UIWidget* pEquip)
{
	CCPoint point = pEquip->getTouchMovePos();

	pEquip->setPosition(point);
}
void Bag::touchEndedEvent(UIWidget* pEquip)
{
	UIPanel* bagPanel = dynamic_cast<UIPanel*>(uiLayer->getWidgetByName("bagpanel"));
	UIPanel* playerPanel = dynamic_cast<UIPanel*>(uiLayer->getWidgetByName("playerpanel"));

	if(hitTestPanel(bagPanel,pEquip))
    {
        if (targetGrid->getChildren()->count()>0)
        {
            UIWidget* originEquip = (UIWidget*)targetGrid->getChildren()->objectAtIndex(0);
            changeParent(startGrid, originEquip);
            if(startGrid->getTag() >= 100) changeEquip(originEquip, startGrid);
        }
        else if(startGrid->getTag()>=100) unequipEquip();
        changeParent(targetGrid, pEquip);
        return;
    }
	if(hitTestPanel(playerPanel,pEquip))
    {
        if (targetGrid->getChildren()->count()>0)
        {
            UIWidget* originEquip = (UIWidget*)targetGrid->getChildren()->objectAtIndex(0);
            changeParent(startGrid, originEquip);
        }
        changeParent(targetGrid, pEquip);
        changeEquip(pEquip, targetGrid);
        return;
    }

	changeParent(startGrid,pEquip);

}
bool Bag::hitTestPanel(UIPanel* pPanel,UIWidget* pEquip)
{
	CCObject* object = NULL;
	CCARRAY_FOREACH(pPanel->getChildren(),object)
	{
		UIWidget* grid = (UIWidget*)object;

		if(grid->hitTest(pEquip->getPosition()))
		{
			if(equipAndGridInSameType(pEquip,grid))
            {
                targetGrid = grid;
                return true;
            }
		}
	}
	return false;
}
bool Bag::equipAndGridInSameType(UIWidget* pEquip,UIWidget* pGrid)
{
	int gridType = pGrid->getTag()*0.01;
	//pGrid is bagGrid and have no children
	if((pGrid->getTag() < 100)&&(!pGrid->getChildren()->count())) return true;
    //pgrid and startGrid is baggrid
    else if((pGrid->getTag() <100)&&(startGrid->getTag() <100)) return true;
    //pGrid and equip in same type
	else if(gridType == (int(pEquip->getTag()*0.01))) return true;
    else if(pGrid->getChildren()->count())
    {
        int originEquipType = ((UIWidget*) pGrid->getChildren()->objectAtIndex(0))->getTag()*0.01;
        int newEquip = pEquip->getTag()*0.01;
        if(originEquipType == newEquip) return true;
    }
	return false;
}

void Bag::swapEquipGrid(UIWidget* pExistEquip,UIWidget* newEquip)
{
	UIWidget* parentGrid = (UIWidget*)pExistEquip->getParent();	
	changeParent(startGrid,pExistEquip);
	changeParent(parentGrid,newEquip);
}

void Bag::changeEquip(UIWidget* pWeapon,UIWidget* pGrid)
{
    int equipType = pGrid->getTag()/100;
    if (equipType == EQUIP_TYPE_SKILL) return;
    if (equipType == EQUIP_TYPE_SHIELD) return;
    if (equipType == EQUIP_TYPE_OTHER) return;

	CCString* weaponName = CCString::createWithFormat("%stex.png",pWeapon->getName());
	CCString* boneName = CCString::createWithFormat("%sbone",pGrid->getName());

	CCSkin* weaponSkin = CCSkin::createWithSpriteFrameName(weaponName->getCString());
	armature->getBone(boneName->getCString())->addDisplay(weaponSkin,0);
	armature->getBone(boneName->getCString())->changeDisplayByIndex(0,true);
}
void Bag::unequipEquip()
{
	int equipType = startGrid->getTag() * 0.01;
	CCString* boneName = CCString::createWithFormat("%sbone",startGrid->getName());
	switch (equipType)
	{
	case EQUIP_TYPE_WEAPON:
	case EQUIP_TYPE_BELT:
	case EQUIP_TYPE_HELMET:
	case EQUIP_TYPE_NECKLACE:
		armature->getBone(boneName->getCString())->changeDisplayByIndex(-1,true);
		break;
	case	EQUIP_TYPE_SKILL:
	case	EQUIP_TYPE_SHIELD:
	case	EQUIP_TYPE_OTHER:
		break;
	default:
		{
			CCString* equipName = CCString::createWithFormat("%s0tex.png",startGrid->getName());
			CCSkin* equipDefaultSkin = CCSkin::createWithSpriteFrameName(equipName->getCString());
			armature->getBone(boneName->getCString())->addDisplay(equipDefaultSkin,0);
			break;
		}
	}
}

void Bag::changeParent(UIWidget* pGrid,UIWidget* pEquip)
{
	pEquip->retain();
	pEquip->removeFromParent();
	pEquip->setPosition(CCPointZero);
	pGrid->addChild(pEquip);
	pEquip->release();
}




void Bag::closeCallback(CCObject* pSender)
{
	cocos2d::extension::CCArmatureDataManager::purge();
	cocos2d::extension::GUIReader::shareReader()->purgeGUIReader();
	cocos2d::extension::ActionManager::shareManager()->purgeActionManager();
#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)
    CCDirector::sharedDirector()->end();
#endif
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    exit(0);
#endif
}

Bag::~Bag(void)
{
}
