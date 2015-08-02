//
//  Cell.cpp
//  IGGameC
//
//  Created by IllusionCui on 15/7/30.
//
//

#include "Cell.h"

Cell * Cell::create(int row, int col) {
    Cell * res = new Cell();
    if (res && res->init(row, col)) {
        res->autorelease();
        return res;
    } else {
        CC_SAFE_DELETE(res);
        return NULL;
    }
}

Cell::Cell() {
    m_row = -1;
    m_col = -1;
    m_value = 0;
    m_label = NULL;
    m_status = CellStatus::normal;
}

Cell::~Cell() {
    CC_SAFE_RELEASE_NULL(m_label);
}

bool Cell::init(int row, int col) {
    if (!Node::init()) {
        return false;
    }
    
    m_row = row;
    m_col = col;
    
    m_label = Label::createWithTTF("Cell", defaultTTF, 70);
    CC_SAFE_RETAIN(m_label);
    m_label->setHorizontalAlignment(TextHAlignment::CENTER);
    m_label->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    addChild(m_label);
    
    setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    
    return true;
}

void Cell::logInfo() {
    log("Cell::logInfo  this = %p, row = %d, col = %d, value = %d, status = %d", this, m_row, m_col, m_value, m_status);
}

void Cell::setContentSize(const Size& contentSize) {
    Node::setContentSize(contentSize);
    updateShow();
}

bool Cell::isSelectable() {
    return m_value > 0 && CellStatus::normal == m_status;
}

bool Cell::canBeMoveTo() {
    return m_value >= 0 && CellStatus::normal == m_status;
}

void Cell::setValue(int value) {
    if (value == m_value) {
        return;
    }
    
    m_value = value;
    updateShow();
}

void Cell::updateShow() {
    m_label->setString(Util::strFormat("%d", m_value));
    if (m_value < 0) {
        m_label->setColor(Color3B::GREEN);
    } else if (m_value > 0) {
        m_label->setColor(Color3B::RED);
    } else {
        m_label->setColor(Color3B::YELLOW);
    }
    Size size = getContentSize();
    Util::fitToSize(m_label, size, true);
    m_label->setPosition(Vec2(size.width/2, size.height/2));
}