#include "LoginRegisterScene.h"
#include "ui\CocosGUI.h"
#include "network\HttpClient.h"
#include "json\document.h"
#include "json\writer.h"
#include "json\stringbuffer.h"
#include "Utils.h"

USING_NS_CC;
using namespace cocos2d::network;
using namespace cocos2d::ui;
using namespace rapidjson;

cocos2d::Scene * LoginRegisterScene::createScene() {
  return LoginRegisterScene::create();
}

bool LoginRegisterScene::init() {
  if (!Scene::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  auto loginButton = MenuItemFont::create("Login", CC_CALLBACK_1(LoginRegisterScene::loginButtonCallback, this));
  if (loginButton) {
    float x = origin.x + visibleSize.width / 2;
    float y = origin.y + loginButton->getContentSize().height / 2;
    loginButton->setPosition(Vec2(x, y));
  }

  auto registerButton = MenuItemFont::create("Register", CC_CALLBACK_1(LoginRegisterScene::registerButtonCallback, this));
  if (registerButton) {
    float x = origin.x + visibleSize.width / 2;
    float y = origin.y + registerButton->getContentSize().height / 2 + 100;
    registerButton->setPosition(Vec2(x, y));
  }

  auto backButton = MenuItemFont::create("Back", [] (Ref* pSender) {
    Director::getInstance()->popScene();
  });
  if (backButton) {
    float x = origin.x + visibleSize.width / 2;
    float y = origin.y + visibleSize.height - backButton->getContentSize().height / 2;
    backButton->setPosition(Vec2(x, y));
  }

  auto menu = Menu::create(loginButton, registerButton, backButton, NULL);
  menu->setPosition(Vec2::ZERO);
  this->addChild(menu, 1);

  usernameInput = TextField::create("username", "arial", 24);
  if (usernameInput) {
    float x = origin.x + visibleSize.width / 2;
    float y = origin.y + visibleSize.height - 100.0f;
    usernameInput->setPosition(Vec2(x, y));
    this->addChild(usernameInput, 1);
  }

  passwordInput = TextField::create("password", "arial", 24);
  if (passwordInput) {
    float x = origin.x + visibleSize.width / 2;
    float y = origin.y + visibleSize.height - 130.0f;
    passwordInput->setPosition(Vec2(x, y));
    this->addChild(passwordInput, 1);
  }

  messageBox = Label::create("", "arial", 30);
  if (messageBox) {
    float x = origin.x + visibleSize.width / 2;
    float y = origin.y + visibleSize.height - 200.0f;
    messageBox->setPosition(Vec2(x, y));
    this->addChild(messageBox, 1);
  }

  return true;
}

void LoginRegisterScene::loginButtonCallback(cocos2d::Ref * pSender) {
	std::string username = usernameInput->getString();
	std::string password = passwordInput->getString();
	HttpRequest* request = new HttpRequest();
	request->setRequestType(HttpRequest::Type::POST);
	request->setUrl("http://127.0.0.1:8000/auth");
	request->setTag("Login");
	std::string postdata = getJson(username, password);
	request->setRequestData(postdata.c_str(), postdata.length());
	request->setResponseCallback(CC_CALLBACK_2(LoginRegisterScene::onLoginRegisterCompleted, this));
	HttpClient::getInstance()->send(request);
	HttpClient::getInstance()->enableCookies(NULL);
	request->release();
}

void LoginRegisterScene::registerButtonCallback(Ref * pSender) {
	std::string username = usernameInput->getString();
	std::string password = passwordInput->getString();
	HttpRequest* request = new HttpRequest();
	request->setRequestType(HttpRequest::Type::POST);
	request->setUrl("http://127.0.0.1:8000/users");
	request->setTag("Register");
	std::string postdata = getJson(username, password);
	request->setRequestData(postdata.c_str(), postdata.length());
	request->setResponseCallback(CC_CALLBACK_2(LoginRegisterScene::onLoginRegisterCompleted, this));
	HttpClient::getInstance()->send(request);
	request->release();
}

void LoginRegisterScene::onLoginRegisterCompleted(HttpClient* sender, HttpResponse* response) {
	if (!response) return;
	if (!response->isSucceed()) {
		CCLOG("response failed");
		CCLOG("error buffer: %s", response->getErrorBuffer());
		return;
	}
	std::vector<char> *buffer = response->getResponseData();
	//CCLOG("Http test, dump data: ");
	std::string result = "";
	for (unsigned int i = 0; i < buffer->size(); ++i)
		result += (*buffer)[i];

	CCLOG("%s", result.c_str());

	//从得到的json结果中解析出msg和status
	std::string msg;
	bool status;
	//创建rapidjson::Document类，用于操作json代码
	rapidjson::Document d;

	//解析json文件内容
	d.Parse<0>(result.c_str());
	//判断解析是否出错
	if (d.HasParseError()) {
		CCLOG("GetParseError: %s\n", d.GetParseError()); return;
	}

	//获取json中的数据
	if (d.IsObject()) {
		//是否有msg属性
		if (d.HasMember("msg")) msg = d["msg"].GetString();
		//是否有status属性
		if (d.HasMember("status")) status = d["status"].GetBool();
	}

	if (status == true) {
		if (strcmp(response->getHttpRequest()->getTag(), "Login") == 0)
			messageBox->setString("LoginOK");
		else messageBox->setString("RegisterOK");
	}
	else {
		if (strcmp(response->getHttpRequest()->getTag(), "Login") == 0)
			messageBox->setString("LoginFailed\n" + msg);
		else messageBox->setString("RegisterFailed\n" + msg);
	}
}

std::string LoginRegisterScene::getJson(std::string username, std::string password) {
	//创建rapidjson::Document类，用于操作json代码
	rapidjson::Document d;

	//获取分配器
	rapidjson::Document::AllocatorType& allocator = d.GetAllocator();

	//设置为对象格式
	d.SetObject();

	//往对象格式的json文件中添加数据
	d.AddMember("username", rapidjson::Value(username.c_str(), allocator), allocator);
	d.AddMember("password", rapidjson::Value(password.c_str(), allocator), allocator);

	//将json数据写入文件中
	StringBuffer buffer;
	rapidjson::Writer<StringBuffer> writer(buffer);
	d.Accept(writer);

	return buffer.GetString();
}