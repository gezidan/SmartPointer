#include <iostream>
#include <memory>
#include "smart_ptr.h"
using namespace smart_ptr;

class CBase
{
public:
    CBase() : data0(0)
    {
        std::cout << "CBase()" << std::endl;
    }

    virtual ~CBase()
    {
        std::cout << "~CBase()" << std::endl;
    }

    int data0;
private:
};

class CDevide : public CBase
{
public:
    CDevide() : data2(3)
    {
        std::cout << "CDevide()" << std::endl;
    }

    ~CDevide()
    {
        std::cout << "~CDevide()" << std::endl;
    }

    int data2;
private:
};


#ifndef CDECL
#if defined(WIN32)
#define CDECL           _cdecl
#else
#define CDECL 
#endif // defined(WIN32)
#endif // !CDECL

int CDECL main(void)
{
    strong_ptr<CBase> spBase(new CDevide());

    {
        strong_ptr<CDevide> spDummy(spBase);

        weak_ptr<CBase> spW1(spBase);
        spW1.lock()->data0++;
        std::cout << spDummy->data0 << std::endl;

        weak_ptr<CDevide> spW2(spBase);
        spW2.lock()->data0++;
        std::cout << spDummy->data0 << std::endl;

        weak_ptr<CBase> spW3(spW1);
        spW3.lock()->data0++;
        std::cout << spDummy->data0 << std::endl;

        weak_ptr<CDevide> spW4(spW1);
        spW4.lock()->data0++;
        std::cout << spDummy->data0 << std::endl;

        weak_ptr<CBase> spW5;
        spW5 = spBase;
        spW5.lock()->data0++;
        std::cout << spDummy->data0 << std::endl;

        weak_ptr<CDevide> spW6;
        spW6 = spBase;
        spW6.lock()->data0++;
        std::cout << spDummy->data0 << std::endl;

        weak_ptr<CBase> spW7;
        spW7 = spW1;
        spW7.lock()->data0++;
        std::cout << spDummy->data0 << std::endl;

        weak_ptr<CDevide> spW8;
        spW8 = spW1;
        spW8.lock()->data0++;
        std::cout << spDummy->data0 << std::endl;
    }

    {
        weak_ptr<CBase> wpDummy(spBase);

        strong_ptr<CBase> sp1(spBase);
        sp1->data0++;
        std::cout << sp1->data0 << std::endl;

        strong_ptr<CDevide> sp2(spBase);
        sp2->data0++;
        std::cout << sp1->data0 << std::endl;

        strong_ptr<CBase> sp3(wpDummy);
        sp3->data0++;
        std::cout << sp1->data0 << std::endl;

        strong_ptr<CDevide> sp4(wpDummy);
        sp4->data0++;
        std::cout << sp1->data0 << std::endl;

        strong_ptr<CBase> sp5;
        sp5 = spBase;
        sp5->data0++;
        std::cout << sp1->data0 << std::endl;

        strong_ptr<CDevide> sp6;
        sp6 = spBase;
        sp6->data0++;
        std::cout << sp1->data0 << std::endl;

        strong_ptr<CBase> sp7;
        sp7 = wpDummy;
        sp7->data0++;
        std::cout << sp1->data0 << std::endl;

        strong_ptr<CDevide> sp8;
        sp8 = wpDummy;
        sp8->data0++;
        std::cout << sp1->data0 << std::endl;
    }

    return 0;
}
