#include <boost/test/unit_test.hpp>
#include <uquad/base/Delegate.h>
#include <boost/chrono.hpp>

using namespace uquad;


class CallbackClass
{
public:
	virtual int event(int b) = 0;
	virtual ~CallbackClass() {}
};

class CallbackClassImpl
	: public CallbackClass
{
public:
	int event(int b)
	{
		return b * b;
	}
};

#define TEST_PERFORMANCE_CYCLES 100000

BOOST_AUTO_TEST_CASE(Delegate)
{
	int c;
	//test delegate performance
	typedef base::Delegate<CallbackClass> Delegate;
	Delegate delegate;

	CallbackClassImpl callback_impl;
	CallbackClass *callback = &callback_impl;
	delegate+=callback;

	auto delegate_start = chrono::high_resolution_clock::now();
	for(uint32_t i=0;i<TEST_PERFORMANCE_CYCLES;++i)
	{
		delegate.call(&CallbackClass::event, i);
	}
	auto delegate_end = chrono::high_resolution_clock::now();

	//test boost::function performance
	boost::function<int (int)> boost_function = boost::bind(&CallbackClass::event, callback, _1);
	auto boost_function_start = chrono::high_resolution_clock::now();
	for(uint32_t i=0;i<TEST_PERFORMANCE_CYCLES;++i)
	{
		c=boost_function(i);
	}
	auto boost_function_end = chrono::high_resolution_clock::now();

	//test native function performance
	auto native_function_start = chrono::high_resolution_clock::now();
	for(uint32_t i=0;i<TEST_PERFORMANCE_CYCLES;++i)
	{
		c=callback->event(i);
	}
	auto native_function_end = chrono::high_resolution_clock::now();

	//test boost::signals performance
	uquad::signal<int (int)> signal_function;
	signal_function.connect(boost::bind(&CallbackClass::event, callback, _1));
	auto signal_function_start = chrono::high_resolution_clock::now();
	for(uint32_t i=0;i<TEST_PERFORMANCE_CYCLES;++i)
	{
		c=signal_function(i);
	}
	auto signal_function_end = chrono::high_resolution_clock::now();

	std::cout << "delegate performance=" << (delegate_end - delegate_start) << std::endl;
	std::cout << "boost::function performance=" << (boost_function_end - boost_function_start) << std::endl;
	std::cout << "native function performance=" << (native_function_end - native_function_start) << std::endl;
	std::cout << "signal function performance=" << (signal_function_end - signal_function_start) << std::endl;
	std::cout << c << std::endl;
	//BOOST_TEST((delegate_end - delegate_start) < (boost_function_end - boost_function_start));
}

