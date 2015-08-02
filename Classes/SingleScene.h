//
//  SingleScene.h
//  IGGameC
//
//  Created by IllusionCui on 15/7/30.
//
//

#ifndef __IGGameC__SingleScene__
#define __IGGameC__SingleScene__

#include "common.h"
#include "GameLayer.h"

class SingleScene : public Scene {
public:
    CREATE_FUNC(SingleScene);
    
    void onSetting(Ref* pSender);

protected:
    SingleScene();
    ~SingleScene();
    
    virtual bool init();
    virtual void onEnter();
    
    void readConfig();
    
    bool showWindow(Node * node);
    bool hideWindow(CallFunc * finishCall);
    LayerColor * getMask();
    
    // 开始
    Node * createStart();
    void onStart(Ref* pSender);
    
    // 设置
    Node * createSetting();
    void onRestart(Ref* pSender);
    void onBack(Ref* pSender);
    
    // 结算
    Node * createEnd();
    void onAgain(Ref* pSender);
    
    // 游戏
    void startGame();
    void gameOver();
private:
    CC_SYNTHESIZE(Size, screenSize, ScreenSize);
    CC_SYNTHESIZE(Config *, m_config, Config);
    
    
    Node * m_currWindow;
    
    // 设置
    Node * m_setting;
    
    // 游戏
    GameLayer * m_gameLayer;
};

#endif /* defined(__IGGameC__SingleScene__) */
