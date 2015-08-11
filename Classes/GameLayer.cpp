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
    m_debugInfoLabel = NULL;
}

GameLayer::~GameLayer() {
    CC_SAFE_RELEASE_NULL(m_scoreLabel);
    CC_SAFE_RELEASE_NULL(m_comboLabel);
    CC_SAFE_RELEASE_NULL(m_debugInfoLabel);
    m_currSelectLines.clear();
    m_cells.clear();
    m_currSelectCells.clear();
    m_hintCells.clear();
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
        userSelectCell(m_cells.at(index));
    }
    
    return true;
}

void GameLayer::onTouchMove(Touch* touch, Event* event) {
    Vec2 pos = touch->getLocation();
    int index = getIndexOfCellsByPos(pos);
//    log("GameLayer::onTouchMove    x = %f, y = %f, index = %d", pos.x, pos.y, index);
    if (index != -1) {
        userSelectCell(m_cells.at(index));
    }
}

void GameLayer::onTouchEnd(Touch* touch, Event* event) {
    Vec2 pos = touch->getLocation();
    int index = getIndexOfCellsByPos(pos);
//    log("GameLayer::onTouchEnd    x = %f, y = %f, index = %d", pos.x, pos.y, index);
    if (index != -1) {
        userSelectCell(m_cells.at(index));
    }

    bool succCombo = false;
    long num = m_currSelectCells.size();
    if (num > 1) {
        Cell * endCell = m_currSelectCells.at(num - 1);
        int endValue = endCell->getValue();
        if (endValue != 0) { // 成功连线
            if (GameType::autoRecover == m_scene->getConfig()->type) {
                autoRecover(endCell);
            } else {
                recoverByLine(endCell);
            }
            
            // 更新结束位置的值
            endCell->setValue(endValue + 1);
            
            // 更新数据
            m_data->score += endValue;
            m_data->combo++;
            updateUI();
            succCombo = true;
        }
    }
    resetSelect();
    
    if (succCombo) {
        checkFailed();
    }
    updateDebugInfo();
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
    Color4F color = Color4F(0, 0, 0.8, 1);
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
    log("GameLayer::startCells");
    // 显示数据
    m_initCellCount = 0;
    for (int i = 0; i < m_cells.size(); i++) {
        int value = Util::getIndexByWeight(m_scene->getConfig()->initWeights);
        m_cells.at(i)->setValue(value);
        if (value > 0) {
            m_initCellCount++;
        }
    }
    updateDebugInfo();
}

int GameLayer::getIndexOfCellsByPos(Vec2 pos) {
    int col = (pos.x - m_gridStartPos.x)/m_gridSize;
    int row = (pos.y - m_gridStartPos.y)/m_gridSize;
    
    return getIndexByRowAndCol(row, col);
}

int GameLayer::getIndexByRowAndCol(int row, int col) {
    int res = -1;
    
    Config * conf = m_scene->getConfig();
    if (col >= 0 && col < conf->cols && row >= 0 && row < conf->rows) {
        res = row*conf->cols + col;
    }
    
    return res;
}

int GameLayer::getValueCellCount() {
    int res = 0;
    
    for (int i = 0; i < m_cells.size(); i++) {
        if (m_cells.at(i)->getValue() > 0) {
            res++;
        }
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
    
    //debug info
    m_debugInfoLabel = Label::createWithTTF("0", defaultTTF, 30);
    CC_SAFE_RETAIN(m_debugInfoLabel);
    m_debugInfoLabel->setVerticalAlignment(TextVAlignment::CENTER);
    m_debugInfoLabel->setPosition(Vec2(size.width/2, size.height/6));
    addChild(m_debugInfoLabel, 1);
}

void GameLayer::updateUI() {
    m_scoreLabel->setString(Util::strFormat("%d", m_data->score));
    m_comboLabel->setString(Util::strFormat("%d", m_data->combo));
}

void GameLayer::updateDebugInfo() {
    m_debugInfoLabel->setString(Util::strFormat("init_count:%d,curr_count:%d%\n%s", m_initCellCount, getValueCellCount(), getHintCellsInfo().c_str()));
}

int GameLayer::selectCell(Cell * cell, Vector<Cell *> &selectCells) {
    if (NULL == cell) {
        return 0;
    }
//    log("GameLayer::selectCell  cell = %p, row = %d, col = %d", cell, cell->getRow(), cell->getCol());
    
    long num = selectCells.size();
    bool add = true;
    Cell * endCell = NULL;
    if (num > 0) {
        endCell = selectCells.at(num - 1);
        long index = selectCells.getIndex(cell);
        if (index > -1) {
            if (index == num - 1) {
                // 选择同一个元素
                return 0;
            } else if (index == num - 2) {
                // 返回上一个元素删除
                add = false;
            } else {
                // 选择其他已选元素 无效
                return 0;
            }
        } // else index = -1 说明新添加元素
    }// else index = -1 说明新添加第一个元素
    
    if (add) {
        // 检测是否合法
        if (0 < num) { // 后选择元素必须跟本身匹配
            if (num > 1 && endCell->getValue() > 0) {
                return 0;   // 已经完成选择
            }
            if (!cell->canBeMoveTo()) {
                return 0; // 不能移动到
            }
            int disRow = cell->getRow() - endCell->getRow();
            int disCol = cell->getCol() - endCell->getCol();
            if (!(0 == disRow && (disCol == -1 || disCol == 1)) && !(0 == disCol && (disRow == -1 || disRow == 1))) {
                // 不是最后元素的正上下左右方
                return 0;
            }
            
            int startValue = selectCells.at(0)->getValue();
            int selectValue = cell->getValue();
            if (selectValue != 0 && selectValue != startValue) {
                return 0;
            }
        } else { // 后选择元素必须跟本身匹配
            if (!cell->isSelectable()) {
                return 0; // 不能选择
            }
        }
        
        return 1;
    } else {
        return -1;
    }
}

void GameLayer::userSelectCell(Cell * cell) {
    int res = selectCell(cell, m_currSelectCells);
    if (0 == res) {
        return;
    }
    Cell * endCell = NULL;
    long num = m_currSelectCells.size();
    if (num > 0) {
        endCell = m_currSelectCells.at(num - 1);
    }
    if (res > 0) {
        // 新添连接线
        if (num > 0) {
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
    } else if (res < 0) {
        // 取消之前选择的Cell
        endCell->setStatus(CellStatus::normal);
        m_currSelectCells.erase(m_currSelectCells.begin() + num - 1);
        
        if (num > 1) {
            // 删除连接线
            m_currSelectLines.at(num - 2)->removeFromParent();
            m_currSelectLines.erase(m_currSelectLines.begin() + num - 2);
        }
    }
}

void GameLayer::resetSelect() {
    log("GameLayer::resetSelect");
    for (int i = 0; i < m_currSelectCells.size(); i++) {
        m_currSelectCells.at(i)->setStatus(CellStatus::normal);
    }
    m_currSelectCells.clear();
    
    for (int i = 0; i < m_currSelectLines.size(); i++) {
        m_currSelectLines.at(i)->removeFromParent();
    }
    m_currSelectLines.clear();
}

void GameLayer::autoRecover(Cell * endCell) {
    log("GameLayer::autoRecover");
    vector<Cell *> cells;
    for (int i = 0; i < m_cells.size(); i++) {
        Cell * cell = m_cells.at(i);
        if (-1 == cell->getValue()) {
            // 将上一轮选中的障碍恢复
            cell->setValue(0);
            cells.push_back(cell);
        } else if (CellStatus::selected == cell->getStatus()) {
            if (cell != endCell) {
                // 将一轮选中的修改中间格子变成障碍
                cell->setValue(-1);
            }
        } else if (0 == cell->getValue()) {
            cells.push_back(cell);
        }
    }
    
    // 新生成元素
    random_shuffle(cells.begin(), cells.end());
    cells[0]->setValue(Util::getIndexByWeight(m_scene->getConfig()->initWeights, 1));
}

void GameLayer::recoverByLine(Cell * endCell) {
    log("GameLayer::recoverByLine");
    for (int i = 0; i < m_currSelectCells.size(); i++) {
        Cell * selectCell = m_currSelectCells.at(i);
        for (int row = -1; row < 2; row++) {
            for (int col = -1; col < 2; col++) {
                if (!(0 == row && (col == -1 || col == 1)) && !(0 == col && (row == -1 || row == 1))) {
                    // 不是最后元素的正上下左右方
                    continue;
                }

                int index = getIndexByRowAndCol(selectCell->getRow() + row, selectCell->getCol() + col);
                if (index >= 0) {
                    Cell * checkCell = m_cells.at(index);
                    if (-1 == checkCell->getValue() && checkCell->getStatus() != CellStatus::selected) {
                        // 旁消除
                        checkCell->setValue(0);
                    }
                }
            }
        }
        if (selectCell != endCell) {
            // 将一轮选中的修改中间格子变成障碍
            selectCell->setValue(-1);
        }
    }
    
    vector<Cell *> cells;
    for (int i = 0; i < m_cells.size(); i++) {
        Cell * cell = m_cells.at(i);
        if (0 == cell->getValue()) {
            cells.push_back(cell);
        }
    }
    
    // 新生成元素
    random_shuffle(cells.begin(), cells.end());
    cells[0]->setValue(Util::getIndexByWeight(m_scene->getConfig()->initWeights, 1));
}

void GameLayer::checkFailed() {
    log("GameLayer::checkFailed");
    m_hintCells.clear();
    Vector<Cell *> unCheckCells;
    for (int i = 0; i < m_cells.size(); i++) {
        Cell * cell = m_cells.at(i);
        if (selectCell(cell, m_hintCells)) {
            // 选中开始点
            cell->setStatus(CellStatus::selected);
            m_hintCells.pushBack(cell);
            if (checkFailRecursion(cell, NULL, m_cells, m_hintCells, unCheckCells)) {
                cell->setStatus(CellStatus::normal);
                break;
            }
            // 失败 恢复数据
            cell->setStatus(CellStatus::normal);
            m_hintCells.eraseObject(cell);
            unCheckCells.clear();
        }
    }
    
    if (m_hintCells.size() > 0) {
//        for (int i = 0; i < m_hintCells.size(); i++) {
//            Cell * cell = m_hintCells.at(i);
//            log("GameLayer::checkFailed lineCells   i = %d", i);
//            cell->logInfo();
//        }
    } else {
        m_scene->gameOver();
    }
}

bool GameLayer::checkFailRecursion(Cell * currEndCell, Cell * lastEndCell, Vector<Cell *> &cells, Vector<Cell *> &selectCells, Vector<Cell *> &unCheckCells) {
    bool res = false;
    
    for (int row = -1; row < 2; row++) {
        for (int col = -1; col < 2; col++) {
            if (!(0 == row && (col == -1 || col == 1)) && !(0 == col && (row == -1 || row == 1))) {
                // 不是最后元素的正上下左右方
                continue;
            }
            int index = getIndexByRowAndCol(currEndCell->getRow() + row, currEndCell->getCol() + col);
            if (index >= 0) {
                Cell * checkCell = cells.at(index);
                if (checkCell != lastEndCell) { // 既不能回退
                    if (unCheckCells.getIndex(checkCell) != -1) {
                        continue;
                    }
                    if (selectCell(checkCell, selectCells)) {
                        checkCell->setStatus(CellStatus::selected);
                        selectCells.pushBack(checkCell);
                        if (checkCell->getValue() == 0) {
                            // 中间元素
                            if (checkFailRecursion(checkCell, currEndCell, cells, selectCells, unCheckCells)) {
                                // 成功
                                res = true;
                            }
                        } else {
                            // 成功
                            res = true;
                        }
                        checkCell->setStatus(CellStatus::normal);
                        if (res) {
                            break;
                        } else {
                            // 失败 恢复数据
                            selectCells.eraseObject(checkCell);
                            unCheckCells.pushBack(checkCell);
                        }
                    }
                }
            }
        }
        if (res) {
            break;
        }
    }
    
    return res;
}

string GameLayer::getHintCellsInfo() {
    string res = "";
    for (int i = 0; i < m_hintCells.size(); i++) {
        Cell * cell = m_hintCells.at(i);
        if (0 == i) {
            res = Util::strFormat("(%d,%d)", cell->getRow(), cell->getCol());
        } else {
            if (i % 5 == 0) {
                res = Util::strFormat("%s->(%d,%d)%\n", res.c_str(), cell->getRow(), cell->getCol());
            } else {
                res = Util::strFormat("%s->(%d,%d)", res.c_str(), cell->getRow(), cell->getCol());
            }
        }
    }
    return res;
}