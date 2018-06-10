#include "GameScene.h"

USING_NS_CC;

Scene* GameScene::createScene()
{
	return GameScene::create();
}

// on "init" you need to initialize your instance
bool GameScene::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Scene::init())
	{
		return false;
	}

	//add touch listener
	EventListenerTouchOneByOne* listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);


	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	stoneLayer = Layer::create();
	stoneLayer->setAnchorPoint(ccp(0, 0));
	stoneLayer->setPosition(0, 0);
	stoneLayer->setName("stoneLayer");
	this->addChild(stoneLayer, 0);

	auto bg = Sprite::create("level-background-0.jpg");
	bg->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	stoneLayer->addChild(bg);

	stone = Sprite::create("stone.png");
	stone->setPosition(560, 480);
	stone->setName("stone");
	stoneLayer->addChild(stone);

	mouseLayer = Layer::create();
	mouseLayer->setAnchorPoint(ccp(0, 0));
	mouseLayer->setPosition(0, visibleSize.height / 2);
	mouseLayer->setName("mouseLayer");
	this->addChild(mouseLayer, 1);

	// load game resource
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("level-sheet.plist");
	char totalFrames = 8;
	char frameName[20];
	Animation* mouseAnimation = Animation::create();

	for (int i = 0; i < totalFrames; i++)
	{
		sprintf(frameName, "gem-mouse-%d.png", i);
		mouseAnimation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName));
	}
	mouseAnimation->setDelayPerUnit(0.1);
	AnimationCache::getInstance()->addAnimation(mouseAnimation, "mouseAnimation");

	mouse = Sprite::createWithSpriteFrameName("gem-mouse-0.png");
	Animate* mouseAnimate = Animate::create(AnimationCache::getInstance()->getAnimation("mouseAnimation"));
	mouse->runAction(RepeatForever::create(mouseAnimate));
	mouse->setPosition(visibleSize.width / 2, 0);
	mouse->setName("mouse");
	mouseLayer->addChild(mouse, 1);

	auto text = Label::create("Shoot", "fonts/Marker Felt.ttf", 54);
	auto shoot = MenuItemLabel::create(text, CC_CALLBACK_1(GameScene::shoot, this));
	shoot->setPosition(visibleSize.width - 150, visibleSize.height / 4);
	auto menu = Menu::create(shoot, NULL);
	menu->setPosition(Vec2::ZERO);
	mouseLayer->addChild(menu);

	auto helicop = Sprite::createWithSpriteFrameName("helicopter-0.png");
	Animate* helicpAnimate = Animate::create(AnimationCache::getInstance()->getAnimation("helicpAnimation"));
	helicop->runAction(RepeatForever::create(helicpAnimate));
	helicop->setPosition(visibleSize.width / 2, visibleSize.height - 50);
	stoneLayer->addChild(helicop);

	auto gold = Sprite::createWithSpriteFrameName("gold-0-0.png");
	Animate* goldAnimate = Animate::create(AnimationCache::getInstance()->getAnimation("goldAnimation"));
	gold->runAction(RepeatForever::create(goldAnimate));
	gold->setPosition(150, visibleSize.height / 2 - 50);
	stoneLayer->addChild(gold);

	return true;
}

bool GameScene::onTouchBegan(Touch *touch, Event *unused_event) {

	auto location = touch->getLocation();
	auto position = mouseLayer->convertToNodeSpace(location);
	auto cheese = Sprite::create("cheese.png");
	cheese->setPosition(position.x, position.y);
	auto mouseLayer = this->getChildByName("mouseLayer");
	mouseLayer->addChild(cheese);

	auto fadeOut = FadeOut::create(3.0f);
	cheese->runAction(fadeOut);
	
	auto mouse = mouseLayer->getChildByName("mouse");
	auto moveTo = MoveTo::create(0.5f, Vec2(position.x, position.y));
	mouse->runAction(moveTo);

	return true;
}

void GameScene::shoot(Ref* sender) {
	auto shoot = (Menu*)sender;
	auto stoneLayer = shoot->getParent()->getParent()->getParent()->getChildByName("stoneLayer");
	auto mouseLayer = shoot->getParent()->getParent();
	auto stone = stoneLayer->getChildByName("stone");
	auto mouse = mouseLayer->getChildByName("mouse");

	auto temp_stone = Sprite::create("stone.png");
	temp_stone->setPosition(stone->getPosition());
	stoneLayer->addChild(temp_stone);

	auto position = mouseLayer->convertToWorldSpace(mouse->getPosition());
	auto moveTo = MoveTo::create(0.5f, Vec2(position.x, position.y));
	auto fadeOut = FadeOut::create(2.0f);
	auto seq = Sequence::create(moveTo, fadeOut, nullptr);
	temp_stone->runAction(seq);

	auto diamond = Sprite::create("diamond.png");
	diamond->setPosition(mouse->getPosition());
	mouseLayer->addChild(diamond);

	timeval tv;
	cocos2d::gettimeofday(&tv, NULL);
	unsigned long int seed = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	srand(seed);
	int y = rand() % (int)(Director::getInstance()->getVisibleSize().height) - Director::getInstance()->getVisibleSize().height / 2;
	int x = rand() % (int)(Director::getInstance()->getVisibleSize().width);
	auto move = MoveTo::create(0.5f, Vec2(x, y));
	mouse->runAction(move);
}