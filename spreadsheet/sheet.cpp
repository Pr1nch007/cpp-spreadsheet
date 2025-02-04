#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::CheckPosIsValid (Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("InvalidPosition");
    }
}

void Sheet::SetCell(Position pos, std::string text) {
    CheckPosIsValid(pos);

    while (table_.size() <= static_cast<size_t>(pos.row)) {
        if (table_.size() == 0) {
            table_.emplace_back();
        } else {
            std::vector<std::unique_ptr<Cell>> new_row(table_[0].size());
            table_.push_back(std::move(new_row));
        }
    }
    if (table_[0].size() <= static_cast<size_t>(pos.col)) {
        for (auto& row : table_) {
            row.resize(pos.col + 1);
        }
    }

    if (min_area_.rows <= pos.row) {
        min_area_.rows = pos.row + 1;
    }
    if (min_area_.cols <= pos.col) {
        min_area_.cols = pos.col + 1;
    }
    
    auto* cell_ptr = &table_[pos.row][pos.col];

    if (*cell_ptr){
        (*cell_ptr)->Set(text);
    } else {
        *cell_ptr = std::make_unique<Cell>(*this);
        (*cell_ptr)->Set(text);
    }

}

const CellInterface* Sheet::GetCell(Position pos) const {
    CheckPosIsValid(pos);

    if (min_area_.rows <= pos.row || min_area_.cols <= pos.col) {
        return nullptr;
    }

    auto* cell_ptr = &table_[pos.row][pos.col];
    if (*cell_ptr){
        return (*cell_ptr).get();
    } else {
        return nullptr;
    }
}
CellInterface* Sheet::GetCell(Position pos) {
    CheckPosIsValid(pos);

    if (min_area_.rows <= pos.row || min_area_.cols <= pos.col) {
        return nullptr;
    }

    auto* cell_ptr = &table_[pos.row][pos.col];
    if (*cell_ptr){
        return (*cell_ptr).get();
    } else {
        return nullptr;
    }
}

void Sheet::CollapseRows () {
    int min_rows = min_area_.rows;
    for (int row = min_rows - 1; row >= 0; --row) {
        for (int col = 0; col < min_area_.cols; ++col) {
            if(table_[row][col]) {
                return;
            }
        }
        --min_area_.rows;
    }
}

void Sheet::CollapseCols () {
    while(min_area_.cols != 0) {
        for (int row = 0; row < min_area_.rows; ++row) {
            if(table_[row][min_area_.cols - 1]) {
                return;
            }
        }
        --min_area_.cols;
    }
}

void Sheet::ClearCell(Position pos) {
    CheckPosIsValid(pos);

    if (min_area_.rows <= pos.row || min_area_.cols <= pos.col) {
        return;
    }

    table_[pos.row][pos.col] = nullptr;

    if (pos.row + 1 == min_area_.rows) {
        CollapseRows();
    }

    if (pos.col + 1 == min_area_.cols) {
        CollapseCols();
    }
}

Size Sheet::GetPrintableSize() const {
    return min_area_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int row = 0; row < min_area_.rows; ++row) {
        for (int col = 0; col < min_area_.cols; ++col) {
            if (table_[row][col]) {
                std::visit([&output](const auto& arg) {
                                            output << arg;
            }, table_[row][col]->GetValue());
            }

            if (col == min_area_.cols - 1) {
                output << "\n";
            } else {
                output << "\t";
            }
        }
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for (int row = 0; row < min_area_.rows; ++row) {
        for (int col = 0; col < min_area_.cols; ++col) {
            if (table_[row][col]) {
                output << table_[row][col]->GetText();
            }

            if (col == min_area_.cols - 1) {
                output << "\n";
            } else {
                output << "\t";
            }
        }
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}