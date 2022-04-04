#include <exception>
#include <iostream>
#include <string>

class MyExcep : public std::exception
{
public:
	const char* what() { return m_what; }
	void setWhat(const char* what) { std::strcpy(m_what, what); }

private:
	char* m_what;
};