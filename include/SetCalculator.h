#pragma once

#include <vector>
#include <memory>
#include <string>
#include <iosfwd>
#include <optional>
#include "MyExcep.h"

const int max = 100;
const int min = 3;

class Operation;

class SetCalculator
{
public:
    SetCalculator(std::istream&, std::ostream&, const bool);
    void run();

private:
    void setMaxNumOfOperations();
    void deleteRestOperations();
    void setOldSize(const int);
    void runAction();
    void read();
    void resize();
    void eval();
    void del();
    void help();
    void exit();

    template <typename FuncType>
    void binaryFunc()
    {
        if (auto f0 = readOperationIndex(), f1 = readOperationIndex(); f0 && f1)
        {
            m_operations.push_back(std::make_shared<FuncType>(m_operations[*f0], m_operations[*f1]));
        }
    }

    void printOperations() const;

    enum class Action
    {
        Invalid,
        Read,       
        Resize,     
        Eval,
        Union,
        Intersection,
        Difference,
        Product,
        Comp,
        Del,
        Help,
        Exit,
    };

    struct ActionDetails
    {
        std::string command;
        std::string description;
        Action action;
    };

    using ActionMap = std::vector<ActionDetails>;
    using OperationList = std::vector<std::shared_ptr<Operation>>;

    const ActionMap m_actions;
    OperationList m_operations;
    
    bool m_running = true;
    bool m_user = true;
    bool m_fileNotOpen = false;
    int m_maxSizeOfOperations = 0;
    
    std::istream& m_istr;
    std::ostream& m_ostr;

    std::optional<int> readOperationIndex() const;
    Action readAction() const;
    void runAction(Action action);
    void checkIfAddedOperation();
    bool checkNumOfOperation()const;

    static ActionMap createActions();
    static OperationList createOperations();
};
