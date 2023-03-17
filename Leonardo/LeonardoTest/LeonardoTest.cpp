#include "CppUnitTest.h"
#include "../LeonardoEngine/engine.hpp"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace LeonardoTest
{
	TEST_CLASS(LeonardoTest)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			Assert::AreEqual(15, test());
		}
	};
}
