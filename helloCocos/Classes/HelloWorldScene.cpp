#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

bool firstIn = true;
int count = 0;

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

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

    if (closeItem == nullptr ||
        closeItem->getContentSize().width <= 0 ||
        closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width/2;
        float y = origin.y + closeItem->getContentSize().height/2;
        closeItem->setPosition(Vec2(x,y));
    }

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
	
	/*// add a "change" text to change the elephant. it's an autorelease object
	auto changeElephant = MenuItemFont::create("Click Me", CC_CALLBACK_1(HelloWorld::ClickMe, this));
	changeElephant->setFontName("fonts/Marker Felt.ttf");
	changeElephant->setColor(ccc3(0, 0, 0));
	changeElephant->setFontSize(14);*/

	/*if (changeElephant == nullptr)
	{
		problemLoading("'fonts/Marker Felt.ttf'");
	}
	else
	{
		float x = origin.x + visibleSize.width - changeElephant->getContentSize().width / 2;
		float y = closeItem->getPositionY() + changeElephant->getContentSize().height;
		changeElephant->setPosition(Vec2(x, y));
	}*/

	/*// create menu, it's an autorelease object
	auto menu_change = Menu::create(changeElephant, NULL);
	menu_change->setPosition(Vec2::ZERO);
	this->addChild(menu_change, 1);*/

	auto start = MenuItemFont::create("ON");
	start->setFontName("fonts/Marker Felt.ttf");
	start->setColor(ccc3(255, 0, 0));

	auto stop = MenuItemFont::create("OFF");
	stop->setFontName("fonts/Marker Felt.ttf");
	stop->setColor(ccc3(0, 0, 0));

	auto toggle = MenuItemToggle::createWithCallback(CC_CALLBACK_1(HelloWorld::ClickMe, this), stop, start, NULL);
	float x = origin.x + visibleSize.width - toggle->getContentSize().width / 2;
	float y = closeItem->getPositionY() + toggle->getContentSize().height;
	toggle->setPosition(Vec2(x, y));
	toggle->setName("OFF");

	auto menu_change = Menu::create(toggle, NULL);
	menu_change->setPosition(Vec2::ZERO);
	this->addChild(menu_change, 1);

    /////////////////////////////
    // 3. add your codes below...

	//创建词典类实例，并将xml文件加载到词典中
	auto *chnStrings = Dictionary::createWithContentsOfFile("name.xml");
	const char *name_str = ((String*)chnStrings->objectForKey("name"))->getCString();

    // add a label shows "Hello World"
    // create and initialize a label

    auto name_label = Label::createWithTTF(name_str, "fonts/STKAITI.TTF", 30);
    if (name_label == nullptr)
    {
        problemLoading("'fonts/STXINGKA.TTF'");
    }
    else
    {
        // position the label on the center of the screen
        name_label->setPosition(Vec2(origin.x + visibleSize.width/2,
                                origin.y + visibleSize.height - name_label->getContentSize().height));

        // add the label as a child to this layer
        this->addChild(name_label, 1);
		name_label->setColor(ccc3(255, 255, 255));
    }

	auto id_label = Label::createWithTTF("16340017", "fonts/Marker Felt.ttf", 24);
	if (id_label == nullptr)
	{
		problemLoading("'fonts/Marker Felt.ttf'");
	}
	else
	{
		// position the label on the center of the screen
		id_label->setPosition(Vec2(origin.x + visibleSize.width / 2,
			origin.y + visibleSize.height - id_label->getContentSize().height - name_label->getContentSize().height));

		// add the label as a child to this layer
		this->addChild(id_label, 1);
		id_label->setColor(ccc3(0, 0, 0));
	}

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("backgroud.jpg");
    if (sprite == nullptr)
    {
        problemLoading("'backgroud.jpg'");
    }
    else
    {
        // position the sprite on the center of the screen
        sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

        // add the sprite as a child to this layer
        this->addChild(sprite, 0);
    }

	//add another sprite
	auto elephant = Sprite::create("elephant1_Normal.png");
	elephant->setName("normalElephant");
	if (elephant == nullptr) {
		problemLoading("'elephant1_Normal.png'");
	}
	else {
		// position the sprite
		elephant->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2));

		// add the elephant as a child to this layer
		this->addChild(elephant, 0);
	}
	
    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() and exit(0) as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);


}

void HelloWorld::ClickMe(Ref* sender) {
	auto change = (Sprite*)sender; 
	auto menu = change->getParent();
	auto scene = menu->getParent();
	auto elephant = (Sprite*)scene->getChildByName("normalElephant");
	if (change->getName() == "OFF") {
		//change->setColor(ccc3(255, 0, 0));
		if (firstIn == true) {
			Action* jump = JumpBy::create(60, ccp(0, 0), 100, 100);
			jump->setTag(1);
			elephant->runAction(jump);
			firstIn = false;
		}
		elephant->resume();
		change->setName("ON");
		Texture2D* texture = TextureCache::sharedTextureCache()->addImage("elephant1_Changed.png");
		elephant->setTexture(texture);
	}
	else {
		//change->setColor(ccc3(0, 0, 0));
		elephant->pause();
		change->setName("OFF");
		Texture2D* texture = TextureCache::sharedTextureCache()->addImage("elephant1_Normal.png");
		elephant->setTexture(texture);
	}
	//elephant->setRotation(count * 90);
	//count++;
	//count %= 4;
}