#pragma execution_character_set("utf-8")
#include "HitBrick.h"
#include "cocos2d.h"
#include "SimpleAudioEngine.h"
#define database UserDefault::getInstance()

#define BALL_TAG 10
#define BAR_TAG 20
#define SHIP_TAG 30

USING_NS_CC;
using namespace CocosDenshion;

void HitBrick::setPhysicsWorld(PhysicsWorld* world) { m_world = world; }

Scene* HitBrick::createScene() {
  srand((unsigned)time(NULL));
  auto scene = Scene::createWithPhysics();

  scene->getPhysicsWorld()->setAutoStep(true);

  // Debug 模式
  //scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
  scene->getPhysicsWorld()->setGravity(Vec2(0, -300.0f));
  auto layer = HitBrick::create();
  layer->setPhysicsWorld(scene->getPhysicsWorld());
  layer->setJoint();
  scene->addChild(layer);
  return scene;
}

// on "init" you need to initialize your instance
bool HitBrick::init() {
  //////////////////////////////
  // 1. super init first
  if (!Layer::init()) {
    return false;
  }
  visibleSize = Director::getInstance()->getVisibleSize();


  auto edgeSp = Sprite::create();  //创建一个精灵
  auto boundBody = PhysicsBody::createEdgeBox(visibleSize, PhysicsMaterial(0.0f, 1.0f, 0.0f), 3);  //edgebox是不受刚体碰撞影响的一种刚体，我们用它来设置物理世界的边界
  edgeSp->setPosition(visibleSize.width / 2, visibleSize.height / 2);  //位置设置在屏幕中央
  edgeSp->setPhysicsBody(boundBody);
  addChild(edgeSp);


  preloadMusic(); // 预加载音效

  addSprite();    // 添加背景和各种精灵
  addListener();  // 添加监听器 
  addPlayer();    // 添加球与板
  BrickGeneraetd();  // 生成砖块


  schedule(schedule_selector(HitBrick::update), 0.01f, kRepeatForever, 0.1f);
  //用于控制移动的调度器
  schedule(schedule_selector(HitBrick::move), 0.01f, kRepeatForever, 0);
  
  onBall = true;
  spFactor = 0;
  
  return true;
}

// 关节连接，固定球与板子
// Todo
void HitBrick::setJoint() {
	joint1 = PhysicsJointPin::construct(ball->getPhysicsBody(), player->getPhysicsBody(), Vec2(0, 0), Vec2(0, ball->getContentSize().height / 20));
	m_world->addJoint(joint1);
}

// 预加载音效
void HitBrick::preloadMusic() {
  auto sae = SimpleAudioEngine::getInstance();
  sae->preloadEffect("gameover.mp3");
  sae->preloadBackgroundMusic("bgm.mp3");
  sae->playBackgroundMusic("bgm.mp3", true);
}

// 添加背景和各种精灵
void HitBrick::addSprite() {
  // add background
  auto bgSprite = Sprite::create("bg.png");
  bgSprite->setPosition(visibleSize / 2);
  bgSprite->setScale(visibleSize.width / bgSprite->getContentSize().width, visibleSize.height / bgSprite->getContentSize().height);
  this->addChild(bgSprite, 0);


  // add ship
  ship = Sprite::create("ship.png");
  ship->setScale(visibleSize.width / ship->getContentSize().width * 0.97, 1.2f);
  ship->setPosition(visibleSize.width / 2, 0);
  auto shipbody = PhysicsBody::createBox(ship->getContentSize(), PhysicsMaterial(100.0f, 0.0f, 1.0f));
  shipbody->setCategoryBitmask(0xFFFFFFFF);
  shipbody->setCollisionBitmask(0xFFFFFFFF);
  shipbody->setContactTestBitmask(0xFFFFFFFF);
  shipbody->setTag(SHIP_TAG);
  shipbody->setDynamic(false);
  ship->setPhysicsBody(shipbody);
  this->addChild(ship, 1);

  // add sun and cloud
  auto sunSprite = Sprite::create("sun.png");
  sunSprite->setPosition(rand() % (int)(visibleSize.width - 200) + 100, 550);
  this->addChild(sunSprite);
  auto cloudSprite1 = Sprite::create("cloud.png");
  cloudSprite1->setPosition(rand() % (int)(visibleSize.width - 200) + 100, rand() % 100 + 450);
  this->addChild(cloudSprite1);
  auto cloudSprite2 = Sprite::create("cloud.png");
  cloudSprite2->setPosition(rand() % (int)(visibleSize.width - 200) + 100, rand() % 100 + 450);
  this->addChild(cloudSprite2);
}

// 添加监听器
void HitBrick::addListener() {
  auto keyboardListener = EventListenerKeyboard::create();
  keyboardListener->onKeyPressed = CC_CALLBACK_2(HitBrick::onKeyPressed, this);
  keyboardListener->onKeyReleased = CC_CALLBACK_2(HitBrick::onKeyReleased, this);
  _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

  auto contactListener = EventListenerPhysicsContact::create();
  contactListener->onContactBegin = CC_CALLBACK_1(HitBrick::onContactBegin, this);
  _eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);
}

// 创建角色
void HitBrick::addPlayer() {
  player = Sprite::create("bar.png");
  int xpos = visibleSize.width / 2;
  player->setScale(0.1f, 0.1f);
  player->setPosition(Vec2(xpos, ship->getContentSize().height - player->getContentSize().height*0.1f));
  auto physicsBody_bar = PhysicsBody::createEdgeBox(Size(player->getContentSize()),
	  PhysicsMaterial(1.0f, 1.0f, 0));
  physicsBody_bar->setDynamic(false);
  physicsBody_bar->setCategoryBitmask(0x03);
  physicsBody_bar->setCollisionBitmask(0x03);
  physicsBody_bar->setContactTestBitmask(0x03);
  player->setPhysicsBody(physicsBody_bar);
  // 设置板的刚体属性
  this->addChild(player, 2);
  
  ball = Sprite::create("ball.png");
  ball->setAnchorPoint(Vec2(0.5, 0.5));
  ball->setPosition(Vec2(xpos, player->getPosition().y + ball->getContentSize().height*0.05f));
  ball->setScale(0.1f, 0.1f);
  ball->setTag(BALL_TAG);
  auto physicsBody_ball = PhysicsBody::createCircle(ball->getContentSize().width / 2,
	  PhysicsMaterial(1.0f, 1.0f, 0));
  physicsBody_ball->setDynamic(true);
  physicsBody_ball->setCategoryBitmask(0x01);
  physicsBody_ball->setCollisionBitmask(0x01);
  physicsBody_ball->setContactTestBitmask(0x01);
  ball->setPhysicsBody(physicsBody_ball);
  addChild(ball, 3);
  
  /*fireball = ParticleFire::create();
  fireball->setPosition(ball->getPosition());
  fireball->setDuration(-1);
  addChild(fireball, 5);*/
}

void HitBrick::update(float dt) {
	// 实现简单的蓄力效果
	if (spHolded)
		spFactor++;
	//火焰效果始终在球上
	//fireball->setPosition(ball->getPosition());
}

//实现按下移动键板子一直移动
void HitBrick::move(float dt) {
	//板子移动
	if (isMove) {
		int dir = (moveKey == 'A') ? -4 : 4;
		//注意板子实际上缩放成了原大小的0.1倍，因此这里除以的是20而不是2
		if ((player->getPositionX() + dir) > player->getContentSize().width / 20 &&
			(player->getPositionX() + dir < visibleSize.width - player->getContentSize().width / 20)) {
			player->setPosition(player->getPosition() + Vec2(dir, 0));
			//球在板子上时要注意和板子一起移动
			if (onBall) ball->setPosition(ball->getPosition() + Vec2(dir, 0));
		}
	}
}

//取消关节，发射小球
void HitBrick::fire(int spFactor) {
	m_world->removeAllJoints(true);
	ball->getPhysicsBody()->setVelocity(Vec2(0, spFactor * 10));
	onBall = false;
}


// Todo
void HitBrick::BrickGeneraetd() {
	int count = 0;
	for (int i = 0; i < 3; i++) {
		int cw = 0;
		int j = 0;
		while (cw <= visibleSize.width - 40) {
			auto box = Sprite::create("box.png");
			auto width = box->getContentSize().width;
			auto height = box->getContentSize().height;
			auto physicsBody = PhysicsBody::createEdgeBox(box->getContentSize(), PhysicsMaterial(0.1f, 1.0f, 0.5f));
			physicsBody->setDynamic(false);
			physicsBody->setContactTestBitmask(0x05);
			physicsBody->setCategoryBitmask(0x05);
			physicsBody->setCollisionBitmask(0x05);
			box->setPhysicsBody(physicsBody);
			box->setPosition( 10 + width / 2 + j * width, visibleSize.height - i * height - height / 2);
			box->setTag(brick_tag_min + count);
			cw = 10 + (j + 1) * width;
			this->addChild(box);
			j++;
			count++;
		}
	 }
	brick_tag_max = brick_tag_min + count;
}


// 左右
void HitBrick::onKeyPressed(EventKeyboard::KeyCode code, Event* event) {

  switch (code) {
  case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
	  moveKey = 'A';
	  isMove = true;
	  Apressed = true;
	break;
  case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
	  moveKey = 'D';
	  isMove = true;
	  Dpressed = true;
    break;
  case cocos2d::EventKeyboard::KeyCode::KEY_SPACE: // 开始蓄力
	  if (onBall) {
		  spHolded = true;
		  spFactor = 0;
	  }
      break;
  default:
    break;
  }
}

// 释放按键
void HitBrick::onKeyReleased(EventKeyboard::KeyCode code, Event* event) {
  switch (code) {
  case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
	  if (Dpressed == false) isMove = false;
	  else moveKey = 'D';
	  Apressed = false;
	  break;
  case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
	  if (Apressed == false) isMove = false;
	  else moveKey = 'A';
	  Dpressed = false;
    break;
  case cocos2d::EventKeyboard::KeyCode::KEY_SPACE:   // 蓄力结束，小球发射
	  if (onBall) {
		  spHolded = false;
		  fire(spFactor);
	  }
    break;

  default:
    break;
  }
}

// 碰撞检测
// Todo
bool HitBrick::onContactBegin(PhysicsContact & contact) {
	auto node1 = (Sprite *)contact.getShapeA()->getBody()->getNode();
	auto node2 = (Sprite *)contact.getShapeB()->getBody()->getNode();

	if (node1 && node2) {
		auto flower = ParticleFlower::create();
		auto explosion = ParticleExplosion::create();
		flower->setDuration(0.5f);
		if (node1->getTag() == BALL_TAG && node2->getTag() >= brick_tag_min && node2->getTag() <= brick_tag_max) {
			flower->setPosition(node2->getPosition());
			node2->removeFromParentAndCleanup(true);
			addChild(flower);
		}
		else if (node2->getTag() == BALL_TAG && node1->getTag() >= brick_tag_min && node1->getTag() <= brick_tag_max) {
			flower->setPosition(node1->getPosition());
			node1->removeFromParentAndCleanup(true);
			addChild(flower);
		}

		else if (node1->getTag() == BALL_TAG && node2->getPhysicsBody()->getTag() == SHIP_TAG) {
			explosion->setPosition(node1->getPosition() - Vec2(0, node1->getContentSize().height / 20));
			addChild(explosion, 3);
			GameOver();
		}
		else if (node1->getPhysicsBody()->getTag() == SHIP_TAG && node2->getTag() == BALL_TAG) {
			explosion->setPosition(node2->getPosition() - Vec2(0, node2->getContentSize().height / 20));
			addChild(explosion, 3);
			GameOver();
		}
	}

  return true;
}


void HitBrick::GameOver() {
	_eventDispatcher->removeAllEventListeners();
	unscheduleAllCallbacks();
	ball->getPhysicsBody()->setVelocity(Vec2(0, 0));
	player->getPhysicsBody()->setVelocity(Vec2(0, 0));
  SimpleAudioEngine::getInstance()->stopBackgroundMusic("bgm.mp3");
  SimpleAudioEngine::getInstance()->playEffect("gameover.mp3", false);

  auto label1 = Label::createWithTTF("Game Over~", "fonts/STXINWEI.TTF", 60);
  label1->setColor(Color3B(0, 0, 0));
  label1->setPosition(visibleSize.width / 2, visibleSize.height / 2);
  this->addChild(label1);

  auto label2 = Label::createWithTTF("重玩", "fonts/STXINWEI.TTF", 40);
  label2->setColor(Color3B(0, 0, 0));
  auto replayBtn = MenuItemLabel::create(label2, CC_CALLBACK_1(HitBrick::replayCallback, this));
  Menu* replay = Menu::create(replayBtn, NULL);
  replay->setPosition(visibleSize.width / 2 - 80, visibleSize.height / 2 - 100);
  this->addChild(replay);

  auto label3 = Label::createWithTTF("退出", "fonts/STXINWEI.TTF", 40);
  label3->setColor(Color3B(0, 0, 0));
  auto exitBtn = MenuItemLabel::create(label3, CC_CALLBACK_1(HitBrick::exitCallback, this));
  Menu* exit = Menu::create(exitBtn, NULL);
  exit->setPosition(visibleSize.width / 2 + 90, visibleSize.height / 2 - 100);
  this->addChild(exit);
}

// 继续或重玩按钮响应函数
void HitBrick::replayCallback(Ref * pSender) {
  Director::getInstance()->replaceScene(HitBrick::createScene());
}

// 退出
void HitBrick::exitCallback(Ref * pSender) {
  Director::getInstance()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}
