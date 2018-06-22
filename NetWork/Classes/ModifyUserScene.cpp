#include "ModifyUserScene.h"
#include "Utils.h"
#include "network\HttpClient.h"
#include "json\document.h"

using namespace cocos2d::network;

cocos2d::Scene * ModifyUserScene::createScene() {
  return ModifyUserScene::create();
}

bool ModifyUserScene::init() {
  if (!Scene::init()) return false;
  
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  auto postDeckButton = MenuItemFont::create("Post Deck", CC_CALLBACK_1(ModifyUserScene::putDeckButtonCallback, this));
  if (postDeckButton) {
    float x = origin.x + visibleSize.width / 2;
    float y = origin.y + postDeckButton->getContentSize().height / 2;
    postDeckButton->setPosition(Vec2(x, y));
  }

  auto backButton = MenuItemFont::create("Back", [](Ref* pSender) {
    Director::getInstance()->popScene();
  });
  if (backButton) {
    float x = origin.x + visibleSize.width / 2;
    float y = origin.y + visibleSize.height - backButton->getContentSize().height / 2;
    backButton->setPosition(Vec2(x, y));
  }

  auto menu = Menu::create(postDeckButton, backButton, NULL);
  menu->setPosition(Vec2::ZERO);
  this->addChild(menu, 1);

  deckInput = TextField::create("Deck json here", "arial", 24);
  if (deckInput) {
    float x = origin.x + visibleSize.width / 2;
    float y = origin.y + visibleSize.height - 100.0f;
    deckInput->setPosition(Vec2(x, y));
    this->addChild(deckInput, 1);
  }

  messageBox = Label::create("", "arial", 30);
  if (messageBox) {
    float x = origin.x + visibleSize.width / 2;
    float y = origin.y + visibleSize.height / 2;
    messageBox->setPosition(Vec2(x, y));
    this->addChild(messageBox, 1);
  }

  return true;
}

void ModifyUserScene::putDeckButtonCallback(Ref * pSender) {
	std::string input = deckInput->getString();
	HttpRequest* request = new HttpRequest();
	request->setRequestType(HttpRequest::Type::PUT);
	request->setUrl("http://127.0.0.1:8000/users");
	request->setTag("Register");
	std::string json = "{\"deck\":" + input + "}";
	request->setRequestData(json.c_str(), json.length());
	request->setResponseCallback(CC_CALLBACK_2(ModifyUserScene::onModifyUserCompleted, this));
	HttpClient::getInstance()->send(request);
	request->release();
}

void ModifyUserScene::onModifyUserCompleted(HttpClient* sender, HttpResponse* response) {
	if (!response) return;
	if (!response->isSucceed()) {
		CCLOG("response failed");
		CCLOG("error buffer: %s", response->getErrorBuffer());
		return;
	}
	std::vector<char> *buffer = response->getResponseData();
	
	std::string result = "";
	for (unsigned int i = 0; i < buffer->size(); ++i)
		result += (*buffer)[i];

	CCLOG("%s", result.c_str());

	rapidjson::Document d;
	d.Parse<0>(result.c_str());
	if (d["status"].GetBool() == false) {
		std::string msg = "PUT Failed\n";
		msg += d["msg"].GetString();
		messageBox->setString(msg);
	}
	else messageBox->setString("PUT OK");
}