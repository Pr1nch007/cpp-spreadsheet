#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

TypeImpl CheckType (const std::string& text) {
    if (text.empty()) {
        return EMPTY;
    } else if (text[0] == '=' && text[1]) {
        return FORMULA;
    } else {
        return TEXT;
    }
}

Cell::Value Cell::EmptyImpl::GetValue(SheetInterface& sheet) const {
    return "";
}

std::string Cell::EmptyImpl::GetText() const {
    return "";
}

TypeImpl Cell::EmptyImpl::GetType() const {
    return EMPTY;
}

std::unordered_set<Cell*> Cell::EmptyImpl::GetSetCells(SheetInterface& sheet) {
    std::unordered_set<Cell*> empty;
    return empty;
}

std::vector<Position> Cell::EmptyImpl::GetVectorCells() {
    std::vector<Position> empty;
    return empty;
}


Cell::TextImpl::TextImpl (const std::string& text) : text_(text) {}

Cell::Value Cell::TextImpl::GetValue(SheetInterface& sheet) const {
    if (text_[0] == '\'') {
        if (text_.size() > 1){
            return text_.substr(1);
        } else {
            return "";
        }
    } else {
        return text_;
    }
}

std::string Cell::TextImpl::GetText() const {
    return text_;
}

TypeImpl Cell::TextImpl::GetType() const {
    return TEXT;
}

std::unordered_set<Cell*> Cell::TextImpl::GetSetCells(SheetInterface& sheet) {
    std::unordered_set<Cell*> empty;
    return empty;
}

std::vector<Position> Cell::TextImpl::GetVectorCells() {
    std::vector<Position> empty;
    return empty;
}

Cell::FormulaImpl::FormulaImpl (const std::string& text) : formula_(std::move(ParseFormula(text))) {}

Cell::Value Cell::FormulaImpl::GetValue(SheetInterface& sheet) const {
    auto result = formula_->Evaluate(sheet);
    if (std::holds_alternative<double>(result)) {
        return std::get<double>(result);
    } else {
        return std::get<FormulaError>(result);
    }
}

std::string Cell::FormulaImpl::GetText() const {
    return '=' + formula_->GetExpression();
}

TypeImpl Cell::FormulaImpl::GetType() const {
    return FORMULA;
}

std::unordered_set<Cell*> Cell::FormulaImpl::GetSetCells(SheetInterface& sheet_) {
    std::unordered_set<Cell*> cells;
    for(auto& cell : formula_->GetReferencedCells()) {
        if (!sheet_.GetCell(cell)) {
            sheet_.SetCell(cell, "");
        }
        cells.insert(static_cast<Cell*>(sheet_.GetCell(cell)));
    }
    return cells;
}

std::vector<Position> Cell::FormulaImpl::GetVectorCells() {
    return formula_->GetReferencedCells();
}

Cell::Cell(SheetInterface& sheet) : impl_ (std::move(std::make_unique<EmptyImpl>())), sheet_(sheet){
//    sheet_ = std::move(sheet);
}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    switch (CheckType(text)) {
        case EMPTY:
            impl_ = std::make_unique<EmptyImpl>();
            break;
        case TEXT:
            impl_ = std::make_unique<TextImpl>(text);
            break;
        case FORMULA: {
            std::unique_ptr<FormulaImpl> temp_formula;
            try{
                temp_formula = std::make_unique<FormulaImpl>(text.substr(1));
            } catch (const std::exception& exc) {
                throw FormulaException ("incorrect formula");
            }

            std::unordered_set<Cell*> insert;
            std::unordered_set<Cell*> in_stack = temp_formula->GetSetCells(sheet_);
            
            if (CheckCyclicDependency(insert, in_stack)) {
                throw CircularDependencyException("Cyclic dependency detected");
            }

            impl_ = std::move(temp_formula);
            break;
        }  
    }

    std::unordered_set<Cell*> visited;
    ClearCache(visited);

    RefreshParents();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    if (!caсhe_.has_value()) {
        caсhe_ = impl_->GetValue(sheet_);
    }
    return caсhe_.value();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetVectorCells();
}

bool Cell::CheckCyclicDependency(std::unordered_set<Cell*>& visited, std::unordered_set<Cell*>& in_stack) {
    if (in_stack.count(this)) {
        return true;
    }
    if (visited.count(this)) {
        return false;
    }

    visited.insert(this);

    for (auto child : children_) {
        if (child->CheckCyclicDependency(visited, in_stack)) {
            return true;
        }
    }

    return false;
}

void Cell::ClearCache(std::unordered_set<Cell*>& visited) {
    if (visited.count(this)) {
        return ;
    }

    visited.insert(this);
    this->caсhe_ = std::nullopt;

    for (auto child : children_) {
        child->ClearCache(visited);
    }
}

void Cell::RefreshParents() {
    auto parents_new = impl_->GetSetCells(sheet_);
    auto copy_parents = parents_;

    for (auto& old_parent : copy_parents) {
        if (!parents_new.count(old_parent)) {
            old_parent->DeletChild(this);
            DeletParent(old_parent);
        }
    }

    for (auto new_parent : parents_new) {
        if (!parents_.count(new_parent)) {
            new_parent->AddChild(this);
            AddParent (new_parent);
        }
    }
}

void Cell::DeletChild (Cell* child) {
    children_.erase(child);
}


void Cell::DeletParent (Cell* parent) {
    parents_.erase(parent);
}

void Cell::AddChild (Cell* child) const {
    children_.insert(child);
}

void Cell::AddParent (Cell* parent) const {
    parents_.insert(parent);
}