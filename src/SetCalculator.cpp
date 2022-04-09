#include "SetCalculator.h"

#include "Union.h"
#include "Intersection.h"
#include "Difference.h"
#include "Product.h"
#include "Comp.h"
#include "Identity.h"

#include <iostream>
#include <fstream>
#include <istream>
#include <ostream>
#include <sstream>
#include <algorithm>
#include <exception>

namespace rng = std::ranges;

SetCalculator::SetCalculator(std::istream& istr, std::ostream& ostr, const bool user)
	: m_actions(createActions()), m_operations(createOperations()),
	m_istr(istr), m_ostr(ostr), m_user(user), m_RunningRefForFile(&m_running)
{}

void SetCalculator::run()
{
	do {
		m_istr.exceptions(std::ios::failbit | std::ios::badbit);

		try {
			if (m_user)
				setMaxNumOfOperations();
			m_ostr << '\n';
			printOperations();
			m_ostr << "Enter command ('help' for the list of available commands): ";
			if (m_readingInFile) m_lineInFile++;
			runAction();
			checkIfAddedOperation();
		}
		catch (std::ios_base::failure& e) { catchIosBase(e.what()); }
		catch (std::out_of_range& s) { m_ostr << s.what(); if (m_readingInFile) handleErrorInFile(); }
		catch (std::invalid_argument& s) {
			m_ostr << s.what(); m_istr.clear(); m_istr.ignore();
			if (m_readingInFile) handleErrorInFile();
		}

	} while (m_running);
}

void SetCalculator::catchIosBase(const char* message)
{
	m_istr.clear();
	m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	if (m_istr.eof()) {
		m_running = false;
		return;
	}
	m_fileNotOpen ? m_ostr << message : m_ostr << "#error you need to input a number\n";
	m_fileNotOpen = false;
	if (m_readingInFile)
		handleErrorInFile();
}

void SetCalculator::handleErrorInFile()
{
	m_ostr << "\nerror in line " << m_lineInFile << "\n";
	char proceed;
	m_ostr << "Do you want to proceed reading the file? (y/n): ";
	std::cin >> proceed;
	if (proceed == 'n')
		m_running = false;
}

void SetCalculator::checkIfAddedOperation()
{
	if (m_maxSizeOfOperations < m_operations->size())
	{
		m_operations->resize(m_operations->size() - 1);
		m_ostr << "Number of Maximum operations: " << m_maxSizeOfOperations << "\n";
		throw std::out_of_range("you have reached the maximum operations.\ndelete operations in order to add more");
	}
}

void SetCalculator::setMaxNumOfOperations()
{
	while (m_user)
	{
		m_ostr << "Enter size of opertaions:\n";
		m_istr >> m_maxSizeOfOperations;
		if (!checkNumOfOperation())
			throw std::out_of_range("#error - number of operations between 3-100\n");
		m_user = false;
		break;
	}
}

void SetCalculator::runAction()
{
	const auto action = readAction();
	runAction(action);
}

bool SetCalculator::checkNumOfOperation()const
{
	return m_maxSizeOfOperations < max && m_maxSizeOfOperations > min;
}

void SetCalculator::read()
{
	std::ifstream file = openFile();
	checkIfFileOpened(file);
	auto readCalc = SetCalculator(file, std::cout, false);
	runFile(readCalc);
	setCalcBackToUser(readCalc);
}

SetCalculator::OpenFile SetCalculator::openFile()
{
	auto filePath = std::string();
	m_istr >> filePath;
	std::ifstream file;
	file.open(filePath);
	return file;
}

void SetCalculator::checkIfFileOpened(std::ifstream& file)
{
	if (!file)
	{
		m_fileNotOpen = true;
		throw std::ios::failure("file didn't open\n");
	}
}

void SetCalculator::runFile(SetCalculator& fileCalc)
{
	fileCalc.m_RunningRefForFile = m_RunningRefForFile;
	fileCalc.m_operations = m_operations;
	fileCalc.m_readingInFile = true;
	fileCalc.m_maxSizeOfOperations = m_maxSizeOfOperations;
	fileCalc.run();
}

void SetCalculator::setCalcBackToUser(const SetCalculator& readCalc)
{
	m_operations = readCalc.m_operations;
	m_maxSizeOfOperations = readCalc.m_maxSizeOfOperations;
}

void SetCalculator::resize()
{
	m_user = true;
	int maxNumOperations = m_maxSizeOfOperations;
	char proceed;
	setMaxNumOfOperations();
	if (m_maxSizeOfOperations < m_operations->size()) {
		m_ostr << "New size is smaller then old size, do you want to proceed with this action? (y/n): ";
		m_istr >> proceed;
		proceed == 'y' ? deleteRestOperations() : setOldSize(maxNumOperations);
	}
}

void SetCalculator::deleteRestOperations()
{
	m_operations->resize(m_maxSizeOfOperations);
}

void SetCalculator::setOldSize(const int maxNumOperations)
{
	m_maxSizeOfOperations = maxNumOperations - 1;
	if (m_readingInFile)
		m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void SetCalculator::eval()
{
	if (auto index = readOperationIndex(); index)
	{
		const auto operation = m_operations->at(*index);
		auto inputs = std::vector<Set>();
		for (auto i = 0; i < operation->inputCount(); ++i)
		{
			inputs.push_back(Set(m_istr));
		}

		operation->print(m_ostr, inputs);
		m_ostr << " = " << operation->compute(inputs) << '\n';
	}
}

void SetCalculator::del()
{
	if (auto i = readOperationIndex(); i)
	{
		m_operations->erase(m_operations->begin() + *i);
	}
}

void SetCalculator::help()
{
	m_ostr << "The available commands are:\n";
	for (const auto& action : m_actions)
	{
		m_ostr << "* " << action.command << action.description << '\n';
	}
	m_ostr << '\n';
}

void SetCalculator::exit()
{
	if (m_readingInFile)
		(*m_RunningRefForFile) = false;
	m_ostr << "Goodbye!\n";
	m_running = false;
}

void SetCalculator::printOperations() const
{
	m_ostr << "List of available set operations:\n";
	for (decltype(m_operations->size()) i = 0; i < m_operations->size(); ++i)
	{
		m_ostr << i << ".\t";
		auto gen = NameGenerator();
		m_operations->at(i)->print(m_ostr, gen);
		m_ostr << '\n';
	}
	m_ostr << '\n';
}

std::optional<int> SetCalculator::readOperationIndex() const
{
	auto i = 0;
	m_istr >> i;
	if (i >= m_operations->size())
		throw std::invalid_argument("Operation " + std::to_string(i) + " doesn't exist\n");

	return i;
}

SetCalculator::Action SetCalculator::readAction() const
{
	auto action = std::string();
	m_istr >> action;

	const auto i = std::ranges::find(m_actions, action, &ActionDetails::command);
	if (i != m_actions.end())
	{
		return i->action;
	}
	return Action::Invalid;
}

void SetCalculator::runAction(Action action)
{
	switch (action)
	{
	default:
		throw std::invalid_argument("Unknown enum entry used!\n");
		break;

	case Action::Invalid:
		throw std::invalid_argument("Command not found\n");

	case Action::Read:         read();                     break;
	case Action::Resize:       resize();                   break;
	case Action::Eval:         eval();                     break;
	case Action::Union:        binaryFunc<Union>();        break;
	case Action::Intersection: binaryFunc<Intersection>(); break;
	case Action::Difference:   binaryFunc<Difference>();   break;
	case Action::Product:      binaryFunc<Product>();      break;
	case Action::Comp:         binaryFunc<Comp>();         break;
	case Action::Del:          del();                      break;
	case Action::Help:         help();                     break;
	case Action::Exit:         exit();                     break;
	}
}

SetCalculator::ActionMap SetCalculator::createActions()
{
	return ActionMap
	{
		{
			"read",
			" file path ... - read file of commands ",
			Action::Read
		},
		{
			"resize",
			" num ... - resize the number of commends in calculator",
			Action::Resize
		},
		{
			"eval",
			"(uate) num ... - compute the result of function #num on the "
			"following set(s); each set is prefixed with the count of numbers to"
			" read",
			Action::Eval
		},
		{
			"uni",
			"(on) num1 num2 - Creates an operation that is the union of "
			"operation #num1 and operation #num2",
			Action::Union
		},
		{
			"inter",
			"(section) num1 num2 - Creates an operation that is the "
			"intersection of operation #num1 and operation #num2",
			Action::Intersection
		},
		{
			"diff",
			"(erence) num1 num2 - Creates an operation that is the "
			"difference of operation #num1 and operation #num2",
			Action::Difference
		},
		{
			"prod",
			"(uct) num1 num2 - Creates an operation that returns the product of"
			" the items from the results of operation #num1 and operation #num2",
			Action::Product
		},
		{
			"comp",
			"(osite) num1 num2 - creates an operation that is the composition "
			"of operation #num1 and operation #num2",
			Action::Comp
		},
		{
			"del",
			"(ete) num - delete operation #num from the operation list",
			Action::Del
		},
		{
			"help",
			" - print this command list",
			Action::Help
		},
		{
			"exit",
			" - exit the program",
			Action::Exit
		}
	};
}

SetCalculator::OperationListPtr SetCalculator::createOperations()
{
	OperationList vec
	{
		std::make_shared<Union>(std::make_shared<Identity>(), std::make_shared<Identity>()),
		std::make_shared<Intersection>(std::make_shared<Identity>(), std::make_shared<Identity>()),
		std::make_shared<Difference>(std::make_shared<Identity>(), std::make_shared<Identity>())
	};
	return std::make_shared<OperationList>(vec);
}
