#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <optional>

class Sheet;

enum TypeImpl {
    EMPTY,
    TEXT,
    FORMULA
    };

TypeImpl CheckType (const std::string& text);

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    
    void ClearCache(std::unordered_set<Cell*>& visited);
    bool CheckCyclicDependency(std::unordered_set<Cell*>& visited, std::unordered_set<Cell*>& in_stack);
    
    void RefreshParents();
    void DeletChild (Cell* child);
    void DeletParent (Cell* parent);
    void AddChild (Cell* child) const;
    void AddParent (Cell* parent) const;

private:
//можете воспользоваться нашей подсказкой, но это необязательно.
class Impl {
    public:
    virtual ~Impl() {}
    virtual Cell::Value GetValue(const SheetInterface& sheet) const = 0;
    virtual std::string GetText() const = 0;
    virtual TypeImpl GetType() const = 0;
    virtual std::unordered_set<Cell*> GetSetCells(SheetInterface& sheet) = 0;
    virtual std::vector<Position> GetVectorCells() = 0;
};

class EmptyImpl : public Impl {
    public:
    Cell::Value GetValue(const SheetInterface& sheet) const override;
    std::string GetText() const override;
    TypeImpl GetType() const override;
    std::unordered_set<Cell*> GetSetCells(SheetInterface& sheet) override;
    std::vector<Position> GetVectorCells() override;
};

class TextImpl : public Impl {
    public:
    TextImpl (const std::string& text);
    Cell::Value GetValue(const SheetInterface& sheet) const override;
    std::string GetText() const override;
    TypeImpl GetType() const override;
    std::unordered_set<Cell*> GetSetCells(SheetInterface& sheet) override;
    std::vector<Position> GetVectorCells() override;

    private:
    std::string text_;
};

class FormulaImpl : public Impl {
    public:
    FormulaImpl (const std::string& text);
    Cell::Value GetValue(const SheetInterface& sheet) const override;
    std::string GetText() const override;
    TypeImpl GetType() const override;
    std::unordered_set<Cell*> GetSetCells(SheetInterface& sheet) override;
    std::vector<Position> GetVectorCells() override;

    private:
    std::unique_ptr<FormulaInterface> formula_;
};

    std::unique_ptr<Impl> impl_;

    mutable std::unordered_set<Cell*> children_;
    mutable std::unordered_set<Cell*> parents_;
    
    mutable std::optional<Cell::Value> caсhe_;
    SheetInterface& sheet_;
};