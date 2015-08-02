//
//  GameLayer.cpp
//  Combo
//
//  Created by IllusionCui on 15/8/1.
//
//

#include "GameLayer.h"
#include "SingleScene.h"
#include "Cell.h"

GameLayer * GameLayer::create(SingleScene * scene) {
    GameLayer * res = new GameLayer();
    if (res && res->init(scene)) {
        res->autorelease();
        return res;
    } else {
        CC_SAFE_DELETE(res);
        return NULL;
    }
}

GameLayer::GameLayer() {
    m_data = NULL;
    m_scene = NULL;
    m_gridSize = 0;
    m_scoreLabel = NULL;
    m_comboLabel = NULL;
}

GameLayer::~GameLayer() {
    CC_SAFE_RELEASE_NULL(m_scoreLabel);
    CC_SAFE_RELEASE_NULL(m_comboLabel);
    m_currSelectLines.clear();
    m_cells.clear();
    m_currSelectCells.clear();
    CC_SAFE_FREE(m_data);
}

bool GameLayer::init(SingleScene * scene) {
    if (!Layer::init()) {
        return false;
    }
    
    m_scene = scene;
    
    initData();
    initBg();
    initCells();
    initUI();
    
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    touchListener->onTouchBegan = CC_CALLBACK_2(GameLayer::onTouchBegan,this);
    touchListener->onTouchMoved = CC_CALLBACK_2(GameLayer::onTouchMove,this);
    touchListener->onTouchEnded = CC_CALLBACK_2(GameLayer::onTouchEnd,this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
    
    return true;
}

void GameLayer::startGame() {
    log("GameLayer::startGame");
    resetData();
    startCells();
    updateUI();
}

bool GameLayer::onTouchBegan(Touch* touch, Event* event) {
    Vec2 pos = touch->getLocation();
    int index = getIndexOfCellsByPos(pos);
//    log("GameLayer::onTouchBegan    x = %f, y = %f, index = %d", pos.x, pos.y, index);
    if (index != -1) {
        selectCell(m_cells.at(index));
    }
    
    return true;
}

void GameLayer::onTouchMove(Touch* touch, Event* event) {
    Vec2 pos = touch->getLocation();
    int index = getIndexOfCellsByPos(pos);
//    log("GameLayer::onTouchMove    x = %f, y = %f, index = %d", pos.x, pos.y, index);
    if (index != -1) {
        selectCell(m_cells.at(index));
    }
}

void GameLayer::onTouchEnd(Touch* touch, Event* event) {
    Vec2 pos = touch->getLocation();
    int index = getIndexOfCellsByPos(pos);
//    log("GameLayer::onTouchEnd    x = %f, y = %f, index = %d", pos.x, pos.y, index);
    if (index != -1) {
        selectCell(m_cells.at(index));
    }

    long num = m_currSelectCells.size();
    if (num > 1) {
        Cell * endCell = m_currSelectCells.at(num - 1);
        int endValue = endCell->getValue();
        if (endValue != 0) { // 成功连线
            vector<Cell *> cells;
            for (int i = 0; i < m_cells.size(); i++) {
                Cell * cell = m_cells.at(i);
                if (-1 == cell->getValue()) {
                    // 将上一轮选中的障碍恢复
                    cell->setValue(0);
                    cells.push_back(cell);
                } else if (CellStatus::selected == cell->getStatus()) {
                    cell->setStatus(CellStatus::normal);
                    if (cell == endCell) {
                        cell->setValue(endValue*2);
                    } else {
                        // 将一轮选中的修改中间格子变成障碍
                        cell->setValue(-1);
                    }
                } else if (0 == cell->getValue()) {
                    cells.push_back(cell);
                }
            }
            
            // 新生成元素
            random_shuffle(cells.begin(), cells.end());
            cells[0]->setValue(1);
            
            // 更新数据
            m_data->score += endValue;
            m_data->combo++;
            updateUI();
        }
    }
    
    
    resetSelect();
}

void GameLayer::initData() {
    m_data = new GameData();
    
    Size size = m_scene->getScreenSize();
    Config * conf = m_scene->getConfig();
    float sizeX = size.width*conf->cheerWidthPercent/conf->cols;
    float sizeY = size.height*(1 - conf->topHeightPercent - conf->buttomHeightPercent)/conf->rows;
    m_gridSize = min(sizeX, sizeY);
    m_gridStartPos = Vec2((size.width - conf->cols*m_gridSize)/2, (size.height - conf->rows*m_gridSize)/2);
    log("GameLayer::initData    m_gridSize = %f, x = %f, y = %f", m_gridSize, m_gridStartPos.x, m_gridStartPos.y);
}

void GameLayer::resetData() {
    // 统计数据
    m_data->score = 0;
    m_data->newHighScore = false;
    m_data->combo = 0;
    m_data->newHighCombo = false;
}

void GameLayer::initBg() {
    log("GameLayer::initBg");
    
    Size size = m_scene->getScreenSize();
    Sprite * bg = Sprite::create("bg.png");
    Util::fitToSize(bg, size, false);
    bg->setPosition(Vec2(size.width/2, size.height/2));
    addChild(bg, -1);
    
    Config * conf = m_scene->getConfig();
    float gridEndX = m_gridStartPos.x + conf->cols*m_gridSize;
    float gridEndY = m_gridStartPos.y + conf->rows*m_gridSize;
    float lineWidth = m_gridSize*conf->gridLineWidthPercent;
    Color4F color = Color4F(50/255.0, 50/255.0, 50/255.0, 1);
    // 横线
    for(int i = 0; i <= conf->rows; i++) {
        DrawNode * drawNode = DrawNode::create();
        drawNode->drawSegment(Vec2(m_gridStartPos.x, m_gridStartPos.y + m_gridSize*i), Vec2(gridEndX, m_gridStartPos.y + m_gridSize*i), lineWidth, color);
        addChild(drawNode, -1);
    }
    // 竖线
    for(int i = 0; i <= conf->cols; i++) {
        DrawNode * drawNode = DrawNode::create();
        drawNode->drawSegment(Vec2(m_gridStartPos.x + m_gridSize*i, m_gridStartPos.y), Vec2(m_gridStartPos.x + m_gridSize*i, gridEndY), lineWidth, color);
        addChild(drawNode, -1);
    }
}

void GameLayer::initCells() {
    log("GameLayer::initCells");
    
    Config * conf = m_scene->getConfig();
    float rate = conf->cheerWidthPercent;
    for(int i = 0; i < conf->rows; i++) {
        for(int j = 0; j < conf->cols; j++) {
            Cell * newCell = Cell::create(i, j);
            newCell->setContentSize(Size(m_gridSize*rate, m_gridSize*rate));
            newCell->setPosition(j*m_gridSize + m_gridSize/2 + m_gridStartPos.x, i*m_gridSize + m_gridSize/2 + m_gridStartPos.y);
            addChild(newCell, 0);
            m_cells.pushBack(newCell);
        }
    }
}

void GameLayer::startCells() {
    // 显示数据
    for (int i = 0; i < m_cells.size(); i++) {
        m_cells.at(i)->setValue(rand()%2);
    }
}

int GameLayer::getIndexOfCellsByPos(Vec2 pos) {
    int res = -1;
    
    Config * conf = m_scene->getConfig();
    int x = (pos.x - m_gridStartPos.x)/m_gridSize;
    int y = (pos.y - m_gridStartPos.y)/m_gridSize;
    if (x >= 0 && x < conf->cols && y >= 0 && y < conf->rows) {
        res = y*conf->cols + x;
    }
    
    return res;
}

void GameLayer::initUI() {
    log("GameLayer::initUI");
    Size size = m_scene->getScreenSize();
    Config * conf = m_scene->getConfig();
    float y1 = size.height*(1 - conf->topHeightPercent/2);
    float y2 = size.height*(1 - conf->topHeightPercent);
    // score
    auto scoreLabel = Label::createWithTTF("Score: ", defaultTTF, 50);
    scoreLabel->setVerticalAlignment(TextVAlignment::CENTER);
    scoreLabel->setPosition(Vec2(size.width/5, y1));
    addChild(scoreLabel, 1);
    
    m_scoreLabel = Label::createWithTTF("0", defaultTTF, 50);
    CC_SAFE_RETAIN(m_scoreLabel);
    m_scoreLabel->setVerticalAlignment(TextVAlignment::CENTER);
    m_scoreLabel->setPosition(Vec2(size.width*2/5, y1));
    addChild(m_scoreLabel, 1);
    
    // lines
    auto comboLabel = Label::createWithTTF("Combo: ", defaultTTF, 50);
    comboLabel->setVerticalAlignment(TextVAlignment::CENTER);
    comboLabel->setPosition(Vec2(size.width/5, y2));
    addChild(comboLabel, 1);
    
    m_comboLabel = Label::createWithTTF("0", defaultTTF, 50);
    CC_SAFE_RETAIN(m_comboLabel);
    m_comboLabel->setVerticalAlignment(TextVAlignment::CENTER);
    m_comboLabel->setPosition(Vec2(size.width*2/5, y2));
    addChild(m_comboLabel, 1);
    
    // menu
    auto settingItem = MenuItemImage::create("setting.png", "", CC_CALLBACK_1(SingleScene::onSetting, m_scene));
    settingItem->setPosition(Vec2(size.width*9/10, size.height*9/10));
    auto menu = Menu::create(settingItem, NULL);
    menu->setPosition(Vec2::ZERO);
    addChild(menu, 1);
}

void GameLayer::updateUI() {
    m_scoreLabel->setString(Util::strFormat("%d", m_data->score));
    m_comboLabel->setString(Util::strFormat("%d", m_data->combo));
}

void GameLayer::selectCell(Cell * cell) {
    if (NULL == cell) {
        return;
    }
    
    long num = m_currSelectCells.size();
    bool add = true;
    if (num > 0) {
        long index = m_currSelectCells.getIndex(cell);
        if (index > -1) {
            if (index == num - 1) {
                // 选择同一个元素
                return;
            } else if (index == num - 2) {
                // 返回上一个元素删除
                add = false;
            } else {
                // 选择其他已选元素 无效
                return;
            }
        } // else index = -1 说明新添加元素
    }// else index = -1 说明新添加第一个元素
    
    if (add) {
        // 检测是否合法
        if (0 < num) { // 后选择元素必须跟本身匹配
            if (!cell->canBeMoveTo()) {
                return; // 不能移动到
            }
            int startValue = m_currSelectCells.at(0)->getValue();
            int selectValue = cell->getValue();
            if (selectValue != 0 && selectValue != startValue) {
                return;
            }
        } else { // 后选择元素必须跟本身匹配
            if (!cell->isSelectable()) {
                return; // 不能选择
            }
        }
        
        Cell * endCell = NULL;
        if (num > 0) {
            endCell = m_currSelectCells.at(num - 1);
            if (num > 1 && endCell->getValue() != 0) {
                // 当前已经合法不支持选择
                return;
            }
            
            // 新添连接线
            Color4F color = Color4F(120/255.0, 180/255.0, 20/255.0, 1);
            Vec2 pos1 = endCell->getPosition();
            Vec2 pos2 = cell->getPosition();
            DrawNode * drawNode = DrawNode::create();
            drawNode->drawSegment(pos1, pos2, m_gridSize*0.5, color);
            m_currSelectLines.pushBack(drawNode);
            addChild(drawNode);
        }
        
        cell->setStatus(CellStatus::selected);
        m_currSelectCells.pushBack(cell);
    } else {
        // 取消之前选择的Cell
        cell->setStatus(CellStatus::normal);
        m_currSelectCells.erase(m_currSelectCells.begin() + num - 1);
        
        if (num > 1) {
            // 删除连接线
            m_currSelectLines.at(num - 2)->removeFromParent();
            m_currSelectLines.erase(m_currSelectLines.begin() + num - 2);
        }
    }
    log("GameLayer::selectCell  cell = %p, row = %d, col = %d, num = %ld, add = %d", cell, cell->getRow(), cell->getCol(), num, add);
}

void GameLayer::resetSelect() {
    log("GameLayer::resetSelect");
    m_currSelectCells.clear();
    for (int i = 0; i < m_currSelectLines.size(); i++) {
        m_currSelectLines.at(i)->removeFromParent();
    }
    m_currSelectLines.clear();
}