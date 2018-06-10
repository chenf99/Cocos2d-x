#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "Monster.h"
#include "sqlite3.h"
#include <string>
#pragma execution_character_set("utf-8")

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    visibleSize = Director::getInstance()->getVisibleSize();
    origin = Director::getInstance()->getVisibleOrigin();

	TMXTiledMap *tileMap = TMXTiledMap::create("map.tmx");
	tileMap->setPosition(visibleSize.width / 2, visibleSize.height / 2);
	tileMap->setAnchorPoint(Vec2(0.5, 0.5));
	tileMap->setScale(Director::getInstance()->getContentScaleFactor());
	this->addChild(tileMap, 0);

	//创建一张贴图
	auto texture = Director::getInstance()->getTextureCache()->addImage("$lucia_2.png");
	//从贴图中以像素单位切割，创建关键帧
	auto frame0 = SpriteFrame::createWithTexture(texture, CC_RECT_PIXELS_TO_POINTS(Rect(0, 0, 113, 113)));
	//使用第一帧创建精灵
	player = Sprite::createWithSpriteFrame(frame0);
	player->setPosition(Vec2(origin.x + visibleSize.width / 2,
		origin.y + visibleSize.height / 2));
	player->setName("player");
	addChild(player, 3);

	//hp条
	Sprite* sp0 = Sprite::create("hp.png", CC_RECT_PIXELS_TO_POINTS(Rect(0, 320, 420, 47)));
	Sprite* sp = Sprite::create("hp.png", CC_RECT_PIXELS_TO_POINTS(Rect(610, 362, 4, 16)));

	//使用hp条设置progressBar
	pT = ProgressTimer::create(sp);
	pT->setScaleX(90);
	pT->setAnchorPoint(Vec2(0, 0));
	pT->setType(ProgressTimerType::BAR);
	pT->setBarChangeRate(Point(1, 0));
	pT->setMidpoint(Point(0, 1));
	pT->setPercentage(100);
	pT->setPosition(Vec2(origin.x + 14 * pT->getContentSize().width, origin.y + visibleSize.height - 2 * pT->getContentSize().height));
	addChild(pT, 1);
	sp0->setAnchorPoint(Vec2(0, 0));
	sp0->setPosition(Vec2(origin.x + pT->getContentSize().width, origin.y + visibleSize.height - sp0->getContentSize().height));
	addChild(sp0, 0);

	// 静态动画
	idle.reserve(1);
	idle.pushBack(frame0);

	// 攻击动画
	attack.reserve(17);
	for (int i = 0; i < 17; i++) {
		auto frame = SpriteFrame::createWithTexture(texture, CC_RECT_PIXELS_TO_POINTS(Rect(113 * i, 0, 113, 113)));
		attack.pushBack(frame);
	}
	auto attack_animation = Animation::createWithSpriteFrames(attack, 0.1f);
	//动画结束后，回到初始帧
	attack_animation->setRestoreOriginalFrame(true);
	AnimationCache::getInstance()->addAnimation(attack_animation, "attack");

	// 可以仿照攻击动画
	// 死亡动画(帧数：22帧，高：90，宽：79）
	auto texture2 = Director::getInstance()->getTextureCache()->addImage("$lucia_dead.png");
	// Todo
	dead.reserve(22);
	for (int i = 0; i < 22; ++i) {
		auto frame = SpriteFrame::createWithTexture(texture2, CC_RECT_PIXELS_TO_POINTS(Rect(79 * i, 0, 79, 90)));
		dead.pushBack(frame);
	}
	auto dead_animation = Animation::createWithSpriteFrames(dead, 0.1f);
	dead_animation->setRestoreOriginalFrame(true);
	AnimationCache::getInstance()->addAnimation(dead_animation, "dead");

	// 运动动画(帧数：8帧，高：101，宽：68）
	auto texture3 = Director::getInstance()->getTextureCache()->addImage("$lucia_forward.png");
	// Todo
	run.reserve(8);
	for (int i = 0; i < 8; ++i) {
		auto frame = SpriteFrame::createWithTexture(texture3, CC_RECT_PIXELS_TO_POINTS(Rect(68 * i, 0, 68, 101)));
		run.pushBack(frame);
	}
	auto run_animation = Animation::createWithSpriteFrames(run, 0.03f);
	run_animation->setRestoreOriginalFrame(false);
	AnimationCache::getInstance()->addAnimation(run_animation, "run");


	//添加虚拟按键，用MenuItem实现
	auto x = Label::create("X", "fonts/arial.ttf", 36);
	auto y = Label::create("Y", "fonts/arial.ttf", 36);
	auto x_menuItem = MenuItemLabel::create(x, CC_CALLBACK_1(HelloWorld::X, this));
	x_menuItem->setPosition(visibleSize.width - 18, 100);
	auto y_menuItem = MenuItemLabel::create(y, CC_CALLBACK_1(HelloWorld::Y, this));
	y_menuItem->setPosition(visibleSize.width - 50, 50);

	auto w = Label::create("W", "fonts/arial.ttf", 36);
	auto a = Label::create("A", "fonts/arial.ttf", 36);
	auto s = Label::create("S", "fonts/arial.ttf", 36);
	auto d = Label::create("D", "fonts/arial.ttf", 36);

	auto w_menuItem = MenuItemLabel::create(w, 
	[&](Ref* sender) {
		auto move = MoveBy::create(0.1f, Vec2(0, 20));
		Animate* animate = Animate::create(AnimationCache::getInstance()->getAnimation("run"));
		if (player->getPositionY() + 20 >= visibleSize.height) {
			player->runAction(MoveTo::create(0.8f, Vec2(player->getPositionX(), visibleSize.height - 20)));
		}
		else player->runAction(move);
		player->runAction(animate);
	});
	auto s_menuItem = MenuItemLabel::create(s,
		[&](Ref* sender) {
		auto move = MoveBy::create(0.1f, Vec2(0, -20));
		Animate* animate = Animate::create(AnimationCache::getInstance()->getAnimation("run"));
		if (player->getPositionY() - 20 <= 0) {
			player->runAction(MoveTo::create(0.8f, Vec2(player->getPositionX(), 20)));
		}
		else player->runAction(move);
		player->runAction(animate);
	});
	auto a_menuItem = MenuItemLabel::create(a,
		[&](Ref* sender) {
		player->setFlipX(true);
		auto move = MoveBy::create(0.1f, Vec2(-20, 0));
		Animate* animate = Animate::create(AnimationCache::getInstance()->getAnimation("run"));
		if (player->getPositionX() - 20 <= 0) {
			player->runAction(MoveTo::create(0.8f, Vec2(20, player->getPositionY())));
		}
		else player->runAction(move);
		player->runAction(animate);
	});
	auto d_menuItem = MenuItemLabel::create(d,
		[&](Ref* sender) {
		player->setFlipX(false);
		auto move = MoveBy::create(0.1f, Vec2(20, 0));
		Animate* animate = Animate::create(AnimationCache::getInstance()->getAnimation("run"));
		if (player->getPositionX() + 20 >= visibleSize.width) {
			player->runAction(MoveTo::create(0.8f, Vec2(visibleSize.width - 20, player->getPositionY())));
		}
		else player->runAction(move);
		player->runAction(animate);
	});
	w_menuItem->setPosition(50, 100);
	s_menuItem->setPosition(50, 11);
	a_menuItem->setPosition(11, 50);
	d_menuItem->setPosition(100, 50);

	auto menu = Menu::create(x_menuItem, y_menuItem, w_menuItem, s_menuItem, a_menuItem, d_menuItem,  NULL);
	menu->setPosition(Vec2::ZERO);
	this->addChild(menu, 1);

	//添加倒计时
	time = Label::createWithTTF("120", "fonts/arial.ttf", 36);
	time->setPosition(visibleSize.width / 2, visibleSize.height - 100);
	this->addChild(time, 1);
	dtime = 120;
	schedule(schedule_selector(HelloWorld::UpdateTime), 1.0f, kRepeatForever, 0);

	//添加判断怪物碰撞的调度器，每0.1秒触发一次
	schedule(schedule_selector(HelloWorld::hitByMonster), 0.1f, kRepeatForever, 0);

	//创建事件监听器监听键盘事件
	auto myKeyListener = EventListenerKeyboard::create(); 
	/*
	//键盘按键按下时的响应													  
	myKeyListener->onKeyPressed = [](EventKeyboard::KeyCode keycode, cocos2d::Event *event)
	{
		CCLOG("key is pressed, keycode is %d", keycode);
	};
	*/
	//键盘按键弹回时的响应
	myKeyListener->onKeyReleased = [&](EventKeyboard::KeyCode keycode, cocos2d::Event *event)
	{
		Action* move;
		Animate* animate;
		switch (keycode)
		{
		case cocos2d::EventKeyboard::KeyCode::KEY_A:
		case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
			player->setFlipX(true);
			move = MoveBy::create(0.1f, Vec2(-20, 0));
			animate = Animate::create(AnimationCache::getInstance()->getAnimation("run"));
			if (player->getPositionX() - 20 <= 0) {
				player->runAction(MoveTo::create(0.8f, Vec2(20, player->getPositionY())));
			}
			else player->runAction(move);
			player->runAction(animate);
			break;

		case cocos2d::EventKeyboard::KeyCode::KEY_D:
		case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
			player->setFlipX(false);
			move = MoveBy::create(0.1f, Vec2(20, 0));
			animate = Animate::create(AnimationCache::getInstance()->getAnimation("run"));
			if (player->getPositionX() + 20 >= visibleSize.width) {
				player->runAction(MoveTo::create(0.8f, Vec2(visibleSize.width - 20, player->getPositionY())));
			}
			else player->runAction(move);
			player->runAction(animate);
			break;

		case cocos2d::EventKeyboard::KeyCode::KEY_W:
		case cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW:
			move = MoveBy::create(0.1f, Vec2(0, 20));
			animate = Animate::create(AnimationCache::getInstance()->getAnimation("run"));
			if (player->getPositionY() + 20 >= visibleSize.height) {
				player->runAction(MoveTo::create(0.8f, Vec2(player->getPositionX(), visibleSize.height - 20)));
			}
			else player->runAction(move);
			player->runAction(animate);
			break;

		case cocos2d::EventKeyboard::KeyCode::KEY_S:
		case cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW:
			move = MoveBy::create(0.1f, Vec2(0, -20));
			animate = Animate::create(AnimationCache::getInstance()->getAnimation("run"));
			if (player->getPositionY() - 20 <= 0) {
				player->runAction(MoveTo::create(0.8f, Vec2(player->getPositionX(), 20)));
			}
			else player->runAction(move);
			player->runAction(animate);
			break;
		
		case cocos2d::EventKeyboard::KeyCode::KEY_X:
			X(NULL);
			break;

		case cocos2d::EventKeyboard::KeyCode::KEY_Y:
			Y(NULL);
			break;
		default:
			break;
		}
	};
	//将事件监听器与场景绑定
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(myKeyListener, this);

	//使用sqlite3记录打死怪物的数量
	//数据库指针
	sqlite3* pdb = NULL;
	//数据库路径
	std::string path = "DataBase/save.db";
	//根据路径打开或创建数据库
	int	result = sqlite3_open(path.c_str(), &pdb);//若成功reult == SQLITE_OK
	std::string sql = "create table killedMonsters(ID int primary key not null, Num int not null);";
	result = sqlite3_exec(pdb, sql.c_str(), NULL, NULL, NULL);
	sql = "insert into killedMonsters values(1, 0)";
	result = sqlite3_exec(pdb, sql.c_str(), NULL, NULL, NULL);
	
	char **re;//查询结果
	int row, col;
	sqlite3_get_table(pdb, "select * from killedMonsters", &re, &row, &col, NULL);
	//显示数量
	killed = Label::createWithTTF(re[3], "fonts/arial.ttf", 36);//re[3]对应表中存储的杀死怪物数量
	killedMonsters = std::atoi(re[3]);
	sqlite3_free_table(re);//一定要释放指针
	killed->setPosition(visibleSize.width / 2, visibleSize.height - 50);
	this->addChild(killed, 1);
	return true;
}

void HelloWorld::X(Ref* sender) {
	if (done == true) {
		auto animation = AnimationCache::getInstance()->getAnimation("dead");
		auto animate = Animate::create(animation);
		auto action = CallFunc::create([&]() {done = true; });
		auto seq = Sequence::create(animate, action, NULL);
		done = false;
		player->runAction(seq);
		auto action2 = ProgressTo::create(1.0f, pT->getPercentage() - 20);
		pT->runAction(action2);
	}
}

void HelloWorld::Y(Ref* sender) {
	if (done == true) {
		auto animation = AnimationCache::getInstance()->getAnimation("attack");
		auto animate = Animate::create(animation);
		auto action = CallFunc::create([&]() {done = true; });
		auto seq = Sequence::create(animate, action, NULL);
		done = false;
		player->runAction(seq);
		//判断是否打到怪物
		Rect playerRect = player->getBoundingBox();
		Rect attackRect = Rect(playerRect.getMinX() - 40, playerRect.getMinY(),
			playerRect.getMaxX() - playerRect.getMinX() + 80,
			playerRect.getMaxY() - playerRect.getMinY());
		auto fac = Factory::getInstance();
		Sprite* collision = fac->collider(attackRect);
		if (collision != NULL) {
			fac->removeMonster(collision);
			auto action2 = ProgressTo::create(1.0f, pT->getPercentage() + 20);
			pT->runAction(action2);

			killedMonsters++;
			char str[5];
			sprintf(str, "%d", killedMonsters);
			killed->setString(str);

			std::string str1 = str;
			sqlite3* pdb = NULL;
			std::string path = "DataBase/save.db";
			int	result = sqlite3_open(path.c_str(), &pdb);
			std::string sql = "update killedMonsters set Num = " + str1 + " where ID = 1;";
			sqlite3_exec(pdb, sql.c_str(), NULL, NULL, NULL);
		}
	}
}

void HelloWorld::UpdateTime(float dt) {
	dtime--;
	if (dtime == 0) unschedule(schedule_selector(HelloWorld::UpdateTime));
	char str[5];
	sprintf(str, "%d", dtime);
	time->setString(str);

	//添加怪物
	auto fac = Factory::getInstance();
	auto m = fac->createMonster();
	float positionx = random(origin.x, visibleSize.width);
	float positiony = random(origin.y, visibleSize.height);
	m->setPosition(positionx, positiony);
	this->addChild(m, 3);

	//怪物向角色移动
	fac->moveMonster(player->getPosition(), 1.0f);
}

void HelloWorld::hitByMonster(float dt) {
	auto fac = Factory::getInstance();
	Sprite* collision = fac->collider(player->getBoundingBox());
	if (collision != NULL) {
		//减少角色血量
		auto action = ProgressTo::create(1.0f, pT->getPercentage() - 20);
		pT->runAction(action);
		fac->removeMonster(collision);
	}
	if (pT->getPercentage() <= 0) {
		auto animation = AnimationCache::getInstance()->getAnimation("dead");
		animation->setRestoreOriginalFrame(false);
		auto animate = Animate::create(animation);
		player->runAction(animate);
		time->setString("Game Over!");
		//取消所有事件监听器
		Director::getInstance()->getEventDispatcher()->removeAllEventListeners();
		unschedule(schedule_selector(HelloWorld::UpdateTime));
		unschedule(schedule_selector(HelloWorld::hitByMonster));
	}
}