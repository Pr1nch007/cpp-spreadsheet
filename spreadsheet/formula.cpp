#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

FormulaError::FormulaError(Category category) : category_(category) {}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
        case Category::Ref :
            return "#REF!"sv;
        case Category::Arithmetic :
            return "#ARITHM!"sv;
        case Category::Value :
            return "#VALUE!"sv;
    }
    return ""sv;
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression) try :  ast_(ParseFormulaAST (expression)) {
    } catch (const std::exception& exc) {
        throw FormulaException ("incorrect formula");
    }

    Value Evaluate(SheetInterface& sheet) const override {
        try {
            return ast_.Execute(sheet);
        } catch (const FormulaError& error) {
            return error;
        }
    }

    std::string GetExpression() const override {
        std::ostringstream stream;
        ast_.PrintFormula(stream);
        return stream.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        std::vector<Position> cells;
        Position visit;
        for(auto& cell : ast_.GetCells()) {
            if (visit == cell && !cells.empty()) {
                continue;
            }
            cells.push_back(cell);
            visit = cell;
        }
        return cells;
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}