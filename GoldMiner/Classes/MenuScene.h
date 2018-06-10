#pragma once
#include <stdio.h>
#include "cocos2d.h"

class MenuScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
	// a selector callback
	//void startMenuCallback(cocos2d::Ref* pSender);
    
	void changeScene(cocos2d::Ref* psender);

    // implement the "static create()" method manually
    CREATE_FUNC(MenuScene);

};
