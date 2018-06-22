#include "UsersInfoScene.h"
#include "network\HttpClient.h"
#include "json\document.h"
#include "Utils.h"

using namespace cocos2d::network;
using namespace rapidjson;

cocos2d::Scene * UsersInfoScene::createScene() {
  return UsersInfoScene::create();
}

bool UsersInfoScene::init() {
  if (!Scene::init()) return false;

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  auto getUserButton = MenuItemFont::create("Get User", CC_CALLBACK_1(UsersInfoScene::getUserButtonCallback, this));
  if (getUserButton) {
    float x = origin.x + visibleSize.width / 2;
    float y = origin.y + getUserButton->getContentSize().height / 2;
    getUserButton->setPosition(Vec2(x, y));
  }

  auto backButton = MenuItemFont::create("Back", [](Ref* pSender) {
    Director::getInstance()->popScene();
  });
  if (backButton) {
    float x = origin.x + visibleSize.width / 2;
    float y = origin.y + visibleSize.height - backButton->getContentSize().height / 2;
    backButton->setPosition(Vec2(x, y));
  }

  auto menu = Menu::create(getUserButton, backButton, NULL);
  menu->setPosition(Vec2::ZERO);
  this->addChild(menu, 1);

  limitInput = TextField::create("limit", "arial", 24);
  if (limitInput) {
    float x = origin.x + visibleSize.width / 2;
    float y = origin.y + visibleSize.height - 100.0f;
    limitInput->setPosition(Vec2(x, y));
    this->addChild(limitInput, 1);
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

void UsersInfoScene::getUserButtonCallback(Ref * pSender) {
	std::string limit = limitInput->getString();
	HttpRequest* request = new HttpRequest();
	request->setUrl("http://127.0.0.1:8000/users?limit=" + limit);
	request->setRequestType(HttpRequest::Type::GET);
	request->setResponseCallback(CC_CALLBACK_2(UsersInfoScene::getUsersInfoCompleted, this));
	request->setTag("Get");
	HttpClient::getInstance()->send(request);
	request->release();
}

void UsersInfoScene::getUsersInfoCompleted(HttpClient* sender, HttpResponse* response) {
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

	//从得到的json结果中解析出data,为数组形式
	std::string info = "";

	rapidjson::Document d;
	d.Parse<0>(result.c_str());
	if (d.HasParseError()) {
		CCLOG("GetParseError: %s\n", d.GetParseError()); return;
	}
	//查询失败
	if (d["status"] == false) {
		std::string msg = d["msg"].GetString();
		messageBox->setString("Get User Failed\n" + msg);
		return;
	}

	rapidjson::Value& data = d["data"];
	for (int i = 0; i < data.Size(); ++i) {				
		info += "Username: ";
		info += data[i]["username"].GetString();
		info += "\n";
		//获得卡组信息
		info += "Deck:\n";
		rapidjson::Value& deck = data[i]["deck"];
		for (int j = 0; j < deck.Size(); ++j) {
			rapidjson::Value& object = deck[j];
			for (auto iterator = object.MemberBegin(); iterator != object.MemberEnd(); ++iterator) {
				info += "  ";
				info += iterator->name.GetString();
				info += " : ";
				info += std::to_string(iterator->value.GetInt());
				info += "\n";
			}
			info += "  ---\n";
		}
		info += "---\n";
	}
	messageBox->setString(info);
}
