//
//  SingleScene.cpp
//  IGGameC
//
//  Created by IllusionCui on 15/7/30.
//
//

#include "SingleScene.h"

#define windowLayerShowHideActionTag    10086

SingleScene::SingleScene() {
    m_setting = NULL;
    m_currWindow = NULL;
    m_config = NULL;
    m_gameLayer = NULL;
}

SingleScene::~SingleScene() {
    CC_SAFE_RELEASE_NULL(m_setting);
    CC_SAFE_RELEASE_NULL(m_currWindow);
    CC_SAFE_RELEASE_NULL(m_gameLayer);
    CC_SAFE_FREE(m_config);
}

bool SingleScene::init() {
    if (!Scene::init()) {
        return false;
    }
    screenSize = Size(640, 1136);   //Director::getInstance()->getOpenGLView()->getFrameSize();
    
    m_config = new Config();
    
    readConfig();
    
    m_gameLayer = GameLayer::create(this);
    CC_SAFE_RETAIN(m_gameLayer);
    addChild(m_gameLayer);
    
    m_setting = createSetting();
    CC_SAFE_RETAIN(m_setting);
    
    return true;
}

void SingleScene::onEnter() {
    Scene::onEnter();
    
    showWindow(createStart());
}

void SingleScene::readConfig() {
    // 游戏相关
    m_config->rows = 8;
    m_config->cols = 8;
    m_config->type = GameType::recoverByLine;
    m_config->initWeights.clear();
    for (int i = 0; i < 5; i++) {
        m_config->initWeights.push_back(1000*(5-i));
    }
    
    // 布局相关
    m_config->gridLineWidthPercent = 0.02;
    m_config->cheerWidthPercent = 0.9;
    m_config->topHeightPercent = 0.15;
    m_config->buttomHeightPercent = 0.05;
}

// 窗口
LayerColor * SingleScene::getMask() {
    LayerColor * res = LayerColor::create(Color4B(0, 0, 0, 0), screenSize.width, screenSize.height);
    // bg
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    touchListener->onTouchBegan = [](Touch * t, Event* e)
    {
        log("SingleScene::getMask   mask touch begin");
        return true;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, res);
    return res;
}

bool SingleScene::showWindow(Node * node) {
    if (m_currWindow) {
        return false;
    }
    
    m_currWindow = node;
    CC_SAFE_RETAIN(m_currWindow);
    m_currWindow->setScale(0);
    Action * action = ScaleTo::create(0.3, 1);
    action->setTag(windowLayerShowHideActionTag);
    m_currWindow->runAction(action);
    addChild(m_currWindow, 1);
    
    return true;
}

bool SingleScene::hideWindow(CallFunc * finishCall) {
    if (m_currWindow == NULL || m_currWindow->getActionByTag(windowLayerShowHideActionTag)) {
        return false;
    }
    
    Action * action = NULL;
    if (finishCall) {
        action = Sequence::create(ScaleTo::create(0.3, 0), finishCall, RemoveSelf::create(), NULL);;
    } else {
        action = Sequence::create(ScaleTo::create(0.3, 0), RemoveSelf::create(), NULL);
    }
    action->setTag(windowLayerShowHideActionTag);
    m_currWindow->runAction(action);
    CC_SAFE_RELEASE_NULL(m_currWindow);
    
    return true;
}

void SingleScene::onStartAuto(Ref* pSender) {
    log("SingleScene::onStartAuto");
    m_config->type = GameType::autoRecover;
    hideWindow(CallFunc::create(CC_CALLBACK_0(SingleScene::startGame, this)));
}

void SingleScene::onStartLine(Ref* pSender) {
    log("SingleScene::onStartLine");
    m_config->type = GameType::recoverByLine;
    hideWindow(CallFunc::create(CC_CALLBACK_0(SingleScene::startGame, this)));
}

// 开始
Node * SingleScene::createStart() {
    Size size = screenSize;
    Layer * res = Layer::create();
    res->setContentSize(size);
    
    // 遮罩
    Node * mask = getMask();
    res->addChild(mask);
    
    // 背景
    LayerColor * bg = LayerColor::create(Color4B(255, 127,   0, 255), screenSize.width*4/5, screenSize.height*2/3);
    bg->ignoreAnchorPointForPosition(false);
    bg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    bg->setPosition(Vec2(size.width/2, size.height/2));
    res->addChild(bg);
    
    // 开始按钮
    auto startItem1 = MenuItemImage::create("start.png", "", CC_CALLBACK_1(SingleScene::onStartAuto, this));
    startItem1->setPosition(Vec2(size.width*3/5, size.height/2));
    auto startItem2 = MenuItemImage::create("start.png", "", CC_CALLBACK_1(SingleScene::onStartLine, this));
    startItem2->setPosition(Vec2(size.width*3/5, size.height/3));
    auto menu = Menu::create(startItem1, startItem2, NULL);
    menu->setPosition(Vec2::ZERO);
    res->addChild(menu);
    
    // 标题
    auto label = Label::createWithTTF("Start Game", defaultTTF, 90);
    label->setPosition(Vec2(size.width/2, size.height*2/3));
    res->addChild(label);
    auto label1 = Label::createWithTTF("auto", defaultTTF, 60);
    label1->setPosition(Vec2(size.width/3, size.height/2));
    res->addChild(label1);
    auto label2 = Label::createWithTTF("line", defaultTTF, 60);
    label2->setPosition(Vec2(size.width/3, size.height/3));
    res->addChild(label2);
    
    
    return res;
}

// 设置
Node * SingleScene::createSetting() {
    Size size = screenSize;
    Layer * res = Layer::create();
    res->setContentSize(size);
    
    // 遮罩
    Node * mask = getMask();
    res->addChild(mask);
    
    // 背景
    LayerColor * bg = LayerColor::create(Color4B(255, 127,   0, 255), screenSize.width*4/5, screenSize.height*2/3);
    bg->ignoreAnchorPointForPosition(false);
    bg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    bg->setPosition(Vec2(size.width/2, size.height/2));
    res->addChild(bg);
    
    // 关闭按钮
    auto closeItem = MenuItemImage::create("close.png", "", CC_CALLBACK_1(SingleScene::onBack, this));
    closeItem->setPosition(Vec2(size.width*3/4, size.height*3/4));
    auto startItem1 = MenuItemImage::create("start.png", "", CC_CALLBACK_1(SingleScene::onStartAuto, this));
    startItem1->setPosition(Vec2(size.width*3/5, size.height/2));
    auto startItem2 = MenuItemImage::create("start.png", "", CC_CALLBACK_1(SingleScene::onStartLine, this));
    startItem2->setPosition(Vec2(size.width*3/5, size.height/3));
    auto menu = Menu::create(closeItem, startItem1, startItem2, NULL);
    menu->setPosition(Vec2::ZERO);
    res->addChild(menu);
    
    // 标题
    auto label = Label::createWithTTF("Setting", defaultTTF, 90);
    label->setPosition(Vec2(size.width/2, size.height*2/3));
    res->addChild(label);
    auto label1 = Label::createWithTTF("auto", defaultTTF, 60);
    label1->setPosition(Vec2(size.width/3, size.height/2));
    res->addChild(label1);
    auto label2 = Label::createWithTTF("line", defaultTTF, 60);
    label2->setPosition(Vec2(size.width/3, size.height/3));
    res->addChild(label2);
    
    return res;
}

void SingleScene::onBack(Ref* pSender) {
    log("SingleScene::onBack");
    hideWindow(NULL);
}

// 结算
Node * SingleScene::createEnd() {
    Size size = screenSize;
    Layer * res = Layer::create();
    res->setContentSize(size);
    
    // 遮罩
    Node * mask = getMask();
    res->addChild(mask);
    
    // 背景
    LayerColor * bg = LayerColor::create(Color4B(255, 127,   0, 255), screenSize.width*4/5, screenSize.height*2/3);
    bg->ignoreAnchorPointForPosition(false);
    bg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    bg->setPosition(Vec2(size.width/2, size.height/2));
    res->addChild(bg);
    
    // 开始按钮
    auto startItem1 = MenuItemImage::create("start.png", "", CC_CALLBACK_1(SingleScene::onStartAuto, this));
    startItem1->setPosition(Vec2(size.width*3/5, size.height*2/5));
    auto startItem2 = MenuItemImage::create("start.png", "", CC_CALLBACK_1(SingleScene::onStartLine, this));
    startItem2->setPosition(Vec2(size.width*3/5, size.height/4));
    auto menu = Menu::create(startItem1, startItem2, NULL);
    menu->setPosition(Vec2::ZERO);
    res->addChild(menu);
    
    auto label1 = Label::createWithTTF("auto", defaultTTF, 60);
    label1->setPosition(Vec2(size.width/3, size.height*2/5));
    res->addChild(label1);
    auto label2 = Label::createWithTTF("line", defaultTTF, 60);
    label2->setPosition(Vec2(size.width/3, size.height/4));
    res->addChild(label2);
    
    // 标题
    auto label = Label::createWithTTF("Game Over", defaultTTF, 90);
    label->setPosition(Vec2(size.width/2, size.height*3/4));
    res->addChild(label);
    
    // 分数
    GameData * date = m_gameLayer->getGameDate();
    if (date->newHighScore) {
        auto scoreNew = Sprite::create("new.png");
        scoreNew->setPosition(Vec2(size.width/3, size.height*3/5));
        res->addChild(scoreNew);
    }
    auto scorelabel = Label::createWithTTF(Util::strFormat("Score: %d", date->score), defaultTTF, 70);
    scorelabel->setPosition(Vec2(date->newHighScore ? size.width*2/3 : size.width/2, size.height*3/5));
    res->addChild(scorelabel);
    
    // 连线
    if (date->newHighCombo) {
        auto comboNew = Sprite::create("new.png");
        comboNew->setPosition(Vec2(size.width/3, size.height/2));
        res->addChild(comboNew);
    }
    auto combolabel = Label::createWithTTF(Util::strFormat("Score: %d", date->score), defaultTTF, 70);
    combolabel->setPosition(Vec2(date->newHighCombo ? size.width*2/3 : size.width/2, size.height/2));
    res->addChild(combolabel);
    
    return res;
}

// 游戏
void SingleScene::onSetting(Ref* pSender) {
    log("SingleScene::onSetting");
    showWindow(m_setting);
}

void SingleScene::startGame() {
    m_gameLayer->startGame();
}

void SingleScene::gameOver() {
    showWindow(createEnd());
}